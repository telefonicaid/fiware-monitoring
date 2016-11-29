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


from commons.rest_client_utils import RestClient, API_ROOT_URL_ARG_NAME
from commons.utils import generate_correlator
from commons.logger_utils import get_logger
from commons.constants import HEADER_REPRESENTATION_TEXTPLAIN, HEADER_CONTENT_TYPE, HEADER_CORRELATOR, HTTP_VERB_POST, \
    HEADER_AUTH_TOKEN

NGSI_ADAPTER_URI_BASE = "{" + API_ROOT_URL_ARG_NAME + "}"
NGSI_ADAPTER_URI_PARSER = NGSI_ADAPTER_URI_BASE + "/{probe_name}"
NGSI_ADAPTER_PARAMETER_ID = "id"
NGSI_ADAPTER_PARAMETER_TYPE = "type"

logger = get_logger("rest_client_utils")


class NgsiAdapterClient:

    headers = dict()

    def __init__(self, protocol, host, port, base_resource=None, auth_token=None):
        """
        Class constructor. Init default headers
        :param protocol: API Protocol
        :param host: API Host
        :param port: API Port
        :param base_resource: base uri resource (if exists)
        :param auth_token: Pep Proxy auth token (only if required)
        :return: None
        """

        self.init_headers(auth_token=auth_token)
        self.rest_client = RestClient(protocol, host, port, base_resource)

    def init_headers(self, content_type=HEADER_REPRESENTATION_TEXTPLAIN, correlator=generate_correlator(),
                     auth_token=None):
        """
        Init header to values (or default values)
        :param content_type: Content-Type header value. By default text/plain
        :param correlator: correlator header value. By default, generated value by utils.generate_correlator()
        :param auth_token: Pep Proxy auth token (only if required)
        :return: None
        """

        if content_type is None:
            if HEADER_CONTENT_TYPE in self.headers:
                del(self.headers[HEADER_CONTENT_TYPE])
        else:
            self.headers.update({HEADER_CONTENT_TYPE: content_type})

        if correlator is None:
            if HEADER_CORRELATOR in self.headers:
                del(self.headers[HEADER_CORRELATOR])
        else:
            self.headers.update({HEADER_CORRELATOR: correlator})

        if auth_token is None:
            if HEADER_AUTH_TOKEN in self.headers:
                del(self.headers[HEADER_AUTH_TOKEN])
        else:
            self.headers.update({HEADER_AUTH_TOKEN: auth_token})

    def set_headers(self, headers):
        """
        Set header.
        :param headers: Headers to be used by next request (dict)
        :return: None
        """

        self.headers = headers

    def send_raw_data(self, raw_data, probe_name, entity_id, entity_type):
        """
        Execute a well-formed POST request. All parameters are mandatory
        :param raw_data: Raw probe data to send (string, text/plain)
        :param probe_name: Parser to be used (string)
        :param entity_id: Entity ID (string)
        :param entity_type: Entity Type (string)
        :return: HTTP Request response ('Requests' lib)
        """

        logger.info("Sending raw data to NGSI-Adapter [Probe: %s, EntityId: %, EntityType: %s", probe_name,
                    entity_id, entity_type)
        parameters = dict()
        parameters.update({NGSI_ADAPTER_PARAMETER_ID: entity_id})
        parameters.update({NGSI_ADAPTER_PARAMETER_TYPE: entity_type})
        return self.rest_client.post(uri_pattern=NGSI_ADAPTER_URI_PARSER, body=raw_data, headers=self.headers,
                                     parametersn=parameters, probe_name=probe_name)

    def send_raw_data_custom(self, raw_data, probe_name=None, entity_id=None, entity_type=None,
                             http_method=HTTP_VERB_POST):
        """
        Execute a 'send_data' request (POST request by default). Should support all testing cases.
         The generated request could be malformed (Testing purpose)
         Parameters with None value will not be in the generated request (missing parameter).
        :param raw_data: Raw probe data to send (string, text/plain)
        :param probe_name: Parser to be used (string)
        :param entity_id: Entity ID (string)
        :param entity_type: Entity Type (string)
        :param http_method: send raw data is a HTTP POST request but, for testing purposes could be interesting to use
         another HTTP verb. By default is defined to 'post'
        :return: HTTP Request response ('Requests' lib)
        """

        logger.info("Sending raw data to NGSI-Adapter (custom operation for testing purpose)")
        parameters = dict()
        if entity_id is not None:
            parameters.update({NGSI_ADAPTER_PARAMETER_ID: entity_id})

        if entity_type is not None:
            parameters.update({NGSI_ADAPTER_PARAMETER_TYPE: entity_type})

        if probe_name is not None:
            return self.rest_client.launch_request(uri_pattern=NGSI_ADAPTER_URI_PARSER, body=raw_data,
                                                   method=http_method, headers=self.headers, parameters=parameters,
                                                   probe_name=probe_name)
        else:
            return self.rest_client.launch_request(uri_pattern=NGSI_ADAPTER_URI_BASE, body=raw_data,
                                                   method=http_method, headers=self.headers, parameters=parameters)
