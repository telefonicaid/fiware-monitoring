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
    var attrs = { nova_api: 0};
    var items = data.split(':');
    if ((items.length)>0 ) {
	if (items[1].indexOf("nova-api") != -1){
		if((items[0].indexOf("PROCS OK") !=-1) ){
			attrs.nova_api=1
		}
		else{
	        	attrs.nova_api=0
		}
	}
    }
    else{
        throw new Error('No valid nova-api found');
    }
    return attrs;
};
exports.parser = parser;
