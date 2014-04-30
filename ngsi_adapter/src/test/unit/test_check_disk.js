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
 * Module that defines unit tests for `check_disk` parser.
 *
 * @module test_check_disk
 */


'use strict';
/* jshint -W101, laxbreak: true */


var util   = require('util'),
    assert = require('assert'),
    common = require('./common');


suite('check_disk', function() {

    suiteSetup(function() {
        this.factory    = require('../../lib/parsers/common/factory');
        this.baseurl    = 'http://host:1234/check_disk';
        this.entityId   = '1';
        this.entityType = 'server';
        this.entityData = {
            freeSpacePct: 29
        };
        this.probeData  = {
            groupName:    'mygroup',
            freeSpaceMB:  1393,
            freeSpacePct: this.entityData.freeSpacePct
        };
        this.probeBody  = {
            singleGroup: {
                data: util.format('DISK OK - free space: %s %d MB (%d%% inode=66%%);',
                      this.probeData.groupName, this.probeData.freeSpaceMB, this.probeData.freeSpacePct),
                perf: util.format('%s=3388MB;5023;5023;0;5038',
                      this.probeData.groupName)
            },
            multiPartition: {
                data: 'DISK OK - free space: / 1393 MB (' + this.entityData.freeSpacePct + '% inode=66%); /data 4195 MB (87% inode=99%);',
                perf: '/=3388MB;5023;5023;0;5038 /data=586MB;5022;5022;0;5037'
            }
        };
    });

    suiteTeardown(function() {
    });

    setup(function() {
    });

    teardown(function() {
    });

    test('get_update_request_fails_with_invalid_check_disk_content', function() {
        var request = {
            url:  this.baseurl + '?id=' + this.entityId + '&type=' + this.entityType,
            body: 'XXX INVALID XXX'
        };
        var parser = this.factory.getParser(request);
        assert.throws(
            function() {
                return parser.updateContextRequest();
            }
        );
    });

    test('get_update_request_fails_with_multiple_partitions_check_disk_content', function() {
        var request = {
            url:  this.baseurl + '?id=' + this.entityId + '&type=' + this.entityType,
            body: util.format('%s|%s', this.probeBody.multiPartition.data, this.probeBody.multiPartition.perf)
        };
        var parser = this.factory.getParser(request);
        assert.throws(
            function() {
                return parser.updateContextRequest();
            }
        );
    });

    test('get_update_request_ok_with_valid_check_disk_content', function() {
        var request = {
            url:  this.baseurl + '?id=' + this.entityId + '&type=' + this.entityType,
            body: util.format('%s|%s', this.probeBody.singleGroup.data, this.probeBody.singleGroup.perf)
        };
        var parser = this.factory.getParser(request);
        var update = parser.updateContextRequest();
        common.assertValidUpdateXML(update, this);
    });

    test('parse_ok_free_space_percentage', function() {
        var request = {
            url:  this.baseurl + '?id=' + this.entityId + '&type=' + this.entityType,
            body: util.format('%s|%s', this.probeBody.singleGroup.data, this.probeBody.singleGroup.perf)
        };
        var parser = this.factory.getParser(request);
        var requestData = parser.parseRequest();
        var contextData = parser.getContextAttrs(requestData.data, requestData.perfData);
        assert(contextData.freeSpacePct);
        assert.equal(contextData.freeSpacePct, this.entityData.freeSpacePct);
    });

});
