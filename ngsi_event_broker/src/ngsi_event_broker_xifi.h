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
 * @file   ngsi_event_broker_xifi.h
 * @brief  XIFI-specific Event Broker macros and declarations
 *
 * This file defines several macros used by the implementation of [XIFI]
 * (@XIFI_ref)-specific [Event Broker](@NagiosModule_ref) (needed by XIFI
 * modules [DEM](@DEM_ref) and [NPM](@NPM_ref)).
 */


#ifndef NGSI_EVENT_BROKER_XIFI_H
#define NGSI_EVENT_BROKER_XIFI_H


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
 *  underscore removed). See [customvariablesmember](@NagiosCustomVarsStruct_ref) for details. */
#define CUSTOM_VAR_ENTITY_TYPE		"ENTITY_TYPE"

/**@}*/


/**
 * @name DEM monitoring
 * @{
 */

/** Entity type of a physical host being monitored */
#define DEM_ENTITY_TYPE_PHYSICAL	"host"

/** Entity type of a virtual machine being monitored */
#define DEM_ENTITY_TYPE_VIRTUAL		"vm"

/** Default entity type of a local host, when no type is specified */
#define DEM_ENTITY_TYPE_LOCAL		DEM_ENTITY_TYPE_PHYSICAL

/** Default entity type of a remote host, when no type is specified */
#define DEM_ENTITY_TYPE_REMOTE		DEM_ENTITY_TYPE_VIRTUAL

/** Default entity type for DEM monitoring, if none of the former applies */
#define DEM_DEFAULT_ENTITY_TYPE		DEM_ENTITY_TYPE_VIRTUAL

/** Adapter request query string fields (`id = region:hostaddr`) */
#define DEM_ADAPTER_REQUEST_FORMAT	ADAPTER_REQUEST_FORMAT

/**@}*/


/**
 * @name NPM monitoring
 * @{
 */

/** Default entity type for network elements being monitored, when no type is specified */
#define NPM_DEFAULT_ENTITY_TYPE		"interface"

/** Adapter request query string fields (`id = region:hostaddr/port`) */
#define NPM_ADAPTER_REQUEST_FORMAT	"%s/%s" \
					"?" ADAPTER_QUERY_FIELD_ID "=%s:%s/%d" \
					"&" ADAPTER_QUERY_FIELD_TYPE "=%s"
/**@}*/


/**
 * @name Host service monitoring
 * @{
 */

/** Default entity type for host services being monitored, when no type is specified */
#define SRV_DEFAULT_ENTITY_TYPE		"host_service"

/** Adapter request query string fields (`id = region:hostname:servname`) */
#define SRV_ADAPTER_REQUEST_FORMAT	"%s/%s" \
					"?" ADAPTER_QUERY_FIELD_ID "=%s:%s:%s" \
					"&" ADAPTER_QUERY_FIELD_TYPE "=%s"
/**@}*/


/**
 * @name XIMM module-specific functions
 * @{
 */


/**
 * Composes the request to NGSI Adapter according to DEM-specific plugin data
 *
 * @param[in] name	The name of the command for the plugin (result of find_plugin_command_name()).
 * @param[in] args	The command line arguments for the plugin (result of find_plugin_command_name()).
 * @param[in] type	The entity type associated to the resource being monitored.
 * @param[in] nrpe	True (non-zero) when plugin is remotely executed via NRPE.
 *
 * @return		The request URL to invoke NGSI Adapter (including query string).
 */
char* dem_get_adapter_request(char* name, char* args, const char* type, int nrpe);


/**
 * Composes the request to NGSI Adapter according to NPM-specific plugin data
 *
 * @param[in] name	The name of the command for the plugin (result of find_plugin_command_name()).
 * @param[in] args	The command line arguments for the plugin (result of find_plugin_command_name()).
 * @param[in] type	The entity type associated to the resource being monitored.
 *
 * @return		The request URL to invoke NGSI Adapter (including query string).
 */
char* npm_get_adapter_request(char* name, char* args, const char* type);


/**
 * Composes the request to NGSI Adapter according to Host service-specific plugin data
 *
 * @param[in] name	The name of the command for the plugin (result of find_plugin_command_name()).
 * @param[in] args	The command line arguments for the plugin (result of find_plugin_command_name()).
 * @param[in] type	The entity type associated to the resource being monitored.
 * @param[in] serv	The details of service definition associated to the plugin.
 *
 * @return		The request URL to invoke NGSI Adapter (including query string).
 */
char* srv_get_adapter_request(char* name, char* args, const char* type, const service* serv);


/**@}*/


#ifdef __cplusplus
}
#endif


#endif /*NGSI_EVENT_BROKER_XIFI_H*/
