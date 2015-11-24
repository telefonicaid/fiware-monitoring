/*
 * Copyright 2013-2015 Telefónica I+D
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
 * Module that defines a base parser object specialized for Nagios probes.
 *
 * @module nagios
 * @see http://nagios.sourceforge.net/docs/3_0/pluginapi.html
 */


'use strict';
/* jshint curly: false */


var baseParser = require('./base').parser;


/**
 * Base parser object (to be extended by Nagios probe-specific parsers).
 * @augments baseParser
 */
var nagiosParser = Object.create(baseParser);


/**
 * Parses the request message body to extract Nagios probe data (both data and optional performance data).
 *
 * @function parseRequest
 * @memberof nagiosParser
 * @param {Domain} reqdomain   Domain handling current request (includes context, timestamp, id, type, body & parser).
 * @returns {EntityData} An object with `data` (and optional `perfData`) members.
 *
 * Probe output format: <code>
 *         TEXT OUTPUT DATA | OPTIONAL PERFDATA
 *         LONG TEXT LINE 1
 *         LONG TEXT LINE 2
 *         ...
 *         LONG TEXT LINE N | PERFDATA LINE 2
 *         PERFDATA LINE 3
 *         ...
 *         PERFDATA LINE N
 * </code>
 */
nagiosParser.parseRequest = function (reqdomain) {
    var entityData = {};
    var lines = reqdomain.body.split('\n');
    var isMultilinePerf = false;
    lines.forEach(function(item) {
        var isFirst = !entityData.data;
        var lineData = item.split('|');
        if (lineData.length > 2 || lineData[0].length === 0) {
            throw new Error('Invalid plugin data format');
        } else if (isFirst) {
            // First line of request body
            entityData.data = lineData[0];
            if (lineData[1]) entityData.perfData = lineData[1];
        } else if (lineData[1]) {
            // Non-first compound line with performance data
            if (!entityData.perfData || isMultilinePerf || lineData[1].length === 0) {
                throw new Error('Invalid optional perfdata format');
            }
            entityData.perfData += '\n' + lineData[1];
            entityData.data += '\n' + lineData[0];
            isMultilinePerf = true;
        } else {
            // Non-first simple line of data / performance data
            if (isMultilinePerf) {
                entityData.perfData += '\n' + lineData[0];
            } else {
                entityData.data += '\n' + lineData[0];
            }
        }
    });
    return entityData;
};


/**
 * Nagios base parser.
 */
exports.parser = nagiosParser;
