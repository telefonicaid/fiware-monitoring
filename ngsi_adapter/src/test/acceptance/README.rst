==========================================================
FIWARE-MONITORING | NGSI-Adapter | Acceptance test project
==========================================================

This project contains the NGSI-Adapter acceptance tests (component, integration and E2E testing).
All test cases have been defined using Gherkin that it is a Business Readable, Domain Specific Language that lets you
describe software’s behaviour without detailing how that behaviour is implemented.
Gherkin has the purpose of serving documentation of test cases.


Test case implementation has been performed using `Python <http://www.python.org/>`_ and 
`Lettuce <http://lettuce.it/>`_.

Project Structure
-----------------
 :: 
 
    ├───acceptance
    │   ├───commons
    │   ├───features
    │   │   ├───component
    │   │   │   └───send_data
    │   │   ├───e2e
    │   │   └───integration
    │   ├───resources
    │   │   └───probe_sample_data
    │   └───settings
    │
