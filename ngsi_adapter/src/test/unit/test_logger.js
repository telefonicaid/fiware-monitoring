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


var util = require('util'),
    cuid = require('cuid'),
    sinon = require('sinon'),
    assert = require('assert'),
    logger = require('../../lib/logger');


/* jshint multistr: true */
suite('logger', function() {

    suiteSetup(function() {
        this.level = 'info';
        logger.setLevel(this.level);
        logger.stream = require('dev-null');
    });

    suiteTeardown(function() {
    });

    setup(function() {
    });

    teardown(function() {
    });

    test('logger_formatted_message_has_timestamp', function() {
        var context = {};
        var message = 'message';
        var time = (new Date()).toISOString();
        var copy = Date.prototype.toISOString;
        Date.prototype.toISOString = sinon.stub().returns(time);
        var formatted = logger.format(this.level, context, message, []);
        Date.prototype.toISOString = copy;
        assert(formatted.indexOf(util.format('time=%s |', time)) >= 0);
    });

    test('logger_formatted_message_has_level', function() {
        var context = {};
        var message = 'message';
        var formatted = logger.format(this.level, context, message, []);
        assert(formatted.indexOf(util.format('| lvl=%s |', this.level)) >= 0);
    });

    test('logger_formatted_message_valid_with_no_context', function() {
        var context = {};
        var message = 'message';
        var formatted = logger.format(this.level, context, message, []);
        assert(formatted.indexOf('| trans=n/a | op=n/a |') >= 0);
    });

    test('logger_formatted_message_valid_with_tx_id', function() {
        var txid = cuid();
        var context = { trans: txid };
        var message = 'message';
        var formatted = logger.format(this.level, context, message, []);
        assert(formatted.indexOf(util.format('| trans=%s | op=n/a |', txid)) >= 0);
    });

    test('logger_formatted_message_valid_with_op_id', function() {
        var opid = 'noop';
        var context = { op: opid };
        var message = 'message';
        var formatted = logger.format(this.level, context, message, []);
        assert(formatted.indexOf(util.format('| trans=n/a | op=%s |', opid)) >= 0);
    });

    test('logger_formatted_message_valid_with_full_context', function() {
        var txid = cuid();
        var opid = 'noop';
        var context = { trans: txid, op: opid };
        var message = 'message';
        var formatted = logger.format(this.level, context, message, []);
        assert(formatted.indexOf(util.format('| trans=%s | op=%s |', txid, opid)) >= 0);
    });

    test('logger_formatted_message_has_message', function() {
        var context = {};
        var message = 'message';
        var formatted = logger.format(this.level, context, message, []);
        assert(formatted.indexOf(util.format('| msg=%s', message)) >= 0);
    });

});
