# -*- coding: utf-8 -*-
Feature: Sending probe data
  As a monitoring element (probe) user
  I want to be able to transform monitoring data from probes to NGSI context attributes
  so that I can constantly check the status and performance of the cloud infrastructure using Context Broker.


  @happy_path
  Scenario: Valid probe data is sent to CB using an existing parser
    Given the probe name "qa_probe"
    And   the monitored resource with id "qa:192.168.1.2" and type "host"
    When  I send raw data according to the selected probe
    Then  the response status code is "200"


  Scenario: Valid probe data is sent to CB using a not existing parser
    Given the probe name "qa_probe_not_existing"
    And   the monitored resource with id "qa:1234567890" and type "host"
    When  I send raw data according to the selected probe
    Then  the response status code is "404"


  Scenario Outline: Valid probe data is sent to CB using an existing parser, with valid entity ID values.
    Given the probe name "qa_probe"
    And   the monitored resource with id "<entity_id>" and type "host"
    When  I send raw data according to the selected probe
    Then  the response status code is "200"

    Examples:
    | entity_id               |
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


  Scenario Outline: Valid probe data is sent to CB using an existing parser, with valid entity TYPE values.
    Given the probe name "qa_probe"
    And   the monitored resource with id "qa:1234567890" and type "<entity_type>"
    When  I send raw data according to the selected probe
    Then  the response status code is "200"

    Examples:
    | entity_type             |
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


  Scenario Outline: Valid probe data is sent to CB using an existing parser, with valid and invalid probe name values.
    Given the probe name "<probe_name>"
    And   the monitored resource with id "qa:1234567890" and type "host"
    When  I send raw data according to the selected probe
    Then  the response status code is "<status_code>"

    Examples:
    | probe_name  | status_code |
    | 1           | 404         |
    | a           | 404         |
    | B           | 404         |
    | 12345678    | 404         |
    | qa.probe    | 404         |
    | qa-probe    | 404         |
    | qa@probe    | 404         |
    | qa_probe    | 200         |


  Scenario Outline: Valid probe data is sent to CB using an existing parser, with invalid entity ID values.
    Given the probe name "qa_probe"
    And   the monitored resource with id "<entity_id>" and type "host"
    When  I send raw data according to the selected probe
    Then  the response status code is "400"

    Examples:
    | entity_id                 |
    |                           |
    | [MISSING_PARAM]           |


  Scenario Outline: Valid probe data is sent to CB using an existing parser, with invalid entity TYPE values.
    Given the probe name "qa_probe"
    And   the monitored resource with id "qa:1234567890" and type "<entity_type>"
    When  I send raw data according to the selected probe
    Then  the response status code is "400"

    Examples:
    | entity_type               |
    |                           |
    | [MISSING_PARAM]           |


  Scenario: Valid probe data is sent to CB with missing probe name.
    Given the probe name "[MISSING_PARAM]"
    And   the monitored resource with id "qa:1234567890" and type "host"
    When  I send raw data according to the selected probe
    Then  the response status code is "404"


  Scenario Outline: Valid probe data is sent to CB using an unsupported HTTP method
    Given the probe name "qa_probe"
    And   the monitored resource with id "qa:1234567890" and type "host"
    When  I send raw data according to the selected probe with "<http_verb>" HTTP operation
    Then  the response status code is "405"

    Examples:
    | http_verb |
    | get       |
    | put       |
    | delete    |


  Scenario Outline: NGSI Adapter reuse the correlator header value given in the request
    Given the probe name "qa_probe"
    And   the monitored resource with id "qa:1234567890" and type "host"
    And   the header Correlator "<correlator>"
    When  I send raw data according to the selected probe
    Then  the response status code is "200"
    And   the given Correlator value is used in logs

    Examples:
    | correlator          |
    | 1                   |
    | 1231asdfgasd        |
    | a/12345.qa          |
    | ABCDEFG#123         |
    | 123-456             |
    | ABC_1av             |


  Scenario Outline: NGSI Adapter generates new correlator value when header is missing or empty
    Given the probe name "<probe_name>"
    And   the monitored resource with id "qa:1234567890" and type "host"
    And   the header Correlator "<correlator>"
    When  I send raw data according to the selected probe
    Then  an auto-generated Correlator value is used in logs

    Examples:
    | correlator      | probe_name     |
    |                 | no_correlator  |
    | [MISSING_PARAM] | no_correlator2 |
