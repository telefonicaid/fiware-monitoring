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


from lettuce import world
from logger_utils import get_logger
import os
import sys
import json
from remote_tail_utils import RemoteTail
from constants import PROPERTIES_FILE, PROPERTIES_CONFIG_ENV, PROPERTIES_CONFIG_ENV_LOGS_PATH, \
    PROPERTIES_CONFIG_ENV_LOCAL_PATH_REMOTE_LOGS, MONITORING_CONFIG_SERVICE_PRIVATEKEY, \
    MONITORING_CONFIG_SERVICE_LOG_PATH, MONITORING_CONFIG_SERVICE_HOST, MONITORING_CONFIG_SERVICE_HOSTUSER, \
    MONITORING_CONFIG_SERVICE_ADAPTER, MONITORING_CONFIG_SERVICE_LOG_FILE_NAME

logger = get_logger("terrain_utils")


def _load_project_properties():
    """
    Parse the JSON configuration file located in the src folder and
    store the resulting dictionary in the lettuce world global variable.
    """

    logger.debug("Loading test properties")
    with open(PROPERTIES_FILE) as config_file:
        try:
            world.config = json.load(config_file)
        except Exception, e:
            logger.error('Error parsing config file: %s' % e)
            sys.exit(1)


def set_up():
    """
    Setup execution and configure global test parameters and environment.
    Init the capture from remote logs
    :return: None
    """

    logger.info("Setting up test execution")
    _load_project_properties()

    """
    Make sure the logs path exists and create it otherwise.
    """
    logger.debug("Generating log directories if not exist")
    log_path = world.config[PROPERTIES_CONFIG_ENV][PROPERTIES_CONFIG_ENV_LOGS_PATH]
    if not os.path.exists(log_path):
        os.makedirs(log_path)

    log_path = world.config[PROPERTIES_CONFIG_ENV][PROPERTIES_CONFIG_ENV_LOCAL_PATH_REMOTE_LOGS]
    if not os.path.exists(log_path):
        os.makedirs(log_path)

    # Init remote logs capturing
    logger.info("Initiating remote log capture")
    remote_host_ip = world.config[MONITORING_CONFIG_SERVICE_ADAPTER][MONITORING_CONFIG_SERVICE_HOST]
    remote_host_user = world.config[MONITORING_CONFIG_SERVICE_ADAPTER][MONITORING_CONFIG_SERVICE_HOSTUSER]
    service_log_path = world.config[MONITORING_CONFIG_SERVICE_ADAPTER][MONITORING_CONFIG_SERVICE_LOG_PATH]
    service_log_file_name = world.config[MONITORING_CONFIG_SERVICE_ADAPTER][MONITORING_CONFIG_SERVICE_LOG_FILE_NAME]
    private_key = world.config[MONITORING_CONFIG_SERVICE_ADAPTER][MONITORING_CONFIG_SERVICE_PRIVATEKEY]
    world.remote_tail_client = RemoteTail(remote_host_ip, remote_host_user, service_log_path,
                                          service_log_file_name, log_path, private_key)
    world.remote_tail_client.init_tailer_connection()
    world.remote_tail_client.start_tailer()


def tear_down():
    """
    Tear down test execution process.
    Stop the capture from remote logs
    :return:
    """

    logger.info("Stopping remote log capture")
    world.remote_tail_client.stop_tailer()
