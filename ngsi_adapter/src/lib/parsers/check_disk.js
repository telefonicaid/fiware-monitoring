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
    var attrs = { freeSpacePct : 0};
    var items = data.split(':');
    if ((items.length)>0 ) {
       var groups = items[1].split('/');
       if(groups[1].trim()){
          var tmp_grp=groups[1].trim();
          var groupvals = tmp_grp.split(' ');
          var percDisk = groupvals[2].trim();
          attrs.freeSpacePct=percDisk.substring(1,3);
       }
       else{
          attrs.freeSpacePct=0
       }
   }
    else{
        throw new Error('No valid freeSpacePct found');
    }
    return attrs;
};
exports.parser = parser;

