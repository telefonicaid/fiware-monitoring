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
 * Module that defines unit tests for the base parser for Nagios probes.
 *
 * @module test_nagios_parser
 */


'use strict';


var util   = require('util'),
    assert = require('assert'),
    parser = require('../../lib/parsers/common/nagios').parser;


/* jshint multistr: true */
suite('nagios_parser', function() {

    suiteSetup(function() {
        this.request = { url: 'http://host:1234/path?query' };
    });

    suiteTeardown(function() {
    });

    setup(function() {
    });

    teardown(function() {
        delete this.request.body;
    });

    test('parse_fails_extra_perf_data_first_line', function() {
        var self = this;
        var data = 'TEXT OUTPUT | OPTIONAL PERFDATA | ANOTHER PERFDATA';
        self.request.body = data;
        assert.throws(
            function() {
                parser.setRequest(self.request);
                parser.parseRequest();
            },
            /Invalid/
        );
    });

    test('parse_fails_extra_perf_data_another_line', function() {
        var self = this;
        var data = 'TEXT OUTPUT | OPTIONAL PERFDATA                         \n\
                    LONG TEXT LINE 1                                        \n\
                    LONG TEXT LINE 2                                        \n\
                    LONG TEXT LINE 3 | PERFDATA LINE 2 | ANOTHER PERFDATA   \n\
                    LONG TEXT LINE 4                                        \n\
                    LONG TEXT LINE 5 | PERFDATA LINE 3                      \n\
                    LONG TEXT LINE 6                                        \
                   ';
        self.request.body = data;
        assert.throws(
            function() {
                parser.setRequest(self.request);
                parser.parseRequest();
            },
            /Invalid/
        );
    });

    test('parse_fails_third_perf_data_compound_line', function() {
        var self = this;
        var data = 'TEXT OUTPUT | OPTIONAL PERFDATA                         \n\
                    LONG TEXT LINE 1                                        \n\
                    LONG TEXT LINE 2                                        \n\
                    LONG TEXT LINE 3 | PERFDATA LINE 2                      \n\
                    LONG TEXT LINE 4                                        \n\
                    LONG TEXT LINE 5 | PERFDATA LINE 3                      \n\
                    LONG TEXT LINE 6                                        \
                   ';
        self.request.body = data;
        assert.throws(
            function() {
                parser.setRequest(self.request);
                parser.parseRequest();
            },
            /Invalid/
        );
    });

    test('parse_ok_singleline_text_output_only', function() {
        var self = this;
        var data = 'TEXT OUTPUT';
        self.request.body = data;
        parser.setRequest(self.request);
        var entityData = parser.parseRequest();
        assert(!entityData.perfData);
        assert.equal(entityData.data, data);
    });

    test('parse_ok_multiline_text_output_only', function() {
        var self = this;
        var data = 'TEXT OUTPUT         \n\
                    LONG TEXT LINE 1    \n\
                    LONG TEXT LINE 2    \
                   ';
        self.request.body = data;
        parser.setRequest(self.request);
        var entityData = parser.parseRequest();
        assert(!entityData.perfData);
        assert(entityData.data.split('\n').length > 1);
        assert.equal(entityData.data, data);
    });

    test('parse_ok_singleline_text_output_singleline_perf_data', function() {
        var self = this;
        var data = 'TEXT OUTPUT';
        var perf = 'OPTIONAL PERFDATA';
        self.request.body = util.format('%s|%s', data, perf);
        parser.setRequest(self.request);
        var entityData = parser.parseRequest();
        assert.equal(entityData.perfData, perf);
        assert.equal(entityData.data, data);
    });

    test('parse_fails_singleline_text_output_multiline_perf_data', function() {
        var self = this;
        var data = 'TEXT OUTPUT';
        var perf = 'OPTIONAL PERFDATA\n| PERFDATA LINE 2';
        self.request.body = util.format('%s|%s', data, perf);
        assert.throws(
            function() {
                parser.setRequest(self.request);
                parser.parseRequest();
            },
            /Invalid/
        );
    });

    test('parse_ok_multiline_text_output_singleline_perf_data', function() {
        var self = this;
        var data = ['TEXT OUTPUT','LONG TEXT LINE 1','LONG TEXT LINE 2'];
        var perf = 'OPTIONAL PERFDATA';
        self.request.body = util.format('%s|%s\n%s\n%s', data[0], perf, data[1], data[2]);
        parser.setRequest(self.request);
        var entityData = parser.parseRequest();
        assert.equal(entityData.perfData, perf);
        assert.deepEqual(entityData.data.split('\n'), data);
    });

    test('parse_ok_multiline_text_output_multiline_perf_data', function() {
        var self = this;
        var data = ['TEXT OUTPUT','LONG TEXT LINE 1','LONG TEXT LINE 2'];
        var perf = ['OPTIONAL PERFDATA','PERFDATA LINE 2','PERFDATA LINE 3'];
        self.request.body = util.format('%s|%s\n%s\n%s|%s\n%s', data[0], perf[0], data[1], data[2], perf[1], perf[2]);
        parser.setRequest(self.request);
        var entityData = parser.parseRequest();
        assert.deepEqual(entityData.perfData.split('\n'), perf);
        assert.deepEqual(entityData.data.split('\n'), data);
    });

    test('get_update_request_fails_missing_entity_id', function() {
        var request = { url: 'http://host:1234/path?type=type&another=another' };
        assert.throws(
            function() {
                parser.setRequest(request);
                return parser.updateContextRequest();
            },
            /entityId/
        );
    });

    test('get_update_request_fails_missing_entity_type', function() {
        var request = { url: 'http://host:1234/path?id=id&another=another' };
        assert.throws(
            function() {
                parser.setRequest(request);
                return parser.updateContextRequest();
            },
            /entityType/
        );
    });

    test('get_update_request_fails_empty_request', function() {
        var request = { url: 'http://host:1234/path?id=id&type=type&another=another' };
        request.body = '';
        assert.throws(
            function() {
                parser.setRequest(request);
                return parser.updateContextRequest();
            }
        );
    });

});
