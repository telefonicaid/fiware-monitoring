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
#define ENTITY_TYPE			"interface"


/* adapter request query string fields (id = region:hostaddr/port) */
#define ADAPTER_REQUEST_SNMP_FORMAT	"%s/%s" \
					"?" ADAPTER_QUERY_FIELD_ID "=%s:%s/%d" \
					"&" ADAPTER_QUERY_FIELD_TYPE "=%s"


/* event broker module identification */
#define MODULE_NAME			PACKAGE_NAME "_snmp"	/* from config.h */
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
	char*		result = NULL;
	char*		plugin = NULL;
	char*		args   = NULL;
	option_list_t	opts   = NULL;

	/* Get plugin name: if it is not SNMP, then ignore; otherwise, take
	   query fields from plugin arguments and set request */
	if ((plugin = find_plugin_name(data, &args)) == NULL) {
		logging("error", "%s - Cannot get plugin name", module_name);
		result = ADAPTER_REQUEST_INVALID;
	} else if (strcmp(plugin, SNMP_PLUGIN)) {
		result = strdup(ADAPTER_REQUEST_IGNORE);
	} else if ((opts = parse_args(args, ":H:C:o:m:")) == NULL) {
		logging("error", "%s - Cannot get plugin options", module_name);
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
					host = opts[i].val;
					break;
				}
				case 'o': {
					char* ptr = strrchr(opts[i].val, '.');
					port = (ptr) ? (atoi(++ptr) - 1) : -1;
					break;
				}
			}
		}
		if (!host || (port == -1)) {
			logging("error", "%s - Missing plugin options", module_name);
			result = ADAPTER_REQUEST_INVALID;
		} else {
			char buffer[MAXBUFLEN];
			snprintf(buffer, sizeof(buffer)-1, ADAPTER_REQUEST_SNMP_FORMAT,
			         adapter_url, plugin,
			         region_id, host, port,
			         ENTITY_TYPE);
			buffer[sizeof(buffer)-1] = '\0';
			result = strdup(buffer);
		}
	}

	free(opts);
	free(args);
	free(plugin);
	return result;
}
