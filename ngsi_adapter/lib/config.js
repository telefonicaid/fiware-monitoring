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
 * Module that defines server configuration options.
 *
 * @module options
 */


'use strict';
/* jshint -W069 */


var fs = require('fs'),
    url = require('url'),
    path = require('path'),
    util = require('util'),
    defaults = require('../lib/common').defaults,
    optsCount = Object.keys(defaults).length - 1,  // brokerApi has default, but is not included in options
    absoluteBaseDir = path.normalize(__dirname + path.sep + '..');


/**
 * @namespace
 * @property {Object} opts                  Command line options.
 * @property {String} opts.logLevel         Logging level.
 * @property {String} opts.brokerUrl        Context Broker URL.
 * @property {String} opts.listenHost       Adapter listen HTTP host.
 * @property {Number} opts.listenPort       Adapter listen HTTP port.
 * @property {String} opts.udpEndpoints     Comma-separated list of UDP endpoints (host:port:parser).
 * @property {String} opts.parsersPath      Colon-separated path with directories to look for parsers.
 * @property {Number} opts.maxRequests      Maximum number of simultaneous outgoing requests.
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
    }).options('P', {
        alias: 'parsersPath',
        'default': process.env['ADAPTER_PARSERS_PATH'] || defaults.parsersPath,
        describe: 'Path to look for parsers'
    }).options('b', {
        alias: 'brokerUrl',
        'default': process.env['ADAPTER_BROKER_URL'] || defaults.brokerUrl,
        describe: 'Context Broker URL'
    }).options('m', {
        alias: 'maxRequests',
        'default': process.env['ADAPTER_MAX_REQUESTS'] || defaults.maxRequests,
        describe: 'Maximum simultaneous requests'
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


/**
 * Command line options, only considering last value of duplicated ones.
 */
var config = module.exports = opts.argv;
for (var name in config) {
    if ((name !== '_') && (opts.argv[name] instanceof Array)) {
        config[name] = config[name].pop();
    }
}


/**
 * Check configuration and normalize values when needed.
 *
 * @function check
 * @memberof config
 * @this config
 * @param {Function} callback   Callback invoked with the first error condition found.
 */
config.check = function (callback) {
    // Show help or abort if extra options/arguments given: expected = short,long * (N opts + h,help) + 3 fixed attrs
    var expectedKeyCount = 2 * (optsCount + 1) + 3;
    if (this.help || (this._.length > 0) || (Object.keys(this).length !== expectedKeyCount)) {
        opts.showHelp();
        process.exit(1);
    }

    // Convert parsersPath to list of unique absolute directories and check whether they are accessible
    var list = [],
        lastDir = null,
        notFound = (this.parsersPath + ':' + defaults.parsersPath).split(':').some(function (dir) {
            try {
                fs.readdirSync(lastDir = path.resolve(absoluteBaseDir, dir));
                list.push(lastDir);
                return false;
            } catch (ex) {
                return true;
            }
        });
    if (notFound) {
        var processUser = require('userid').username(process.getuid());
        callback(util.format('Server user "%s" cannot access parsers directory "%s"', processUser, lastDir));
    } else {
        this.parsersPath = list.filter(function (item, index) { return list.indexOf(item) === index; }).join(':');
    }

    // Get Context Broker API version from URL and normalize values
    var urlobj = url.parse(this.brokerUrl),
        urlpath = urlobj.pathname.replace(/(.+)\/$/, '$1').toLowerCase();
    urlobj.pathname = '/';
    this.brokerUrl = url.format(urlobj);
    this.brokerApi = (urlpath === '/') ? defaults.brokerApi : urlpath.replace(/^\/(.+)/, '$1');
};
