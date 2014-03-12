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
#include "neberrors.h"
#include "broker.h"
#include "argument_parser.h"
#include "ngsi_event_broker_common.h"


/* specify event broker API version (required) */
NEB_API_VERSION(CURRENT_NEB_API_VERSION)


/* event broker module identification */
#define MODULE_NAME			PACKAGE_NAME "_xifi"	/* from config.h */
#define MODULE_VERSION			PACKAGE_VERSION		/* from config.h */


/* [host monitoring] default entity types */
#define HOST_DEFAULT_ENTITY_TYPE_LOCAL	"host"
#define HOST_DEFAULT_ENTITY_TYPE_REMOTE	"vm"


/* [host monitoring] adapter request query string fields (id = region:hostaddr) */
#define HOST_ADAPTER_REQUEST_FORMAT	ADAPTER_REQUEST_FORMAT


/* [snmp monitoring] default entity type */
#define SNMP_DEFAULT_ENTITY_TYPE	"interface"


/* [snmp monitoring] adapter request query string fields (id = region:hostaddr/port) */
#define SNMP_ADAPTER_REQUEST_FORMAT	"%s/%s" \
					"?" ADAPTER_QUERY_FIELD_ID "=%s:%s/%d" \
					"&" ADAPTER_QUERY_FIELD_TYPE "=%s"


/* define module constants and variables */
char* const module_name			= MODULE_NAME;
char* const module_version		= MODULE_VERSION;
void*       module_handle		= NULL;


/* initializes module handle and info (name and version) */
int init_module_handle_info(void* handle)
{
	int result = NEB_OK;

	module_handle = handle;
	logging("info", "%s - Starting up (version %s)...", module_name, module_version);
	neb_set_module_info(module_handle, NEBMODULE_MODINFO_TITLE,   module_name);
	neb_set_module_info(module_handle, NEBMODULE_MODINFO_VERSION, module_version);

	return result;
}


/* gets adapter request URL including query fields */
char* get_adapter_request(nebstruct_service_check_data* data)
{
	char* result = NULL;
	char* plugin = NULL;
	char* args   = NULL;

	/* Plugin-specific functions */
	char* host_get_adapter_request(char* plugin, char* args);
	char* snmp_get_adapter_request(char* plugin, char* args);

	/* Build request according to plugin name */
	if ((plugin = find_plugin_name(data, &args)) == NULL) {
		logging("error", "%s - Cannot get plugin name", module_name);
		result = ADAPTER_REQUEST_INVALID;
	} else if (!strcmp(plugin, SNMP_PLUGIN)) {
		result = snmp_get_adapter_request(plugin, args);
	} else {
		result = host_get_adapter_request(plugin, args);
	}

	free(args);
	args = NULL;
	free(plugin);
	plugin = NULL;
	return result;
}


/* [snmp monitoring] gets adapter request URL */
char* snmp_get_adapter_request(char* plugin, char* args)
{
	char*		result = NULL;
	option_list_t	opts   = NULL;

	/* Take adapter query fields from plugin arguments */
	if ((opts = parse_args(args, ":H:C:o:m:")) == NULL) {
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
			logging("error", "%s - Missing plugin options", module_name);
			result = ADAPTER_REQUEST_INVALID;
		} else {
			char buffer[MAXBUFLEN];
			snprintf(buffer, sizeof(buffer)-1, SNMP_ADAPTER_REQUEST_FORMAT,
			         adapter_url, plugin,
			         region_id, host, port,
			         SNMP_DEFAULT_ENTITY_TYPE);
			buffer[sizeof(buffer)-1] = '\0';
			result = strdup(buffer);
		}
	}

	free(opts);
	opts = NULL;
	return result;
}


/* [host monitoring] gets adapter request URL */
char* host_get_adapter_request(char* plugin, char* args)
{
	char		buffer[MAXBUFLEN];
	char*		result = NULL;
	option_list_t	opts   = NULL;

	/* Take adapter query fields from plugin arguments, distinguishing
	   between local and NRPE (remote) plugin executions */
	if (strcmp(plugin, NRPE_PLUGIN)) {
		snprintf(buffer, sizeof(buffer)-1, HOST_ADAPTER_REQUEST_FORMAT,
		         adapter_url, plugin,
		         region_id, host_addr,
		         HOST_DEFAULT_ENTITY_TYPE_LOCAL);
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
		if ((host == NULL) || (name == NULL)) {
			logging("error", "%s - Missing NRPE plugin options", module_name);
			result = ADAPTER_REQUEST_INVALID;
		} else if (resolve_address(host, addr, INET_ADDRSTRLEN)) {
			logging("error", "%s - Cannot resolve remote address for %s", module_name, host);
			result = ADAPTER_REQUEST_INVALID;
		} else {
			snprintf(buffer, sizeof(buffer)-1, HOST_ADAPTER_REQUEST_FORMAT,
			         adapter_url, name,
			         region_id, addr,
			         HOST_DEFAULT_ENTITY_TYPE_REMOTE);
			buffer[sizeof(buffer)-1] = '\0';
			result = strdup(buffer);
		}
	}

	free(opts);
	opts = NULL;
	return result;
}
