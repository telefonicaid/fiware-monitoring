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
 * Module that defines unit tests for the base parser.
 *
 * @module test_base_parser
 */


'use strict';


var sinon  = require('sinon'),
    assert = require('assert'),
    common = require('./common'),
    parser = require('../../lib/parsers/common/base').parser;


/* jshint multistr: true */
suite('base_parser', function() {

    suiteSetup(function() {
        this.baseurl    = 'http://hostname:1234/path';
        this.entityId   = '1';
        this.entityType = 'host';
        this.entityData = {
            attr: 4321
        };
    });

    suiteTeardown(function() {
    });

    setup(function() {
        this.request = {
            url: this.baseurl + '?id=' + this.entityId + '&type=' + this.entityType,
            timestamp: Date.now()
        };
        this.entityData[parser.timestampAttrName] = this.request.timestamp;
        this.parseRequestFunction = parser.parseRequest;
        this.getContextAttrsFunction = parser.getContextAttrs;
    });

    teardown(function() {
        delete this.request;
        delete this.entityData[parser.timestampAttrName];
        parser.parseRequest = this.parseRequestFunction;
        parser.getContextAttrs = this.getContextAttrsFunction;
    });

    test('get_update_request_fails_unimplemented_method_parse_request', function() {
        var self = this;
        assert.throws(
            function() {
                return parser.updateContextRequest(self.request);
            },
            /implement/
        );
    });

    test('get_update_request_fails_unimplemented_method_get_context_attrs', function() {
        var self = this;
        parser.parseRequest = sinon.spy(function() { return {}; });
        assert.throws(
            function() {
                return parser.updateContextRequest(self.request);
            },
            /implement/
        );
    });

    test('get_update_request_fails_missing_entity_id', function() {
        var self = this;
        parser.parseRequest = sinon.spy(function() { return {}; });
        parser.getContextAttrs = sinon.spy(function() { return {}; });
        self.request.url = self.baseurl + '?type=type&another=another';
        assert.throws(
            function() {
                return parser.updateContextRequest(self.request);
            },
            /entityId/
        );
    });

    test('get_update_request_fails_missing_entity_type', function() {
        var self = this;
        parser.parseRequest = sinon.spy(function() { return {}; });
        parser.getContextAttrs = sinon.spy(function() { return {}; });
        self.request.url = self.baseurl + '?id=id&another=another';
        assert.throws(
            function() {
                return parser.updateContextRequest(self.request);
            },
            /entityType/
        );
    });

    test('get_update_request_fails_missing_entity_attributes', function() {
        var self = this;
        parser.parseRequest = sinon.spy(function() { return {}; });
        parser.getContextAttrs = sinon.spy(function() { return {}; });
        self.request.url = self.baseurl + '?id=id&type=type&another=another';
        self.request.body = '';
        assert.throws(
            function() {
                return parser.updateContextRequest(self.request);
            }
        );
    });

    test('get_update_request_ok_with_timestamp_added', function() {
        var self = this;
        parser.parseRequest = sinon.spy(function() { return {}; });
        parser.getContextAttrs = sinon.spy(function() { return self.entityData; });
        var update = parser.updateContextRequest(self.request);
        common.assertValidUpdateXML(update, self);
    });

});
