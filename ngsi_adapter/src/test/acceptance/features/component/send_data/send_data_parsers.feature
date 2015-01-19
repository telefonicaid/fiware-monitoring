# -*- coding: utf-8 -*-
Feature: Checking implemented parsers
  As a monitoring element (probe) user
  I want to be able to transform monitoring data from probes to NGSI context attributes using all defined parsers
  so that I can constantly check the status and performance of the cloud infrastructure using NGSI Context Broker.


  @happy_path
  Scenario Outline: Valid probe data is sent to CB using an existing parser
    Given the probe "<probe_name>" and its associated parser "<parser_name>"
    And   the monitored resource with id "<entity_id>" and type "<entity_type>"
    When  I send raw data according to the selected probe
    Then  the response status code is "200"

    Examples:
    | probe_name    | parser_name     | entity_id      | entity_type |
    | check_disk    | check_disk.js   | qa:192.168.1.2 | host        |
    | check_load    | check_load.js   | qa:192.168.1.3 | vm          |
    | check_mem.sh  | check_mem.sh.js | qa:192.168.1.4 | host        |
    | check_procs   | check_procs.js  | qa:192.168.1.5 | host        |
    | check_users   | check_users.js  | qa:192.168.1.6 | host        |


  Scenario: Valid probe data is sent to CB using non-existent parser
    Given the probe name "my_probe" with not existing parser
    And   the monitored resource with id "qa:1234567890" and type "host"
    When  I send raw data
    Then  the response status code is "404"


  Scenario: Valid probe data is sent to CB using an existing but invalid parser
    Given the probe "qa_probe_invalid" and its associated parser "qa_probe_invalid.js"
    And   the monitored resource with id "qa:1234567890" and type "host"
    When  I send raw data according to the selected probe
    Then  the response status code is "200"
    And   NGSI-Adapter has logged properly asynchronous errors for malformed parser


  Scenario Outline: Valid probe data is sent to CB using an existing parser
    Given the probe "<probe_name>" and its associated parser "<parser_name>"
    And   the monitored resource with id "qa:192.168.1.2" and type "host"
    And   raw data loaded from resource file "<resource_raw_data_template>" with values "<value_list>"
    When  I send raw data according to the selected probe
    Then  the response status code is "200"
    And   NGSI-Adapter has sent the properly request to NGSI Context Broker

    Examples:
    | probe_name   | parser_name     | resource_raw_data_template           | value_list                                    |
    | check_disk   | check_disk.js   | check_disk_grouping_valid_template   | FREE_SPACE_VALUE=22                           |
    | check_disk   | check_disk.js   | check_disk_grouping_valid_template   | FREE_SPACE_VALUE=16.5                         |
    | check_disk   | check_disk.js   | check_disk_grouping_valid_template   | FREE_SPACE_VALUE=100.0                        |
    | check_disk   | check_disk.js   | check_disk_grouping_valid_template   | FREE_SPACE_VALUE=0                            |
    | check_disk   | check_disk.js   | check_disk_nogrouping_valid_template | FREE_SPACE_VALUE_1=14,FREE_SPACE_VALUE_2=12.5 |
    | check_disk   | check_disk.js   | check_disk_nogrouping_valid_template | FREE_SPACE_VALUE_1=0.5,FREE_SPACE_VALUE_2=48  |
    | check_load   | check_load.js   | check_load_valid_template            | CPU_LOAD_VALUE=12                             |
    | check_load   | check_load.js   | check_load_valid_template            | CPU_LOAD_VALUE=48.5                           |
    | check_load   | check_load.js   | check_load_valid_template            | CPU_LOAD_VALUE=100.0                          |
    | check_load   | check_load.js   | check_load_valid_template            | CPU_LOAD_VALUE=0                              |
    | check_mem.sh | check_mem.sh.js | check_mem.sh_valid_template          | USED_MEM_VALUE=33                             |
    | check_mem.sh | check_mem.sh.js | check_mem.sh_valid_template          | USED_MEM_VALUE=61.6                           |
    | check_mem.sh | check_mem.sh.js | check_mem.sh_valid_template          | USED_MEM_VALUE=100                            |
    | check_mem.sh | check_mem.sh.js | check_mem.sh_valid_template          | USED_MEM_VALUE=0                              |
    | check_procs  | check_procs.js  | check_procs_valid_template           | NUM_PROCESSES_VALUE=0                         |
    | check_procs  | check_procs.js  | check_procs_valid_template           | NUM_PROCESSES_VALUE=15                        |
    | check_procs  | check_procs.js  | check_procs_valid_template           | NUM_PROCESSES_VALUE=2354154                   |
    | check_users  | check_users.js  | check_users_valid_template           | NUM_USERS_VALUE=0                             |
    | check_users  | check_users.js  | check_users_valid_template           | NUM_USERS_VALUE=12                            |
    | check_users  | check_users.js  | check_users_valid_template           | NUM_USERS_VALUE=12654881                      |


  Scenario Outline: Probe data with wrong parameter value is sent to CB using an existing parser
    Given the probe "<probe_name>" and its associated parser "<parser_name>"
    And   the monitored resource with id "qa:192.168.1.2" and type "host"
    And   raw data loaded from resource file "<resource_raw_data_template>" with values "<value_list>"
    When  I send raw data according to the selected probe
    Then  the response status code is "200"
    And   NGSI-Adapter has logged properly asynchronous errors for invalid data

    Examples:
    | probe_name   | parser_name     | resource_raw_data_template           | value_list                                    |
    | check_disk   | check_disk.js   | check_disk_grouping_valid_template   | FREE_SPACE_VALUE=22ab                         |
    | check_disk   | check_disk.js   | check_disk_grouping_valid_template   | FREE_SPACE_VALUE=                             |
    | check_disk   | check_disk.js   | check_disk_nogrouping_valid_template | FREE_SPACE_VALUE_1=14,FREE_SPACE_VALUE_2=zzz  |
    | check_disk   | check_disk.js   | check_disk_nogrouping_valid_template | FREE_SPACE_VALUE_1=,FREE_SPACE_VALUE_2=48     |
    | check_load   | check_load.js   | check_load_valid_template            | CPU_LOAD_VALUE=                               |
    | check_load   | check_load.js   | check_load_valid_template            | CPU_LOAD_VALUE=3by                            |
    | check_mem.sh | check_mem.sh.js | check_mem.sh_valid_template          | USED_MEM_VALUE=                               |
    | check_mem.sh | check_mem.sh.js | check_mem.sh_valid_template          | USED_MEM_VALUE=xf                             |
    | check_procs  | check_procs.js  | check_procs_valid_template           | NUM_PROCESSES_VALUE=                          |
    | check_procs  | check_procs.js  | check_procs_valid_template           | NUM_PROCESSES_VALUE=15.2                      |
    | check_procs  | check_procs.js  | check_procs_valid_template           | NUM_PROCESSES_VALUE=ft4                       |
    | check_users  | check_users.js  | check_users_valid_template           | NUM_USERS_VALUE=                              |
    | check_users  | check_users.js  | check_users_valid_template           | NUM_USERS_VALUE=12.6                          |
    | check_users  | check_users.js  | check_users_valid_template           | NUM_USERS_VALUE=21dd                          |


  Scenario Outline: Invalid probe data is sent to CB using an existing parser
    Given the probe "<probe_name>" and its associated parser "<parser_name>"
    And   the monitored resource with id "qa:192.168.1.2" and type "host"
    And   raw data loaded from resource file "<resource_raw_data_template>"
    When  I send raw data according to the selected probe
    Then  the response status code is "200"
    And   NGSI-Adapter has logged properly asynchronous errors for invalid data

    Examples:
    | probe_name   | parser_name     | resource_raw_data_template              |
    | check_disk   | check_disk.js   | check_disk_grouping_invalid_malformed   |
    | check_disk   | check_disk.js   | check_disk_nogrouping_invalid_malformed |
    | check_load   | check_load.js   | check_load_invalid_malformed            |
    | check_mem.sh | check_mem.sh.js | check_mem.sh_invalid_malformed          |
    | check_procs  | check_procs.js  | check_procs_invalid_malformed           |
    | check_users  | check_users.js  | check_users_invalid_malformed           |


  Scenario Outline: Valid probe data is sent to CB using an existing parser with invalid Content-Type header
    Given the probe "qa_probe" and its associated parser "qa_probe_parser"
    And   the monitored resource with id "qa:1234567890" and type "host"
    And   the header Content-Type "<content_type>"
    When  I send valid raw data according to the selected probe
    Then  the response status code is "415"

    Examples:
    | content_type        |
    | application/json    |
    | application/xml     |
    | multipart/from-data |
    | text/html           |
    | text1/plain         |
    | text/plain1         |
    |                     |
