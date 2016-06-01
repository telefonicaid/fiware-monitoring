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
 * Module that defines unit tests for `check_mem.sh` parser.
 *
 * @module test_check_mem.sh
 */


'use strict';


var util = require('util'),
    assert = require('assert'),
    common = require('./common');


/* jshint laxbreak: true */
suite('check_mem.sh', function () {

    suiteSetup(function () {
        this.factory = require('../../lib/parsers/common/factory');

        this.baseurl = 'http://hostname:1234/check_mem.sh';
        this.entityId = '1';
        this.entityType = 'host';
        this.entityData = {
            usedMemPct: 19
        };

        this.probeData = {
            usedMemPct: this.entityData.usedMemPct
        };

        this.probeBody = {
            data: 'Memory: OK Total: 1877 MB - Used: 369 MB - ' + this.probeData.usedMemPct + '% used',
            perf: 'TOTAL=1969020928;;;; USED=386584576;;;; CACHE=999440384;;;; BUFFER=201584640;;;;'
        };
    });

    suiteTeardown(function () {
    });

    setup(function () {
        this.request = {
            url: this.baseurl + '?id=' + this.entityId + '&type=' + this.entityType
        };
        this.reqdomain = common.domain(this);
        this.entityData[common.timestampAttrName] = this.reqdomain.timestamp;
    });

    teardown(function () {
        delete this.request;
        delete this.reqdomain;
        delete this.entityData[common.timestampAttrName];
    });

    test('get_update_request_fails_with_invalid_check_mem.sh_content', function () {
        this.reqdomain.body = 'XXX INVALID XXX';
        var self = this;
        var parser = this.factory.getParser(self.request);
        assert.throws(
            function () {
                return parser.getUpdateRequest(self.reqdomain);
            }
        );
    });

    test('get_update_request_ok_with_valid_check_mem.sh_content', function () {
        this.reqdomain.body = util.format('%s|%s', this.probeBody.data, this.probeBody.perf);
        var parser = this.factory.getParser(this.request);
        var update = parser.getUpdateRequest(this.reqdomain);
        common.assertValidUpdateJSON(update, this);
    });

    test('parse_ok_used_mem_percentage', function () {
        this.reqdomain.body = util.format('%s|%s', this.probeBody.data, this.probeBody.perf);
        var parser = this.factory.getParser(this.request);
        var requestData = parser.parseRequest(this.reqdomain);
        var contextData = parser.getContextAttrs(requestData);
        assert(contextData.usedMemPct);
        assert.equal(contextData.usedMemPct, this.entityData.usedMemPct);
    });

});
