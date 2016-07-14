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
 * @file   ngsi_event_broker_xifi.c
 * @brief  XIFI-specific Event Broker implementation
 *
 * This file consists of the implementation of [XIFI](@XIFI_ref)-specific [Event Broker](@NagiosModule_ref), which is
 * used to monitor the following resources:
 *
 * - Instances (i.e. VMs), as part of the XIFI [DEM](@XIFI_DEM_ref) module.
 * - Network elements, as part of the XIFI [NPM](@XIFI_NPM_ref) module.
 * - OpenStack services.
 *
 * Monitoring of network elements requires using Nagios plugin "check_snmp". The entity type "interface" is assumed by
 * default (unless explicitly given with a custom variable "_entity_type" in the service definition) and the resulting
 * entity identifier will be "{region}:{ifaddr}/{ifport}", taking address and port from the check command arguments.
 *
 * For host service (i.e. OpenStack service) monitoring, there are no restrictions on the command names and the plugins
 * to be used. The entity identifier will be "{region}:{host_name}:{service_desc}" and the entity type "host_service"
 * should be explicitly given with a custom variable "_entity_type" at service definition (or using a service template).
 *
 * For any other plugin executed locally, the entity type "host" will be assumed and resulting entity identifier will be
 * "{region}:{localaddr}". But if plugin is executed remotely via NRPE, we will assume an instance (i.e. VM) is being
 * monitored: entity type "vm" will be taken by default and entity identifier will consist of "{region}:{remoteaddr}"
 * (the name of the resource won't be "check_nrpe" but the remote plugin name included in its arguments).
 *
 * Default entity types may be superseded in any case by including in the service definition the aforementioned custom
 * variable "_entity_type".
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include "config.h"
#include "nebstructs.h"
#include "nebmodules.h"
#include "nebcallbacks.h"
#include "neberrors.h"
#include "broker.h"
#include "argument_parser.h"
#include "ngsi_event_broker_common.h"
#include "ngsi_event_broker_xifi.h"


/* specify event broker API version (required) */
NEB_API_VERSION(CURRENT_NEB_API_VERSION)


/**
 * @name Event Broker module identification
 * @{
 */

/** Module name (from package details defined by `config.h`) */
#define MODULE_NAME			PACKAGE_NAME "_xifi"

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
	neb_set_module_info(module_handle, NEBMODULE_MODINFO_TITLE,   module_name);
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
		result = ADAPTER_REQUEST_INVALID;
	} else if (serv == NULL) {
		logging(LOG_WARN, context, "Cannot get plugin service details");
		result = ADAPTER_REQUEST_INVALID;
	} else {
		const char*		type = NULL;
		customvariablesmember*	vars = serv->custom_variables;

		/* get entity type, either explicitly from custom variable or implicitly, assuming that:
		 * - SNMP plugin implies NPM monitoring
		 * - NRPE plugin implies DEM remote instance monitoring
		 * - None of the above implies DEM local host monitoring
		 */
		if ((vars != NULL) && !strcmp(vars->variable_name, CUSTOM_VAR_ENTITY_TYPE)) {
			type = vars->variable_value;
			logging(LOG_DEBUG, context, "Explicit entity type %s from custom variables", type);
		} else if (!strcmp(name, SNMP_PLUGIN)) {
			type = NPM_DEFAULT_ENTITY_TYPE;
			logging(LOG_DEBUG, context, "Implicit entity type %s for plugin %s", type, name);
		} else if (nrpe) {
			type = DEM_ENTITY_TYPE_HOST_REMOTE;
			logging(LOG_DEBUG, context, "Implicit entity type %s when using NRPE", type);
		} else {
			type = DEM_ENTITY_TYPE_HOST_LOCAL;
			logging(LOG_DEBUG, context, "Implicit entity type %s for local plugin %s", type, name);
		}

		/* get request URL */
		if (!strcmp(type, SRV_DEFAULT_ENTITY_TYPE)) {
			result = srv_get_adapter_request(context, name, args, type, serv);
		} else if (!strcmp(type, NPM_DEFAULT_ENTITY_TYPE)) {
			result = npm_get_adapter_request(context, name, args, type);
		} else {
			result = dem_get_adapter_request(context, name, args, type, nrpe);
		}
	}

	free(args);
	args = NULL;
	free(name);
	name = NULL;
	return result;
}


/* [NPM monitoring] gets adapter request URL */
char* npm_get_adapter_request(context_t* context, char* name, char* args, const char* type)
{
	char*		result = NULL;
	option_list_t	opts   = NULL;

	/* Take adapter query fields from plugin arguments */
	if ((opts = parse_args(args, ":H:C:o:m:")) == NULL) {
		logging(LOG_WARN, context, "Cannot get plugin options");
		result = ADAPTER_REQUEST_INVALID;
	} else {
		char*	host = NULL;
		int	port = -1;
		size_t	i;

		/* For interface 10.11.100.80/20, check_snmp plugin options should
		   look like "-H 10.11.100.80 -o .1.3.6.1.2.1.2.2.1.8.21", where
		   last number of -o option is port number + 1 */
		for (i = 0; opts[i].opt != -1; i++) {
			switch(opts[i].opt) {
				case 'H': {
					host = (char*) opts[i].val;
					break;
				}
				case 'o': {
					char* ptr = strrchr(opts[i].val, '.');
					port = (ptr) ? (atoi(++ptr) - 1) : -1;
					break;
				}
			}
		}
		if ((host == NULL) || (port == -1)) {
			logging(LOG_WARN, context, "Missing plugin options");
			result = ADAPTER_REQUEST_INVALID;
		} else {
			char buffer[MAXBUFLEN];
			snprintf(buffer, sizeof(buffer)-1, NPM_ADAPTER_REQUEST_FORMAT,
			         adapter_url, name, region_id, host, port, type);
			buffer[sizeof(buffer)-1] = '\0';
			result = strdup(buffer);
		}
	}

	free_option_list(opts);
	opts = NULL;
	return result;
}


/* [DEM monitoring] gets adapter request URL */
char* dem_get_adapter_request(context_t* context, char* name, char* args, const char* type, int nrpe)
{
	char		buffer[MAXBUFLEN];
	char*		result = NULL;
	option_list_t	opts   = NULL;

	/* Take adapter query fields from plugin arguments, distinguishing
	   between local executions (host_addr as identifier) and NRPE plugin
	   executions (-H plugin argument as identifier) */
	if (!nrpe) {
		char* addr = host_addr;
		snprintf(buffer, sizeof(buffer)-1, DEM_ADAPTER_REQUEST_FORMAT,
		         adapter_url, name, region_id, addr, type);
		buffer[sizeof(buffer)-1] = '\0';
		result = strdup(buffer);
	} else if ((opts = parse_args(args, ":H:nup:t:c:a:")) == NULL) {
		logging(LOG_WARN, context, "Cannot get NRPE plugin options");
		result = ADAPTER_REQUEST_INVALID;
	} else {
		char        addr[INET_ADDRSTRLEN];
		const char* host = NULL;
		size_t      i;

		for (i = 0; opts[i].opt != -1; i++) {
			switch(opts[i].opt) {
				case 'H': {
					host = opts[i].val;
					break;
				}
			}
		}
		if (host == NULL) {
			logging(LOG_WARN, context, "Missing NRPE plugin options");
			result = ADAPTER_REQUEST_INVALID;
		} else if (resolve_address(host, addr, INET_ADDRSTRLEN) == NEB_ERROR) {
			logging(LOG_WARN, context, "Cannot resolve remote address for %s", host);
			result = ADAPTER_REQUEST_INVALID;
		} else {
			snprintf(buffer, sizeof(buffer)-1, DEM_ADAPTER_REQUEST_FORMAT,
			         adapter_url, name, region_id, addr, type);
			buffer[sizeof(buffer)-1] = '\0';
			result = strdup(buffer);
		}
	}

	free_option_list(opts);
	opts = NULL;
	return result;
}


/* [Host service monitoring] gets adapter request URL */
char* srv_get_adapter_request(context_t* context, char* name, char* args, const char* type, const service* serv)
{
	char* result = NULL;
	char  buffer[MAXBUFLEN];

	snprintf(buffer, sizeof(buffer)-1, SRV_ADAPTER_REQUEST_FORMAT,
	         adapter_url, name, region_id, serv->host_name, serv->description, type);
	buffer[sizeof(buffer)-1] = '\0';

	result = strdup(buffer);
	return result;
}
