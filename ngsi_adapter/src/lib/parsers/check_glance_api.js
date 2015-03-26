//
// Copyright 2014 Create-net.org
// All Rights Reserved.
//

'use strict';
var nagios = require('./common/nagios');

// Information about the status of the service glance-api
// PROCS OK: 4 processes with command name 'glance-api'|(null)


var parser = Object.create(nagios.parser);
parser.getContextAttrs = function(probeEntityData) {
    var data  = probeEntityData.data.split('\n')[0];
    var attrs = { glance_api: 0};
    var items = data.split(':');
    if ((items.length)>0 ) {
	if (items[1].indexOf("glance-api") != -1){
		if(items[0].indexOf("PROCS OK") !=-1){
			attrs.glance_api=1
		}
		else{
	        	attrs.glance_api=0
		}
	}
    }
    else{
        throw new Error('No valid glance-api data found');
    }
    return attrs;
};
exports.parser = parser;
