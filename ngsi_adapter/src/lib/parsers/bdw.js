//
// Copyright 2013 Universidad Politecnica de Madrid
// All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License"); you may
// not use this file except in compliance with the License. You may obtain
// a copy of the License at
//
//       http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
// WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
// License for the specific language governing permissions and limitations
// under the License.
//


'use strict';


// nam base parser object (to be extended by probe-specific parsers)
var baseParser = require('./common/base').parser;
var parser = Object.create(baseParser);

//var baseParser = require('./base').parser;

// NAM "BDW" probe-specific parser
// References: http://wiki.fi-xifi.eu/Xifi:Wp3:Components:NAM#Architecture_design
//
// Context attributes to be calculated:
//     Bandwith = measuring maximum TCP and UDP bandwidth performance.
//
// Sample data:
//
//  {
//  "idTest": "BDW-1393612588346",
//  "type": "Bdw",

//  "timestamp": "28-2-2014 / 19:36",
//  "hostSource": "138.4.47.33",
//  "hostDestination": "193.1.202.133",
//  "error": false,
//  "result": "------------------------------------------------------------\n
// 		Client connecting to 193.1.202.133, TCP port 5001\n
//  		TCP window size: 0.02 MByte (default)\n
//		------------------------------------------------------------\n
//		[  3] local 192.168.0.233 port 40310 connected with 193.1.202.133 port 5001\n
//
//		[ ID] Interval       Transfer     Bandwidth\n
//		[  3]  0.0- 2.0 sec  4.38 MBytes  18.4 Mbits/sec\n
//		[  3]  2.0- 4.0 sec  8.75 MBytes  36.7 Mbits/sec\n
//		[  3]  4.0- 6.0 sec  15.1 MBytes  63.4 Mbits/sec\n
//		[  3]  6.0- 8.0 sec  16.8 MBytes  70.3 Mbits/sec\n
//		[  3]  8.0-10.0 sec  11.5 MBytes  48.2 Mbits/sec\n
//		[  3]  0.0-10.0 sec  56.6 MBytes  47.5 Mbits/sec\n",
//  "_id": "5310d72c0b50d751022d9bd4"
//}
//

     
parser.parseRequest = function(request) {
   var entityData = {};
   entityData.data = request.body;
   return entityData;
};

parser.getContextAttrs = function(probeEntityData) {
   var data  = JSON.parse(probeEntityData.data);   
   var attrs = { timeIntervals: NaN, bandwidth: NaN, bandwidth_avg: NaN };
   var interval = "10;2";
   var bandwidth = data.result.match(/([0-9.]+)( Mbits)/gm)

   var i = 0;
   for (i in bandwidth){
     bandwidth[i]=parseFloat(bandwidth[i].match(/[0-9.]+/));
   }

   // attrs.timestamp = data.timestamp; 
   attrs.timeIntervals = interval;
   attrs.bandwidth = bandwidth;
   attrs.bandwidth_avg = bandwidth[bandwidth.length-1];

   if (attrs.bandwith==NaN) {
     throw new Error('No valid bandwith data found');
   }

   return attrs;
};

exports.parser = parser;
