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


/**
 * @namespace
 * @property {Object} defaults              Default values for configuration options.
 * @property {String} defaults.brokerUrl    Default Context Broker URL.
 * @property {String} defaults.listenHost   Default adapter listen host.
 * @property {Number} defaults.listenPort   Default adapter listen port.
 * @property {Number} defaults.retries      Default maximum number of invocation retries.
 */
var defaults = {
    brokerUrl:  'http://127.0.0.1:1338/',
    listenHost: '127.0.0.1',
    listenPort: 1337,
    retries:    2
};


/**
 * @namespace
 * @property {Object} opts                  Command line options.
 * @property {String} opts.brokerUrl        Context Broker URL.
 * @property {String} opts.listenHost       Adapter listen host.
 * @property {Number} opts.listenPort       Adapter listen port.
 * @property {Number} opts.retries          Maximum number of invocation retries.
 */
var opts = require('optimist')
    .options('b', { alias: 'brokerUrl',  'default': defaults.brokerUrl,  describe: 'Context Broker URL'  })
    .options('H', { alias: 'listenHost', 'default': defaults.listenHost, describe: 'Adapter listen host' })
    .options('p', { alias: 'listenPort', 'default': defaults.listenPort, describe: 'Adapter listen port' })
    .options('r', { alias: 'retries',    'default': defaults.retries,    describe: 'Maximum retries'     })
    .options('h', { alias: 'help',       'boolean': true,                describe: 'Show help'           })
    .demand([]);


// Do not allow extra options/arguments
var extra = (opts.argv._.length > 0) || (Object.keys(opts.argv).length !== 2 + 2 * (Object.keys(defaults).length + 1));
if (opts.argv.help || extra) {
    opts.showHelp();
    process.exit(1);
}


module.exports = opts.argv;
