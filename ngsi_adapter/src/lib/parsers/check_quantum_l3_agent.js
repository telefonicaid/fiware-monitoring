//
// Copyright 2014 Create-net.org
// All Rights Reserved.
//

'use strict';
var nagios = require('./common/nagios');

// Information about the status of the service quantum-l3-agent
// PROCS OK: 4 processes with command name 'quantum-l3-agent'|(null)
var parser = Object.create(nagios.parser);
parser.getContextAttrs = function(probeEntityData) {
    var data  = probeEntityData.data.split('\n')[0];
    var attrs = { quantum_l3_agent: 0 };
    var items = data.split(':');
    if (items.length > 0) {
        if (items[1].match(/(quantum|neutron)-l3-agent/)) {
            if (items[0].indexOf("PROCS OK") != -1) {
                attrs.quantum_l3_agent = 1;
            } else {
                attrs.quantum_l3_agent = 0;
            }
        }
    } else {
        throw new Error('No valid quantum-l3-agent data found');
    }
    return attrs;
};

exports.parser = parser;
