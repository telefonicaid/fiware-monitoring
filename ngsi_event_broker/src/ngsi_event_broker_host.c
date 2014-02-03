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
#include <sys/param.h>
#include "config.h"
#include "nebstructs.h"
#include "nebmodules.h"
#include "nebcallbacks.h"
#include "broker.h"
#include "curl/curl.h"
#include "metadata_parser.h"
#include "ngsi_event_broker_common.h"


/* specify event broker API version (required) */
NEB_API_VERSION(CURRENT_NEB_API_VERSION)


/* default attributes for entity being monitored */
#define ENTITY_TYPE			"vm"


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

	module_handle  = handle;
	logging("info", "%s - Starting up (version %s)...", module_name, module_version);
	neb_set_module_info(module_handle, NEBMODULE_MODINFO_TITLE,   module_name);
	neb_set_module_info(module_handle, NEBMODULE_MODINFO_VERSION, module_version);

	return result;
}


/* gets adapter request URL including query fields */
char* get_adapter_request(nebstruct_service_check_data* data)
{
	char*    result = NULL;
	char*    plugin = NULL;

	size_t   curl_callback(char*, size_t, size_t, void*);
	CURL*    curl_handle = NULL;
	CURLcode curl_result;

	/* Get plugin name: if it is SNMP, then ignore; otherwise, get adapter
	   query fields (POST request to OpenStack metadata service) and return request */
	if ((plugin = find_plugin_name(data, NULL)) == NULL) {
		logging("error", "%s - Cannot get plugin name", module_name);
		result = ADAPTER_REQUEST_INVALID;
	} else if (!strcmp(plugin, SNMP_PLUGIN)) {
		logging("info",  "%s - Ignoring plugin %s execution", module_name, plugin);
		result = strdup(ADAPTER_REQUEST_IGNORE);
	} else if ((curl_handle = curl_easy_init()) == NULL) {
		logging("error", "%s - Cannot open HTTP session", module_name);
		result = ADAPTER_REQUEST_INVALID;
	} else {
		char* request_url = OPENSTACK_METADATA_SERVICE_URL;
		char  buffer[MAXBUFLEN] = { "" };

		curl_easy_setopt(curl_handle, CURLOPT_URL, request_url);
		curl_easy_setopt(curl_handle, CURLOPT_HTTPGET, 1);
		curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, buffer);
		curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, curl_callback);
		if ((curl_result = curl_easy_perform(curl_handle)) == CURLE_OK) {
			/* check for a valid JSON to detect unsupported OpenStack version */
			host_metadata_t metadata;
			if (parse_metadata(buffer, &metadata)) {
				logging("error", "%s - Invalid OpenStack version (>= %s required)",
				        module_name, OPENSTACK_MIN_REQ_VERSION);
			/* check whether region is supplied as metadata */
			} else if (metadata.region == NULL) {
				logging("error", "%s - No region supplied as part of metadata",
				        module_name);
			/* format adapter request */
			} else {
				snprintf(buffer, sizeof(buffer)-1, ADAPTER_REQUEST_FORMAT,
				        adapter_url, plugin,
				        metadata.region, metadata.uuid,
				        ENTITY_TYPE);
				buffer[sizeof(buffer)-1] = '\0';
				result = strdup(buffer);
			}
			free_metadata(&metadata);
		} else {
			logging("error", "%s - Could not access OpenStack metadata service: %s",
			        module_name, curl_easy_strerror(curl_result));
		}
		curl_easy_cleanup(curl_handle);
	}

	free(plugin);
	return result;
}


size_t curl_callback(char* ptr, size_t size, size_t nmemb, void* userdata)
{
	char*  buffer = (char*) userdata;
	size_t offset = strlen(buffer);
	size_t count  = MIN(MAXBUFLEN - offset - 1, size * nmemb);
	memmove(buffer + offset, ptr, count);
	buffer[offset + count] = '\0';
	return count;
}
