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
 * Module for common definitions.
 *
 * @module common
 */


'use strict';


/**
 * Name of the timestamp attribute added automatically to any update context request.
 */
exports.timestampAttrName = '_timestamp';


/**
 * HTTP header for transaction id.
 */
exports.txIdHttpHeader = 'txId';


/**
 * Context Broker API 'v0' (i.e. NGSI10).
 */
exports.BROKER_API_V0 = 'ngsi10';


/**
 * Context Broker API 'v1'.
 */
exports.BROKER_API_V1 = 'v1';


/**
 * Context Broker API 'v2'.
 */
exports.BROKER_API_V2 = 'v2';


/**
 * Default values.
 *
 * @namespace
 * @property {Object} defaults              Default values for configuration options.
 * @property {String} defaults.logLevel     Default logging level.
 * @property {String} defaults.brokerUrl    Default Context Broker URL.
 * @property {String} defaults.brokerApi    Default Context Broker API version.
 * @property {String} defaults.listenHost   Default adapter HTTP listen host.
 * @property {Number} defaults.listenPort   Default adapter HTTP listen port.
 * @property {String} defaults.udpEndpoints Default list of UDP endpoints (host:port:parser).
 * @property {String} defaults.parsersPath  Default path with directories to look for parsers.
 * @property {Number} defaults.maxRequests  Default maximum number of simultaneous outgoing requests.
 * @property {Number} defaults.retries      Default maximum number of Context Broker invocation retries.
 */
exports.defaults = {
    logLevel: 'INFO',
    brokerUrl: 'http://127.0.0.1:1026/',
    brokerApi: exports.BROKER_API_V0,
    listenHost: '0.0.0.0',
    listenPort: 1337,
    udpEndpoints: null,
    parsersPath: 'lib/parsers:lib/parsers/nagios',
    maxRequests: 5,
    retries: 2
};
