/*
 * Copyright 2013-2016 Telefónica I+D
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
 * Module that defines a base parser object.
 *
 * @module base
 */


'use strict';
/* jshint unused: false, laxbreak: true */


var url = require('url'),
    util = require('util'),
    common = require('../../common'),
    config = require('../../config');


/**
 * Base parser object (to be extended by probe-specific parsers).
 */
var baseParser = Object.create(null);


/**
 * Gets the content type for Context Broker requests.
 *
 * @function getContentType
 * @memberof baseParser
 * @returns {String} The content type (the format) for ContextBroker requests.
 */
baseParser.getContentType = function () {
    return 'application/json';
};


/**
 * Returns request body and adds `reqdomain` the HTTP options to issue an update attributes operation in ContextBroker.
 *
 * @function getUpdateRequest
 * @memberof baseParser
 * @this baseParser
 * @param {Domain} reqdomain   Domain handling request (includes context, timestamp, id, type, body, parser & req_opts).
 * @returns {String} The request body in JSON format.
 */
baseParser.getUpdateRequest = function (reqdomain) {
    var entityData = this.parseRequest(reqdomain),
        entityAttrs = this.getContextAttrs(entityData),
        entityId = reqdomain.entityId,
        entityType = reqdomain.entityType;

    if (Object.keys(entityAttrs).length === 0) {
        throw new Error('Missing entity context attributes');
    } else if (!entityId || !entityType) {
        throw new Error('Missing entityId and/or entityType');
    }

    // feature #4: automatically add request timestamp to entity attributes
    entityAttrs[common.timestampAttrName] = reqdomain.timestamp;

    // body and options for the HTTP request to ContextBroker
    var contentType = this.getContentType(),
        brokerUrl = url.parse(config.brokerUrl),
        brokerPath = (config.brokerApi === common.BROKER_API_V2) ?
            util.format('/%s/entities/%s/attrs?type=%s&options=append', config.brokerApi, entityId, entityType) :
            util.format('/%s/updateContext', config.brokerApi);

    var json = (config.brokerApi === common.BROKER_API_V2) ? this.getUpdateRequestJSONv2 : this.getUpdateRequestJSON,
        requestBody = json(entityId, entityType, entityAttrs),
        requestOptions = {
            hostname: brokerUrl.hostname,
            port: brokerUrl.port,
            path: brokerPath,
            method: 'POST',
            headers: {
                'Content-Length': requestBody.length,
                'Content-Type': contentType,
                'Accept': contentType
            }
        };

    // add transaction id
    requestOptions.headers[common.txIdHttpHeader] = reqdomain.context.trans;

    // add options to `reqdomain` and return request body
    reqdomain.options = requestOptions;
    return requestBody;
};


/**
 * Parses the request message body to extract raw probe data, and optionally sets `entityId` and `entityType`.
 *
 * @abstract
 * @function parseRequest
 * @memberof baseParser
 * @param {Domain} reqdomain   Domain handling current request (includes context, timestamp, id, type, body & parser).
 * @returns {EntityData} An object holding entity data taken from request message.
 */
baseParser.parseRequest = function (reqdomain) {
    throw new Error('Must implement');
};


/**
 * Parses raw probe data to extract an object whose members are NGSI context attributes.
 *
 * @abstract
 * @function getContextAttrs
 * @memberof baseParser
 * @param {EntityData} data    Object holding raw entity data.
 * @returns {Object} Context attributes.
 */
baseParser.getContextAttrs = function (data) {
    throw new Error('Must implement');
};


/**
 * Returns the JSON payload for the body of an update context attributes request of ContextBroker API v0/v1.
 *
 * @function getUpdateRequestJSON
 * @memberof baseParser
 * @param {String} id          The entity identifier.
 * @param {String} type        The entity type.
 * @param {Object} attrs       The entity context attributes.
 * @returns {String} The request body in JSON format.
 */
baseParser.getUpdateRequestJSON = function (id, type, attrs) {

    var payload = {
        'contextElements': [
            {
                'type': type,
                'isPattern': 'false',
                'id': id,
                'attributes': [ ]
            }
        ],
        'updateAction': 'APPEND'
    };

    for (var name in attrs) {
        payload.contextElements[0].attributes.push({
            'name': name,
            'type': 'string',
            'value': attrs[name].toString()
        });
    }

    return JSON.stringify(payload);
};


/**
 * Returns the JSON payload for the body of an update context attributes request of ContextBroker API v2.
 *
 * @function getUpdateRequestJSONv2
 * @memberof baseParser
 * @param {String} id          The entity identifier.
 * @param {String} type        The entity type.
 * @param {Object} attrs       The entity context attributes.
 * @returns {String} The request body in JSON format.
 */
baseParser.getUpdateRequestJSONv2 = function (id, type, attrs) {

    var payload = {};

    for (var name in attrs) {
        payload[name] = {
            'value': attrs[name].toString()
        };
    }

    return JSON.stringify(payload);
};


/**
 * Base parser to be extended.
 */
exports.parser = baseParser;
