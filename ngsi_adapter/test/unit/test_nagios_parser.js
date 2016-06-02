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
 * Module that defines unit tests for the base parser for Nagios probes.
 *
 * @module test_nagios_parser
 */


'use strict';


var util = require('util'),
    assert = require('assert'),
    parser = require('../../lib/parsers/common/nagios').parser;


suite('nagios_parser', function () {

    suiteSetup(function () {
    });

    suiteTeardown(function () {
    });

    setup(function () {
    });

    teardown(function () {
    });

    test('parse_fails_extra_perf_data_first_line', function () {
        var reqdomain = {
            body: 'TEXT DATA OUTPUT | OPTIONAL PERFDATA | ANOTHER PERFDATA'
        };
        assert.throws(
            function () {
                parser.parseRequest(reqdomain);
            },
            /Invalid/
        );
    });

    test('parse_fails_extra_perf_data_another_line', function () {
        var reqdomain = {
            body: 'TEXT DATA OUTPUT | OPTIONAL PERFDATA                    \n' +
                  'LONG TEXT LINE 1                                        \n' +
                  'LONG TEXT LINE 2                                        \n' +
                  'LONG TEXT LINE 3 | PERFDATA LINE 2 | ANOTHER PERFDATA   \n' +
                  'LONG TEXT LINE 4                                        \n' +
                  'LONG TEXT LINE 5 | PERFDATA LINE 3                      \n' +
                  'LONG TEXT LINE 6'
        };
        assert.throws(
            function () {
                parser.parseRequest(reqdomain);
            },
            /Invalid/
        );
    });

    test('parse_fails_third_perf_data_compound_line', function () {
        var reqdomain = {
            body: 'TEXT OUTPUT | OPTIONAL PERFDATA                         \n' +
                  'LONG TEXT LINE 1                                        \n' +
                  'LONG TEXT LINE 2                                        \n' +
                  'LONG TEXT LINE 3 | PERFDATA LINE 2                      \n' +
                  'LONG TEXT LINE 4                                        \n' +
                  'LONG TEXT LINE 5 | PERFDATA LINE 3                      \n' +
                  'LONG TEXT LINE 6'
        };
        assert.throws(
            function () {
                parser.parseRequest(reqdomain);
            },
            /Invalid/
        );
    });

    test('parse_ok_singleline_text_output_only', function () {
        var data = 'TEXT OUTPUT',
            reqdomain = {
                body: data
            },
            entityData = parser.parseRequest(reqdomain);
        assert(!entityData.perfData);
        assert.equal(entityData.data, data);
    });

    test('parse_ok_multiline_text_output_only', function () {
        var data = 'TEXT OUTPUT      \n' +
                   'LONG TEXT LINE 1 \n' +
                   'LONG TEXT LINE 2',
            reqdomain = {
                body: data
            },
            entityData = parser.parseRequest(reqdomain);
        assert(!entityData.perfData);
        assert(entityData.data.split('\n').length > 1);
        assert.equal(entityData.data, data);
    });

    test('parse_ok_singleline_text_output_singleline_perf_data', function () {
        var data = 'TEXT OUTPUT',
            perf = 'OPTIONAL PERFDATA',
            reqdomain = {
                body: util.format('%s|%s', data, perf)
            },
            entityData = parser.parseRequest(reqdomain);
        assert.equal(entityData.perfData, perf);
        assert.equal(entityData.data, data);
    });

    test('parse_fails_singleline_text_output_multiline_perf_data', function () {
        var data = 'TEXT OUTPUT',
            perf = 'OPTIONAL PERFDATA\n| PERFDATA LINE 2',
            reqdomain = {
                body: util.format('%s|%s', data, perf)
            };
        assert.throws(
            function () {
                parser.parseRequest(reqdomain);
            },
            /Invalid/
        );
    });

    test('parse_ok_multiline_text_output_singleline_perf_data', function () {
        var data = ['TEXT OUTPUT', 'LONG TEXT LINE 1', 'LONG TEXT LINE 2'],
            perf = 'OPTIONAL PERFDATA',
            reqdomain = {
                body: util.format('%s|%s\n%s\n%s', data[0], perf, data[1], data[2])
            },
            entityData = parser.parseRequest(reqdomain);
        assert.equal(entityData.perfData, perf);
        assert.deepEqual(entityData.data.split('\n'), data);
    });

    test('parse_ok_multiline_text_output_multiline_perf_data', function () {
        var data = ['TEXT OUTPUT', 'LONG TEXT LINE 1', 'LONG TEXT LINE 2'],
            perf = ['OPTIONAL PERFDATA', 'PERFDATA LINE 2', 'PERFDATA LINE 3'],
            reqdomain = {
                body: util.format('%s|%s\n%s\n%s|%s\n%s', data[0], perf[0], data[1], data[2], perf[1], perf[2])
            };
        var entityData = parser.parseRequest(reqdomain);
        assert.deepEqual(entityData.perfData.split('\n'), perf);
        assert.deepEqual(entityData.data.split('\n'), data);
    });

});
