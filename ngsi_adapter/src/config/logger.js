/*
 * Copyright 2013 Telefónica I+D
 * All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License. You may obtain
 * a copy of the License at
 *
 *         http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */


/**
 * Module that configures logging.
 *
 * @module logger
 */


'use strict';


var winston    = require('winston'),
    dateformat = require('dateformat');


/**
 * @namespace
 * @property {Object}     opts              Logging options.
 * @property {String}     opts.logLevel     Logging level.
 * @property {String}     opts.logFile      Log file name.
 * @property {Number}     opts.logMaxSize   Log file maximum size (in bytes).
 * @property {Number}     opts.logMaxFiles  Maximum number of log files.
 * @property {Function()} opts.logTimestamp Timestamp format
 */
var opts = {
    logLevel:       'info',
    logFile:        process.cwd() + '/ngsi_adapter.log',
    logMaxSize:     5*1024*1024,    // 5 MB
    logMaxFiles:    10,
    logTimestamp:   function() { return dateformat(new Date(), 'yyyy-mm-dd H:MM:ss'); }
};


// Create logger with several transports
/**
 * @namespace
 * @property {Object}     logger            Default logger.
 * @property {Object[]}   logger.transports Array of logger transports.
 */
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
