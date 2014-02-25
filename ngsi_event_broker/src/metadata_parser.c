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


#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "yajl/yajl_gen.h"
#include "yajl/yajl_parse.h"
#include "metadata_parser.h"


/*
 * OpenStack Metadata Service
 * [http://docs.openstack.org/grizzly/openstack-compute/admin/content/metadata-service.html]
 *
 * Sample JSON:
 *	{
 *	    "availability_zone": "nova",
 *	    "hostname": "test.novalocal",
 *	    "launch_index": 0,
 *	    "meta": {
 *	        "priority": "low",
 *	        "role": "webserver"
 *	    },
 *	    "name": "test",
 *	    "public_keys": {
 *	        "mykey": "ssh-rsa ...\n"
 *	    },
 *	    "uuid": "d8e02d56-2648-49a3-bf97-6be8f1204f38"
 *	}
 *
 * REGIONS SUPPORT:
 * A key "region" should be supplied as part of the server's metadata, invoking
 * the Compute API [http://api.openstack.org/api-ref-compute.html] either on
 * server creation or as an update. Then, sample JSON would look like this:
 *	{
 *	    "availability_zone": "nova",
 *	    "hostname": "test.novalocal",
 *	    "launch_index": 0,
 *	    "meta": {
 *	        "priority": "low",
 *	        "role": "webserver",
 *	        "region": "myregion"
 *	    },
 *	    "name": "test",
 *	    "public_keys": {
 *	        "mykey": "ssh-rsa ...\n"
 *	    },
 *	    "uuid": "d8e02d56-2648-49a3-bf97-6be8f1204f38"
 *	}
 */


#define YAJL_PARSE_CONTINUE	1
#define YAJL_PARSE_CANCEL	0


/* keys for uuid and region */
#define KEY_UUID		"uuid"
#define KEY_REGION		"region"


/* define internal variables */
static host_metadata_t*		yajl_metadata = NULL;
static char*			yajl_last_key = NULL;
static char*			key_uuid_str;
static size_t			key_uuid_len;
static char*			key_region_str;
static size_t			key_region_len;


/* initialize metadata */
static void init_metadata(host_metadata_t* metadata)
{
	assert(metadata != NULL);
	yajl_metadata = metadata;

	metadata->uuid		= NULL;
	key_uuid_str		= KEY_UUID;
	key_uuid_len		= strlen(key_uuid_str);

	metadata->region	= NULL;
	key_region_str		= KEY_REGION;
	key_region_len		= strlen(key_region_str);
}


/* callback for strings */
static int callback_string(void* ctx, const unsigned char* val, unsigned int len)
{
	if (!strncmp(yajl_last_key, key_uuid_str, key_uuid_len)) {
		free(yajl_metadata->uuid);
		yajl_metadata->uuid = strndup((const char*) val, len);
	} else if (!strncmp(yajl_last_key, key_region_str, key_region_len)) {
		free(yajl_metadata->region);
		yajl_metadata->region = strndup((const char*) val, len);
	}
	return YAJL_PARSE_CONTINUE;
}


/* callback for JSON keys */
static int callback_key(void* ctx, const unsigned char* val, unsigned int len)
{
	yajl_last_key = (char*) val;
	return YAJL_PARSE_CONTINUE;
}


/* parser callback functions */
static yajl_callbacks callbacks = {
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	callback_string,
	NULL,
	callback_key,
	NULL,
	NULL,
	NULL
};


/* main parsing function */
int parse_metadata(const char* json, host_metadata_t* metadata)
{
	yajl_handle		handle;
	yajl_gen		gen;
	yajl_gen_config		gencfg = { 0, "  " };
	yajl_parser_config	parcfg = { 0, 0 };
	yajl_status		status;
	int			result = EXIT_SUCCESS;

	init_metadata(metadata);
	gen = yajl_gen_alloc(&gencfg, NULL);
	handle = yajl_alloc(&callbacks, &parcfg, NULL, gen);
	status = yajl_parse(handle, (const unsigned char*) json, strlen(json));
	if (status != yajl_status_ok && status != yajl_status_insufficient_data) {
		result = EXIT_FAILURE;
	} else {
		yajl_gen_clear(gen);
	}

	status = yajl_parse_complete(handle);
	result = result || (status != yajl_status_ok);
	yajl_gen_free(gen);
	yajl_free(handle);
	return result;
}


/* free metadata resources */
void free_metadata(host_metadata_t* metadata)
{
	assert(metatada != NULL);
	free(metadata->uuid);
	metadata->uuid = NULL;
	free(metadata->region);
	metadata->region = NULL;
	yajl_metadata = NULL;
	yajl_last_key = NULL;
}
