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


#ifndef NGSI_EVENT_BROKER_COMMON_H
#define NGSI_EVENT_BROKER_COMMON_H


#ifdef __cplusplus
extern "C" {
#endif


#include "nebmodules.h"
#include "nebstructs.h"


/* multipurpose buffer length */
#define MAXBUFLEN			512


/* name of the SNMP plugin */
#define SNMP_PLUGIN			"check_snmp"


/* name of the NRPE plugin (remote plugin executions) */
#define NRPE_PLUGIN			"check_nrpe"


/* get_adapter_request() results */
#define ADAPTER_REQUEST_INVALID		NULL
#define ADAPTER_REQUEST_IGNORE		""


/* adapter request query string fields (id = region:uniqueid) */
#define ADAPTER_QUERY_FIELD_ID		"id"
#define ADAPTER_QUERY_FIELD_TYPE	"type"
#define ADAPTER_REQUEST_FORMAT		"%s/%s" \
					"?" ADAPTER_QUERY_FIELD_ID "=%s:%s" \
					"&" ADAPTER_QUERY_FIELD_TYPE "=%s"


/* Nagios internal symbols */
extern int				__nagios_object_structure_version;


/* declare global constants and variables */
extern char* const			module_name;
extern char* const			module_version;
extern void*				module_handle;
extern char*				adapter_url;
extern char*				region_id;
extern char*				host_addr;


/* declare module functions */
int    init_module_handle_info(void* handle);
int    init_module_variables(char* args);
int    free_module_variables(void);
int    check_nagios_object_version(void);
int    callback_service_check(int callback_type, void* data);
char*  find_plugin_name(nebstruct_service_check_data* data, char** args);
char*  find_plugin_command_name(nebstruct_service_check_data* data, char** args, int* nrpe, const service** serv);
char*  get_adapter_request(nebstruct_service_check_data* data);
int    resolve_address(const char* hostname, char* addr, size_t addrmaxlen);
void   logging(const char* level, const char* format, ...);


#ifdef __cplusplus
}
#endif


#endif /*NGSI_EVENT_BROKER_COMMON_H*/
