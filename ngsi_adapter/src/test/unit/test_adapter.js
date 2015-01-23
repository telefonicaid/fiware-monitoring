/*
 * Copyright 2015 Telefónica I+D
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
 * Module that defines unit tests for the adapter.
 *
 * @module test_adapter
 */


'use strict';


/** Fake command line arguments (required to load `adapter` without complaining) */
process.argv = [];


var http = require('http'),
    sinon = require('sinon'),
    assert = require('assert'),
    Emitter = require('events').EventEmitter,
    factory = require('../../lib/parsers/common/factory'),
    parser = require('../../lib/parsers/common/base').parser,
    logger = require('../../lib/logger'),
    adapter = require('../../lib/adapter'),
    opts = require('../../config/options');


/* jshint multistr: true */
suite('adapter', function() {

    suiteSetup(function() {
        var self = this;
        self.baseurl = 'http://hostname:1234';
        self.resource = 'check_load';
        self.body = 'invalid load data';
        self.headers = {};
        sinon.stub(http, 'createServer', function() {
            self.listener = arguments[0];
            return {
                listen: function(port, host, callback) {
                    this.address = sinon.stub().returns({ address: host, port: port });
                    callback.call(this);
                }
            };
        });
        logger.stream = require('dev-null')();
        logger.setLevel('DEBUG');
    });

    suiteTeardown(function() {
        http.createServer.restore();
    });

    setup(function() {
        adapter.main();
        this.request = new Emitter();
        this.request.timestamp = Date.now();
        this.request.headers = this.headers;
        this.request.method = 'POST';
    });

    teardown(function() {
        delete this.request;
    });

    test('request_fails_if_not_post_method', function() {
        var self = this;
        var response = {
            writeHead: sinon.stub(),
            end: sinon.stub()
        };
        self.request.url = self.baseurl;
        self.request.method = 'GET';
        self.listener(self.request, response);
        assert(response.writeHead.calledOnce);
        assert.equal(response.writeHead.args[0][0], 405);  // not allowed
    });

    test('request_fails_missing_entity_id', function() {
        var self = this;
        var response = {
            writeHead: sinon.stub(),
            end: sinon.stub()
        };
        self.request.url = self.baseurl + '?type=type';
        self.listener(self.request, response);
        assert(response.writeHead.calledOnce);
        assert.equal(response.writeHead.args[0][0], 400);  // bad request
    });

    test('request_fails_missing_entity_type', function() {
        var self = this;
        var response = {
            writeHead: sinon.stub(),
            end: sinon.stub()
        };
        self.request.url = self.baseurl + '?id=id';
        self.listener(self.request, response);
        assert(response.writeHead.calledOnce);
        assert.equal(response.writeHead.args[0][0], 400);  // bad request
    });

    test('request_fails_unknown_url_resource', function() {
        var self = this;
        var response = {
            writeHead: sinon.stub(),
            end: sinon.stub()
        };
        self.request.url = self.baseurl + '/invalid_resource' + '?id=id&type=type';
        self.listener(self.request, response);
        assert(response.writeHead.calledOnce);
        assert.equal(response.writeHead.args[0][0], 404);  // not found
    });

    test('request_ok_valid_url_resource', function() {
        var self = this;
        var response = {
            writeHead: sinon.stub(),
            end: sinon.stub()
        };
        self.request.url = self.baseurl + '/' + self.resource + '?id=id&type=type';
        self.listener(self.request, response);
        assert(response.writeHead.calledOnce);
        assert.equal(response.writeHead.args[0][0], 200);  // ok
    });

    test('request_asynchronous_callback_error_on_invalid_resource_data', function(done) {
        var self = this;
        var response = {
            writeHead: sinon.stub(),
            end: sinon.stub()
        };
        var callback = (function() {
            var original = adapter.requestCallback;
            return sinon.stub(adapter, 'requestCallback', function() {
                original.apply(null, arguments);
                callback.restore();
                assert(callback.calledOnce);
                var err = callback.args[0][0];
                assert.notEqual(err, null);
                done();
            });
        }());
        self.timeout(500);
        self.request.url = self.baseurl + '/' + self.resource + '?id=id&type=type';
        self.listener(self.request, response);
        self.request.emit('data', self.body);
        self.request.emit('end');
    });

    test('request_asynchronous_callback_error_after_all_retries', function(done) {
        var self = this;
        var response = {
            writeHead: sinon.stub(),
            end: sinon.stub()
        };
        var factoryGetParser = sinon.stub(factory, 'getParser', function() {
            var mockParser = Object.create(parser);
            mockParser.updateContextRequest = sinon.stub().returns('');
            return mockParser;
        });
        var httpRequest = sinon.stub(http, 'request', function() {
            var clientRequest = new Emitter();
            clientRequest.end = function() { this.emit('error', new Error()); };
            return clientRequest;
        });
        var callback = (function() {
            var original = adapter.requestCallback;
            return sinon.stub(adapter, 'requestCallback', function() {
                original.apply(null, arguments);
                callback.restore();
                httpRequest.restore();
                factoryGetParser.restore();
                assert(callback.calledOnce);
                var err = callback.args[0][0];
                assert.notEqual(err, null);
                done();
            });
        }());
        opts.retries = 1;
        self.timeout(1500);
        self.request.url = self.baseurl + '/' + self.resource + '?id=id&type=type';
        self.listener(self.request, response);
        self.request.emit('data', self.body);
        self.request.emit('end');
    });

    test('request_asynchronous_callback_ok_with_valid_resource_data', function(done) {
        var self = this;
        var response = {
            writeHead: sinon.stub(),
            end: sinon.stub()
        };
        var factoryGetParser = sinon.stub(factory, 'getParser', function() {
            var mockParser = Object.create(parser);
            mockParser.updateContextRequest = sinon.stub().returns('');
            return mockParser;
        });
        var httpRequest = sinon.stub(http, 'request', function(opts, callback) {
            var clientRequest = new Emitter();
            var serverResponse = new Emitter();
            serverResponse.setEncoding = sinon.stub();
            serverResponse.statusCode = 200;
            callback(serverResponse);
            serverResponse.emit('data');
            serverResponse.emit('end');
            clientRequest.end = sinon.stub();
            return clientRequest;
        });
        var callback = (function() {
            var original = adapter.requestCallback;
            return sinon.stub(adapter, 'requestCallback', function() {
                original.apply(null, arguments);
                callback.restore();
                httpRequest.restore();
                factoryGetParser.restore();
                assert(callback.calledOnce);
                var err = callback.args[0][0];
                var status = callback.args[0][1];
                assert.equal(err, null);
                assert.equal(status, 200);
                done();
            });
        }());
        self.timeout(500);
        self.request.url = self.baseurl + '/' + self.resource + '?id=id&type=type';
        self.listener(self.request, response);
        self.request.emit('data', self.body);
        self.request.emit('end');
    });

});
