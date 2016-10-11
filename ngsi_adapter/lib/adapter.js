/*
 * Copyright 2013-2016 Telefónica I+D
 * All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License. You may obtain
 * a copy of the License at
 *
 *         http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */


/**
 * Module that implements a HTTP asynchronous server processing requests for adaptation from raw monitoring data into
 * NGSI format, then using the results to invoke ContextBroker.
 *
 * @module adapter
 */


'use strict';
/* jshint expr: true, sub: true, maxparams: 4 */


var http = require('http'),
    dgram = require('dgram'),
    url = require('url'),
    retry = require('retry'),
    domain = require('domain'),
    uuid = require('node-uuid').v1,
    txid = require('cuid'),
    logger = require('./logger'),
    common = require('./common'),
    config = require('./config'),
    parser = require('./parsers/common/factory');


/**
 * Restrict the number of simultaneous outgoing requests.
 */
http.globalAgent.maxSockets = config.maxRequests;


/**
 * Asynchronously process incoming requests and then invoke update context attributes operation on ContextBroker.
 *
 * @param {Domain}          reqdomain  Domain handling request (includes context, timestamp, id, type, body & parser).
 * @param {RequestCallback} callback   The callback for responses from ContextBroker.
 */
function updateContext(reqdomain, callback) {
    try {
        reqdomain.context.op = 'Parse';
        logger.debug('Probe data "%s"', reqdomain.body);

        var parser = reqdomain.parser,
            updateReqBody = parser.getUpdateRequest(reqdomain),
            updateReqOpts = reqdomain.options,
            responseType = updateReqOpts.headers['Accept'];

        /* jshint unused: false */
        var operation = retry.operation({ retries: config.retries });
        operation.attempt(function (currentAttempt) {
            reqdomain.context.op = 'UpdateContext';
            logger.info('Request to ContextBroker at %s...', config.brokerUrl);
            logger.debug('%s %s %s', updateReqOpts.method, updateReqOpts.path, { toString: function () {
                return updateReqBody.split('\n').map(function (line) {return line.trim();}).join('');
            }});
            var updateReq = http.request(updateReqOpts, function (response) {
                var responseBody = '';
                response.setEncoding('utf8');
                response.on('data', function (chunk) {
                    responseBody += chunk;
                });
                response.on('end', function () {
                    var context = logger.getContext();
                    context.corr = response.headers[common.correlatorHttpHeader.toLowerCase()] || context.corr;
                    callback(null, response.statusCode, responseBody, responseType);
                });
            });
            updateReq.on('error', function (err) {
                if (operation.retry(err)) {
                    logger.info('Temporary error "%s". Retrying...', err.message);
                    return;
                }
                callback(err);
            });
            updateReq.end(updateReqBody, 'utf8');
        });
    } catch (err) {
        callback(err);
    }
}


/**
 * Callback for requests to updateContext().
 *
 * @callback RequestCallback
 * @param {Error}   err                     The error occurred in request, or null.
 * @param {Number} [responseStatus]         The response status code.
 * @param {String} [responseBody]           The response body contents.
 * @param {String} [responseContentType]    The response content type.
 */
function updateContextCallback(err, responseStatus, responseBody, responseContentType) {
    if (err) {
        logger.error(err.message);
    } else {
        var response = (responseBody && responseContentType === 'application/json') ? JSON.parse(responseBody) : {},
            status = (response.orionError) ? response.orionError.code : responseStatus,
            success = (config.brokerApi === common.BROKER_API_V2) ? 204 : 200,
            log = (status === success) ? logger.debug : logger.error;
        logger.info('Response status %d %s', status, http.STATUS_CODES[status]);
        log('%s', {
            toString: function () {
                return responseBody.split('\n').map(function (line) {
                    return line.trim();
                }).join('');
            }
        });
    }
}


/**
 * HTTP requests listener.
 *
 * Request URL looks like `http://host:port/path?query`:
 *
 * - Request query string MUST include arguments `id` and `type`
 * - Request path will denote the name of the originating probe
 * - Request headers may include a correlation identifier ({@link common#correlatorHttpHeader})
 *
 * @param {http.IncomingMessage} request    The HTTP request to this server.
 * @param {http.ServerResponse}  response   The HTTP response from this server.
 */
function asyncRequestListener(request, response) {
    var reqdomain = domain.create();
    reqdomain.add(request);
    reqdomain.add(response);
    reqdomain.context = {
        trans: txid(),
        corr: request.headers[common.correlatorHttpHeader.toLowerCase()] || uuid(),
        op: request.method
    };
    var responseHeaders = {};
    responseHeaders[common.correlatorHttpHeader] = reqdomain.context.corr;
    reqdomain.on('error', function (err) {
        logger.error(err.message);
        response.writeHead(500, responseHeaders);  // server error
        response.end();
    });
    reqdomain.run(function () {
        logger.info('Request on resource %s', request.url.split('?').join(' with params '));
        var status = 405;  // not allowed
        if (request.method === 'POST') {
            var query = url.parse(request.url, true).query;
            reqdomain.entityId = query.id;
            reqdomain.entityType = query.type;
            try {
                status = 400;  // bad request
                if (!reqdomain.entityId || !reqdomain.entityType) {
                    throw new Error('Missing entityId and/or entityType');
                }
                status = 404;  // not found
                reqdomain.parser = parser.getParser(request);
                status = 200;  // ok
                reqdomain.timestamp = Date.now();
                reqdomain.body = '';
                request.on('data', function (chunk) {
                    reqdomain.body += chunk;
                });
                request.on('end', function () {
                    process.nextTick(function () {
                        updateContext(reqdomain, exports.updateContextCallback);
                    });
                });
            } catch (err) {
                logger.error(err.message);
            }
        }
        logger.info('Response status %d %s', status, http.STATUS_CODES[status]);
        response.writeHead(status, responseHeaders);
        response.end();
    });
}


/**
 * UDP requests listener.
 *
 * @param {dgram.Socket} socket             The UDP socket listening to requests.
 * @param {String}       message            The incoming message of the request.
 * @param {String}       parserName         The name of the parser that will process the request.
 */
function udpRequestListener(socket, message, parserName) {
    var reqdomain = domain.create();
    reqdomain.add(socket);
    reqdomain.context = {
        trans: txid(),
        op: 'UDP'
    };
    reqdomain.on('error', function (err) {
        logger.error(err.message);
    });
    reqdomain.run(function () {
        logger.info('UDP request to adapt data using parser %s', parserName);
        try {
            reqdomain.body = message;
            reqdomain.parser = parser.getParserByName(parserName);
            reqdomain.timestamp = Date.now();
            process.nextTick(function () {
                updateContext(reqdomain, exports.updateContextCallback);
            });

        } catch (err) {
            logger.error(err.message);
        }
    });
}


/**
 * Server main.
 */
function main() {
    process.once('uncaughtException', function (err) {
        logger.error({op: 'Exit'}, err.message);
        process.exit(1);
    });
    process.once('exit', function () {
        logger.info({op: 'Exit'}, 'Server stopped');
    });
    process.once('SIGINT', function () {
        process.exit();
    });
    process.once('SIGTERM', function () {
        process.exit();
    });

    config.check(function (err) {
        logger.error({op: 'Init'}, err);
        process.exit(1);
    });

    http.createServer(asyncRequestListener).listen(config.listenPort, config.listenHost, function () {
        logger.info({op: 'Init'}, 'Server listening at http://%s:%d/', this.address().address, this.address().port);
    });

    /* Optionally bind this Adapter to a list of UDP endpoints, forwarding requests to the corresponding parser */
    config.udpEndpoints && config.udpEndpoints.split(',').map(function (item) {
        var itemElements = item.split(':'),
            udpListenHost = itemElements[0] || config.listenHost,
            udpListenPort = parseInt(itemElements[1] || config.listenPort, 10),
            udpParserName = itemElements[2];

        if (udpParserName) {
            var udpServer = dgram.createSocket('udp4');
            udpServer.on('error', function (err) {
                logger.error({op: 'UDP'}, 'Server error: %s', err.stack);
                udpServer.close();
            });
            udpServer.on('message', function (msg) {
                udpRequestListener(udpServer, msg, udpParserName);
            });
            udpServer.bind(udpListenPort, udpListenHost, function () {
                logger.info({op: 'Init'}, 'Listening to UDP requests for parser "%s" at %s:%d',
                            udpParserName, this.address().address, this.address().port);
            });
        } else {
            logger.warn({op: 'Init'}, 'Ignoring UDP endpoint "%s": missing parser name', item);
        }
    });
}


/** @export */
exports.main = main;

/** @export */
exports.updateContextCallback = updateContextCallback;


if (require.main === module) {
    main();
}
