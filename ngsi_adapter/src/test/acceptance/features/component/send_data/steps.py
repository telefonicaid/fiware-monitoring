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
from nose.tools import assert_equal, assert_true
from commons.dataset_utils import prepare_param
from lettuce_tools.logs_checking.log_utils import LogUtils
import time
from commons.constants import PROPERTIES_CONFIG_ENV, \
    PROPERTIES_CONFIG_ENV_LOCAL_PATH_REMOTE_LOGS, MONITORING_CONFIG_ENV_DEFAULT_PARSER_PARAMS, \
    MONITORING_CONFIG_ENV_DEFAULT_PARSER_DATA, MONITORING_CONFIG_SERVICE_ADAPTER, \
    MONITORING_CONFIG_SERVICE_LOG_FILE_NAME

# Wait X seconds for remote logging
WAIT_FOR_REMOTE_LOGGING = 5


def _set_default_dataset():
    """
    Ser default dataset vars for testing when data is not specified in the Scenarios
    :return None
    """
    #default_parser = world.config[PROPERTIES_CONFIG_ENV][MONITORING_CONFIG_ENV_DEFAULT_PARSER]
    world.raw_data_filename = world.config[PROPERTIES_CONFIG_ENV][MONITORING_CONFIG_ENV_DEFAULT_PARSER_DATA]
    world.raw_data_params = world.config[PROPERTIES_CONFIG_ENV][MONITORING_CONFIG_ENV_DEFAULT_PARSER_PARAMS]


@step(u'the probe "(.*)" and its associated parser "(.*)"$')
def the_parser(step, probe_name, parser_name):
    world.probe = prepare_param(probe_name)
    world.parser = prepare_param(parser_name)


@step(u'the probe name "(.*)"')
def the_probe_name(step, probe_name):
    world.probe = prepare_param(probe_name)


@step(u'the monitored resource with id "(.*)" and type "(.*)"$')
def the_monitored_resource_with_id_and_type(step, id, type):
    world.entity_id = prepare_param(id)
    world.entity_type = prepare_param(type)


@step(u'I send raw data according to the selected probe$')
def i_sed_raw_data_according_to_the_selected_parser(step):
    if world.raw_data_filename is None:
        _set_default_dataset()

    probe_data = get_probe_data_from_resource_file(world.raw_data_filename, world.raw_data_params)
    world.response = world.ngsi_adapter_client.send_raw_data_custom(probe_data, world.probe,
                                                                    world.entity_id, world.entity_type)


@step(u'I send raw data according to the selected probe with "(.*)" HTTP operation$')
def i_sed_raw_data_according_to_the_selected_parser_with_http_verb(step, http_verb):
    if world.raw_data_filename is None:
        _set_default_dataset()

    probe_data = get_probe_data_from_resource_file(world.raw_data_filename, world.raw_data_params)
    world.response = world.ngsi_adapter_client.send_raw_data_custom(probe_data, world.probe,
                                                                    world.entity_id, world.entity_type,
                                                                    http_method=http_verb)


@step(u'the response status code is "(.*)"$')
def the_response_status_code_is(step, status_code):
    assert_equal(str(world.response.status_code), status_code)


@step(u'the header Transaction-Id "(.*)"$')
def the_header_transaction_id(step, transaction_id):
    world.transaction_id = prepare_param(transaction_id)
    world.ngsi_adapter_client.init_headers(transaction_id=world.transaction_id)


@step(u'an auto-generated Transaction-Id value is used in logs')
@step(u'the given Transaction-Id value is used in logs')
def the_given_transaction_id_value_is_used_in_logs(step):
    log_utils = LogUtils()

    remote_log_local_path = world.config[PROPERTIES_CONFIG_ENV][PROPERTIES_CONFIG_ENV_LOCAL_PATH_REMOTE_LOGS]
    service_log_file_name = world.config[MONITORING_CONFIG_SERVICE_ADAPTER][MONITORING_CONFIG_SERVICE_LOG_FILE_NAME]

    # Wait for remote logging
    time.sleep(WAIT_FOR_REMOTE_LOGGING)

    if world.transaction_id is not None and len(world.transaction_id) != 0:
        log_value_transaction_id = {"TRANSACTION_ID": "trans={transaction_id}".format(
            transaction_id=world.transaction_id)}
        log_utils.search_in_log(remote_log_local_path, service_log_file_name, log_value_transaction_id)
    else:
        log_value_message = {"MESSAGE": "msg={probe}".format(probe=world.probe)}
        log_line = log_utils.search_in_log(remote_log_local_path, service_log_file_name, log_value_message)

        transaction_id = log_line[log_utils.LOG_TAG["TRANSACTION_ID"].replace("=", "")]
        assert_true(len(transaction_id) != 0,
                    "Transaction-ID not found in logs. Expected value. Value in logs: " + transaction_id)
