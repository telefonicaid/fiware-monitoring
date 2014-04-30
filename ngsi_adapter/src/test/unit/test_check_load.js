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
 * Module that defines unit tests for `check_load` parser.
 *
 * @module test_check_load
 */


'use strict';


var util   = require('util'),
    assert = require('assert'),
    common = require('./common');


/* jshint laxbreak: true */
suite('check_load', function() {

    suiteSetup(function() {
        this.factory    = require('../../lib/parsers/common/factory');
        this.baseurl    = 'http://host:1234/check_load';
        this.entityId   = '1';
        this.entityType = 'server';
        this.entityData = {
            cpuLoadPct: 0.01
        };
        this.probeData  = {
            load1:      this.entityData.cpuLoadPct,
            load5:      2*this.entityData.cpuLoadPct,
            load15:     5*this.entityData.cpuLoadPct
        };
        this.probeBody = {
            data: util.format('OK - load average: %d, %d, %d',
                  this.probeData.load1, this.probeData.load5, this.probeData.load15),
            perf: util.format('load1=%d0;1.000;1.000;0; load5=%d0;5.000;5.000;0; load15=%d0;15.000;15.000;0;',
                  this.probeData.load1, this.probeData.load5, this.probeData.load15)
        };
    });

    suiteTeardown(function() {
    });

    setup(function() {
    });

    teardown(function() {
    });

    test('get_update_request_fails_with_invalid_check_load_content', function() {
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

    test('get_update_request_ok_with_valid_check_load_content', function() {
        var request = {
            url:  this.baseurl + '?id=' + this.entityId + '&type=' + this.entityType,
            body: util.format('%s|%s', this.probeBody.data, this.probeBody.perf)
        };
        var parser = this.factory.getParser(request);
        var update = parser.updateContextRequest();
        common.assertValidUpdateXML(update, this);
    });

    test('parse_ok_cpu_load_percentage', function() {
        var request = {
            url:  this.baseurl + '?id=' + this.entityId + '&type=' + this.entityType,
            body: util.format('%s|%s', this.probeBody.data, this.probeBody.perf)
        };
        var parser = this.factory.getParser(request);
        var requestData = parser.parseRequest();
        var contextData = parser.getContextAttrs(requestData.data, requestData.perfData);
        assert(contextData.cpuLoadPct);
        assert.equal(contextData.cpuLoadPct, this.entityData.cpuLoadPct);
        assert.equal(contextData.cpuLoadPct, this.probeData.load1);
    });

});
