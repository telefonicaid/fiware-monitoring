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


from lettuce import world, step

from commons.utils import get_probe_data_from_resource_file
from nose.tools import assert_equal
from commons.dataset_utils import prepare_param
from commons.constants import PROPERTIES_CONFIG_ENV, \
    MONITORING_CONFIG_ENV_DEFAULT_PARSER, MONITORING_CONFIG_ENV_DEFAULT_PARSER_PARAMS, \
    MONITORING_CONFIG_ENV_DEFAULT_PARSER_DATA


def _set_default_dataset():
    """
    Ser default dataset vars for testing when data is not specified in the Scenarios
    :return None
    """
    #default_parser = world.config[PROPERTIES_CONFIG_ENV][MONITORING_CONFIG_ENV_DEFAULT_PARSER]
    world.raw_data_filename = world.config[PROPERTIES_CONFIG_ENV][MONITORING_CONFIG_ENV_DEFAULT_PARSER_DATA]
    world.raw_data_params = world.config[PROPERTIES_CONFIG_ENV][MONITORING_CONFIG_ENV_DEFAULT_PARSER_PARAMS]


@step(u'the parser "(.*)"$')
def the_parser(step, parser):
    world.parser = prepare_param(parser)


@step(u'the monitored resource with id "(.*)" and type "(.*)"$')
def the_monitored_resource_with_id_and_type(step, id, type):
    world.probe_id = prepare_param(id)
    world.probe_type = prepare_param(type)


@step(u'I send raw data according to the selected parser$')
def i_sed_raw_data_according_to_the_selected_parser(step):
    if world.raw_data_filename is None:
        _set_default_dataset()

    probe_data = get_probe_data_from_resource_file(world.raw_data_filename, world.raw_data_params)
    world.response = world.ngsi_adapter_client.send_raw_data_custom(probe_data, world.parser,
                                                                    world.probe_id, world.probe_type)


@step(u'I send raw data according to the selected parser with "(.*)" HTTP operation$')
def i_sed_raw_data_according_to_the_selected_parser_with_http_verb(step, http_verb):
    if world.raw_data_filename is None:
        _set_default_dataset()

    probe_data = get_probe_data_from_resource_file(world.raw_data_filename, world.raw_data_params)
    world.response = world.ngsi_adapter_client.send_raw_data_custom(probe_data, world.parser,
                                                                    world.probe_id, world.probe_type,
                                                                    http_method=http_verb)


@step(u'the response status code is "(.*)"$')
def the_response_status_code_is(step, status_code):
    assert_equal(str(world.response.status_code), status_code)

