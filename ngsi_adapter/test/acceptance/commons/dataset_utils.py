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


from lettuce_tools.dataset_utils.dataset_utils import DatasetUtils

dataset_utils = DatasetUtils()


def prepare_param(param):
    """
    Generate a fixed length data for elements tagged with the text [LENGTH] in lettuce
    Removes al the data elements tagged with the text [MISSING_PARAM] in lettuce
    :param param: Lettuce parameter
    :return data without not desired params
    """

    if "[MISSING_PARAM]" in param:
        new_param = None
    else:
        new_param = dataset_utils.generate_fixed_length_param(param)

    return new_param
