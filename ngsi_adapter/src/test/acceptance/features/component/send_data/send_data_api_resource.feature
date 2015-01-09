# -*- coding: utf-8 -*-
Feature: Sending probe data
  As a monitoring element (probe) user
  I want to be able to transform monitoring data from probes to NGSI context attributes
  so that I can constantly check the status and performance of the cloud infrastructure using NGSI Context Broker.


  @happy_path
  Scenario: Probe sends valid raw data using an existing and well-formed parser
    Given the parser "check_disk"
    And   the monitored resource with id "qa:192.168.1.2" and type "host"
    When  I send raw data according to the selected parser
    Then  the response status code is "200"


  Scenario: Probe sends valid raw data using a not existing parser
    Given the parser "not_existing"
    And   the monitored resource with id "qa:1234567890" and type "host"
    When  I send raw data
    Then  the response status code is "404"


  Scenario Outline: Probe sends valid raw data using an existing and well-formed parser, with valid probe ID values.
    Given the parser "check_disk"
    And   the monitored resource with id "<probe_id>" and type "host"
    When  I send raw data according to the selected parser
    Then  the response status code is "200"

    Examples:
    | probe_id                |
    | 1                       |
    | a                       |
    | A                       |
    | 1-2                     |
    | a_b                     |
    | 1#B                     |
    | c:4                     |
    | 12345678                |
    | 123.4567                |
    | abc.1234                |
    | A123B-c1                |
    | 123_vB12                |
    | Ab:123.4                |
    | [STRING_WITH_LENGTH_60] |


  Scenario Outline: Probe sends valid raw data using an existing and well-formed parser, with valid probe TYPE values.
    Given the parser "check_disk"
    And   the monitored resource with id "qa:1234567890" and type "<probe_type>"
    When  I send raw data according to the selected parser
    Then  the response status code is "200"

    Examples:
    | probe_type              |
    | 1                       |
    | a                       |
    | A                       |
    | 1-2                     |
    | a_b                     |
    | 1#B                     |
    | c:4                     |
    | 12345678                |
    | 123.4567                |
    | abc.1234                |
    | A123B-c1                |
    | 123_vB12                |
    | Ab:123.4                |
    | [STRING_WITH_LENGTH_60] |


  Scenario Outline: Probe sends valid raw data using an existing and well-formed parser, with valid parser name values.
    Given the parser "<parser_name>"
    And   the monitored resource with id "qa:1234567890" and type "host"
    When  I send raw data according to the selected parser
    Then  the response status code is "200"

    Examples:
    | parser_name             |
    | 1                       |
    | a                       |
    | A                       |
    | 12345678                |
    | qa.parser               |
    | qa-parser               |
    | qa_parser               |
    | qa#parser               |
    | [STRING_WITH_LENGTH_60] |


  Scenario Outline: Probe sends valid raw data using an existing and well-formed parser, with invalid probe ID values.
    Given the parser "check_disk"
    And   the monitored resource with id "<probe_id>" and type "host"
    When  I send raw data according to the selected parser
    Then  the response status code is "400"

    Examples:
    | probe_id                  |
    | abc 1234                  |
    |                           |
    | [STRING_WITH_LENGTH_2000] |
    | [MISSING_PARAM]           |


  Scenario Outline: Probe sends valid raw data using an existing and well-formed parser, with invalid probe TYPE values.
    Given the parser "check_disk"
    And   the monitored resource with id "qa:1234567890" and type "<probe_type>"
    When  I send raw data according to the selected parser
    Then  the response status code is "400"

    Examples:
    | probe_type                |
    | abc 1234                  |
    |                           |
    | [STRING_WITH_LENGTH_2000] |
    | [MISSING_PARAM]           |


  Scenario Outline: Probe sends valid raw data using an existing and well-formed parser, with invalid parser name values.
    Given the parser "<parser_name>"
    And   the monitored resource with id "qa:1234567890" and type "host"
    When  I send raw data according to the selected parser
    Then  the response status code is "400"

    Examples:
    | parser_name               |
    | abc 1234                  |
    |                           |
    | [STRING_WITH_LENGTH_2000] |
    | [MISSING_PARAM]           |


  Scenario Outline: Probe sends valid raw data using an unsupported HTTP method
    Given the parser "check_disk"
    And   the monitored resource with id "qa:1234567890" and type "host"
    And   http operation is "<http_verb>"
    When  I send raw data according to the selected parser
    Then  the response status code is "400"

    Examples:
    | http_verb |
    | get       |
    | put       |
    | delete    |
    | update    |
