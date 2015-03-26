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
  var attrs = { hostname: "undefined"};
  if (data) {
    if (data.indexOf('NRPE')!=-1 || data.indexOf('(Return')!=-1)
      attrs.hostname='None'
    else
      attrs.hostname=data

  }
  else{
    attrs.hostname="undefined"
    throw new Error('No valid nova-api found');
  }
  return attrs;
};
exports.parser = parser;
