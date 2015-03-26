//
// Copyright 2014 Create-net.org
// All Rights Reserved.
//

'use strict';
var nagios = require('./common/nagios');

// Information about the status of the service glance-registry
// PROCS OK: 4 processes with command name 'glance-registry'|(null)

var parser = Object.create(nagios.parser);
parser.getContextAttrs = function(probeEntityData) {
    var data  = probeEntityData.data.split('\n')[0];
    var attrs = { glance_registry: 0};
    var items = data.split(':');
    if ((items.length)>0 ) {
	if (items[1].indexOf("glance-registry") != -1){
		if(items[0].indexOf("PROCS OK") !=-1){
			attrs.glance_registry=1
		}
		else{
	        	attrs.glance_registry=0
		}
	}
    }
    else{
        throw new Error('No valid glance-registry data found');
    }
    return attrs;
};
exports.parser = parser;
