# -*- coding: utf-8 -*-

# Copyright 2014 Telefonica Investigación y Desarrollo, S.A.U
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
from commons.constants import HEADER_REPRESENTATION_TEXTPLAIN, HEADER_CONTENT_TYPE, HEADER_TRANSACTION_ID, \
    HTTP_VERB_POST
from commons.utils import generate_transaction_id
from commons.logger_utils import get_logger, configure_level_debug
from pkg_resources import resource_string

NGSI_ADAPTER_URI_BASE = "{" + API_ROOT_URL_ARG_NAME + "}"
NGSI_ADAPTER_URI_PARSER = NGSI_ADAPTER_URI_BASE + "/{parser_name}"
NGSI_ADAPTER_PARAMETER_ID = "id"
NGSI_ADAPTER_PARAMETER_TYPE = "type"

logger = get_logger("rest_client_utils")


class NgsiAdapterClient:

    headers = dict()

    def __init__(self, protocol, host, port, base_resource=None):
        """
        Class constructor. Init default headers
        :param protocol: API Protocol
        :param host: API Host
        :param port: API Port
        :param base_resource: base uri resource (if exists)
        :return: None
        """
        self.init_headers()
        self.rest_client = RestClient(protocol, host, port, base_resource)

    def init_headers(self, content_type=HEADER_REPRESENTATION_TEXTPLAIN, transaction_id=generate_transaction_id()):
        """
        Init header to values (or default values)
        :param content_type: Content-Type header value. By default text/plain
        :param transaction_id: txId header value. By default, generated value by Utils.generate_transaction_id()
        :return: None
        """
        self.headers.update({HEADER_CONTENT_TYPE: content_type, HEADER_TRANSACTION_ID: transaction_id})

    def set_headers(self, headers):
        """
        Set header.
        :param headers: Headers to be used by next request (dict)
        :return: None
        """
        self.headers = headers

    def send_raw_data(self, raw_data, parser_name, probe_id, probe_type):
        """
        Execute a well-formed POST request. All parameters are mandatory
        :param raw_data: Raw probe data to send (string, text/plain)
        :param parser_name: Parser to be used (string)
        :param probe_id: Probe ID (string)
        :param probe_type: Probe Type (string)
        :return: HTTP Request response ('Requests' lib)
        """
        parameters = dict()
        parameters.update({NGSI_ADAPTER_PARAMETER_ID: probe_id})
        parameters.update({NGSI_ADAPTER_PARAMETER_TYPE: probe_type})
        return self.rest_client.post(uri_patter=NGSI_ADAPTER_URI_PARSER, body=raw_data, headers=self.headers,
                                     parameters=parameters, parser_name=parser_name)

    def send_raw_data_custom(self, raw_data, parser_name=None, probe_id=None, probe_type=None,
                             http_method=HTTP_VERB_POST):
        """
        Execute a 'send_data' request (POST request by default). Should support all testing cases.
         The generated request could be malformed (Testing purpose)
         Parameters with None value will not be in the generated request (missing parameter).
        :param raw_data: Raw probe data to send (string, text/plain)
        :param parser_name: Parser to be used (string)
        :param probe_id: Probe ID (string)
        :param probe_type: Probe Type (string)
        :param http_method: send raw data is a HTTP POST request but, for testing purposes could be interesting to use
         another HTTP verb. By default is defined to 'post'
        :return: HTTP Request response ('Requests' lib)
        """

        parameters = dict()
        if probe_id is not None:
            parameters.update({NGSI_ADAPTER_PARAMETER_ID: probe_id})

        if probe_type is not None:
            parameters.update({NGSI_ADAPTER_PARAMETER_TYPE: probe_type})

        if parser_name is not None:
            return self.rest_client.launch_request(uri_patter=NGSI_ADAPTER_URI_PARSER, body=raw_data,
                                                   method=http_method, headers=self.headers, parameters=parameters,
                                                   parser_name=parser_name)
        else:
            return self.rest_client.launch_request(uri_patter=NGSI_ADAPTER_URI_BASE, body=raw_data,
                                                   method=http_method, headers=self.headers, parameters=parameters)

if __name__ == '__main__':
    configure_level_debug()
    logger.info("START TEST")

    ngsi_adapter_client = NgsiAdapterClient("http", "130.206.81.245", "1337")
    file_content = resource_string('resources.probe_sample_data', 'check_disk_grouping_invalid_malformed.txt')
    #f = open('resources/probe_sample_data/check_disk_grouping_valid_template.txt', 'r')
    #file_content = f.read()
    logger.info("File content: " + file_content)
    ngsi_adapter_client.send_raw_data_custom(file_content, 'check_disk', 'qa-probe', 'host')
