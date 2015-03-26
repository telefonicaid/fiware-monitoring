//
// Copyright 2014 Create-net.org
// All Rights Reserved.
//

'use strict';
var nagios = require('./common/nagios');

// Information about the status of the service nova-objectstore
// PROCS OK: 4 processes with command name 'nova-objectstore'|(null)

var parser = Object.create(nagios.parser);
parser.getContextAttrs = function(probeEntityData) {
    var data  = probeEntityData.data.split('\n')[0];
    var attrs = { nova_objectstore: 0};
    var items = data.split(':');
    if ((items.length)>0 ) {
	if (items[1].indexOf("nova-objectstor") != -1){
		if(items[0].indexOf("PROCS OK") !=-1){
			attrs.nova_objectstore=1
		}
		else{
	        	attrs.nova_objectstore=0
		}
	}
    }
    else{
        throw new Error('No valid nova-objectstore data found..');
    }
    return attrs;
};
exports.parser = parser;
