===================
 FIWARE Monitoring
===================

|License Badge| |Documentation Badge| |StackOverflow| |Build Status| |Coverage Status| 

This is the code repository for FIWARE Monitoring, the reference implementation
of the Monitoring GE.

This project is part of FIWARE_. Check also the
`FIWARE Catalogue entry for Monitoring`__.

__ `FIWARE Catalogue - Monitoring GE`_

Any feedback on this documentation is highly welcome, including bugs, typos
or things you think should be included but aren't. You can use `github issues`__
to provide feedback.

__ `FIWARE Monitoring - GitHub issues`_

For documentation previous to release 4.4.2 please check the manuals at FIWARE
public wiki:

- `FIWARE Monitoring - Installation and Administration Guide`_
- `FIWARE Monitoring - User and Programmers Guide`_


GEi overall description
=======================

FIWARE Monitoring is the key component to allow incorporating monitoring and
metering mechanisms in order be able to constantly check the performance of
the cloud infrastructure.

This involves gathering operational data in a running system, which usually
requires collecting data from heterogeneous sources. Besides, the monitoring
architecture should be easily extended to collect additional data for any
other required needs.

FIWARE Monitoring is agnostic to the *framework* used to gather monitoring
data. It just assumes there are several *monitoring probes* collecting
information, which somehow must be forwarded to an *adaptation layer*,
responsible for transforming data into a common representation (*NGSI*)
and publishing through a *Context Broker* (see Orion__).

__ `FIWARE Orion Context Broker`_

Collected information can be used for several purposes:

- Cloud users to track the performance of their own instances.
- SLA management, in order to check adherence to agreement terms.
- Optimization of virtual machines.


Components
----------

Monitoring framework
    It is up to the infrastructure owner which tool (like Nagios_, Zabbix_,
    openNMS_, perfSONAR_, etc.) is installed for this purpose.

Collector
    Framework-specific component to forward monitoring data being gathered
    to the adaptation layer (i.e. *NGSI Adapter*). Monitoring GE provides a
    Nagios loadable module `NGSI Event Broker <ngsi_event_broker/README.rst>`_
    as collector for such monitoring framework.

Adaptation layer
    `NGSI Adapter <ngsi_adapter/README.rst>`_ serves as generic adapter to
    transform monitoring data from probes to NGSI context attributes.


Build and Install
=================

The recommended procedure is to install using *rpm* packages in CentOS 6.x,
or *deb* packages in Ubuntu 12.04/14.04 LTS. If you are interested in building
from sources, check `this document <doc/manuals/admin/build_source.rst>`_.


Requirements
------------

- System resources: see `these recommendations
  <doc/manuals/admin/index.rst#resource-availability>`_.
- Operating systems: CentOS (or RedHat) and Ubuntu (or Debian),
  being CentOS 6.3 the reference operating system.
- Package dependencies: some required packages may not be present in official
  repositories, or their versions are too old (for example, ``nodejs``). In any
  case, checking for such dependencies and configuration of alternative sources
  is automatically managed by the package installation scripts when using the
  proper tool (``yum`` in CentOS or ``apt-get``/``gdebi`` in Ubuntu).


Installation
------------

Using FIWARE package repository (recommended)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Refer to the documentation of your Linux distribution to set up the URL of the
repository where FIWARE packages are available (and update cache, if needed):

**CentOS** ::

    [fiware]
    name=FIWARE Repository
    baseurl=http://repositories.lab.fiware.org/repo/rpm/$releasever
    gpgcheck=0
    enabled=1

**Ubuntu** ::

    deb http://repositories.lab.fiware.org/repo/deb <your_release> main
    deb-src http://repositories.lab.fiware.org/repo/deb <your_release> main

Then, use the proper tool to install the packages (this depends on monitoring
framework used in the cloud infrastructure, but at least NGSI Adapter will be
installed in any case):

**CentOS** ::

    $ sudo yum install fiware-monitoring-ngsi-adapter

**Ubuntu** ::

    $ sudo apt-get install fiware-monitoring-ngsi-adapter

Additionally, in case Nagios 3.4/3.5 and its probes (*Nagios Plugins*) are
being used as the framework to gather monitoring data, then we may install
the package ``fiware-monitoring-ngsi-event-broker`` (see Components_ above).


Using the .rpm/.deb files
~~~~~~~~~~~~~~~~~~~~~~~~~

Download the package(s) from the `FIWARE Files area`__ and use the proper
tool to install it. Take into account that you may need to manually install
dependencies, as some tools aren't able to manage them when installing from
file:

__ `FIWARE Monitoring - Forge files area`_

**CentOS** ::

    $ sudo rpm -i fiware-monitoring-ngsi-adapter-X.Y.Z.noarch.rpm
    $ sudo rpm -i fiware-monitoring-ngsi-event-broker-X.Y.Z.x86_64.rpm

**Ubuntu** ::

    $ sudo dpkg -i fiware-monitoring-ngsi-adapter_X.Y.Z_all.deb
    $ sudo dpkg -i fiware-monitoring-ngsi-event-broker_X.Y.Z_amd64.deb


Upgrading from a previous version
---------------------------------

Unless explicitly stated, no migration steps are required to upgrade to a
newer version of the Monitoring components:

- When using the package repositories, just follow the same directions
  described in the Installation_ section (the ``install`` subcommand also
  performs upgrades).
- When upgrading from downloaded package files, use ``rpm -U`` in CentOS, or
  use same ``dpkg -i`` command in Ubuntu.


Running
=======

As explained in the `overall description`__ section, there are a variety of
elements involved in the monitoring architecture, apart from those components
provided by this Monitoring GE (at least, an instance of *Context Broker* is
required and some underlying monitoring framework, such as *Nagios*). Please
refer to their respective documentation for instructions to run them.

__ `GEi overall description`_

From the Monitoring GE components, only NGSI Adapter runs as standalone server.
Once installed, there are two ways of running NGSI Adapter: manually from the
command line or as a system service (the latter only available if installed as
a package). It is not recommended to mix both ways (e.g. start it manually but
use the service scripts to stop it). This section assumes you are using the
system service (recommended): for the command line alternative, please refer
to `this document <doc/manuals/admin/index.rst#from-the-command-line>`_.

In order to start the adapter service, run::

    $ sudo service ngsi_adapter start

Then, to stop the service, run::

    $ sudo service ngsi_adapter stop

We can also force a service restart::

    $ sudo service ngsi_adapter restart


Configuration file
------------------

The configuration used by the adapter service is optionally read from the file
``/etc/sysconfig/ngsi_adapter`` (in CentOS) or ``/etc/default/ngsi_adapter``
(in Ubuntu):

::

    # ADAPTER_LOGFILE - Logging file
    ADAPTER_LOGFILE=/var/log/ngsi_adapter/ngsi_adapter.log

    # ADAPTER_LOGLEVEL - Logging level
    ADAPTER_LOGLEVEL=INFO

    # ADAPTER_LISTEN_HOST - The host where NGSI Adapter listens to requests
    ADAPTER_LISTEN_HOST=0.0.0.0

    # ADAPTER_LISTEN_PORT - The port where NGSI Adapter listens to requests
    ADAPTER_LISTEN_PORT=1337

    # ADAPTER_UDP_ENDPOINTS - UDP listen endpoints (host:port:parser,...)

    # ADAPTER_PARSERS_PATH - Path with directories to look for parsers
    ADAPTER_PARSERS_PATH=lib/parsers/nagios

    # ADAPTER_BROKER_URL - The endpoint where Context Broker is listening
    ADAPTER_BROKER_URL=http://127.0.0.1:1026/

    # ADAPTER_MAX_REQUESTS - Maximum number of simultaneous requests
    ADAPTER_MAX_REQUESTS=5

    # ADAPTER_RETRIES - Maximum number of retries invoking Context Broker
    ADAPTER_RETRIES=2


Most of these attributes map to options of the `command line interface
<doc/manuals/admin/index.rst#from-the-command-line>`_ as follows:

- ``ADAPTER_LOGLEVEL`` maps to ``-l`` or ``--logLevel`` option
- ``ADAPTER_LISTEN_HOST`` maps to ``-H`` or ``--listenHost`` option
- ``ADAPTER_LISTEN_PORT`` maps to ``-p`` or ``--listenPort`` option
- ``ADAPTER_UDP_ENDPOINTS`` maps to ``-u`` or ``--udpEndpoints`` option
- ``ADAPTER_PARSERS_PATH`` maps to ``-P`` or ``--parsersPath`` option
- ``ADAPTER_BROKER_URL`` maps to ``-b`` or ``--brokerUrl`` option
- ``ADAPTER_MAX_REQUESTS`` maps to ``-m`` or ``--maxRequests`` option
- ``ADAPTER_RETRIES`` maps to ``-r`` or ``--retries`` option

Default values are found in ``/opt/fiware/ngsi_adapter/lib/common.js``.


Checking status
---------------

In order to check the status of the adapter service, use the following command
(no special privileges required):

::

    $ service ngsi_adapter status


API Overview
============

To transform monitoring data into NGSI attributes, probe raw data should be
sent as body of a POST request to the adapter, identifying the source entity
being monitored in the query fields.

For example, if using the ``check_load`` Nagios probe to measure CPU load,
then the request would look like:

::

    $ curl "{adapter_endpoint}/check_load?id={myhostname}&type=host" -s -S \
    --header 'Content-Type: text/plain' -X POST -d @- <<-EOF
    OK - load average: 5.00, 7.01, 7.05|load1=5.000;10.000;10.000;0;
    load5=7.010;15.000;15.000;0; load15=7.050;30.000;30.000;0;
    EOF

This would result in an invocation to Context Broker updating the context
of an entity of type ``host`` identified by ``myhostname`` with a new
attribute ``cpuLoadPct`` with value ``5.00``.

Please have a look at the `API Reference Documentation`_ section bellow and
at the `programmer guide <doc/manuals/user/index.rst#programmer-guide>`_.


API Reference Documentation
---------------------------

- `FIWARE Monitoring v1 (Apiary)`__

__ `FIWARE Monitoring - Apiary`_


Testing
=======

End-to-end tests
----------------

Please refer to the `Installation and administration guide
<doc/manuals/admin/index.rst#end-to-end-testing>`_ for details.


Unit tests
----------

The ``test`` target is used for running the unit tests in both components of
Monitoring GE:

::

    $ cd ngsi_adapter
    $ grunt test

    $ cd ngsi_event_broker
    $ make test  # synonym of standard 'check' target


Please have a look at the section `building from source code
<doc/manuals/admin/build_source.rst>`_ in order to get more
information about how to prepare the environment to run the
unit tests.


Acceptance tests
----------------

In the following documents you will find a business readable description of the
features provided by the components of the Monitoring GE, as well as automated
tests for them:

- `NGSI Adapter acceptance tests <ngsi_adapter/test/acceptance/README.rst>`_


Advanced topics
===============

- `Installation and administration <doc/manuals/admin/README.rst>`_

  * `Building from sources <doc/manuals/admin/build_source.rst>`_
  * `Running Adapter from command line <doc/manuals/admin/index.rst#from-the-command-line>`_
  * `Logs <doc/manuals/admin/logs.rst>`_
  * `Resources & I/O Flows <doc/manuals/admin/index.rst#resource-availability>`_

- `User and programmers guide <doc/manuals/user/README.rst>`_

  * `NGSI Adapter custom probe parsers <doc/manuals/user/index.rst#ngsi-adapter-parsers>`_
  * `Retrieval of historical data <doc/manuals/user/index.rst#monitoring-api>`_


License
=======

\(c) 2013-2016 Telefónica I+D, Apache License 2.0


.. IMAGES

.. |Build Status| image:: https://travis-ci.org/telefonicaid/fiware-monitoring.svg?branch=develop
   :target: https://travis-ci.org/telefonicaid/fiware-monitoring
   :alt: Build Status
.. |Coverage Status| image:: https://img.shields.io/coveralls/telefonicaid/fiware-monitoring/develop.svg
   :target: https://coveralls.io/r/telefonicaid/fiware-monitoring
   :alt: Coverage Status
.. |StackOverflow| image:: https://img.shields.io/badge/support-sof-yellowgreen.svg
   :target: https://stackoverflow.com/questions/tagged/fiware-monitoring
   :alt: Help, ask questions
.. |License Badge| image:: https://img.shields.io/badge/license-Apache_2.0-blue.svg
   :target: ngsi_adapter/LICENSE
.. |Documentation Badge| image:: https://readthedocs.org/projects/fiware-monitoring/badge/?version=latest
   :target: http://fiware-monitoring.readthedocs.org/en/latest/?badge=latest
   :alt: License


.. REFERENCES

.. _FIWARE: http://www.fiware.org
.. _FIWARE Catalogue - Monitoring GE: http://catalogue.fiware.org/enablers/monitoring-ge-sextant
.. _FIWARE Monitoring - GitHub issues: https://github.com/telefonicaid/fiware-monitoring/issues/new
.. _FIWARE Monitoring - User and Programmers Guide: https://forge.fiware.org/plugins/mediawiki/wiki/fiware/index.php/Monitoring_-_User_and_Programmers_Guide
.. _FIWARE Monitoring - Installation and Administration Guide: https://forge.fiware.org/plugins/mediawiki/wiki/fiware/index.php/Monitoring_-_Installation_and_Administration_Guide
.. _FIWARE Monitoring - Forge files area: https://forge.fiware.org/frs/?group_id=7#title_cloud-monitoring
.. _FIWARE Monitoring - Apiary: https://jsapi.apiary.io/apis/fiwaremonitoring/reference.html
.. _FIWARE Orion Context Broker: https://github.com/telefonicaid/fiware-orion
.. _Nagios: http://www.nagios.org/
.. _Zabbix: http://www.zabbix.com/
.. _openNMS: http://www.opennms.org/
.. _perfSONAR: http://www.perfsonar.net/
