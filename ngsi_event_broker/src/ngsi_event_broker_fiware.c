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
 * @file   ngsi_event_broker_fiware.c
 * @brief  FIWARE Lab-specific Event Broker implementation
 *
 * This file consists of the implementation of [FIWARE Lab](@FIWARE_Lab_ref)-specific [Event Broker](@NagiosModule_ref),
 * which is used to monitor the following resources:
 *
 * - [FIWARE GEri global instances](@FIWARE_GEri_ref). There are no restrictions on the command names and the plugins to
 *   be used, but the resulting NGSI entity type must be explicitly given with a custom variable "_entity_type" in the
 *   service definition (or using a service template). Entity identifier will be "{region}:{host_name}:{service_desc}".
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <netinet/in.h>
#include "config.h"
#include "nebstructs.h"
#include "nebmodules.h"
#include "nebcallbacks.h"
#include "neberrors.h"
#include "broker.h"
#include "argument_parser.h"
#include "ngsi_event_broker_common.h"
#include "ngsi_event_broker_fiware.h"


/* specify event broker API version (required) */
NEB_API_VERSION(CURRENT_NEB_API_VERSION)


/**
 * @name Event Broker module identification
 * @{
 */

/** Module name (from package details defined by `config.h`) */
#define MODULE_NAME			PACKAGE_NAME "_fiware"

/** Module version (from package details defined by `config.h`) */
#define MODULE_VERSION			PACKAGE_VERSION

/**@}*/


/* define global variables (previously declared) */
char* const module_name			= MODULE_NAME;
char* const module_version		= MODULE_VERSION;
void*       module_handle		= NULL;


/* initializes module handle and info (name and version) */
int init_module_handle_info(void* handle, context_t* context)
{
	int result = NEB_OK;

	module_handle = handle;
	logging(LOG_INFO, context, "Starting up (version %s)...", module_version);
	neb_set_module_info(module_handle, NEBMODULE_MODINFO_TITLE, module_name);
	neb_set_module_info(module_handle, NEBMODULE_MODINFO_VERSION, module_version);

	return result;
}


/* gets adapter request URL including query fields */
char* get_adapter_request(nebstruct_service_check_data* data, context_t* context)
{
	char* result = NULL;
	char* name   = NULL;
	char* args   = NULL;
	int   nrpe   = 0;

	/* Build request according to plugin details */
	const service* serv;

	if ((name = find_plugin_command_name(data, &args, &nrpe, &serv)) == NULL) {
		logging(LOG_WARN, context, "Cannot get plugin command name");
		result = STRDUP(ADAPTER_REQUEST_INVALID);
	} else if (args == NULL) {
		logging(LOG_WARN, context, "Cannot get plugin command arguments");
		result = STRDUP(ADAPTER_REQUEST_INVALID);
	} else if (serv == NULL) {
		logging(LOG_WARN, context, "Cannot get plugin service details");
		result = STRDUP(ADAPTER_REQUEST_INVALID);
	} else {
		const char*		type = NULL;
		customvariablesmember*	vars = serv->custom_variables;

		/* get entity type (must be explicitly defined as custom variable) and request URL */
		if ((vars == NULL) || strcmp(vars->variable_name, CUSTOM_VAR_ENTITY_TYPE)) {
			char var_lowercase[MAXBUFLEN] = CUSTOM_VAR_ENTITY_TYPE;
			char* p; for (p = var_lowercase ; *p; ++p) *p = tolower(*p);
			logging(LOG_DEBUG, context, "No custom variable _%s found", var_lowercase);
		} else {
			type = vars->variable_value;
			if (!strcmp(type, GE_ENTITY_TYPE)) {
				result = get_adapter_request_for_ge(context, name, args, type, serv);
			} else {
				logging(LOG_DEBUG, context, "Unknown entity type %s", type);
				type = NULL;
			}
		}

		/* ignore services not defining a known entity_type */
		if (type == NULL) {
			logging(LOG_DEBUG, context, "Ignoring data from service %s", serv->description);
			result = STRDUP(ADAPTER_REQUEST_IGNORE);
		}
	}

	free(args);
	args = NULL;
	free(name);
	name = NULL;
	return result;
}


/* [GEri global instance] gets adapter request URL */
char* get_adapter_request_for_ge(context_t* context, char* name, char* args, const char* type, const service* serv)
{
	char* result = NULL;
	char  buffer[MAXBUFLEN];

	snprintf(buffer, sizeof(buffer)-1, GE_ADAPTER_REQUEST_FORMAT,
	         adapter_url, name, region_id, serv->host_name, serv->description);
	buffer[sizeof(buffer)-1] = '\0';

	result = STRDUP(buffer);
	return result;
}
