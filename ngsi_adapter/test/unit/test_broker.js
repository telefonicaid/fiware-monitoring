/*
 * Copyright 2016 Telefónica I+D
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
 * Module that defines unit tests related to requests to Context Broker.
 *
 * @module test_broker
 */


'use strict';
/* jshint -W098,-W069 */


/** Fake command line arguments (required to load `adapter` without complaining) */
process.argv = [];


var util = require('util'),
    http = require('http'),
    sinon = require('sinon'),
    assert = require('assert'),
    Emitter = require('events').EventEmitter,
    factory = require('../../lib/parsers/common/factory'),
    parser = require('../../lib/parsers/common/base').parser,
    logger = require('../../lib/logger'),
    config = require('../../lib/config'),
    common = require('../../lib/common'),
    adapter = require('../../lib/adapter');


/* jshint multistr: true */
suite('broker', function () {

    suiteSetup(function () {
        this.contentType = parser.getContentType();
        this.processEvents = ['SIGINT', 'SIGTERM', 'uncaughtException', 'exit'];
        this.brokerHost = '127.0.0.1';
        this.brokerPort = 1026;
        this.brokerStatusCodeV1 = 200;
        this.brokerStatusCodeV2 = 204;
        this.baseurl = 'http://hostname:1234';
        this.resource = 'check_load';
        this.body = 'some load data';
        this.attrs = {'attr': 'value'};
        this.headers = {'Content-Type': this.contentType, 'Accept': this.contentType};
        logger.stream = require('dev-null')();
        logger.setLevel('DEBUG');
    });

    suiteTeardown(function () {
    });

    setup(function () {
        var self = this;
        sinon.stub(http, 'createServer', function () {
            self.httpListener = arguments[0];
            return {
                listen: function (port, host, callback) {
                    this.address = sinon.stub().returns({ address: host, port: port });
                    callback.call(this);
                }
            };
        });
        self.request = new Emitter();
        self.request.method = 'POST';
        self.request.headers = {};
    });

    teardown(function () {
        http.createServer.restore();
        this.processEvents.map(function (event) { process.removeListener(event, process.listeners(event).pop()); });
        delete this.request;
        delete this.reqdomain;
        delete this.httpListener;
        delete config.brokerApi;
    });

    test('v0_api_valid_request', function (done) {
        var self = this;
        var response = {
            writeHead: sinon.stub(),
            end: sinon.stub()
        };
        var factoryGetParser = sinon.stub(factory, 'getParser', function () {
            var mockParser = Object.create(parser);
            mockParser.parseRequest = function (reqdomain) {
                self.reqdomain = reqdomain;
            };
            mockParser.getContextAttrs = function (data) {
                return self.attrs;
            };
            return mockParser;
        });
        var httpRequest = sinon.stub(http, 'request', function (opts, callback) {
            httpRequest.restore();
            factoryGetParser.restore();
            loggerError.restore();
            assert.equal(opts.port, self.brokerPort);
            assert.equal(opts.hostname, self.brokerHost);
            assert.notEqual(opts.path.indexOf(common.BROKER_API_V0), -1);
            assert.notEqual(opts.path.indexOf('/updateContext'), -1);
            done();
        });
        var loggerError = sinon.spy(logger, 'error');
        config.brokerUrl = util.format('http://%s:%d', self.brokerHost, self.brokerPort);
        adapter.main();
        self.timeout(500);
        self.request.url = self.baseurl + '/' + self.resource + '?id=id&type=type';
        self.httpListener(self.request, response);
        self.request.emit('data', self.body);
        self.request.emit('end');
    });

    test('v1_api_valid_request', function (done) {
        var self = this;
        var response = {
            writeHead: sinon.stub(),
            end: sinon.stub()
        };
        var factoryGetParser = sinon.stub(factory, 'getParser', function () {
            var mockParser = Object.create(parser);
            mockParser.parseRequest = function (reqdomain) {
                self.reqdomain = reqdomain;
            };
            mockParser.getContextAttrs = function (data) {
                return self.attrs;
            };
            return mockParser;
        });
        var httpRequest = sinon.stub(http, 'request', function (opts, callback) {
            httpRequest.restore();
            factoryGetParser.restore();
            loggerError.restore();
            assert.equal(opts.port, self.brokerPort);
            assert.equal(opts.hostname, self.brokerHost);
            assert.notEqual(opts.path.indexOf(common.BROKER_API_V1), -1);
            assert.notEqual(opts.path.indexOf('/updateContext'), -1);
            done();
        });
        var loggerError = sinon.spy(logger, 'error');
        config.brokerUrl = util.format('http://%s:%d/v1', self.brokerHost, self.brokerPort);
        adapter.main();
        self.timeout(500);
        self.request.url = self.baseurl + '/' + self.resource + '?id=id&type=type';
        self.httpListener(self.request, response);
        self.request.emit('data', self.body);
        self.request.emit('end');
    });

    test('v1_api_valid_status_code', function (done) {
        var self = this;
        var response = {
            writeHead: sinon.stub(),
            end: sinon.stub()
        };
        var factoryGetParser = sinon.stub(factory, 'getParser', function () {
            var mockParser = Object.create(parser);
            mockParser.getUpdateRequest = function (reqdomain) {
                reqdomain.options = {headers: self.headers};
                return '';
            };
            return mockParser;
        });
        var httpRequest = sinon.stub(http, 'request', function (opts, callback) {
            var clientRequest = new Emitter();
            var serverResponse = new Emitter();
            serverResponse.setEncoding = sinon.stub();
            serverResponse.statusCode = self.brokerStatusCodeV1;
            callback(serverResponse);
            serverResponse.emit('data', '{"key": "value"}');
            serverResponse.emit('end');
            clientRequest.end = sinon.stub();
            return clientRequest;
        });
        var loggerError = sinon.spy(logger, 'error');
        var callback = (function () {
            var original = adapter.updateContextCallback;
            return sinon.stub(adapter, 'updateContextCallback', function () {
                original.apply(null, arguments);
                callback.restore();
                httpRequest.restore();
                factoryGetParser.restore();
                loggerError.restore();
                assert(callback.calledOnce);
                var err = callback.args[0][0];
                var status = callback.args[0][1];
                var responseBody = callback.args[0][2];
                var responseType = callback.args[0][3];
                assert.equal(err, null);
                assert.equal(status, self.brokerStatusCodeV1);
                assert.equal(responseType, self.headers['Accept']);
                assert(responseBody);
                assert(loggerError.notCalled);
                done();
            });
        }());
        config.brokerUrl = util.format('http://%s:%d/v1', self.brokerHost, self.brokerPort);
        adapter.main();
        self.timeout(500);
        self.request.url = self.baseurl + '/' + self.resource + '?id=id&type=type';
        self.httpListener(self.request, response);
        self.request.emit('data', self.body);
        self.request.emit('end');
    });

    test('v2_api_valid_request', function (done) {
        var self = this;
        var response = {
            writeHead: sinon.stub(),
            end: sinon.stub()
        };
        var factoryGetParser = sinon.stub(factory, 'getParser', function () {
            var mockParser = Object.create(parser);
            mockParser.parseRequest = function (reqdomain) {
                self.reqdomain = reqdomain;
            };
            mockParser.getContextAttrs = function (data) {
                return self.attrs;
            };
            return mockParser;
        });
        var httpRequest = sinon.stub(http, 'request', function (opts, callback) {
            httpRequest.restore();
            factoryGetParser.restore();
            loggerError.restore();
            assert.equal(opts.port, self.brokerPort);
            assert.equal(opts.hostname, self.brokerHost);
            assert.notEqual(opts.path.indexOf(common.BROKER_API_V2), -1);
            assert.notEqual(opts.path.indexOf('/entities/' + self.reqdomain.entityId), -1);
            assert.notEqual(opts.path.indexOf('type=' + self.reqdomain.entityType), -1);
            done();
        });
        var loggerError = sinon.spy(logger, 'error');
        config.brokerUrl = util.format('http://%s:%d/v2', self.brokerHost, self.brokerPort);
        adapter.main();
        self.timeout(500);
        self.request.url = self.baseurl + '/' + self.resource + '?id=id&type=type';
        self.httpListener(self.request, response);
        self.request.emit('data', self.body);
        self.request.emit('end');
    });

    test('v2_api_valid_status_code', function (done) {
        var self = this;
        var response = {
            writeHead: sinon.stub(),
            end: sinon.stub()
        };
        var factoryGetParser = sinon.stub(factory, 'getParser', function () {
            var mockParser = Object.create(parser);
            mockParser.getUpdateRequest = function (reqdomain) {
                reqdomain.options = {headers: self.headers};
                return '';
            };
            return mockParser;
        });
        var httpRequest = sinon.stub(http, 'request', function (opts, callback) {
            var clientRequest = new Emitter();
            var serverResponse = new Emitter();
            serverResponse.setEncoding = sinon.stub();
            serverResponse.statusCode = self.brokerStatusCodeV2;
            callback(serverResponse);
            //serverResponse.emit('data', '{"key": "value"}');
            serverResponse.emit('end');
            clientRequest.end = sinon.stub();
            return clientRequest;
        });
        var loggerError = sinon.spy(logger, 'error');
        var callback = (function () {
            var original = adapter.updateContextCallback;
            return sinon.stub(adapter, 'updateContextCallback', function () {
                original.apply(null, arguments);
                callback.restore();
                httpRequest.restore();
                factoryGetParser.restore();
                loggerError.restore();
                assert(callback.calledOnce);
                var err = callback.args[0][0];
                var status = callback.args[0][1];
                var responseBody = callback.args[0][2];
                assert.equal(err, null);
                assert.equal(status, self.brokerStatusCodeV2);
                assert.equal(responseBody, '');
                assert(loggerError.notCalled);
                done();
            });
        }());
        config.brokerUrl = util.format('http://%s:%d/v2', self.brokerHost, self.brokerPort);
        adapter.main();
        self.timeout(500);
        self.request.url = self.baseurl + '/' + self.resource + '?id=id&type=type';
        self.httpListener(self.request, response);
        self.request.emit('data', self.body);
        self.request.emit('end');
    });

});
