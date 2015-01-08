# -*- coding: utf-8 -*-
Feature: Checking implemented parsers
  As a monitoring element (probe) user
  I want to be able to transform monitoring data from probes to NGSI context attributes using all defined parsers
  so that I can constantly check the status and performance of the cloud infrastructure using NGSI Context Broker.


  @happy_path
  Scenario Outline: Probe sends valid raw data using an existing and well-formed parser
    Given the parser "<parser>"
    And   the monitored resource with id "<probe_id>" and type "<probe_type>"
    When  I send valid raw data according to the selected parser
    Then  the response status code is "200"

    Examples:
    | parser       | probe_id       | probe_type |
    | check_disk   | qa:192.168.1.2 | host       |
    | check_load   | qa:192.168.1.3 | host       |
    | check_mem.sh | qa:192.168.1.4 | host       |
    | check_procs  | qa:192.168.1.5 | host       |
    | check_users  | qa:192.168.1.6 | host       |


  Scenario: Probe sends raw data using a malformed parser
    Given the parser "qa_malformed"
    And   the monitored resource with id "qa:1234567890" and type "host"
    When  I send valid raw data
    Then  the response status code is "200"
    And   NGSI-Adapter has logged properly asynchronous errors for malformed parser


  Scenario Outline: Probe sends valid raw data using an existing and well-formed parser
    Given the parser "<parser>"
    And   the monitored resource with id "qa:192.168.1.2" and type "host"
    And   raw data loaded from resource file "<resource_raw_data_template>" with values "<value_list>"
    When  I send valid raw data according to the selected parser
    Then  the response status code is "200"
    And   NGSI-Adapter has sent the properly request to NGSI Context Broker

    Examples:
    | parser       | resource_raw_data_template           | value_list                                    |
    | check_disk   | check_disk_grouping_valid_template   | FREE_SPACE_VALUE=22                           |
    | check_disk   | check_disk_grouping_valid_template   | FREE_SPACE_VALUE=16.5                         |
    | check_disk   | check_disk_grouping_valid_template   | FREE_SPACE_VALUE=100.0                        |
    | check_disk   | check_disk_grouping_valid_template   | FREE_SPACE_VALUE=0                            |
    | check_disk   | check_disk_nogrouping_valid_template | FREE_SPACE_VALUE_1=14,FREE_SPACE_VALUE_2=12.5 |
    | check_disk   | check_disk_nogrouping_valid_template | FREE_SPACE_VALUE_1=0.5,FREE_SPACE_VALUE_2=48  |
    | check_load   | check_load_valid_template            | CPU_LOAD_VALUE=12                             |
    | check_load   | check_load_valid_template            | CPU_LOAD_VALUE=48.5                           |
    | check_load   | check_load_valid_template            | CPU_LOAD_VALUE=100.0                          |
    | check_load   | check_load_valid_template            | CPU_LOAD_VALUE=0                              |
    | check_mem.sh | check_mem.sh_valid_template          | USED_MEM_VALUE=33                             |
    | check_mem.sh | check_mem.sh_valid_template          | USED_MEM_VALUE=61.6                           |
    | check_mem.sh | check_mem.sh_valid_template          | USED_MEM_VALUE=100                            |
    | check_mem.sh | check_mem.sh_valid_template          | USED_MEM_VALUE=0                              |
    | check_procs  | check_procs_valid_template           | NUM_PROCESSES_VALUE=0                         |
    | check_procs  | check_procs_valid_template           | NUM_PROCESSES_VALUE=15                        |
    | check_procs  | check_procs_valid_template           | NUM_PROCESSES_VALUE=2354154                   |
    | check_users  | check_users_valid_template           | NUM_USERS_VALUE=0                             |
    | check_users  | check_users_valid_template           | NUM_USERS_VALUE=12                            |
    | check_users  | check_users_valid_template           | NUM_USERS_VALUE=12654881                      |


  Scenario Outline: Probe sends well-formed raw data using an existing parser with wrong parameter values
    Given the parser "<parser>"
    And   the monitored resource with id "qa:192.168.1.2" and type "host"
    And   raw data loaded from resource file "<resource_raw_data_template>" with values "<value_list>"
    When  I send valid raw data according to the selected parser
    Then  the response status code is "200"
    And   NGSI-Adapter has logged properly asynchronous errors for invalid data

    Examples:
    | parser       | resource_raw_data_template             | value_list                                  |
    | check_disk   | check_disk_grouping_valid_template   | FREE_SPACE_VALUE=22ab                         |
    | check_disk   | check_disk_grouping_valid_template   | FREE_SPACE_VALUE=                             |
    | check_disk   | check_disk_nogrouping_valid_template | FREE_SPACE_VALUE_1=14,FREE_SPACE_VALUE_2=zzz  |
    | check_disk   | check_disk_nogrouping_valid_template | FREE_SPACE_VALUE_1=,FREE_SPACE_VALUE_2=48     |
    | check_load   | check_load_valid_template            | CPU_LOAD_VALUE=                               |
    | check_load   | check_load_valid_template            | CPU_LOAD_VALUE=3by                            |
    | check_mem.sh | check_mem.sh_valid_template          | USED_MEM_VALUE=                               |
    | check_mem.sh | check_mem.sh_valid_template          | USED_MEM_VALUE=xf                             |
    | check_procs  | check_procs_valid_template           | NUM_PROCESSES_VALUE=                          |
    | check_procs  | check_procs_valid_template           | NUM_PROCESSES_VALUE=15.2                      |
    | check_procs  | check_procs_valid_template           | NUM_PROCESSES_VALUE=ft4                       |
    | check_users  | check_users_valid_template           | NUM_USERS_VALUE=                              |
    | check_users  | check_users_valid_template           | NUM_USERS_VALUE=12.6                          |
    | check_users  | check_users_valid_template           | NUM_USERS_VALUE=21dd                          |


  Scenario Outline: Probe sends malformed raw data using an existing parser
    Given the parser "<parser>"
    And   the monitored resource with id "qa:192.168.1.2" and type "host"
    And   raw data loaded from resource file "<resource_raw_data_template>"
    When  I send valid raw data according to the selected parser
    Then  the response status code is "200"
    And   NGSI-Adapter has logged properly asynchronous errors for invalid data

    Examples:
    | parser       | resource_raw_data_template              |
    | check_disk   | check_disk_grouping_invalid_missing     |
    | check_disk   | check_disk_grouping_invalid_malformed   |
    | check_disk   | check_disk_nogrouping_invalid_missing   |
    | check_disk   | check_disk_nogrouping_invalid_malformed |
    | check_load   | check_load_invalid_missing              |
    | check_load   | check_load_invalid_malformed            |
    | check_mem.sh | check_mem.sh_invalid_missing            |
    | check_mem.sh | check_mem.sh_invalid_malformed          |
    | check_procs  | check_procs_invalid_missing             |
    | check_procs  | check_procs_invalid_malformed           |
    | check_users  | check_users_invalid_missing             |
    | check_users  | check_users_invalid_malformed           |
