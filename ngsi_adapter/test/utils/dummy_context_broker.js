#!/usr/bin/env node
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
 * Module that implements a dummy Context Broker for testing purposes.
 *
 * @module dummy_context_broker
 */


'use strict';


var domain = require('domain'),
    logger = require('logops'),
    http = require('http'),
    util = require('util'),
    txid = require('cuid'),
    uuid = require('node-uuid').v1,
    common = require('../../lib/common');


var defaults = {
    logLevel: 'DEBUG',
    listenHost: '127.0.0.1',
    listenPort: 1026
};


function doPost(request, response) {
    var correlator = request.domain.context.corr,
        contentType = request.headers['content-type'],
        brokerApi = (request.url.indexOf('/v2') !== -1) ? common.BROKER_API_V2 : common.BROKER_API_V1,
        responseCode = null,
        responseBody = null;
    switch (brokerApi) {
        case common.BROKER_API_V2:
            responseCode = (request.url.match(/\/entities\/\S+\/attrs.*[?&]type=\S+/)) ? 204 : 400;
            responseBody = (responseCode === 204) ? '' : '{"error": "BadRequest"}';
            break;
        default:
            var code = (request.url.match(/\/(NGSI10|ngsi10|v1)\/updateContext/)) ? 200 : 400;
            var body = (contentType === 'application/xml') ?
                util.format('<orionError><code>%d</code></orionError>', code) :
                util.format('{"orionError": {"code": %d}}', code);
            responseCode = 200;
            responseBody = (code === 400) ? body : request.body;
    }
    var responseHeaders = {
        'Content-Length': responseBody.length,
        'Content-Type': contentType
    };
    responseHeaders[common.correlatorHttpHeader] = correlator;
    response.writeHead(responseCode, responseHeaders);
    response.end(responseBody, 'utf8');
    logger.info('Response status %d %s', response.statusCode, http.STATUS_CODES[response.statusCode]);
    logger.debug('Response "%s"', { toString: function () {
        return responseBody.split('\n').map(function (line) {return line.trim();}).join('');
    }});
}


function doError(request, response, status) {
    var correlator = request.domain.context.corr,
        contentType = request.headers['content-type'],
        brokerApi = (request.url.indexOf('/v2') !== -1) ? common.BROKER_API_V2 : common.BROKER_API_V1,
        responseBody = null;
    switch (brokerApi) {
        case common.BROKER_API_V2:
            responseBody = util.format('{"error": "%s"}', http.STATUS_CODES[response.status]);
            break;
        default:
            responseBody = '';
    }
    var responseHeaders = {
        'Content-Length': responseBody.length,
        'Content-Type': contentType
    };
    responseHeaders[common.correlatorHttpHeader] = correlator;
    response.writeHead(status, responseHeaders);
    response.end(responseBody, 'utf8');
    logger.error('Response status %d %s %s', response.statusCode, http.STATUS_CODES[response.statusCode], responseBody);
}


function syncRequestListener(request, response) {
    var contentType = request.headers['content-type'],
        contentLen = request.headers['content-length'],
        correlator = request.headers[common.correlatorHttpHeader.toLowerCase()] || uuid(),
        reqd = domain.create();
    reqd.add(request);
    reqd.add(response);
    reqd.context = {
        trans: txid(),
        corr: correlator,
        op: request.method
    };
    reqd.on('error', function (err) {
        logger.error(err.message);
        doError(request, response, 500);  // server error
    });
    reqd.run(function () {
        logger.info('Request to resource %s, Content-Type=%s Content-Length=%s',
            request.url, contentType || 'n/a', contentLen || 'n/a');
        if (request.method !== 'POST') {
            doError(request, response, 405);  // not allowed
        } else {
            request.body = '';
            request.on('data', function (chunk) {
                request.body += chunk;
            });
            request.on('end', function () {
                logger.debug('Request "%s"', { toString: function () {
                    return request.body.split('\n').map(function (line) {return line.trim();}).join('');
                }});
                doPost(request, response);
            });
        }
    });
}


exports.main = function () {
    // Parse command line options (do not allow extra options/arguments)
    var opts = require('optimist')
        .options('H', { alias: 'listenHost', 'default': defaults.listenHost, describe: 'Context Broker listen host' })
        .options('p', { alias: 'listenPort', 'default': defaults.listenPort, describe: 'Context Broker listen port' })
        .options('l', { alias: 'logLevel',   'default': defaults.logLevel,   describe: 'Logging level'              })
        .options('h', { alias: 'help',       'boolean': true,                describe: 'Show help'                  })
        .demand([]),
        optsCount = Object.keys(defaults).length,
        expectedKeyCount = 2 * (optsCount + 1) + 2;
    if (opts.argv.help || (opts.argv._.length > 0) || (Object.keys(opts.argv).length !== expectedKeyCount)) {
        opts.showHelp();
        process.exit(1);
    } else {
        opts = opts.argv;
        logger.setLevel(opts.logLevel);
        logger.getContext = function () {
            return (domain.active) ? domain.active.context : {};
        };
    }

    // Create HTTP server
    process.on('uncaughtException', function (err) {
        logger.error({op: 'Exit'}, err.message);
        process.exit(1);
    });
    process.on('exit', function () {
        logger.info({op: 'Exit'}, 'Context Broker stopped');
    });
    process.on('SIGINT', function () {
        process.exit();
    });
    process.on('SIGTERM', function () {
        process.exit();
    });
    http.createServer(syncRequestListener).listen(opts.listenPort, opts.listenHost, function () {
        var bind = this.address();
        logger.info({op: 'Init'}, 'Context Broker listening at http://%s:%d/', bind.address, bind.port);
    });
};


if (require.main === module) {
    exports.main();
}
