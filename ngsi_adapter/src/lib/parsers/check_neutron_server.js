//
// Copyright 2014 Create-net.org
// All Rights Reserved.
//

'use strict';
var nagios = require('./common/nagios');

// Information about the status of the service quantum-server
// PROCS OK: 4 processes with command name 'neutron-server'|(null)
// DEPRECATED: use 'check_quantum_server.js' instead
var parser = Object.create(nagios.parser);
parser.getContextAttrs = function(probeEntityData) {
    var data  = probeEntityData.data.split('\n')[0];
    var attrs = { quantum_server: 0};
    var items = data.split(':');
    if ((items.length)>0 ) {
	if (items[1].indexOf("neutron-server") != -1){
		if(items[0].indexOf("PROCS OK") !=-1){
			attrs.quantum_server=1
		}
		else{
	        	attrs.quantum_server=0
		}
	}
    }
    else{
        throw new Error('No valid neutron-server data found..');
    }
    return attrs;
};
exports.parser = parser;