/*
 * Copyright 2013 Telefónica I+D
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
 * Module that implements a HTTP asynchronous server processing requests for
 * adaptation from raw monitoring data into NGSI format, then using the results
 * to invoke Context Broker.
 *
 * @module adapter
 */


'use strict';


var http = require('http'),
    url = require('url'),
    retry = require('retry'),
    domain = require('domain'),
    cuid = require('cuid'),
    logger = require('./logger'),
    common = require('./common'),
    parser = require('./parsers/common/factory'),
    opts = require('../config/options');


/**
 * Asynchronously process POST requests and then invoke updateContext() on ContextBroker.
 *
 * @param {http.IncomingMessage} request    The request to this server.
 * @param {RequestCallback}      callback   The callback that handles the response.
 */
function doPost(request, callback) {
    try {
        domain.active.context.op = 'Parse';
        logger.debug('Probe data "%s"', request.body);
        var remoteUrl = url.parse(opts.brokerUrl);
        var dataParser = request.parser;
        var updateReqType = dataParser.getContentType();
        var updateReqBody = dataParser.updateContextRequest(request);
        var updateReqOpts = {
            hostname: remoteUrl.hostname,
            port: remoteUrl.port,
            path: '/NGSI10/updateContext',
            method: 'POST',
            headers: {
               'Accept': updateReqType,
               'Content-Type': updateReqType,
               'Content-Length': updateReqBody.length
            }
        };
        updateReqOpts.headers[common.txIdHttpHeader] = domain.active.context.trans;
        /* jshint unused: false */
        var operation = retry.operation({ retries: opts.retries });
        operation.attempt(function(currentAttempt) {
            domain.active.context.op = 'UpdateContext';
            logger.info('Request to ContextBroker at %s...', opts.brokerUrl);
            logger.debug('%s', { toString: function() {
                return updateReqBody.split('\n').map(function(line) {return line.trim();}).join('');
            }});
            var updateReq = http.request(updateReqOpts, function(response) {
                var responseBody = '';
                response.setEncoding('utf8');
                response.on('data', function(chunk) {
                    responseBody += chunk;
                });
                response.on('end', function() {
                    callback(null, response.statusCode, responseBody);
                });
            });
            updateReq.on('error', function(err) {
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
 * @param {Error}   err                     The error ocurred in request, or null.
 * @param {Number} [responseStatus]         The response status code.
 * @param {String} [responseBody]           The response body contents.
 */
function callback(err, responseStatus, responseBody) {
    if (err) {
        logger.error(err.message);
    } else {
        logger.info('Response status %d %s', responseStatus, http.STATUS_CODES[responseStatus]);
        logger.debug('%s', { toString: function() {
            return responseBody.split('\n').map(function(line) {return line.trim();}).join('');
        }});
    }
}


/**
 * Server requests listener.
 *
 * Request URL looks like `http://host:port/path?query`:
 *
 * - Request query string MUST include arguments `id` and `type`
 * - Request path will denote the name of the originating probe
 * - Request headers may include a transaction identifier ({@link common#txIdHttpHeader})
 *
 * @param {http.IncomingMessage} request    The request to this server.
 * @param {http.ServerResponse}  response   The response from this server.
 */
function asyncRequestListener(request, response) {
    var reqd = domain.create();
    reqd.add(request);
    reqd.add(response);
    reqd.context = {
        trans: request.headers[common.txIdHttpHeader.toLowerCase()] || cuid(),
        op: request.method
    };
    reqd.on('error', function(err) {
        logger.error(err.message);
        response.writeHead(500);  // server error
        response.end();
    });
    reqd.run(function() {
        logger.info('Request on resource %s', request.url.split('?').join(' with params '));
        var status = 405;  // not allowed
        if (request.method === 'POST') {
            var query = url.parse(request.url, true).query;
            var entityId = query.id;
            var entityType = query.type;
            try {
                status = 400;  // bad request
                if (!entityId || !entityType) {
                    throw new Error('Missing entityId and/or entityType');
                }
                status = 404;  // not found
                request.parser = parser.getParser(request);
                status = 200;  // ok
                request.timestamp = Date.now();
                request.body = '';
                request.on('data', function(chunk) {
                    request.body += chunk;
                });
                request.on('end', function() {
                    process.nextTick(function() {
                        doPost(request, exports.requestCallback);
                    });
                });
            } catch (err) {
                logger.error(err.message);
            }
        }
        logger.info('Response status %d %s', status, http.STATUS_CODES[status]);
        response.writeHead(status);
        response.end();
    });
}


/**
 * Server main.
 */
function main() {
    process.on('uncaughtException', function(err) {
        logger.error({op: 'Exit'}, err.message);
        process.exit(1);
    });
    process.on('exit', function() {
        logger.info({op: 'Exit'}, 'Server stopped');
    });
    process.on('SIGINT', function() {
        process.exit();
    });
    process.on('SIGTERM', function() {
        process.exit();
    });
    http.createServer(asyncRequestListener).listen(opts.listenPort, opts.listenHost, function() {
        logger.info({op: 'Init'}, 'Server listening at http://%s:%d/', this.address().address, this.address().port);
    });
}


/** @export */
exports.main = main;

/** @export */
exports.requestCallback = callback;


if (require.main === module) {
    main();
}
