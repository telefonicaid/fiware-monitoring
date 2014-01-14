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
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/param.h>
#include "config.h"
#include "metadata_parser.h"
#include "nebstructs.h"
#include "nebmodules.h"
#include "nebcallbacks.h"
#include "broker.h"
#include "curl/curl.h"


#define MODULE_NAME			PACKAGE_NAME	/* from config.h */
#define MODULE_VERSION			PACKAGE_VERSION	/* from config.h */
#define MAXBUFLEN			512


/* default attributes for entity being monitored */
#ifndef ENTITY_TYPE
#define ENTITY_TYPE			"host"
#endif /*ENTITY_TYPE*/


/* adapter request query string fields (id = region:uniqueid) */
#define ADAPTER_QUERY_FIELD_ID		"id"
#define ADAPTER_QUERY_FIELD_TYPE	"type"
#define ADAPTER_QUERY_FORMAT		ADAPTER_QUERY_FIELD_ID "=%s:%s" \
					"&" ADAPTER_QUERY_FIELD_TYPE "=%s"


/* metadata service from OpenStack */
#define OPENSTACK_METADATA_SERVICE_URL	"http://169.254.169.254/openstack/2012-08-10/meta_data.json"
#define OPENSTACK_MIN_REQ_VERSION	"Folsom"


/* specify event broker API version (required) */
NEB_API_VERSION(CURRENT_NEB_API_VERSION)


/* Nagios internal symbols */
extern int   __nagios_object_structure_version;


/* declare module functions */
static int   init_module_variables(char* args);
static int   free_module_variables(void);
static int   check_nagios_object_version(void);
static int   callback_service_check(int callback_type, void* data);
static char* find_plugin(nebstruct_service_check_data* data);
static char* get_adapter_request_query(void);
static void  logging(const char* level, const char* format, ...);


/* define global variables */
static void* module_handle = NULL;
static char* adapter_url   = NULL;
static char* adapter_query = NULL;


/* deinitialization */
int nebmodule_deinit(int flags, int reason)
{
	int result = 0;

	curl_global_cleanup();
	free_module_variables();

	if (reason != NEBMODULE_ERROR_BAD_INIT) {
		logging("info", "%s - Finished", MODULE_NAME);
	}

	return result;
}


/* initialization */
int nebmodule_init(int flags, char* args, void* handle)
{
	int result = 0;

	logging("info", "%s - Starting up (version %s)...", MODULE_NAME, MODULE_VERSION);
	module_handle = handle;

	if (check_nagios_object_version()) {
		result = -1;
	} else if (curl_global_init(CURL_GLOBAL_ALL)) {
		logging("error", "%s - Could not initialize libcurl", MODULE_NAME);
		result = -1;
	} else if (init_module_variables(args)) {
		result = -1;
	} else {
		result = neb_register_callback(NEBCALLBACK_SERVICE_CHECK_DATA,
		                               module_handle, 0, callback_service_check);
	}

	if (!result) {
		neb_set_module_info(module_handle, NEBMODULE_MODINFO_TITLE,   MODULE_NAME);
		neb_set_module_info(module_handle, NEBMODULE_MODINFO_VERSION, MODULE_VERSION);
	} else {
		nebmodule_deinit(0, NEBMODULE_ERROR_BAD_INIT);
	}

	return result;
}


/************************************************************************/


/* checks to make sure Nagios object version matches what we know about */
int check_nagios_object_version(void)
{
	int result = 0;

	if (__nagios_object_structure_version != CURRENT_OBJECT_STRUCTURE_VERSION) {
		logging("error", "%s - Nagios object version mismatch: %d,%d", MODULE_NAME,
		        __nagios_object_structure_version, CURRENT_OBJECT_STRUCTURE_VERSION);
		result = 1;
	}

	return result;
}


/* initializes module variables */
int init_module_variables(char* args)
{
	int result = 0;

	/* process arguments passed to module in Nagios configuration file */
	if (args == NULL) {
		logging("error", "%s - Missing adapter URL required as module argument", MODULE_NAME);
		result = 1;
	} else {
		/* adapter URL */
		size_t len = strlen(adapter_url = strdup(args));
		if ((len > 0) && (adapter_url[len-1] == '/')) adapter_url[len-1] = '\0';

		/* adapter query (from metadata of host being monitored) */
		adapter_query = get_adapter_request_query();
		if (adapter_query == NULL) result = 1;
	}

	if (!result) {
		logging("info", "%s - Adapter URL = %s", MODULE_NAME, adapter_url);
		logging("info", "%s - Adapter query = %s", MODULE_NAME, adapter_query);
	}

	return result;
}


/* deinitializes module variables */
int free_module_variables(void)
{
	free(adapter_url);
	free(adapter_query);
	return 0;
}


/* writes a formatted string to Nagios logs */
void logging(const char* level, const char* format, ...)
{
	char	buffer[MAXBUFLEN];
	va_list	ap;

	va_start(ap, format);
	vsnprintf(buffer, sizeof(buffer)-1, format, ap);
	buffer[sizeof(buffer)-1] = '\0';
	va_end(ap);

	write_to_all_logs(buffer, NSLOG_INFO_MESSAGE);
}


/* gets the name of the executed plugin */
char* find_plugin(nebstruct_service_check_data* data)
{
	service* check_service	= NULL;
	command* check_command	= NULL;
	char*    check_plugin	= NULL;
	char*    ptr;

	if ((check_service = find_service(data->host_name, data->service_description)) != NULL) {
		char* service_check_command = strdup(check_service->service_check_command);
		strtok_r(service_check_command, "!", &ptr);
		if ((check_command = find_command(service_check_command)) != NULL) {
			char* command_line = strdup(check_command->command_line);
			strtok_r(command_line, " ", &ptr);
			ptr = strrchr(command_line, '/');
			check_plugin = strdup((ptr) ? ptr+1 : command_line);
			free(command_line);
		}
		free(service_check_command);
	}

	return check_plugin;
}


/* Nagios service check callback */
int callback_service_check(int callback_type, void* data)
{
	int result = 0;

	nebstruct_service_check_data*	check_data	= NULL;
	char*				check_name	= NULL;
	struct curl_slist*		curl_headers	= NULL;
	CURL*				curl_handle	= NULL;
	CURLcode			curl_result;

	assert(callback_type == NEBCALLBACK_SERVICE_CHECK_DATA);
	check_data = (nebstruct_service_check_data*) data;

	/* Process output only AFTER plugin is executed */
	if (check_data->type != NEBTYPE_SERVICECHECK_PROCESSED) {
		return result;
	}

	/* Async POST request to adapter */
	if ((curl_handle = curl_easy_init()) == NULL) {
		logging("error", "%s - Cannot open HTTP session", MODULE_NAME);
	} else if ((check_name = find_plugin(check_data)) == NULL) {
		logging("error", "%s - Cannot get plugin name", MODULE_NAME);
	} else {
		char request_url[MAXBUFLEN];
		char request_txt[MAXBUFLEN];

		snprintf(request_url, sizeof(request_url)-1, "%s/%s?%s",
		         adapter_url, check_name, adapter_query);
		request_url[sizeof(request_url)-1] = '\0';
		free(check_name);

		snprintf(request_txt, sizeof(request_txt)-1, "%s|%s",
		         check_data->output, check_data->perf_data);
		request_txt[sizeof(request_txt)-1] = '\0';

		curl_headers = curl_slist_append(curl_headers, "Content-Type: text/plain");
		curl_easy_setopt(curl_handle, CURLOPT_URL, request_url);
		curl_easy_setopt(curl_handle, CURLOPT_POST, 1);
		curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS, request_txt);
		curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDSIZE, strlen(request_txt));
		curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER, curl_headers);
		if ((curl_result = curl_easy_perform(curl_handle)) == CURLE_OK) {
			logging("info", "%s - Request sent to NGSI adapter", MODULE_NAME);
		} else {
			logging("error", "%s - Could not send request to adapter: %s",
			        MODULE_NAME, curl_easy_strerror(curl_result));
		}
		curl_slist_free_all(curl_headers);
		curl_easy_cleanup(curl_handle);
	}

	return result;
}


/* gets adapter query fields from metadata of host being monitored */
char* get_adapter_request_query()
{
	char*    result = NULL;

	size_t   curl_callback(char*, size_t, size_t, void*);
	CURL*    curl_handle = NULL;
	CURLcode curl_result;

	/* POST request to OpenStack metadata service */
	if ((curl_handle = curl_easy_init()) == NULL) {
		logging("error", "%s - Cannot open HTTP session", MODULE_NAME);
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
				        MODULE_NAME, OPENSTACK_MIN_REQ_VERSION);
			/* check whether region is supplied as metadata */
			} else if (metadata.region == NULL) {
				logging("error", "%s - No region supplied as part of metadata",
				        MODULE_NAME);
			/* format adapter query */
			} else {
				snprintf(buffer, sizeof(buffer)-1, ADAPTER_QUERY_FORMAT,
				        metadata.region, metadata.uuid,
				        ENTITY_TYPE);
				buffer[sizeof(buffer)-1] = '\0';
				result = strdup(buffer);
			}
			free_metadata(&metadata);
		} else {
			logging("error", "%s - Could not access OpenStack metadata service: %s",
			        MODULE_NAME, curl_easy_strerror(curl_result));
		}
		curl_easy_cleanup(curl_handle);
	}

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
