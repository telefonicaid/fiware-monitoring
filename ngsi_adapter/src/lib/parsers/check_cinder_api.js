//
// Copyright 2014 Create-net.org
// All Rights Reserved.
//

'use strict';
var nagios = require('./common/nagios');

// Information about the status of the service cinder-api
// PROCS OK: 4 processes with command name 'cinder-api'|(null)
var parser = Object.create(nagios.parser);
parser.getContextAttrs = function(probeEntityData) {
    var data  = probeEntityData.data.split('\n')[0];
    var attrs = { cinder_api: 0};
    var items = data.split(':');
    if ((items.length)>0 ) {
	if (items[1].indexOf("cinder-api") != -1){
		if(items[0].indexOf("PROCS OK") !=-1){
			attrs.cinder_api=1
		}
		else{
	        	attrs.cinder_api=0
		}
	}
    }
    else{
        throw new Error('No valid cinder-api found..');
    }
    return attrs;
};
exports.parser = parser;
