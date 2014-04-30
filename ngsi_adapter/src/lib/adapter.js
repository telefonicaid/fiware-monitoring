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


var http   = require('http'),
    url    = require('url'),
    retry  = require('retry'),
    logger = require('../config/logger'),
    opts   = require('../config/options'),
    parser = require('./parsers/common/factory');


/**
 * Asynchronously process POST requests and then invoke updateContext() on ContextBroker.
 *
 * @param {http.IncomingMessage} request    The request to this server.
 * @param {RequestCallback}      callback   The callback that handles the response.
 */
function doPost(request, callback) {
    try {
        logger.info('<< Body: %s', request.body);
        var remoteUrlData = url.parse(opts.brokerUrl);
        var updateReqType = request.parser.getRequestContentType();
        var updateReqBody = request.parser.updateContextRequest();
        var updateReqOpts = {
            hostname: remoteUrlData.hostname,
            port:     remoteUrlData.port,
            path:     '/NGSI10/updateContext',
            method:   'POST',
            headers: {
               'Accept':            updateReqType,
               'Content-Type':      updateReqType,
               'Content-Length':    updateReqBody.length
            }
        };
        /* jshint unused: false */
        var operation = retry.operation({ retries: opts.retries });
        operation.attempt(function(currentAttempt) {
            logger.info('%s %s...', updateReqOpts.method, opts.brokerUrl);
            logger.debug('%s', updateReqBody);
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
        logger.error('updateContext(): "%s"', err.message);
    } else {
        logger.info('updateContext(): status=%d response=%s', responseStatus, responseBody);
    }
}


/**
 * Server requests listener.
 *
 * Request URL looks like `http://host:port/path?query`:
 *
 * - Request query string MUST include arguments `id` and `type`
 * - Request path will denote the name of the originating probe
 *
 * @param {http.IncomingMessage} request    The request to this server.
 * @param {http.ServerResponse}  response   The response from this server.
 */
function asyncRequestListener(request, response) {
    logger.info('<< HTTP %s', request.method);
    var status  = 405;      // not allowed
    var allowed = (request.method === 'POST');
    if (allowed) {
        try {
            status = 200;   // ok
            request.parser = parser.getParser(request);
            request.body = '';
            request.on('data', function(chunk) {
                request.body += chunk;
            });
            request.on('end', function() {
                process.nextTick(function() {
                    doPost(request, callback);
                });
            });
        } catch (err) {
            status = 404;   // not found
            logger.error(err.message);
        }
    }
    response.writeHead(status);
    response.end();
    logger.info('>> %d %s', response.statusCode, http.STATUS_CODES[response.statusCode]);
}


/**
 * Server main.
 */
exports.main = function() {
    process.on('uncaughtException', function(err) {
        logger.error(err.message);
        process.exit(1);
    });
    http.createServer(asyncRequestListener).listen(opts.listenPort, opts.listenHost, function() {
        logger.info('Server running at http://%s:%d/', this.address().address, this.address().port);
    });
};


if (require.main === module) {
    exports.main();
}
