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


from lettuce import before, after, world
from commons.terrain_utils import set_up, tear_down
from commons.ngsi_adapter_api_utils.ngsi_adapter_client import NgsiAdapterClient
from commons.logger_utils import get_logger
from commons.constants import MONITORING_CONFIG_SERVICE_ADAPTER, MONITORING_CONFIG_SERVICE_HOST, \
    MONITORING_CONFIG_SERVICE_PORT, MONITORING_CONFIG_SERVICE_PROTOCOL

logger = get_logger("terrain_utils")


@before.all
def before_all():
    set_up()


@before.each_feature
def before_each_feature(feature):
    world.ngsi_adapter_client = NgsiAdapterClient(world.config[MONITORING_CONFIG_SERVICE_ADAPTER]
                                                  [MONITORING_CONFIG_SERVICE_PROTOCOL],
                                                  world.config[MONITORING_CONFIG_SERVICE_ADAPTER]
                                                  [MONITORING_CONFIG_SERVICE_HOST],
                                                  world.config[MONITORING_CONFIG_SERVICE_ADAPTER]
                                                  [MONITORING_CONFIG_SERVICE_PORT])


@before.each_scenario
def before_each_scenario(scenario):
    world.parser = None
    world.probe_id = "qa"
    world.probe_type = "host"
    world.raw_data_filename = None
    world.raw_data_params = None

    logger.info("#######################################################")
    logger.info("#######################################################")


@after.all
def after_all(total):
    tear_down()
