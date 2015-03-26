//
// Copyright 2014 Create-net.org
// All Rights Reserved.
//

'use strict';
var nagios = require('./common/nagios');

// Region information plugin parses all information from the OpenStack Data Collector component
// nRegion, nVMActive, nVMTot, nCoreUsed, nCoreTot, nRamUsed, nRamTot,nHDUsed, nHDTot,nUsers, location, latitude, longitude
// PROCS OK: 4 processes with command name 'nova-api'|(null)

var parser = Object.create(nagios.parser);
parser.getContextAttrs = function(probeEntityData) {
    var data  = probeEntityData.data.split('\n')[0];

    var attrs = { keystone_proxy : 0};
    if (data.indexOf("HTTP") != -1){
		if(data.indexOf("HTTP OK") !=-1){
			attrs.keystone_proxy=1
		}
		else{
	        	attrs.keystone_proxy=0
		}
    }
    else{
        throw new Error('No valid keystone_proxy found..');
    }
    return attrs;
};
exports.parser = parser;
