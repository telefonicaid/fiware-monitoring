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
    xml2js = require('xml2js');


function assertIsNumber(value) {
    assert(!isNaN(value) && !isNaN(parseFloat(value)));
}
exports.assertIsNumber = assertIsNumber;


function assertValidUpdateXML(updateXML, test) {
    assert.ok(updateXML);
    assert.ok(test.entityType);
    assert.ok(test.entityData);
    assertIsNumber(test.entityId);
    var entityAttrList = Object.keys(test.entityData);
    xml2js.parseString(updateXML, function(err, result) {
        // check <updateContextRequest> element
        assert.ok(result.updateContextRequest);
        // check <contextElementList> element
        assert.ok(result.updateContextRequest.contextElementList);
        assert(result.updateContextRequest.contextElementList.length > 0);
        // check <contextElement> element
        var contextElement = result.updateContextRequest.contextElementList[0].contextElement[0];
        assert.ok(contextElement);
        // check <entityId> element and its attributes/subelements
        assert.ok(contextElement.entityId);
        assert.equal(contextElement.entityId[0].$.type, test.entityType);
        assert.equal(contextElement.entityId[0].id[0], test.entityId);
        // check <contextAttributeList> element
        assert(contextElement.contextAttributeList.length === 1);
        var contextAttrList = contextElement.contextAttributeList[0].contextAttribute;
        assert.ok(contextAttrList);
        // ensure element has one and only one subelement corresponding to expected attributes
        var attrcount = {};
        entityAttrList.forEach(function(item) { attrcount[item] = 0; });
        contextAttrList.forEach(function(item) { attrcount[item.name[0]]++; });
        assert.equal(Object.keys(attrcount).length, entityAttrList.length);
        entityAttrList.forEach(function(item) { assert(attrcount[item] === 1); });
        // ensure subelements are numbers
        contextAttrList.forEach(function(item) { assertIsNumber(item.contextValue[0]); });
    });
}
exports.assertValidUpdateXML = assertValidUpdateXML;
