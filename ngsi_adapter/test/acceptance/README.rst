===============================
 NGSI Adapter acceptance tests
===============================

This project contains the NGSI Adapter acceptance tests (component, integration
and E2E testing). All test cases have been defined using Gherkin_, that it is a
Business Readable, Domain Specific Language that lets you describe software’s
behaviour without detailing how that behaviour is implemented. Gherkin has the
purpose of serving documentation of test cases.

Test case implementation has been performed using Python_ and Lettuce_.


Acceptance Project Structure
============================

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
======================================

Features:

- Lettuce-Tools support
- Settings using json files and Lettuce-Tools utility
- Test report using Lettuce-Tools XUnit output
- NGSI Adapter Client
- Logging
- Remote NGSI Adapter log capturing
- Test data management using templates (resources)


Acceptance tests execution
==========================

Execute the following command in the test project root directory:

::

    $ cd ngsi_adapter/src/test/acceptance
    $ lettuce_tools -ft send_data_api_resource -ts comp -sd features/ --tags=-skip -en dev

With this command, you will execute:

- Components test cases in the "Development" environment configured in file
  ``settings/dev-properties.json``
- The "send_data_api_resource" feature
- Skipping all scenarios tagged with ``"skip"``


**Prerequisites**

- Python 2.7 or newer (2.x) (https://www.python.org/downloads/)
- pip (https://pypi.python.org/pypi/pip)
- virtualenv (https://pypi.python.org/pypi/virtualenv)
- NGSI Adapter from FIWARE Monitoring (`download sources`__)

__ `NGSI Adapter sources`_

**Test case execution using virtualenv**

1. Create a virtual environment somewhere::

    $ virtualenv $WORKON_HOME/venv

#. Activate the virtual environment::

    $ source $WORKON_HOME/venv/bin/activate

#. Go to the acceptance tests folder in the project::

    $ cd ngsi_adapter/src/test/acceptance

#. Install requirements for the acceptance tests in the virtual environment::

    $ pip install -r requirements.txt --allow-all-external

**Test case execution using Vagrant (optional)**

Instead of using ``virtualenv``, you can use Vagrant_ to deploy a local VM from
the given *Vagrantfile*, providing all environment configurations to launch the
test cases.

As a prerequisite, first download and install Vagrant
(https://www.vagrantup.com/downloads.html)

1. Go to the acceptance tests folder in the project::

    $ cd ngsi_adapter/src/test/acceptance

#. Launch a VM from the provided *Vagrantfile*::

    $ vagrant up

#. After Vagrant provision, your VM is properly configured to launch acceptance
   tests. You have to access the VM and change to the Vagrant directory mapping
   the *test/acceptance* workspace::

    $ vagrant ssh
    $ cd /vagrant

For more information about how to use Vagrant, please check `this document`__.

__ `Vagrant Getting Started`_

**Settings**

Before executing the acceptance tests, you will have configure the properties
file ``settings/dev-properties.json`` and setup the attributes to run the
acceptance tests on the experimentation environment.

You will also need a valid private key (*private_key_location*) to connect to
NGSI Adapter host to capture remote logs. This way you will be able to execute
scenarios that require the logs capturing for test validations.


.. REFERENCES

.. _Gherkin: https://github.com/cucumber/cucumber/wiki/Gherkin
.. _Lettuce: http://lettuce.it/
.. _Python: http://www.python.org/
.. _Vagrant: https://www.vagrantup.com/
.. _Vagrant Getting Started: https://docs.vagrantup.com/v2/getting-started/index.html
.. _NGSI Adapter sources: https://github.com/telefonicaid/fiware-monitoring/
