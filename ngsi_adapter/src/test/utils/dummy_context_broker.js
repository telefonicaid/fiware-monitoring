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
 * Module that implements a dummy Context Broker for testing purposes.
 *
 * @module dummy_context_broker
 */


'use strict';


var domain = require('domain'),
    logger = require('logops'),
    http   = require('http'),
    util   = require('util'),
    cuid   = require('cuid'),
    common = require('../../lib/common');


var defaults = {
    logLevel:   'DEBUG',
    listenHost: '127.0.0.1',
    listenPort: 1026
};


function doPost(request, response) {
    var responseBody = '<ngsi>OK</ngsi>';
    var responseCode = 200;
    if (request.url !== '/NGSI10/updateContext') {
        responseBody = util.format('<ngsi>Invalid resource %s</ngsi>', request.url);
        responseCode = 404;
    } else if (!request.headers['content-length']) {
        responseBody = '<ngsi>Missing Content-Length header</ngsi>';
        responseCode = 500;
    }
    response.writeHead(responseCode, {
        'Content-Length': responseBody.length,
        'Content-Type':   'application/xml'
    });
    response.end(responseBody, 'utf8');
    logger.info('Response %d %s', response.statusCode, http.STATUS_CODES[response.statusCode]);
    logger.debug('%s', { toString: function() {
        return responseBody.split('\n').map(function(line) {return line.trim();}).join('');
    }});
}


function doError(request, response, status) {
    response.writeHead(status);
    response.end();
    logger.info('Response %d %s', response.statusCode, http.STATUS_CODES[response.statusCode]);
}


function syncRequestListener(request, response) {
    var requestTxId = request.headers[common.txIdHttpHeader.toLowerCase()],
        contentType = request.headers['content-type'],
        contentLen  = request.headers['content-length'],
        reqd = domain.create();
    reqd.add(request);
    reqd.add(response);
    reqd.context = {
        trans: requestTxId || cuid(),
        op: request.method
    };
    reqd.on('error', function(err) {
        logger.error(err.message);
        doError(request, response, 500);  // server error
    });
    reqd.run(function() {
        logger.info('Request %s, Content-Type=%s Content-Length=%s %s=%s',
            request.url,
            contentType || 'n/a',
            contentLen  || 'n/a',
            common.txIdHttpHeader,
            requestTxId || 'n/a');
        if (request.method !== 'POST') {
            doError(request, response, 405);  // not allowed
        } else {
            request.body = '';
            request.on('data', function(chunk) {
                request.body += chunk;
            });
            request.on('end', function() {
                logger.debug('%s', { toString: function() {
                    return request.body.split('\n').map(function(line) {return line.trim();}).join('');
                }});
                doPost(request, response);
            });
        }
    });
}


exports.main = function() {
    // Parse command line options (do not allow extra options/arguments)
    var opts = require('optimist')
        .options('H', { alias: 'listenHost', 'default': defaults.listenHost, describe: 'Context Broker listen host' })
        .options('p', { alias: 'listenPort', 'default': defaults.listenPort, describe: 'Context Broker listen port' })
        .options('l', { alias: 'logLevel',   'default': defaults.logLevel,   describe: 'Logging level'              })
        .options('h', { alias: 'help',       'boolean': true,                describe: 'Show help'                  })
        .demand([]);
    var extra = (opts.argv._.length > 0) || (Object.keys(opts.argv).length !== 2 + 2*(Object.keys(defaults).length+1));
    if (opts.argv.help || extra) {
        opts.showHelp();
        process.exit(1);
    } else {
        opts = opts.argv;
        logger.setLevel(opts.logLevel);
        logger.getContext = function() {
            return (domain.active) ? domain.active.context : {};
        };
    }

    // Create HTTP server
    process.on('uncaughtException', function(err) {
        logger.error({op: 'Exit'}, err.message);
        process.exit(1);
    });
    process.on('exit', function() {
        logger.info({op: 'Exit'}, 'Context Broker stopped');
    });
    process.on('SIGINT', function() {
        process.exit();
    });
    process.on('SIGTERM', function() {
        process.exit();
    });
    http.createServer(syncRequestListener).listen(opts.listenPort, opts.listenHost, function() {
        logger.info({op: 'Init'}, 'Context Broker listening at http://%s:%d/', this.address().address,this.address().port);
    });
};


if (require.main === module) {
    exports.main();
}
