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
 * Module that defines a parser for `check_disk` Nagios plugin.
 *
 * Context attributes to be calculated:
 *
 * - freeSpacePct = percentage of free space at given group/single partition (NO MULTIPLE PARTITIONS SUPPORTED UNLESS GROUPED!)
 *
 * @module check_disk
 * @see https://www.nagios-plugins.org/doc/man/check_disk.html
 * @see https://github.com/nagios-plugins/nagios-plugins/blob/maint/plugins/check_disk.c
 */


'use strict';
/* jshint -W101, unused: false */


var nagios = require('./common/nagios');


//
// Sample data #1 (all partitions grouped, -g option):
//      "DISK OK - free space: mygroup 25484 MB (85% inode=95%);| mygroup=4151MB;31071;31121;0;31171"
//                                ^      ^        ^         ^        ^      ^      ^     ^   ^   ^
//      Group name ---------------+      |        |         |        |      |      |     |   |   |
//      Amount free space ---------------+        |         |        |      |      |     |   |   |
//      % free space -----------------------------+         |        |      |      |     |   |   |
//      % free inodes --------------------------------------+        |      |      |     |   |   |
//      =============                                                |      |      |     |   |   |
//      Group name --------------------------------------------------+      |      |     |   |   |
//      Amount used space (including units)---------------------------------+      |     |   |   |
//      Warning high tide ---------------------------------------------------------+     |   |   |
//      Critical high tide --------------------------------------------------------------+   |   |
//      Reserved ----------------------------------------------------------------------------+   |
//      Total space -----------------------------------------------------------------------------+
//
// Sample data #2 (no grouping, checks per partition):
//      "DISK OK - free space: / 1393 MB (29% inode=66%); /data 4195 MB (87% inode=99%);| /=3388MB;5023;5023;0;5038 /data=586MB;5022;5022;0;5037"
//                             ^    ^      ^         ^    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^   ^   ^     ^    ^   ^  ^   ^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//      Filesystem #1:         |    |      |         |                 |                  |   |     |    |   |  |                |
//      * Mount point ---------+    |      |         |                 |                  |   |     |    |   |  |                |
//      * Amount free space --------+      |         |                 |                  |   |     |    |   |  |                |
//      * % free space --------------------+         |                 |                  |   |     |    |   |  |                |
//      * % free inodes -----------------------------+                 |                  |   |     |    |   |  |                |
//                                                                     |                  |   |     |    |   |  |                |
//      Filesystem #2:                                                 |                  |   |     |    |   |  |                |
//      * Idem --------------------------------------------------------+                  |   |     |    |   |  |                |
//                                                                                        |   |     |    |   |  |                |
//      ...                                                                               |   |     |    |   |  |                |
//                                                                                        |   |     |    |   |  |                |
//      Filesystem #n                                                                     |   |     |    |   |  |                |
//                                                                                        |   |     |    |   |  |                |
//      ===========================================================================       |   |     |    |   |  |                |
//                                                                                        |   |     |    |   |  |                |
//      Filesystem #1:                                                                    |   |     |    |   |  |                |
//      * Mount point --------------------------------------------------------------------+   |     |    |   |  |                |
//      * Amount used space (including units)-------------------------------------------------+     |    |   |  |                |
//      * Warning high tide ------------------------------------------------------------------------+    |   |  |                |
//      * Critical high tide ----------------------------------------------------------------------------+   |  |                |
//      * Reserved ------------------------------------------------------------------------------------------+  |                |
//      * Total space ------------------------------------------------------------------------------------------+                |
//                                                                                                                               |
//      Filesystem #2:                                                                                                           |
//      * Idem ------------------------------------------------------------------------------------------------------------------+
//
//      ...
//
//      Filesystem #n
//
var parser = Object.create(nagios.parser);
parser.getContextAttrs = function(multilineData, multilinePerfData) {
    var data  = multilineData.split('\n')[0];   // only consider first line of data, discard perfData
    var attrs = { freeSpacePct: NaN };

    var items = data.split(':');
    if (items[1]) {
        var groups = items[1].split(';');
        // TODO: what if there are multiple, non-grouped partitions?
        if (groups.length === 2 && !groups[1].trim()) {
            attrs.freeSpacePct = parseFloat(groups[0].replace(/([^(]+\()(\w+)%([^)]+\))/, '$2'));
        }
    }

    if (isNaN(attrs.freeSpacePct)) {
        throw new Error('No valid disk data found');
    }

    return attrs;
};


exports.parser = parser;
