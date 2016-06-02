/*
s * Copyright 2015-2016 Telefónica I+D
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
 * @module test_config
 */


'use strict';


/** Command line with duplicated options */
process.argv = [ '--retries=9999', '--retries=1' ];


var util = require('util'),
    http = require('http'),
    dgram = require('dgram'),
    sinon = require('sinon'),
    assert = require('assert'),
    Emitter = require('events').EventEmitter,
    common = require('../../lib/common'),
    logger = require('../../lib/logger'),
    config = require('../../lib/config'),
    adapter = require('../../lib/adapter'),
    defaults = common.defaults;


/* jshint multistr: true */
suite('config', function () {

    suiteSetup(function () {
        this.baseurl = 'http://hostname:1234';
        this.resource = 'check_load';
        this.body = 'invalid load data';
        this.headers = {};
        this.availPort = 1234;
        this.boundPort = 4321;
        logger.stream = require('dev-null')();
        logger.setLevel('DEBUG');
    });

    suiteTeardown(function () {
    });

    setup(function () {
        var self = this;
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
    });

    teardown(function () {
        http.createServer.restore();
        dgram.createSocket.restore();
        delete this.udpServer;
        delete config.brokerApi;
    });

    test('config_takes_last_value_of_duplicated_command_line_options', function () {
        config.check(sinon.stub());
        assert.notEqual(config.retries, 9999);
    });

    test('config_assumes_broker_api_v0_by_default', function () {
        config.brokerUrl = 'http://localhost:1234';
        config.check(sinon.stub());
        assert.equal(config.brokerApi, common.BROKER_API_V0);
    });

    test('config_assumes_broker_api_v0_by_default_url_with_slash', function () {
        config.brokerUrl = 'http://localhost:1234/';
        config.check(sinon.stub());
        assert.equal(config.brokerApi, common.BROKER_API_V0);
    });

    test('config_detects_broker_api_v0_from_given_url_lowercase', function () {
        config.brokerUrl = 'http://localhost:1234/ngsi10/';
        config.check(sinon.stub());
        assert.equal(config.brokerApi, common.BROKER_API_V0);
    });

    test('config_detects_broker_api_v0_from_given_url_uppercase', function () {
        config.brokerUrl = 'http://localhost:1234/NGSI10/';
        config.check(sinon.stub());
        assert.equal(config.brokerApi, common.BROKER_API_V0);
    });

    test('config_detects_broker_api_v1_from_given_url', function () {
        config.brokerUrl = 'http://localhost:1234/v1';
        config.check(sinon.stub());
        assert.equal(config.brokerApi, common.BROKER_API_V1);
    });

    test('config_detects_broker_api_v1_from_given_url_with_slash', function () {
        config.brokerUrl = 'http://localhost:1234/v1/';
        config.check(sinon.stub());
        assert.equal(config.brokerApi, common.BROKER_API_V1);
    });

    test('config_detects_broker_api_v2_from_given_url', function () {
        config.brokerUrl = 'http://localhost:1234/v2';
        config.check(sinon.stub());
        assert.equal(config.brokerApi, common.BROKER_API_V2);
    });

    test('config_detects_broker_api_v2_from_given_url_with_slash', function () {
        config.brokerUrl = 'http://localhost:1234/v2/';
        config.check(sinon.stub());
        assert.equal(config.brokerApi, common.BROKER_API_V2);
    });

    test('adapter_shows_help_message_when_required_at_command_line', function (done) {
        config.help = true;
        sinon.stub(console, 'error');
        sinon.stub(process, 'exit', function (code) {
            config.help = false;
            assert(console.error.calledOnce);
            assert.notEqual(code, 0);
            console.error.restore();
            process.exit.restore();
            done();
        });
        adapter.main();
    });

    test('adapter_starts_listening_to_http_requests_at_default_port', function () {
        adapter.main();
        assert(this.serverListen.calledOnce);
        assert(this.serverListen.args[0][0], defaults.listenPort);
    });

    test('adapter_starts_but_logs_warning_message_when_invalid_udp_endpoints', function () {
        var self = this,
            udpEndpointNoParser = 'host:port:',
            configUdpEndpointsSave = config.udpEndpoints,
            logWarn = sinon.stub(logger, 'warn', function () {
                self.logWarnErrMsg = util.format.apply(null, arguments);
                logWarn.restore();
            });
        config.udpEndpoints = udpEndpointNoParser;
        adapter.main();
        config.udpEndpoints = configUdpEndpointsSave;
        assert(this.serverListen.calledOnce);
        assert(!this.udpServer);
        assert.notEqual(this.logWarnErrMsg.indexOf('Ignoring UDP endpoint'), -1);
        assert.notEqual(this.logWarnErrMsg.indexOf('missing parser name'), -1);
    });

    test('adapter_starts_listening_to_both_http_and_udp_requests', function () {
        var udpParser = 'parser',
            udpHost = 'localhost',
            udpPort = this.availPort,
            configUdpEndpointsSave = config.udpEndpoints;
        config.udpEndpoints = util.format('%s:%d:%s', udpHost, udpPort, udpParser);
        adapter.main();
        config.udpEndpoints = configUdpEndpointsSave;
        assert(this.serverListen.calledOnce);
        assert(this.serverListen.args[0][0], defaults.listenPort);
        assert(this.udpServer.bind.calledOnce);
        assert(this.udpServer.bind.args[0][0], udpPort);
        assert(this.udpServer.bind.args[0][1], udpHost);
    });

    test('adapter_starts_but_logs_error_when_udp_bind_fails', function () {
        var udpParser = 'parser',
            udpHost = 'localhost',
            udpPort = this.boundPort,
            configUdpEndpointsSave = config.udpEndpoints;
        config.udpEndpoints = util.format('%s:%d:%s', udpHost, udpPort, udpParser);
        sinon.spy(logger, 'error');
        adapter.main();
        config.udpEndpoints = configUdpEndpointsSave;
        assert(this.serverListen.calledOnce);
        assert(this.serverListen.args[0][0], defaults.listenPort);
        assert(this.udpServer.bind.calledOnce);
        assert(logger.error.calledOnce);
        logger.error.restore();
    });

    test('adapter_does_not_start_if_inaccessible_parsers_path', function (done) {
        var configParsersPathSave = config.parsersPath;
        config.parsersPath = '/invalid/path';
        sinon.spy(logger, 'error');
        sinon.stub(process, 'exit', function (code) {
            config.parsersPath = configParsersPathSave;
            assert(http.createServer.notCalled);
            assert(logger.error.calledOnce);
            assert.notEqual(code, 0);
            logger.error.restore();
            process.exit.restore();
            done();
        });
        adapter.main();
    });

});
