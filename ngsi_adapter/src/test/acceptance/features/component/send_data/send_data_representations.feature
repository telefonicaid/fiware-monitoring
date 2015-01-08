# -*- coding: utf-8 -*-
Feature: Sending probe data using invalid representations
  As a monitoring element (probe) user
  I want to be able to get error messages when using invalid representations for the resource
  so that I can use safely and in the right way this API


  @happy_path
  Scenario Outline: Probe sends valid raw data using an existing and well-formed parser with invalid Content-Type
    Given the parser "check_disk"
    And   the monitored resource with id "qa:1234567890" and type "host"
    And   the header Content-Type "<content_type>"
    When  I send valid raw data according to the selected parser
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
