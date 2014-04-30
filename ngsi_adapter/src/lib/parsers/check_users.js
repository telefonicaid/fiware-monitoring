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
 * Module that defines a parser for `check_users` Nagios plugin.
 *
 * Context attributes to be calculated:
 *
 * - users = number of users logged in
 *
 * @module check_users
 * @see https://www.nagios-plugins.org/doc/man/check_users.html
 * @see https://github.com/nagios-plugins/nagios-plugins/blob/maint/plugins/check_users.c
 */


'use strict';
/* jshint -W101, unused: false */


var nagios = require('./common/nagios');


//
// Sample data: "USERS OK - 1 users currently logged in |users=1;10;15;0"
//                          ^                                  ^  ^  ^ ^
//     # of users ----------+----------------------------------+  |  | |
//     ==========                                                 |  | |
//     Warning threshold -----------------------------------------+  | |
//     Critical threshold -------------------------------------------+ |
//     Reserved -------------------------------------------------------+
//
var parser = Object.create(nagios.parser);
parser.getContextAttrs = function(multilineData, multilinePerfData) {
    var data  = multilineData.split('\n')[0];   // only consider first line of data, discard perfData
    var attrs = { users: NaN };

    var items = data.split('-');
    if (items[1]) {
        attrs.users = parseFloat(items[1].trim().split(' ')[0]);
    }

    if (isNaN(attrs.users)) {
        throw new Error('No valid users data found');
    }

    return attrs;
};


exports.parser = parser;
