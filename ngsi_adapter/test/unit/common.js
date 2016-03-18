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


var assert = require('assert'),
    timestamp = require('../../lib/parsers/common/base').parser.timestampAttrName;


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
    assert.ok(testSuite.entityData[timestamp]);
    assertIsNumber(testSuite.entityId);
    var entityAttrList = Object.keys(testSuite.entityData);
    var update = JSON.parse(updateJSON);
    // check id element
    assert.ok(update);
    assert.ok(update.id);
    // check isPattern element
    assert.ok(update.isPattern === "false");
    // check type element
    assert.ok(update.type);
    // check attributes
    update.attributes.forEach(function(item) { 
        assert.ok(item['name']);
        assert.ok(item['type']);
        assert.ok(item['value']);
    });
}

/**
 * assertIsNumber.
 */
exports.assertIsNumber = assertIsNumber;


/**
 * assertValidUpdateJSON.
 */
exports.assertValidUpdateJSON = assertValidUpdateJSON;
