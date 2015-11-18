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
 * Module that defines server configuration options.
 *
 * @module options
 */


'use strict';


var defaults = require('../lib/common').defaults;


/**
 * @namespace
 * @property {Object} opts                  Command line options.
 * @property {String} opts.logLevel         Logging level.
 * @property {String} opts.brokerUrl        Context Broker URL.
 * @property {String} opts.listenHost       Adapter listen HTTP host.
 * @property {Number} opts.listenPort       Adapter listen HTTP port.
 * @property {String} opts.udpEndpoints     Comma-separated list of UDP endpoints (host:port:parser).
 * @property {Number} opts.retries          Maximum number of Context Broker invocation retries.
 */
var opts = require('optimist')
    .options('l', {
        alias: 'logLevel',
        'default': process.env['ADAPTER_LOGLEVEL'] || defaults.logLevel,
        describe: 'Logging level'
    }).options('H', {
        alias: 'listenHost',
        'default': process.env['ADAPTER_LISTEN_HOST'] || defaults.listenHost,
        describe: 'Adapter listen host'
    }).options('p', {
        alias: 'listenPort',
        'default': process.env['ADAPTER_LISTEN_PORT'] || defaults.listenPort,
        describe: 'Adapter listen port'
    }).options('u', {
        alias: 'udpEndpoints',
        'default': process.env['ADAPTER_UDP_ENDPOINTS'] || defaults.udpEndpoints,
        describe: 'List of UDP endpoints (host:port:parser)'
    }).options('b', {
        alias: 'brokerUrl',
        'default': process.env['ADAPTER_BROKER_URL'] || defaults.brokerUrl,
        describe: 'Context Broker URL'
    }).options('r', {
        alias: 'retries',
        'default': process.env['ADAPTER_RETRIES'] || defaults.retries,
        describe: 'Maximum retries'
    }).options('h', {
        alias: 'help',
        'boolean': true,
        describe: 'Show help'
    })
    .demand([]);


// Do not allow extra options/arguments
var extra = (opts.argv._.length > 0) || (Object.keys(opts.argv).length !== 2 + 2 * (Object.keys(defaults).length + 1));
if (opts.argv.help || extra) {
    opts.showHelp();
    process.exit(1);
}


/**
 * Command line options.
 */
module.exports = opts.argv;
