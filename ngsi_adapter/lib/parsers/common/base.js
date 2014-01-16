//
// Copyright 2013 Telefónica I+D
// All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License"); you may
// not use this file except in compliance with the License. You may obtain
// a copy of the License at
//
//       http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
// WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
// License for the specific language governing permissions and limitations
// under the License.
//


'use strict';
/* jshint unused: false */


var url = require('url');


// Base parser object (to be extended by probe-specific parsers)
var baseParser = Object.create(null);


// Adapter calls this function to set the HTTP request
baseParser.setRequest = function(request) {
    this.request = request;
};


// Adapter calls this function to get the Content-Type of requests
baseParser.getRequestContentType = function() {
    return 'application/xml';
}


// Adapter calls this function to get the updateContext() request
baseParser.updateContextRequest = function() {
    var query = url.parse(this.request.url, true).query;
    var entityId = query.id;
    var entityType = query.type;
    if (!entityId || !entityType) {
        throw new Error('Missing entityId and/or entityType');
    } else {
        var entityData = this.parseRequest();
        var entityAttrs = this.getContextAttrs(entityData.data, entityData.perfData);
        return (this.getRequestContentType() === 'application/xml')
            ? this.getUpdateContextXML(entityId, entityType, entityAttrs)
            : this.getUpdateContextJSON(entityId, entityType, entityAttrs);
    }
};


// Functions that must be superseded by probe-specific parsers
baseParser.parseRequest = function() {
    throw new Error('Must implement');
};
baseParser.getContextAttrs = function(data, optionalPerfData) {
    throw new Error('Must implement');
};


// Generate a JSON updateContext() request body
baseParser.getUpdateContextJSON = function(id, type, attrs) {
    throw new Error('TO-DO');
};


// Generate a XML updateContext() request body
baseParser.getUpdateContextXML = function(id, type, attrs) {
    var result = '';
    result += '<?xml version="1.0" encoding="UTF-8"?>\n';
    result += '<updateContextRequest>\n';
    result += '    <contextElementList>\n';
    result += '        <contextElement>\n';
    result += '            <entityId type="' + type +'" isPattern="false">\n';
    result += '                <id>' + id + '</id>\n';
    result += '            </entityId>\n';
    result += '            <contextAttributeList>\n';
    for (var name in attrs) {
    result += '                <contextAttribute>\n';
    result += '                    <name>' + name + '</name>\n';
    result += '                    <type>string</type>\n';
    result += '                    <contextValue>' + attrs[name] + '</contextValue>\n';
    result += '                </contextAttribute>\n';
    }
    result += '            </contextAttributeList>\n';
    result += '        </contextElement>\n';
    result += '    </contextElementList>\n';
    result += '    <updateAction>UPDATE</updateAction>\n';
    result += '</updateContextRequest>\n';
    return result;
};


exports.parser = baseParser;
