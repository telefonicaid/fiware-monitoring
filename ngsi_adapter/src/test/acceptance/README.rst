==========================================================
FIWARE-MONITORING | NGSI-Adapter | Acceptance test project
==========================================================

This project contains the NGSI-Adapter acceptance tests (component, integration and E2E testing).
All test cases have been defined using Gherkin that it is a Business Readable, Domain Specific Language that lets you
describe software’s behaviour without detailing how that behaviour is implemented.
Gherkin has the purpose of serving documentation of test cases.


Test case implementation has been performed using `Python <http://www.python.org/>`_ and 
`Lettuce <http://lettuce.it/>`_.

Acceptance Project Structure
----------------------------
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


FIWARE Monitoring Automation Framework
---------------------------------------

Features:

- Lettuce-Tools support
- Settings using json files and Lettuce-Tools utility
- Test report using Lettuce-Tools XUnit output
- NGSI-Adapter Client
- Logging
- Remote NGSI-Adapter log capturing
- Test data management using templates (resources)


Acceptance test execution
-------------------------

Execute the following command in the test project root directory:

::

  $> cd ngsi_adapter/src/test/acceptance
  $> lettuce_tools -ft send_data_api_resource -ts comp -sd features/ --tags=-skip -en dev

With this command, you will execute:

- components Test Cases in the 'Development' environment configured in settings/dev-properties.json
- the send_data_api_resource feature
- Skipping all Scenarios with tagged with "skip"


**Prerequisites**

- Python 2.7 or newer (2.x) (https://www.python.org/downloads/)
- pip (https://pypi.python.org/pypi/pip)
- virtualenv (https://pypi.python.org/pypi/virtualenv)
- Monitoring [NGSI-Adapter] (`Download NGSI-Adapter <https://github.com/telefonicaid/fiware-monitoring>`_)

**Test case execution using virtualenv**

1. Create a virtual environment somewhere *(virtualenv $WORKON_HOME/venv)*
#. Activate the virtual environment *(source $WORKON_HOME/venv/bin/activate)*
#. Go to *ngsi_adapter/src/test/acceptance* folder in the project
#. Install the requirements for the acceptance tests in the virtual environment *(pip install -r requirements.txt --allow-all-external)*

**Test case execution using Vagrant (optional)**

Instead of using virtualenv, you can use the provided Vagrantfile to deploy a local VM using `Vagrant <https://www.vagrantup.com/>`_,
that will provide all environment configurations for launching test cases.

1. Download and install Vagrant (https://www.vagrantup.com/downloads.html)
#. Go to *ngsi_adapter/src/test/acceptance* folder in the project
#. Execute *vagrant up* to launch a VM based on Vagrantfile provided.
#. After Vagrant provision, your VM is properly configured to launch acceptance tests. You have to access to the VM using *vagrant ssh* and change to */vagrant* directory that will have mounted your workspace *(test/acceptance)*.

If you need more information about how to use Vagrant, you can see
`Vagrant Getting Started <https://docs.vagrantup.com/v2/getting-started/index.html>`_

**Settings**

Before executing the acceptance tests, you will need configure the properties file. To execute acceptance test on the
experimentation environment, you will have to configured the file *settings/dev-properties*.

You will need a valid private key (*private_key_location*) to connect to NGSI-Adapter Host to capture remote logs.
In this way, you will be able to execute Scenarios that require the logs capturing for test validations.
