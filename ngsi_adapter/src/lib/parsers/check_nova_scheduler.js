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
    var attrs = { nova_scheduler: 0};
    var items = data.split(':');
    if ((items.length)>0 ) {
	if (items[1].indexOf("nova-scheduler") != -1){
		if(items[0].indexOf("PROCS OK") !=-1){
			attrs.nova_scheduler=1
		}
		else{
	        	attrs.nova_scheduler=0
		}
	}
    }
    else{
        throw new Error('No valid nova-scheduler data found');
    }
    return attrs;
};
exports.parser = parser;
