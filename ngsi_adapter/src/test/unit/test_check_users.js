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
 * Module that defines unit tests for `check_users` parser.
 *
 * @module test_check_users
 */


'use strict';


var util   = require('util'),
    assert = require('assert'),
    common = require('./common');


/* jshint laxbreak: true */
suite('check_users', function() {

    suiteSetup(function() {
        this.factory    = require('../../lib/parsers/common/factory');
        this.baseurl    = 'http://host:1234/check_users';
        this.entityId   = '1';
        this.entityType = 'server';
        this.entityData = {
            users: 3
        };
        this.probeData  = {
            users: this.entityData.users
        };
        this.probeBody  = {
            data: util.format('USERS OK - %d users currently logged in ', this.probeData.users),
            perf: util.format('users=%d;10;15;0', this.probeData.users)
        };
    });

    suiteTeardown(function() {
    });

    setup(function() {
    });

    teardown(function() {
    });

    test('get_update_request_fails_with_invalid_check_users_content', function() {
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

    test('get_update_request_ok_with_valid_check_users_content', function() {
        var request = {
            url:  this.baseurl + '?id=' + this.entityId + '&type=' + this.entityType,
            body: util.format('%s|%s', this.probeBody.data, this.probeBody.perf)
        };
        var parser = this.factory.getParser(request);
        var update = parser.updateContextRequest();
        common.assertValidUpdateXML(update, this);
    });

    test('parse_ok_number_of_users_logged_in', function() {
        var request = {
            url:  this.baseurl + '?id=' + this.entityId + '&type=' + this.entityType,
            body: util.format('%s|%s', this.probeBody.data, this.probeBody.perf)
        };
        var parser = this.factory.getParser(request);
        var requestData = parser.parseRequest();
        var contextData = parser.getContextAttrs(requestData.data, requestData.perfData);
        assert(contextData.users);
        assert.equal(contextData.users, this.entityData.users);
    });

});
