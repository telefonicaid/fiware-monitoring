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
 * Module that defines unit tests for `check_procs` parser.
 *
 * @module test_check_procs
 */


'use strict';


var util   = require('util'),
    assert = require('assert'),
    common = require('./common');


/* jshint laxbreak: true */
suite('check_procs', function() {

    suiteSetup(function() {
        this.factory    = require('../../lib/parsers/common/factory');
        this.baseurl    = 'http://host:1234/check_procs';
        this.entityId   = '1';
        this.entityType = 'server';
        this.entityData = {
            procs: 136
        };
        this.probeData  = {
            procs: this.entityData.procs
        };
        this.probeBody  = {
            data: util.format('PROCS OK: %d processes', this.probeData.procs)
        };
    });

    suiteTeardown(function() {
    });

    setup(function() {
    });

    teardown(function() {
    });

    test('get_update_request_fails_with_invalid_check_procs_content', function() {
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

    test('get_update_request_ok_with_valid_check_procs_content', function() {
        var request = {
            url:  this.baseurl + '?id=' + this.entityId + '&type=' + this.entityType,
            body: util.format('%s|%s', this.probeBody.data, this.probeBody.perf)
        };
        var parser = this.factory.getParser(request);
        var update = parser.updateContextRequest();
        common.assertValidUpdateXML(update, this);
    });

    test('get_update_request_ok_with_another_threshold_metric', function() {
        var request = {
            url:  this.baseurl + '?id=' + this.entityId + '&type=' + this.entityType,
            body: util.format('%s|%s', this.probeBody.data, this.probeBody.perf).
                  replace(/^PROCS/, 'VSZ')
        };
        var parser = this.factory.getParser(request);
        var update = parser.updateContextRequest();
        common.assertValidUpdateXML(update, this);
    });

    test('parse_ok_number_of_procs', function() {
        var request = {
            url:  this.baseurl + '?id=' + this.entityId + '&type=' + this.entityType,
            body: util.format('%s|%s', this.probeBody.data, this.probeBody.perf)
        };
        var parser = this.factory.getParser(request);
        var requestData = parser.parseRequest();
        var contextData = parser.getContextAttrs(requestData.data, requestData.perfData);
        assert(contextData.procs);
        assert.equal(contextData.procs, this.entityData.procs);
    });

});
