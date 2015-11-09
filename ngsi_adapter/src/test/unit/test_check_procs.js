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
 * Module that defines unit tests for `check_procs` parser.
 *
 * @module test_check_procs
 */


'use strict';


var util = require('util'),
    assert = require('assert'),
    common = require('./common');


/* jshint laxbreak: true */
suite('check_procs', function () {

    suiteSetup(function () {
        this.timestampName = require('../../lib/parsers/common/base').parser.timestampAttrName;
        this.factory = require('../../lib/parsers/common/factory');

        this.baseurl = 'http://hostname:1234/check_procs';
        this.entityId = '1';
        this.entityType = 'host';
        this.entityData = {
            procs: 136
        };

        this.probeData = {
            procs: this.entityData.procs
        };

        this.probeBody = {
            data: util.format('PROCS OK: %d processes', this.probeData.procs)
        };
    });

    suiteTeardown(function () {
    });

    setup(function () {
        this.request = {
            url: this.baseurl + '?id=' + this.entityId + '&type=' + this.entityType
        };
        this.reqdomain = {
            timestamp: Date.now(),
            entityId: this.entityId,
            entityType: this.entityType
        };
        this.entityData[this.timestampName] = this.reqdomain.timestamp;
    });

    teardown(function () {
        delete this.request;
        delete this.reqdomain;
        delete this.entityData[this.timestampName];
    });

    test('get_update_request_fails_with_invalid_check_procs_content', function () {
        this.reqdomain.body = 'XXX INVALID XXX';
        var self = this;
        var parser = this.factory.getParser(self.request);
        assert.throws(
            function () {
                return parser.updateContextRequest(self.reqdomain);
            }
        );
    });

    test('get_update_request_ok_with_valid_check_procs_content', function () {
        this.reqdomain.body = util.format('%s', this.probeBody.data);
        var parser = this.factory.getParser(this.request);
        var update = parser.updateContextRequest(this.reqdomain);
        common.assertValidUpdateXML(update, this);
    });

    test('get_update_request_ok_with_another_threshold_metric', function () {
        this.reqdomain.body = util.format('%s', this.probeBody.data).replace(/^PROCS/, 'VSZ');
        var parser = this.factory.getParser(this.request);
        var update = parser.updateContextRequest(this.reqdomain);
        common.assertValidUpdateXML(update, this);
    });

    test('parse_ok_number_of_procs', function () {
        this.reqdomain.body = util.format('%s', this.probeBody.data);
        var parser = this.factory.getParser(this.request);
        var requestData = parser.parseRequest(this.reqdomain.body);
        var contextData = parser.getContextAttrs(requestData);
        assert(contextData.procs);
        assert.equal(contextData.procs, this.entityData.procs);
    });

});
