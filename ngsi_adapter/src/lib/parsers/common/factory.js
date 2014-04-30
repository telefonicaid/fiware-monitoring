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
 * Module that implements a parser factory.
 *
 * @module factory
 */


'use strict';


var url    = require('url'),
    util   = require('util'),
    logger = require('../../../config/logger');


/**
 * Parser factory: given a request URL `http://host:port/path?query`, takes `path` as
 * the name of the probe whose data (request body) will be parsed. Tries to dynamically
 * load a parser object from module at `lib/parsers/` directory named after the probe.
 *
 * @param {http.IncomingMessage} request    The request to this server.
 * @returns {Object} The parser been loaded according to probe mentioned in request.
 */
function getParser(request) {
    var probeName = url.parse(request.url).pathname.slice(1);
    try {
        var parser = require(util.format('../%s', probeName)).parser;
        parser.setRequest(request);
        return parser;
    } catch (err) {
        logger.debug(err.message);
        throw new Error(util.format('Unknown probe "%s" (url path %s)', probeName, request.url));
    }
}


exports.getParser = getParser;
