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

import logging
import xml
import json
from constants import HEADER_CONTENT_TYPE, HEADER_REPRESENTATION_XML, HEADER_REPRESENTATION_JSON

"""
Part of this code has been taken from:
 https://pdihub.hi.inet/fiware/fiware-iotqaUtils/raw/develop/iotqautils/iotqaLogger.py
"""

LOG_PATTERN = "%(asctime)s - %(name)s - %(levelname)s - %(message)s"


def configure_level_debug():
    """
    Configures global log level to DEBUG
    """
    logging.basicConfig(level=logging.DEBUG, format=LOG_PATTERN)


def configure_level_info():
    """
    Configures global log level to INFO
    """
    logging.basicConfig(level=logging.INFO, format=LOG_PATTERN)


def configure_level_warning():
    """
    Configures global log level to WARNING
    """
    logging.basicConfig(level=logging.WARNING, format=LOG_PATTERN)


def configure_level_error():
    """
    Configures global log level to ERROR
    """
    logging.basicConfig(level=logging.ERROR, format=LOG_PATTERN)


def get_logger(name):
    """
    Creates new logger with the given name
    :param name: Name of the logger
    :return: Logger
    """
    logger = logging.getLogger(name)
    return logger


def _get_pretty_body(headers, body):
    """
    Returns a pretty printed body using the Content-Type header information
    :param headers: Headers for the request/response (dict)
    :param body: Body to pretty print (string)
    :return: Body pretty printed (string)
    """
    if HEADER_CONTENT_TYPE in headers:
        if HEADER_REPRESENTATION_XML == headers[HEADER_CONTENT_TYPE]:
            xml_parsed = xml.dom.minidom.parseString(body)
            pretty_xml_as_string = xml_parsed.toprettyxml()
            return pretty_xml_as_string
        else:
            if HEADER_REPRESENTATION_JSON in headers[HEADER_CONTENT_TYPE]:
                parsed = json.loads(body)
                return json.dumps(parsed, sort_keys=True, indent=4)
            else:
                return body
    else:
        return body


def log_print_request(logger, method, url, query_params=None, headers=None, body=None):
    """
    Logs an HTTP request data.
    :param logger: Logger to use
    :param method: HTTP method
    :param url: URL
    :param query_params: Query parameters in the URL
    :param headers: Headers (dict)
    :param body: Body (raw body, string)
    :return: None
    """
    log_msg = '>>>>>>>>>>>>>>>>>>>>> Request >>>>>>>>>>>>>>>>>>> \n'
    log_msg += '\t> Method: %s\n' % method
    log_msg += '\t> Url: %s\n' % url
    if query_params is not None:
        log_msg += '\t> Query params: {}\n'.format(str(query_params))
    if headers is not None:
        log_msg += '\t> Headers: {}\n'.format(str(headers))
    if body is not None:
        log_msg += '\t> Payload sent:\n {}\n'.format(_get_pretty_body(headers, body))

    logger.debug(log_msg)


def log_print_response(logger, response):
    """
    Logs an HTTP response data
    :param logger: logger to use
    :param response: HTTP response ('Requests' lib)
    :return: None
    """
    log_msg = '<<<<<<<<<<<<<<<<<<<<<< Response <<<<<<<<<<<<<<<<<<\n'
    log_msg += '\t< Response code: {}\n'.format(str(response.status_code))
    log_msg += '\t< Headers: {}\n'.format(str(dict(response.headers)))
    try:
        log_msg += '\t< Payload received:\n {}'.format(_get_pretty_body(dict(response.headers), response.content))
    except ValueError:
        log_msg += '\t< Payload received:\n {}'.format(_get_pretty_body(dict(response.headers), response.content.text))

    logger.debug(log_msg)
