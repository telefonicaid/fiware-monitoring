//
// Copyright 2014 Create-net.org
// All Rights Reserved.
//

'use strict';
var nagios = require('./common/nagios');

// Information about the status of the service nova-cert
// PROCS OK: 4 processes with command name 'nova-cert'|(null)
var parser = Object.create(nagios.parser);
parser.getContextAttrs = function(probeEntityData) {
    var data  = probeEntityData.data.split('\n')[0];
    var attrs = { nova_cert: 0};
    var items = data.split(':');
    if ((items.length)>0 ) {
	if (items[1].indexOf("nova-cert") != -1){
		if(items[0].indexOf("PROCS OK") !=-1){
			attrs.nova_cert=1
		}
		else{
	        	attrs.nova_cert=0
		}
	}
    }
    else{
        throw new Error('No valid nova-cert found');
    }
    return attrs;
};
exports.parser = parser;
