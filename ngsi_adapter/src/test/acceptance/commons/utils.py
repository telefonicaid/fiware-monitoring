# -*- coding: utf-8 -*-

# Copyright 2015 Telefonica Investigación y Desarrollo, S.A.U
#
# This file is part of FIWARE project.
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


import uuid
from pkg_resources import resource_string
from logger_utils import get_logger
from constants import TRANSACTION_ID_PATTERN, RESOURCES_SAMPLEDATA_MODULE, RESOURCES_PARAMETER_PATTERN

logger = get_logger("utils")


def generate_transaction_id():
    """
    Generate a transaction ID value following defined pattern.
    :return: New transactionId
    """

    return TRANSACTION_ID_PATTERN.format(uuid=uuid.uuid4())


def get_probe_data_from_resource_file(filename, replacement_values=None):
    """
    Get probe data from resource files. If replacement_values is not empty,
    :param filename: Resource filename to be used for loading probe data
    :param param_values: (key, value) pairs. (list of dict)
    :return: File content with param value replacements
    """

    filename = filename + ".txt" if ".txt" not in filename else filename
    logger.debug("Getting resource file content [Filename: %s]", filename)
    file_content = resource_string(RESOURCES_SAMPLEDATA_MODULE, filename)

    if replacement_values is not None:
        logger.debug("Configuring template [Params: %s]", str(replacement_values))
        for param in replacement_values:
            file_content = file_content.replace(RESOURCES_PARAMETER_PATTERN.replace('param_name', param['key']),
                                                param['value'])

    return file_content
