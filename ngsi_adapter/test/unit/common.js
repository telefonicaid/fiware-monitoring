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
 * Module that defines common functions for testing purposes.
 *
 * @module common
 */


'use strict';


var cuid = require('cuid'),
    assert = require('assert'),
    timestampAttrName = require('../../lib/common').timestampAttrName;


/**
 * Asserts given argument is a valid number.
 *
 * @function assertIsNumber
 * @param {Object} value        Any object.
 */
function assertIsNumber(value) {
    assert(!isNaN(value) && !isNaN(parseFloat(value)));
}


/**
 * Asserts given JSON is well formed according to test suite data.
 *
 * @function assertValidUpdateJSON
 * @param {String} updateJSON   The JSON payload of an updateContext request.
 * @param {Object} testSuite    The test suite whose data produced the JSON.
 */
function assertValidUpdateJSON(updateJSON, testSuite) {
    assert.ok(testSuite.entityType);
    assert.ok(testSuite.entityData);
    // feature #4: automatically add request timestamp to entity attributes
    assert.ok(testSuite.entityData[timestampAttrName]);
    assertIsNumber(testSuite.entityId);
    var update = JSON.parse(updateJSON);
    // check id element
    assert.ok(update);
    assert.ok(update.contextElements[0].id);
    // check isPattern element
    assert.ok(update.contextElements[0].isPattern === 'false');
    // check type element
    assert.ok(update.contextElements[0].type);
    // check attributes
    update.contextElements[0].attributes.forEach(function(item) {
        assert.ok(item.name);
        assert.ok(item.type);
        assert.ok(item.value);
    });
    assert.ok(update.updateAction === 'APPEND');
}


/**
 * Returns a new request domain.
 *
 * @function domain
 * @param {Object} testSuite    The test suite that will use the domain.
 */
function domain(testSuite) {
    return {
        context: {trans: cuid()},
        timestamp: Date.now(),
        entityId: testSuite.entityId,
        entityType: testSuite.entityType
    };
}


/**
 * assertIsNumber.
 */
exports.assertIsNumber = assertIsNumber;


/**
 * assertValidUpdateJSON.
 */
exports.assertValidUpdateJSON = assertValidUpdateJSON;


/**
 * domain.
 */
exports.domain = domain;


/**
 * timestampAttrName.
 */
exports.timestampAttrName = timestampAttrName;
