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
 * @file   ngsi_event_broker_common.h
 * @brief  NGSI Event Broker common macros and declarations
 *
 * This file defines several macros and declares functions to be used by any
 * specific implementation of an [Event Broker](@NagiosModule_ref) module.
 */


#ifndef NGSI_EVENT_BROKER_COMMON_H
#define NGSI_EVENT_BROKER_COMMON_H


#ifdef __cplusplus
extern "C" {
#endif


#include "nebmodules.h"
#include "nebstructs.h"


/**
 * @name Common macros
 * @{
 */

/** Multipurpose buffer length */
#define MAXBUFLEN			512

/**@}*/


/**
 * @name Nagios plugins macros
 * @{
 */

/** Name of the SNMP plugin */
#define SNMP_PLUGIN			"check_snmp"

/** Name of the NRPE plugin (remote plugin executions) */
#define NRPE_PLUGIN			"check_nrpe"

/**@}*/


/**
 * @name NGSI Adapter macros
 * @{
 */

/** Invalid request as result from get_adapter_request() */
#define ADAPTER_REQUEST_INVALID		NULL

/** Ignore request as result from get_adapter_request() */
#define ADAPTER_REQUEST_IGNORE		""

/** Query string field holding the NGSI entity identifier */
#define ADAPTER_QUERY_FIELD_ID		"id"

/** Query string field holding the NGSI entity type */
#define ADAPTER_QUERY_FIELD_TYPE	"type"

/** Format spec, printf()-like, used in composing NGSI Adapter request URL. Please
 *  note that `id` comprises two colon-separated values (`id = region:uniqueid`)
 *  where `region` denotes the domain the entity belongs to. */
#define ADAPTER_REQUEST_FORMAT		"%s/%s" \
					"?" ADAPTER_QUERY_FIELD_ID "=%s:%s" \
					"&" ADAPTER_QUERY_FIELD_TYPE "=%s"
/**@}*/


/**
 * @name Nagios internal symbols
 * @{
 */

/** Nagios object version */
extern int				__nagios_object_structure_version;

/**@}*/


/**
 * @name Module global variables to be defined by specific implementations
 * @{
 */

/** Name of the specific module */
extern char* const			module_name;

/** Version of the specific module */
extern char* const			module_version;

/** Global handle of the module */
extern void*				module_handle;

/**@}*/


/**
 * @name Module global variables related to common module arguments
 * @{
 */

/** URL of the NGSI Adapter to invoke to */
extern char*				adapter_url;

/** Identifier of the region the monitored entities belong to */
extern char*				region_id;

/** Address of the local host (used as part of identifiers in local entities) */
extern char*				host_addr;

/**@}*/


/**
 * @name Functions to be implemented by specific Event Broker module implementations
 * @{
 */


/**
 * Initializes global module handle and info (name and version)
 *
 * @param[in] handle		The module handle passed by Nagios to nebmodule_init().
 *
 * @retval NEB_OK		Successfully initialized.
 * @retval NEB_ERROR		Not successfully initialized.
 */
int init_module_handle_info(void* handle);


/**
 * Composes the request to NGSI Adapter according to plugin data
 *
 * @param[in] data		The plugin data passed by Nagios to the registered callback_service_check().
 *
 * @return			The request URL to invoke NGSI Adapter (including query string).
 */
char* get_adapter_request(nebstruct_service_check_data* data);


/**@}*/


/**
 * @name Rest of functions implemented in ngsi_event_broker_commmon.c
 * @{
 */


/**
 * Initializes module global variables, parsing module arguments in configuration file
 *
 * @param[in] args		The module arguments as a space-separated string.
 *
 * @retval NEB_OK		Successfully initialized.
 * @retval NEB_ERROR		Not successfully initialized.
 */
int init_module_variables(char* args);


/**
 * Releases resources for module global variables
 *
 * @retval NEB_OK		Success.
 */
int free_module_variables(void);


/**
 * Checks current Nagios object version
 *
 * @retval NEB_OK		Valid (compatible) version.
 * @retval NEB_ERROR		Invalid version.
 */
int check_nagios_object_version(void);


/**
 * Callback function invoked on `NEBCALLBACK_SERVICE_CHECK_DATA` events
 *
 * @param[in] callback_type	The event type (always `NEBCALLBACK_SERVICE_CHECK_DATA`).
 * @param[in] data		The event data (`nebstruct_service_check_data*`).
 *
 * @retval NEB_OK		Regardless event processing result, NEB_OK is returned.
 */
int callback_service_check(int callback_type, void* data);


/**
 * Gets details of executed plugin from event data passed to callback_service_check()
 *
 * @param[in]  data		The event data.
 * @param[out] args		The command line arguments of executed plugin.
 *
 * @return			The executable name (without path) of executed plugin.
 */
char* find_plugin_name(nebstruct_service_check_data* data, char** args);


/**
 * Gets command details of executed plugin from event data passed to callback_service_check()
 *
 * @param[in]  data		The event data.
 * @param[out] args		The command line arguments of executed plugin.
 * @param[out] nrpe		True (non-zero) when plugin is remotely executed via NRPE.
 * @param[out] serv		The details of service definition associated to the plugin.
 *
 * @return			The command name (may not coincide with executable name) of executed plugin.
 */
char* find_plugin_command_name(nebstruct_service_check_data* data, char** args, int* nrpe, const service** serv);


/**
 * Resolves a given hostname to get the IP address
 *
 * @param[in]  hostname		The hostname.
 * @param[in]  addr		The buffer where IP address will be written to.
 * @param[in]  addrmaxlen	The length of the buffer.
 *
 * @retval NEB_OK		Successfully resolved.
 * @retval NEB_ERROR		Not successfully resolved.
 */
int resolve_address(const char* hostname, char* addr, size_t addrmaxlen);


/**
 * Writes a formatted message to logfiles
 *
 * @param[in] level		The logging level (`"INFO"`, `"ERROR"`, etc).
 * @param[in] format		The printf()-like format spec of the message.
 * @param[in] ...		The variable list of arguments to format.
 */
void logging(const char* level, const char* format, ...);


/**@}*/


#ifdef __cplusplus
}
#endif


#endif /*NGSI_EVENT_BROKER_COMMON_H*/
