/*
 * Copyright 2016 Telefónica I+D
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
 * @file   suite_config.h
 * @brief  Common macros and declarations for test suites
 */


#ifndef SUITE_CONFIG_H
#define SUITE_CONFIG_H


#include "config.h"


/// Fake adapter URL
#define ADAPTER_URL		"http://adapter_host:5000"


/// Fake local host address
#define LOCALHOST_ADDR		"10.95.0.6"


/// Fake local host name
#define LOCALHOST_NAME		"my_local_host"


/// Fake remote address
#define REMOTEHOST_ADDR		"169.254.0.1"


/// Fake remote host name
#define REMOTEHOST_NAME		"my_remote_host"


/// Fake cURL handle
#define CURL_HANDLE		((void*) 1)


/// Fake NEB module handle
#define MODULE_HANDLE		((void*) 2)


/// Some region id
#define REGION_ID		"some_region"


/// Some check name
#define SOME_CHECK_NAME		"some_check"


/// Some check arguments
#define SOME_CHECK_ARGS		"-w 10 -c 20"


/// Some check output data
#define SOME_CHECK_OUTPUT_DATA	"some_data"


/// Some check output data
#define SOME_CHECK_PERF_DATA	"some_perf_data"


/// Some service description
#define SOME_DESCRIPTION	"some_description"


#endif /*SUITE_CONFIG_H*/
