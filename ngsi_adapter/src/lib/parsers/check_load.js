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
 * Module that defines a parser for `check_load` Nagios plugin.
 *
 * Context attributes to be calculated:
 *
 * - cpuLoadPct = percentage of CPU load (considered as load in last minute)
 *
 * @module check_load
 * @see https://www.nagios-plugins.org/doc/man/check_load.html
 * @see https://github.com/nagios-plugins/nagios-plugins/blob/maint/plugins/check_load.c
 */


'use strict';
/* jshint -W101, unused: false */


var nagios = require('./common/nagios');


//
// Sample data: "OK - load average: 0.00, 0.00, 0.00|load1=0.000;1.000;1.000;0; load5=0.000;5.000;5.000;0; load15=0.000;15.000;15.000;0;"
//                                    ^     ^     ^           ^     ^     ^  ^           ^     ^     ^  ^            ^      ^      ^  ^
//     % CPU load in last minute -----+     |     |           |     |     |  |           |     |     |  |            |      |      |  |
//     % CPU load in last 5 minutes --------+     |           |     |     |  |           |     |     |  |            |      |      |  |
//     % CPU load in last 15 minutes -------------+           |     |     |  |           |     |     |  |            |      |      |  |
//     =============================                          |     |     |  |           |     |     |  |            |      |      |  |
//     Last minute:     load average -------------------------+     |     |  |           |     |     |  |            |      |      |  |
//                      warning threshold --------------------------+     |  |           |     |     |  |            |      |      |  |
//                      critical threshold -------------------------------+  |           |     |     |  |            |      |      |  |
//                      reserved --------------------------------------------+           |     |     |  |            |      |      |  |
//                                                                                       |     |     |  |            |      |      |  |
//     Last 5 minutes:  load average ----------------------------------------------------+     |     |  |            |      |      |  |
//                      warning threshold -----------------------------------------------------+     |  |            |      |      |  |
//                      critical threshold ----------------------------------------------------------+  |            |      |      |  |
//                      reserved -----------------------------------------------------------------------+            |      |      |  |
//                                                                                                                   |      |      |  |
//     Last 15 minutes: load average --------------------------------------------------------------------------------+      |      |  |
//                      warning threshold ----------------------------------------------------------------------------------+      |  |
//                      critical threshold ----------------------------------------------------------------------------------------+  |
//                      reserved -----------------------------------------------------------------------------------------------------+
//
var parser = Object.create(nagios.parser);
parser.getContextAttrs = function(multilineData, multilinePerfData) {
    var data  = multilineData.split('\n')[0];   // only consider first line of data, discard perfData
    var attrs = { cpuLoadPct: NaN };

    var items = data.split(':');
    if (items[1]) {
        var loads = items[1].split(',');
        attrs.cpuLoadPct = parseFloat(loads[0]);
    }

    if (isNaN(attrs.cpuLoadPct)) {
        throw new Error('No valid load data found');
    }

    return attrs;
};


exports.parser = parser;
