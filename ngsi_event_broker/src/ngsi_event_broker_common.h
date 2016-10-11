/*
 * Copyright 2013-2016 Telefónica I+D
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


#include "objects.h"
#include "nebmodules.h"
#include "nebstructs.h"


/**
 * @name Common macros
 * @{
 */

/** Multipurpose buffer length */
#ifndef MAXBUFLEN
#define MAXBUFLEN			512
#endif /*MAXBUFLEN*/

/** Safe string duplication */
#define STRDUP(s)			((s) ? strdup(s) : NULL)

/**@}*/


#define FOREACH_LEVEL(LEVEL) \
	LEVEL(LOG_ERROR) \
	LEVEL(LOG_WARN) \
	LEVEL(LOG_INFO) \
	LEVEL(LOG_DEBUG)

#define GENERATE_ENUM(ENUM)		ENUM,
#define GENERATE_STRING(STRING)		#STRING + 4,	/* skip "LOG_" */

/** Logging levels */
typedef enum {
	FOREACH_LEVEL(GENERATE_ENUM)
} loglevel_t;

/** Logging level names, indexed by value */
static const char* loglevel_names[] = {
	FOREACH_LEVEL(GENERATE_STRING)
	NULL
};


/**
 * @name Operations & Support
 * @{
 */

/** Operations context */
typedef struct {
	const char* corr;		/**< The correlation id */
	const char* op;			/**< The operation name */
} context_t;

/** HTTP header for correlation */
#define CORRELATOR_HTTP_HEADER		"Fiware-Correlator"

/** Length of ::CORRELATOR_HTTP_HEADER */
#define CORRELATOR_HTTP_HEADER_LEN	17

/**@}*/


/**
 * @name Nagios API macros
 * @{
 */

/** Macro to get service check command from Nagios 3.x and 4.x APIs */
#if (CURRENT_OBJECT_STRUCTURE_VERSION < 400)
#define SERVICE_CHECK_COMMAND(ptr)	(ptr)->service_check_command
#else
#define SERVICE_CHECK_COMMAND(ptr)	(ptr)->check_command
#endif

/**@}*/


/**
 * @name Nagios plugins macros
 * @{
 */

/** Name of the SNMP plugin */
#define SNMP_PLUGIN			"check_snmp"

/** Name of the HTTP plugin */
#define HTTP_PLUGIN			"check_http"

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

/** Logging level */
extern loglevel_t			log_level;

/**@}*/


/**
 * @name Nagios NEB API entry points
 * @{
 */


/**
 * Initializes the module (entry point for [NEB API](@NagiosModule_ref))
 *
 * @param[in] flags			The initialization flags (ignored).
 * @param[in] args			The module arguments as a space-separated string.
 * @param[in] handle			The module handle passed by Nagios Core server.
 *
 * @retval NEB_OK			Successfully initialized.
 * @retval NEB_ERROR			Not successfully initialized.
 */
int nebmodule_init(int flags, char* args, void* handle);


/**
 * Deinitializes the module (entry point for [NEB API](@NagiosModule_ref))
 *
 * @param[in] flags			The deinitialization flags (ignored).
 * @param[in] reason			The reason why this module is being deinitialized.
 *
 * @retval NEB_OK			Successfully deinitialized.
 */
int nebmodule_deinit(int flags, int reason);


/**@}*/


/**
 * @name Functions to be implemented by specific Event Broker module implementations
 * @{
 */


/**
 * Initializes global module handle and info (name and version)
 *
 * @param[in] handle			The module handle passed by Nagios to ::nebmodule_init.
 * @param[in] context			The operations context (may be null).
 *
 * @retval NEB_OK			Successfully initialized.
 * @retval NEB_ERROR			Not successfully initialized.
 */
int init_module_handle_info(void* handle, context_t* context);


/**
 * Composes the request to NGSI Adapter according to plugin data
 *
 * @param[in] data			The plugin data passed by Nagios to the registered ::callback_service_check.
 * @param[in] context			The operations context (may be null).
 *
 * @return				The request URL to invoke NGSI Adapter (including query string).
 * @retval ADAPTER_REQUEST_INVALID	An error ocurred and no request could be composed.
 * @retval ADAPTER_REQUEST_IGNORE	Skip request to NGSI Adapter.
 */
char* get_adapter_request(nebstruct_service_check_data* data, context_t* context);


/**@}*/


/**
 * @name Rest of functions implemented in ngsi_event_broker_common.c
 * @{
 */


/**
 * Initializes module global variables, parsing module arguments in configuration file
 *
 * @param[in] args			The module arguments as a space-separated string.
 * @param[in] context			The operations context (may be null).
 *
 * @retval NEB_OK			Successfully initialized.
 * @retval NEB_ERROR			Not successfully initialized.
 */
int init_module_variables(char* args, context_t* context);


/**
 * Releases resources for module global variables
 *
 * @retval NEB_OK		Success.
 */
int free_module_variables(void);


/**
 * Checks current Nagios object version
 *
 * @param[in] context			The operations context (may be null).
 *
 * @retval NEB_OK			Valid (compatible) version.
 * @retval NEB_ERROR			Invalid version.
 */
int check_nagios_object_version(context_t* context);


/**
 * Callback function invoked on ::NEBCALLBACK_SERVICE_CHECK_DATA events
 *
 * @param[in] callback_type		The event type (always ::NEBCALLBACK_SERVICE_CHECK_DATA).
 * @param[in] data			The event data (::nebstruct_service_check_data*).
 *
 * @retval NEB_OK			Regardless event processing result, NEB_OK is returned.
 */
int callback_service_check(int callback_type, void* data);


/**
 * Gets command details of executed plugin from event data passed to ::callback_service_check
 *
 * @param[in]  data			The event data.
 * @param[out] args			The command line arguments of executed plugin.
 * @param[out] nrpe			True (non-zero) when plugin is remotely executed via NRPE.
 * @param[out] serv			The details of service definition associated to the plugin.
 *
 * @return			The command name (may not coincide with executable name) of executed plugin.
 */
char* find_plugin_command_name(nebstruct_service_check_data* data, char** args, int* nrpe, const service** serv);


/**
 * Resolves a given hostname to get the IP address
 *
 * @param[in]  hostname			The hostname.
 * @param[in]  addr			The buffer where IP address will be written to.
 * @param[in]  addrmaxlen		The length of the buffer.
 *
 * @retval NEB_OK			Successfully resolved.
 * @retval NEB_ERROR			Not successfully resolved.
 */
int resolve_address(const char* hostname, char* addr, size_t addrmaxlen);


/**
 * Writes a formatted message to Nagios log
 *
 * @param[in] level			The logging level.
 * @param[in] context			The operations context (may be null).
 * @param[in] format			The printf()-like format spec of the message.
 * @param[in] ...			The variable list of arguments to format.
 */
void logging(loglevel_t level, context_t* context, const char* format, ...);


/**@}*/


#ifdef __cplusplus
}
#endif


#endif /*NGSI_EVENT_BROKER_COMMON_H*/
