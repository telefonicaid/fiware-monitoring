/*
 * Copyright 2013 Telefónica I+D
 * All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License. You may obtain
 * a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include "config.h"
#include "nebstructs.h"
#include "nebmodules.h"
#include "nebcallbacks.h"
#include "broker.h"
#include "argument_parser.h"
#include "ngsi_event_broker_common.h"


/* specify event broker API version (required) */
NEB_API_VERSION(CURRENT_NEB_API_VERSION)


/* default attributes for entity being monitored */
#define ENTITY_TYPE_LOCAL		"host"
#define ENTITY_TYPE_REMOTE		"vm"


/* event broker module identification */
#define MODULE_NAME			PACKAGE_NAME "_host"	/* from config.h */
#define MODULE_VERSION			PACKAGE_VERSION		/* from config.h */


/* define module constants and variables */
char* const module_name			= MODULE_NAME;
char* const module_version		= MODULE_VERSION;
void*       module_handle		= NULL;


/* initializes module handle and info (name and version) */
int init_module_handle_info(void* handle)
{
	int result = 0;

	module_handle = handle;
	logging("info", "%s - Starting up (version %s)...", module_name, module_version);
	neb_set_module_info(module_handle, NEBMODULE_MODINFO_TITLE,   module_name);
	neb_set_module_info(module_handle, NEBMODULE_MODINFO_VERSION, module_version);

	return result;
}


/* gets adapter request URL including query fields */
char* get_adapter_request(nebstruct_service_check_data* data)
{
	char		buffer[MAXBUFLEN];
	char*		result = NULL;
	char*		plugin = NULL;
	char*		args   = NULL;
	option_list_t	opts   = NULL;

	/* Get plugin name: if it is SNMP, then ignore; otherwise, take adapter query fields from
	   plugin arguments, distinguishing between local and NRPE (remote) plugin executions */
	if ((plugin = find_plugin_name(data, &args)) == NULL) {
		logging("error", "%s - Cannot get plugin name", module_name);
		result = ADAPTER_REQUEST_INVALID;
	} else if (!strcmp(plugin, SNMP_PLUGIN)) {
		result = strdup(ADAPTER_REQUEST_IGNORE);
	} else if (strcmp(plugin, NRPE_PLUGIN)) {
		snprintf(buffer, sizeof(buffer)-1, ADAPTER_REQUEST_FORMAT,
		         adapter_url, plugin,
		         region_id, host_addr,
		         ENTITY_TYPE_LOCAL);
		buffer[sizeof(buffer)-1] = '\0';
		result = strdup(buffer);
	} else if ((opts = parse_args(args, ":H:c:t:")) == NULL) {
		logging("error", "%s - Cannot get NRPE plugin options", module_name);
		result = ADAPTER_REQUEST_INVALID;
	} else {
		char        addr[INET_ADDRSTRLEN];
		const char* host = NULL;
		const char* name = NULL;
		size_t      i;
		for (i = 0; opts[i].opt != -1; i++) {
			switch(opts[i].opt) {
				case 'H': {
					host = opts[i].val;
					break;
				}
				case 'c': {
					name = opts[i].val;
					break;
				}
			}
		}
		if (!host || !name) {
			logging("error", "%s - Missing NRPE plugin options", module_name);
			result = ADAPTER_REQUEST_INVALID;
		} else if (resolve_address(host, addr, INET_ADDRSTRLEN)) {
			logging("error", "%s - Cannot resolve remote address for %s", module_name, host);
			result = ADAPTER_REQUEST_INVALID;
		} else {
			snprintf(buffer, sizeof(buffer)-1, ADAPTER_REQUEST_FORMAT,
			         adapter_url, name,
			         region_id, addr,
			         ENTITY_TYPE_REMOTE);
			buffer[sizeof(buffer)-1] = '\0';
			result = strdup(buffer);
		}
	}

	free(opts);
	free(args);
	free(plugin);
	return result;
}
