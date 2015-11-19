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
 * Module that defines unit tests for the configuration options of the adapter.
 *
 * @module test_options
 */


'use strict';


/** Fake command line arguments (required to load `adapter` without complaining) */
process.argv = [];


var util = require('util'),
    http = require('http'),
    dgram = require('dgram'),
    sinon = require('sinon'),
    assert = require('assert'),
    Emitter = require('events').EventEmitter,
    defaults = require('../../lib/common').defaults,
    logger = require('../../lib/logger'),
    adapter = require('../../lib/adapter'),
    opts = require('../../config/options');


/* jshint multistr: true */
suite('options', function () {

    suiteSetup(function () {
        var self = this;
        self.baseurl = 'http://hostname:1234';
        self.resource = 'check_load';
        self.body = 'invalid load data';
        self.headers = {};
        self.availPort = 1234;
        self.boundPort = 4321;
        sinon.stub(http, 'createServer', function () {
            self.httpListener = arguments[0];
            self.serverListen = sinon.spy(function (port, host, callback) {
                this.address = sinon.stub().returns({address: host, port: port});
                callback.call(this);
            });
            return {
                listen: self.serverListen
            };
        });
        sinon.stub(dgram, 'createSocket', function () {
            self.udpServer = new Emitter();
            self.udpServer.bind = sinon.spy(function (port, host, callback) {
                if (port === self.boundPort) {
                    this.emit('error', new Error('bind error'));
                } else {
                    this.address = sinon.stub().returns({address: host, port: port});
                    callback.call(this);
                }
            });
            /* jshint -W072 */
            self.udpServer.send = function (buf, offset, length, port, address, callback) {
                this.emit('message', buf.toString('utf8', offset, length - offset));
                callback.call(this, null, length - offset);
            };
            self.udpServer.close = sinon.stub();
            return self.udpServer;
        });
        logger.stream = require('dev-null')();
        logger.setLevel('DEBUG');
    });

    suiteTeardown(function () {
        http.createServer.restore();
        dgram.createSocket.restore();
    });

    setup(function () {
    });

    teardown(function () {
        delete this.udpServer;
    });

    test('adapter_starts_listening_to_http_requests_at_default_port', function () {
        adapter.main();
        assert(this.serverListen.calledOnce);
        assert(this.serverListen.args[0][0], defaults.listenPort);
    });

    test('adapter_starts_but_logs_warning_message_when_invalid_udp_endpoints', function () {
        var self = this,
            udpEndpointNoParser = 'host:port:',
            optsUdpEndpointsSave = opts.udpEndpoints,
            logWarn = sinon.stub(logger, 'warn', function () {
                self.logWarnErrMsg = util.format.apply(null, arguments);
                logWarn.restore();
            });
        opts.udpEndpoints = udpEndpointNoParser;
        adapter.main();
        opts.udpEndpoints = optsUdpEndpointsSave;
        assert(this.serverListen.calledOnce);
        assert(!this.udpServer);
        assert.notEqual(this.logWarnErrMsg.indexOf('Ignoring UDP endpoint'), -1);
        assert.notEqual(this.logWarnErrMsg.indexOf('missing parser name'), -1);
    });

    test('adapter_starts_listening_to_both_http_and_udp_requests', function () {
        var self = this,
            udpParser = 'parser',
            udpHost = 'localhost',
            udpPort = self.availPort,
            optsUdpEndpointsSave = opts.udpEndpoints;
        opts.udpEndpoints = util.format('%s:%d:%s', udpHost, udpPort, udpParser);
        adapter.main();
        opts.udpEndpoints = optsUdpEndpointsSave;
        assert(this.serverListen.calledOnce);
        assert(this.serverListen.args[0][0], defaults.listenPort);
        assert(this.udpServer.bind.calledOnce);
        assert(this.udpServer.bind.args[0][0], udpPort);
        assert(this.udpServer.bind.args[0][1], udpHost);
    });

    test('adapter_starts_but_logs_error_when_udp_bind_fails', function () {
        var self = this,
            udpParser = 'parser',
            udpHost = 'localhost',
            udpPort = self.boundPort,
            optsUdpEndpointsSave = opts.udpEndpoints;
        opts.udpEndpoints = util.format('%s:%d:%s', udpHost, udpPort, udpParser);
        sinon.spy(logger, 'error');
        adapter.main();
        opts.udpEndpoints = optsUdpEndpointsSave;
        assert(this.serverListen.calledOnce);
        assert(this.serverListen.args[0][0], defaults.listenPort);
        assert(this.udpServer.bind.calledOnce);
        assert(logger.error.calledOnce);
        logger.error.restore();
    });

});
