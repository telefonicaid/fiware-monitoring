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


#ifndef METADATA_PARSER_H
#define METADATA_PARSER_H


#ifdef __cplusplus
extern "C" {
#endif


typedef struct {
	char* uuid;
	char* availability_zone;
} host_metadata_t;


int parse_metadata(const char* json, host_metadata_t* metadata);
void free_metadata(host_metadata_t* metadata);


#ifdef __cplusplus
}
#endif


#endif /*METADATA_PARSER_H*/
