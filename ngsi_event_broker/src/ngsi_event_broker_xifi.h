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


#ifndef NGSI_EVENT_BROKER_XIFI_H
#define NGSI_EVENT_BROKER_XIFI_H


#ifdef __cplusplus
extern "C" {
#endif


/* custom variables in service definitions */
#define CUSTOM_VAR_ENTITY_TYPE		"ENTITY_TYPE"	/* = _entity_type */


/* [DEM monitoring] entity types for monitored instances*/
#define DEM_ENTITY_TYPE_PHYSICAL	"host"
#define DEM_ENTITY_TYPE_VIRTUAL		"vm"
#define DEM_ENTITY_TYPE_LOCAL		DEM_ENTITY_TYPE_PHYSICAL
#define DEM_ENTITY_TYPE_REMOTE		DEM_ENTITY_TYPE_VIRTUAL
#define DEM_DEFAULT_ENTITY_TYPE		DEM_ENTITY_TYPE_VIRTUAL


/* [NPM monitoring] entity types */
#define NPM_DEFAULT_ENTITY_TYPE		"interface"


/* [Host service monitoring] entity types */
#define SRV_DEFAULT_ENTITY_TYPE		"host_service"


#ifdef __cplusplus
}
#endif


#endif /*NGSI_EVENT_BROKER_XIFI_H*/
