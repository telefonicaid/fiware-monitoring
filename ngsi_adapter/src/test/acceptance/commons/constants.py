# -*- coding: utf-8 -*-

# Copyright 2015 Telefonica Investigación y Desarrollo, S.A.U
#
# This file is part of FI-WARE project.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
#
# You may obtain a copy of the License at:
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
# WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#
# See the License for the specific language governing permissions and
# limitations under the License.
#
# For those usages not covered by the Apache version 2.0 License please
# contact with opensource@tid.es

__author__ = 'jfernandez'


# HEADERS
HEADER_CONTENT_TYPE = u'content-type'
HEADER_ACCEPT = u'accept'
HEADER_REPRESENTATION_JSON = u'application/json'
HEADER_REPRESENTATION_XML = u'application/xml'
HEADER_REPRESENTATION_TEXTPLAIN = u'text/plain'
HEADER_AUTH_TOKEN = u'X-Auth-Token'
HEADER_TENANT_ID = u'Tenant-Id'
HEADER_TRANSACTION_ID = u'txid'

# HTTP VERBS
HTTP_VERB_POST = 'post'
HTTP_VERB_GET = 'get'
HTTP_VERB_PUT = 'put'
HTTP_VERB_DELETE = 'delete'
HTTP_VERB_UPDATE = 'update'

# TRANSACTION ID
TRANSACTION_ID_PATTERN = "qa/{uuid}"

# NGSI CLIENT
NGSI_ADAPTER_URI_BASE = "{api_root_url}"
NGSI_ADAPTER_URI_PARSER = NGSI_ADAPTER_URI_BASE + "/{parser_name}"
NGSI_ADAPTER_PARAMETER_ID = "id"
NGSI_ADAPTER_PARAMETER_TYPE = "type"

# CONFIGURATION PROPERTIES
PROPERTIES_FILE = "properties.json"
PROPERTIES_CONFIG_ENV = "environment"
PROPERTIES_CONFIG_ENV_NAME = "name"
PROPERTIES_CONFIG_ENV_LOGS_PATH = "log_path"
PROPERTIES_CONFIG_ENV_LOCAL_PATH_REMOTE_LOGS = "local_path_remote_logs"
MONITORING_CONFIG_ENV_DEFAULT_PARSER = "default_parser"
MONITORING_CONFIG_ENV_DEFAULT_PARSER_DATA = "default_parser_data"
MONITORING_CONFIG_ENV_DEFAULT_PARSER_PARAMS = "default_parser_parameters"
MONITORING_CONFIG_SERVICE_ADAPTER = "monitoring_adapter_service"
MONITORING_CONFIG_SERVICE_PROTOCOL = "protocol"
MONITORING_CONFIG_SERVICE_HOST = "host"
MONITORING_CONFIG_SERVICE_PORT = "port"
MONITORING_CONFIG_SERVICE_RESOURCE = "resource"
MONITORING_CONFIG_SERVICE_PRIVATEKEY = "private_key_location"
MONITORING_CONFIG_SERVICE_HOSTUSER = "host_user"
MONITORING_CONFIG_SERVICE_HOSTPASSWORD = "host_password"
MONITORING_CONFIG_SERVICE_LOG_PATH = "service_log_path"
MONITORING_CONFIG_SERVICE_LOG_FILE_NAME = "service_log_file_name"

# RESOURCES
RESOURCES_SAMPLEDATA_MODULE = "resources.probe_sample_data"
RESOURCES_PARAMETER_PATTERN = "${param_name}"
