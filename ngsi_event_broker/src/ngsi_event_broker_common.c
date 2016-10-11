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
 * @file   ngsi_event_broker_common.c
 * @brief  NGSI Event Broker common implementation
 *
 * This file defines some functions that are common to any specific implementation
 * of this [Event Broker](@NagiosModule_ref) module.
 */


#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <limits.h>
#include <time.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "config.h"
#include "nebstructs.h"
#include "nebmodules.h"
#include "nebcallbacks.h"
#include "neberrors.h"
#include "macros.h"
#include "broker.h"
#include "curl/curl.h"
#include "argument_parser.h"
#include "ngsi_event_broker_common.h"


/**
 * @name Global variables definition
 * @{
 */

char*			adapter_url = NULL;
char*			region_id   = NULL;
char*			host_addr   = NULL;
loglevel_t		log_level   = LOG_INFO;

/**@}*/


/* deinitializes the module */
int nebmodule_deinit(int flags, int reason)
{
	int		result = NEB_OK;
	context_t	context = { .op = "Exit" };

	curl_global_cleanup();
	free_module_variables();

	if (reason != NEBMODULE_ERROR_BAD_INIT) {
		logging(LOG_INFO, &context, "Finishing...");
	}

	return result;
}


/* initializes the module */
int nebmodule_init(int flags, char* args, void* handle)
{
	int		result  = NEB_OK;
	context_t	context = { .op = "Init" };

	init_module_handle_info(handle, &context);
	if (check_nagios_object_version(&context) != NEB_OK) {
		result = NEB_ERROR;
	} else if (init_module_variables(args, &context) != NEB_OK) {
		result = NEB_ERROR;
	} else if (curl_global_init(CURL_GLOBAL_ALL) != CURLE_OK) {
		logging(LOG_ERROR, &context, "Could not initialize libcurl");
		result = NEB_ERROR;
	} else {
		result = neb_register_callback(NEBCALLBACK_SERVICE_CHECK_DATA,
		                               module_handle, 0, callback_service_check);
	}

	/* check for errors in initialization */
	if (result != NEB_OK) {
		nebmodule_deinit(0, NEBMODULE_ERROR_BAD_INIT);
	}

	return result;
}


/* checks to make sure Nagios object version matches what we know about */
int check_nagios_object_version(context_t* context)
{
	int result = NEB_OK;

	if (__nagios_object_structure_version != CURRENT_OBJECT_STRUCTURE_VERSION) {
		logging(LOG_ERROR, context, "Nagios object version mismatch: %d,%d",
		        __nagios_object_structure_version, CURRENT_OBJECT_STRUCTURE_VERSION);
		result = NEB_ERROR;
	}

	return result;
}


/* initializes module variables */
int init_module_variables(char* args, context_t* context)
{
	char		name[HOST_NAME_MAX];
	char		addr[INET_ADDRSTRLEN];
	option_list_t	opts	= NULL;
	int		result	= NEB_OK;

	/* process arguments passed to module in Nagios configuration file */
	if ((opts = parse_args(args, ":u:r:l:")) != NULL) {
		size_t	i;
		for (i = 0; opts[i].opt != NO_CHAR; i++) {
			switch(opts[i].opt) {
				case 'u': { /* adapter URL without trailing slash */
					size_t len = strlen(adapter_url = STRDUP(opts[i].val));
					if ((len > 0) && (adapter_url[len-1] == '/')) adapter_url[len-1] = '\0';
					break;
				}
				case 'r': { /* region id */
					region_id = STRDUP(opts[i].val);
					break;
				}
				case 'l': { /* loglevel */
					size_t lvl;
					char** ptr = (char**) loglevel_names;
					for (lvl = 0; *ptr && strcmp(*ptr, opts[i].val); ptr++, lvl++);
					if (*ptr) log_level = lvl;
					break;
				}
				case MISSING_VALUE: {
					logging(LOG_ERROR, context, "Missing value for option -%c", (char) opts[i].err);
					break;
				}
				case UNKNOWN_OPTION: {
					logging(LOG_ERROR, context, "Unrecognized option -%c", (char) opts[i].err);
					break;
				}
				default: {
					logging(LOG_ERROR, context, "Unhandled option -%c", (char) opts[i].opt);
				}
			}
		}
	}

	if (!adapter_url || !region_id) {
		logging(LOG_ERROR, context, "Missing required broker module options");
		result = NEB_ERROR;
	} else if (gethostname(name, HOST_NAME_MAX)) {
		logging(LOG_ERROR, context, "Cannot get localhost name");
		result = NEB_ERROR;
	} else if (resolve_address(name, addr, INET_ADDRSTRLEN)) {
		logging(LOG_ERROR, context, "Cannot get localhost address");
		result = NEB_ERROR;
	} else {
		host_addr = STRDUP(addr); /* keep a global copy of addr string */
	}

	free_option_list(opts);
	opts = NULL;

	if (result == NEB_ERROR) {
		logging(LOG_ERROR, context, "Deinitializing broker module...");
	} else {
		logging(LOG_INFO, context, "{"
			" \"adapter_url\": \"%s\","
			" \"region_id\": \"%s\","
			" \"host_addr\": \"%s\""
			" }",
			adapter_url, region_id, host_addr);
	}

	return result;
}


/* deinitializes module variables */
int free_module_variables(void)
{
	free(adapter_url);
	adapter_url = NULL;
	free(region_id);
	region_id = NULL;
	free(host_addr);
	host_addr = NULL;
	return NEB_OK;
}


/* writes a formatted string to Nagios log */
void logging(loglevel_t level, context_t* context, const char* format, ...)
{
	if (level <= log_level) {
		char	buffer[MAXBUFLEN];
		size_t	len;
		va_list	ap;

		len = snprintf(buffer, sizeof(buffer)-1, "lvl=%s | corr=%s | comp=%s | op=%s | msg=",
			loglevel_names[level],
			(context && context->corr) ? context->corr : "n/a",
			module_name,
			(context && context->op) ? context->op : "n/a");
		va_start(ap, format);
		len += vsnprintf(buffer+len, sizeof(buffer)-len-1, format, ap);
		buffer[len] = '\0';
		va_end(ap);

		write_to_log(buffer, NSLOG_INFO_MESSAGE, NULL);
	}
}


/* resolves the IP address of a hostname */
int resolve_address(const char* hostname, char* addr, size_t addrmaxlen)
{
	int result = NEB_OK;

	struct hostent* hostent = gethostbyname(hostname);
	if (!hostent || (inet_ntop(AF_INET, hostent->h_addr_list[0], addr, addrmaxlen) == NULL)) {
		result = NEB_ERROR;
	}

	return result;
}


/* gets the command name, arguments and other details of the executed plugin */
char* find_plugin_command_name(nebstruct_service_check_data* data, char** args, int* nrpe, const service** serv)
{
	host*    check_host	= NULL;
	service* check_service	= NULL;
	command* check_command	= NULL;
	char*    result		= NULL;
	int      is_nrpe	= 0;

	if (((check_host = find_host(data->host_name)) != NULL)
	    && ((check_service = find_service(data->host_name, data->service_description)) != NULL)) {
		char* ptr;
		char* last;
		char* command_name;
		char* command_args;
		char* service_check_command = STRDUP(SERVICE_CHECK_COMMAND(check_service));
		command_name = strtok_r(service_check_command, "!", &command_args);
		/* finds plugin command */
		if ((check_command = find_command(command_name)) != NULL) {
			/* fill in plugin arguments */
			if (args != NULL) {
				nagios_macros	mac;
				char*		raw = NULL;
				char*		cmd = NULL;
				memset(&mac, 0, sizeof(mac));
				grab_host_macros_r(&mac, check_host);
				grab_service_macros_r(&mac, check_service);
				get_raw_command_line_r(&mac, check_command,
				                       check_service->service_check_command,
				                       &raw, 0);
				if (raw == NULL) {
					*args = NULL;
				} else {
					char* exec;
					process_macros_r(&mac, raw, &cmd, 0);
					strtok_r(cmd, " \t", &last);
					ptr = strrchr(cmd, '/');
					exec = (ptr) ? ++ptr : cmd;
					*args = STRDUP(last);
					is_nrpe = !strcmp(exec, NRPE_PLUGIN);
				}
				my_free(raw);
				my_free(cmd);
			}
			/* command name (after resolving NRPE remote command) */
			result = STRDUP((is_nrpe) ? command_args : command_name);
		}
		free(service_check_command);
		service_check_command = NULL;
	}
	/* output arguments */
	if (nrpe != NULL) *nrpe = is_nrpe;
	if (serv != NULL) *serv = check_service;
	return result;
}


/* Nagios service check callback */
int callback_service_check(int callback_type, void* data)
{
	int				result		= NEB_OK;
	nebstruct_service_check_data*	check_data	= NULL;
	char*				request_url	= NULL;
	struct curl_slist*		curl_headers	= NULL;
	CURL*				curl_handle	= NULL;
	CURLcode			curl_result	= CURLE_OK;

	#define HDRLEN			MAXBUFLEN
	#define HDRSTRING		CORRELATOR_HTTP_HEADER ": " "n/a"
	#define HDRTXOFFSET		(CORRELATOR_HTTP_HEADER_LEN + 2)	/* includes length of ": " separator */
	#define CORRELATOR_PREFIX	"......"				/* six chars for the l64a prefix     */
	#define CORRELATOR_PATTERN	"XXXXXX"				/* six chars for the mktemp pattern  */

	char				corrHdr[HDRLEN]	= HDRSTRING;
	char*				corrPrefix	= NULL;
	char*				correlator	= corrHdr + HDRTXOFFSET;
	const char*			operation	= "NGSIAdapter";
	context_t			context		= { .corr = correlator, .op = operation };

	assert(strlen(CORRELATOR_HTTP_HEADER) == CORRELATOR_HTTP_HEADER_LEN);

	assert(callback_type == NEBCALLBACK_SERVICE_CHECK_DATA);
	check_data = (nebstruct_service_check_data*) data;

	/* Process output only AFTER plugin is executed */
	if (check_data->type != NEBTYPE_SERVICECHECK_PROCESSED) {
		return result;
	}

	/* Generate correlator to include in a HTTP header for the request */
	strncpy(correlator, CORRELATOR_PREFIX "" CORRELATOR_PATTERN, HDRLEN - HDRTXOFFSET - 1);
	corrPrefix = l64a((long) time(NULL));
	mktemp(correlator);
	memcpy(correlator, corrPrefix, strlen(corrPrefix));
	corrHdr[HDRLEN - 1] = '\0';
	logging(LOG_DEBUG, &context, "New service check");

	/* Async POST request to NGSI Adapter */
	if ((request_url = get_adapter_request(check_data, &context)) == ADAPTER_REQUEST_INVALID) {
		logging(LOG_ERROR, &context, "Cannot set adapter request URL");
	} else if (!strcmp(request_url, ADAPTER_REQUEST_IGNORE)) {
		/* nothing to do: plugin is ignored */
	} else if ((curl_handle = curl_easy_init()) == NULL) {
		logging(LOG_ERROR, &context, "Cannot open HTTP session");
	} else {
		char request_txt[MAXBUFLEN];
		snprintf(request_txt, sizeof(request_txt)-1, "%s|%s",
		         check_data->output, check_data->perf_data);
		request_txt[sizeof(request_txt)-1] = '\0';
		curl_headers = curl_slist_append(curl_headers, "Content-Type: text/plain");
		curl_headers = curl_slist_append(curl_headers, corrHdr);
		curl_easy_setopt(curl_handle, CURLOPT_URL, request_url);
		curl_easy_setopt(curl_handle, CURLOPT_POST, 1);
		curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS, request_txt);
		curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDSIZE, strlen(request_txt));
		curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER, curl_headers);
		if ((curl_result = curl_easy_perform(curl_handle)) == CURLE_OK) {
			logging(LOG_INFO, &context, "Request sent to %s",
			        request_url);
		} else {
			logging(LOG_WARN, &context, "Request to %s failed: %s",
			        request_url, curl_easy_strerror(curl_result));
		}
		curl_slist_free_all(curl_headers);
		curl_easy_cleanup(curl_handle);
		curl_handle = NULL;
	}
	free(request_url);
	request_url = NULL;
	return result;
}
