//
// Copyright 2013 Telefónica I+D
// All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License"); you may
// not use this file except in compliance with the License. You may obtain
// a copy of the License at
//
//       http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
// WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
// License for the specific language governing permissions and limitations
// under the License.
//


'use strict';


var http = require('http'),
    util = require('util');


var defaults = {
    listenHost: '127.0.0.1',
    listenPort: 1338
};


function doPost(request, response) {
    console.log(util.format('[INFO ] << Body: %s', request.body));
    var responseBody = '<ngsi>RESPONSE</ngsi>';
    response.writeHead(200, {
        'Content-Length': responseBody.length,
        'Content-Type':   'text/xml'
    });
    response.end(responseBody, 'utf8');
    console.log(util.format('[INFO ] >> %d %s: %s', response.statusCode, http.STATUS_CODES[response.statusCode],
        responseBody));
}


function doNotAllowed(request, response) {
    response.writeHead(405);
    response.end();
    console.log(util.format('[INFO ] >> %d %s', response.statusCode, http.STATUS_CODES[response.statusCode]));
}


function syncRequestListener(request, response) {
    console.log(util.format('\n[INFO ] << HTTP %s', request.method));
    var allowed = (request.method === 'POST');
    if (allowed) {
        request.body = '';
        request.on('data', function(chunk) {
            request.body += chunk;
        });
        request.on('end', function() {
            doPost(request, response);
        });
    } else {
        doNotAllowed(request, response);
    }
}


exports.main = function() {
    // Parse command line options (do not allow extra options/arguments)
    var opts = require('optimist')
        .options('H', { alias: 'listenHost', 'default': defaults.listenHost, describe: 'Broker listen host' })
        .options('p', { alias: 'listenPort', 'default': defaults.listenPort, describe: 'Broker listen port' })
        .options('h', { alias: 'help',       'boolean': true,                describe: 'Show help'          })
        .demand([]);
    var extra = (opts.argv._.length > 0) || (Object.keys(opts.argv).length !== 2 + 2*(Object.keys(defaults).length+1));
    if (opts.argv.help || extra) {
        opts.showHelp();
        process.exit(1);
    }

    // Create HTTP server
    process.on('uncaughtException', function(err) {
        console.log(util.format('[ERROR] %s', err.message));
        process.exit(1);
    });
    http.createServer(syncRequestListener).listen(opts.argv.listenPort, opts.argv.listenHost, function() {
        console.log(util.format('[INFO ] Server running at http://%s:%d/', this.address().address,this.address().port));
    });
};


if (require.main === module) {
    exports.main();
}
