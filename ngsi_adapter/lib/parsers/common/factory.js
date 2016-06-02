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
 * Module that implements a parser factory.
 *
 * @module factory
 */


'use strict';
/* jshint expr: true */


var url = require('url'),
    util = require('util'),
    path = require('path'),
    config = require('../../config'),
    baseParser = require('./base').parser;


/**
 * Cache of parsers already loaded.
 */
var parsersCache = {};


/**
 * Parser factory: returns a parser object from a dynamically loaded parser prototype from a module `name` located at
 * any of the directories specified in `parsersPath` option, either absolute directories or relative to Adapter's root.
 *
 * @param {String} name  The name of the parser.
 * @returns {Object} The parser been loaded according to given module name.
 */
function getParserByName(name) {
    var parser = parsersCache[name],
        errorCode = 'MODULE_NOT_FOUND';

    parser || config.parsersPath.split(':').some(function (dir) {
        try {
            /* jshint proto: true, camelcase: false */
            var prototype = require(path.join(dir, util.format('%s.js', name))).parser;
            if (prototype.__proto__ === null) {
                prototype.__proto__ = baseParser;  // ensure baseParser is part of the prototype chain
            }
            parser = Object.create(prototype);
            parsersCache[name] = parser;
            return true;
        } catch (err) {
            errorCode = (err.code === 'MODULE_NOT_FOUND') ? errorCode : 'INVALID';
            return false;
        }
    });
    if (!parser) {
        var errorMsg = (errorCode === 'MODULE_NOT_FOUND') ?
            util.format('Module "%s.js" could not be found at path %s', name, config.parsersPath) :
            util.format('Invalid parser module "%s.js" found at path %s', name, config.parsersPath);
        throw new Error(errorMsg);
    }
    return parser;
}


/**
 * Parser factory: given a request URL `http://host:port/path?query`, takes `path` as
 * the name of the probe whose data (request body) will be parsed. Tries to dynamically
 * load a parser object from module named after the probe.
 *
 * @param {http.IncomingMessage} request  The request to this server.
 * @returns {Object} The parser been loaded according to probe mentioned in request.
 */
function getParser(request) {
    var name = url.parse(request.url).pathname.slice(1);
    try {
        return getParserByName(name);
    } catch (err) {
        throw new Error((!name) ? 'Missing resource in request' : util.format('Unknown probe "%s" (%s)', name, err));
    }
}


/**
 * Parser factory: lookup by name.
 */
exports.getParserByName = getParserByName;


/**
 * Parser factory: lookup by request URL.
 */
exports.getParser = getParser;
