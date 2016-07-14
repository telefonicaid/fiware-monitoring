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
 * @file   ngsi_event_broker_fiware.h
 * @brief  FIWARE Lab-specific Event Broker macros and declarations
 *
 * This file defines several macros used by the implementation of [FIWARE Lab] (@FIWARE_Lab_ref)-specific
 * [Event Broker](@NagiosModule_ref).
 */


#ifndef NGSI_EVENT_BROKER_FIWARE_H
#define NGSI_EVENT_BROKER_FIWARE_H


#ifdef __cplusplus
extern "C" {
#endif


#include "ngsi_event_broker_common.h"


/**
 * @name Nagios service custom variables
 * @{
 */

/** Name of custom variable in service description. A custom variable "_entity_type" may
 *  be included in Nagios configuration file when [defining a service](@NagiosCustomVars_ref);
 *  such variable is renamed to be accesible from this module (converted to uppercase and
 *  underscore removed). See ::customvariablesmember for details. */
#define CUSTOM_VAR_ENTITY_TYPE		"ENTITY_TYPE"

/**@}*/


/**
 * @name GEri global instance monitoring
 * @{
 */

/** Entity type of a GEri global instance */
#define GE_ENTITY_TYPE			"ge"

/** Adapter request query string fields (`id = region:host_name:service_description`) */
#define GE_ADAPTER_REQUEST_FORMAT	"%s/%s" \
					"?" ADAPTER_QUERY_FIELD_ID "=%s:%s:%s" \
					"&" ADAPTER_QUERY_FIELD_TYPE "=" GE_ENTITY_TYPE

/**
 * Composes the request to NGSI Adapter according to plugin data from GEri global instance
 *
 * @param[in] context	The operations context (may be null).
 * @param[in] name	The name of the command for the plugin (result of ::find_plugin_command_name).
 * @param[in] args	The command line arguments for the plugin (result of ::find_plugin_command_name).
 * @param[in] type	The entity type associated to the resource being monitored.
 * @param[in] serv	The details of service definition associated to the plugin.
 *
 * @return		The request URL to invoke NGSI Adapter (including query string).
 */
char* get_adapter_request_for_ge(context_t* context, char* name, char* args, const char* type, const service* serv);

/**@}*/


#ifdef __cplusplus
}
#endif


#endif /*NGSI_EVENT_BROKER_FIWARE_H*/
