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
 * Module that defines unit tests for the logger.
 *
 * @module test_logger
 */


'use strict';


/** Fake command line arguments (required to load logger without complaining) */
process.argv = [];


/** Production environment */
process.env.NODE_ENV = 'production';


var uuid = require('node-uuid').v1,
    txid = require('cuid'),
    util = require('util'),
    sinon = require('sinon'),
    assert = require('assert'),
    logger = require('../../lib/logger');


suite('logger', function () {

    suiteSetup(function () {
        this.level = 'INFO';
        logger.setLevel(this.level);
        logger.stream = require('dev-null')();
    });

    suiteTeardown(function () {
    });

    setup(function () {
    });

    teardown(function () {
    });

    test('logger_formatted_message_has_timestamp', function () {
        var context = {},
            message = 'message',
            time = (new Date()).toISOString(),
            copy = Date.prototype.toISOString;
        Date.prototype.toISOString = sinon.stub().returns(time);
        var formatted = logger.format(this.level, context, message, []);
        Date.prototype.toISOString = copy;
        assert(formatted.indexOf(util.format('time=%s |', time)) >= 0);
    });

    test('logger_formatted_message_has_level', function () {
        var context = {},
            message = 'message',
            formatted = logger.format(this.level, context, message, []);
        assert(formatted.indexOf(util.format('lvl=%s |', this.level)) >= 0);
    });

    test('logger_formatted_message_valid_with_no_context', function () {
        var context = {},
            message = 'message',
            formatted = logger.format(this.level, context, message, []);
        assert(formatted.indexOf('corr=n/a |') >= 0);
        assert(formatted.indexOf('trans=n/a |') >= 0);
        assert(formatted.indexOf('op=n/a |') >= 0);
    });

    test('logger_formatted_message_valid_with_context_corr', function () {
        var context = {corr: uuid()},
            message = 'message',
            formatted = logger.format(this.level, context, message, []);
        assert(formatted.indexOf(util.format('corr=%s |', context.corr)) >= 0);
        assert(formatted.indexOf(util.format('trans=%s |', 'n/a')) >= 0);
        assert(formatted.indexOf(util.format('op=%s |', 'n/a')) >= 0);
    });

    test('logger_formatted_message_valid_with_context_trans', function () {
        var context = {trans: txid()},
            message = 'message',
            formatted = logger.format(this.level, context, message, []);
        assert(formatted.indexOf(util.format('corr=%s |', 'n/a')) >= 0);
        assert(formatted.indexOf(util.format('trans=%s |', context.trans)) >= 0);
        assert(formatted.indexOf(util.format('op=%s |', 'n/a')) >= 0);
    });

    test('logger_formatted_message_valid_with_context_op', function () {
        var context = {op: 'opname'},
            message = 'message',
            formatted = logger.format(this.level, context, message, []);
        assert(formatted.indexOf(util.format('corr=%s |', 'n/a')) >= 0);
        assert(formatted.indexOf(util.format('trans=%s |', 'n/a')) >= 0);
        assert(formatted.indexOf(util.format('op=%s |', context.op)) >= 0);
    });

    test('logger_formatted_message_valid_with_full_context', function () {
        var context = {corr: uuid(), trans: txid(), op: 'opname'},
            message = 'message',
            formatted = logger.format(this.level, context, message, []);
        assert(formatted.indexOf(util.format('corr=%s |', context.corr)) >= 0);
        assert(formatted.indexOf(util.format('trans=%s |', context.trans)) >= 0);
        assert(formatted.indexOf(util.format('op=%s |', context.op)) >= 0);
    });

    test('logger_formatted_message_ends_with_message', function () {
        var context = {},
            message = 'message',
            formatted = logger.format(this.level, context, message, []);
        assert(formatted.indexOf(util.format('| msg=%s', message)) >= 0);
    });

});
