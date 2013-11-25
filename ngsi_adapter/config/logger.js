//
// Copyright 2013 Telefónica I+D
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


var winston    = require('winston'),
    dateformat = require('dateformat');


// Logging options
var opts = {
    logLevel:       'info',
    logFile:        process.cwd() + '/nsgi_adapter.log',
    logMaxSize:     5*1024*1024,    // 5 MB
    logMaxFiles:    10,
    logTimestamp:   function() { return dateformat(new Date(), 'yyyy-mm-dd H:MM:ss'); }
};


// Create logger with several transports
var logger = new (winston.Logger)({
    exitOnError: false,
    transports: [
        new (winston.transports.Console)({
            json:       false,
            timestamp:  false,
            colorize:   true,
            level:      opts.logLevel
        }),
        new (winston.transports.File)({
            json:       false,
            timestamp:  opts.logTimestamp,
            level:      opts.logLevel,
            filename:   opts.logFile,
            maxsize:    opts.logMaxSize,
            maxFiles:   opts.logMaxFiles
        })
    ]
});


module.exports = logger;
