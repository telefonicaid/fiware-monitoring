//
// Copyright 2014 Create-net.org
// All Rights Reserved.
//

'use strict';
var nagios = require('./common/nagios');

// Region information plugin parses all information from the OpenStack Data Collector component
// nRegion, nVMActive, nVMTot, nCoreUsed, nCoreTot, nRamUsed, nRamTot,nHDUsed, nHDTot,nUsers, location, latitude, longitude

//var parser = Object.create(nagios.parser);
//parser.getContextAttrs = function(multilineData, multilinePerfData) {
//    var data  = multilineData.split('\n')[0];   // only consider first line of data, discard perfData

var parser = Object.create(nagios.parser);
parser.getContextAttrs = function(probeEntityData) {
    var data  = probeEntityData.data.split('\n')[0];    // only consider first line of probe data, discard perfData   

    var attrs = { uid: NaN, host_id: NaN,host_name: NaN};
    var items = data.split(',');
    if ((items.length)>0 ) {
        for (var i = 0; i < items.length; i++){
            var element=items[i]
            if (element.split('::')[0].replace(/\s/g, '')=="uid")
                attrs.uid=element.split('::')[1].replace(/\s/g, '')
            if (element.split('::')[0].replace(/\s/g, '')=="host_id")
                attrs.host_id=element.split('::')[1].replace(/\s/g, '')
            if (element.split('::')[0].replace(/\s/g, '')=="host_name")
                attrs.host_name=element.split('::')[1].replace(/\s/g, '')
        }
    } else {
        throw new Error('No valid users data found');
    }

    // Remove empty or NaN attributes
    for (var i in attrs) if (!attrs[i]) delete attrs[i];

    return attrs;
};
exports.parser = parser;
