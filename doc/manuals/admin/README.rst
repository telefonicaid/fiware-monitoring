=======================================
 Installation and Administration Guide
=======================================

Introduction
============

This guide defines the procedure to install the different components that build
up the Monitoring GE, including its requirements and possible troubleshooting.

For general information, please refer to `this document </README.rst>`_.


Installation
============

Monitoring infrastructure comprises several elements distributed across
different hosts, as depicted in the following figure:

.. figure:: Monitoring_IO_Flows.png
   :alt: Monitoring components.

   ..

   #. **Probes** gather raw monitoring data, which a **Collector** (for Nagios,
      this is *NGSI Event Broker*) forwards to *NGSI Adapter*.
   #. **NGSI Adapter**, responsible for translating probe raw data into a
      common format (NGSI).
   #. **Parsers** at NGSI Adapter, specific for the different probes that
      generate monitoring data.
   #. **Context Broker**, where monitoring data (transformed into NGSI context
      updates) will be published.
   #. **Hadoop**, for storing historical context data.
   #. **Connector** between Context Broker and data storage (for example, this
      could be *Cygnus*).


Installation of probes
----------------------

Monitoring GE is agnostic to the framework used to gather monitoring data. It
just assumes there are several probes collecting such data, which somehow will
be forwarded to the adaptation layer (NGSI Adapter).

It is up to the infrastructure owner which tool (like Nagios_, Zabbix_,
openNMS_, perfSONAR_, etc.) is installed for this purpose.


Installation of collector
-------------------------

Probes must "publish" their data to NGSI Adapter. Depending on the exact
monitoring tool installed, a kind of *collector* has to be deployed in
order to send data to the adapter:

- **NGSI Event Broker** is an example specific for Nagios, implemented as
  a loadable module. Description and installation details can be found
  `here </ngsi_event_broker/README.rst>`_.


Installation of NGSI Adapter
----------------------------

Requirements
~~~~~~~~~~~~

NGSI Adapter should work on a variety of operating systems, particularly on the
majority of GNU/Linux distributions (e.g. Debian, Ubuntu, CentOS), as it only
requires a V8 JavaScript Engine to run a Node.js server.

Hardware Requirements
^^^^^^^^^^^^^^^^^^^^^

The minimal requirements are:

- RAM: 2 GB


Software Requirements
^^^^^^^^^^^^^^^^^^^^^

NGSI Adapter is a standalone Node.js process, so ``node`` and its package
manager ``npm`` should be installed previously. These requirements are
automatically checked when installing the ``fiware-monitoring-ngsi-adapter``
package. However, for manual installation please visit NodeSource_.


Downloads
~~~~~~~~~

Please refer to `this document </README.rst#Installation>`_ for details.


Additional parsers
~~~~~~~~~~~~~~~~~~

NGSI Adapter currently includes a predefined set of parsers for Nagios probes
at ``lib/parsers/nagios`` directory, each named after its corresponding probe.

This can be extended with additional parsers found at from other directories.
To do so, please configure ``--parsersPath`` command line option (or set the
variable ``ADAPTER_PARSERS_PATH``) with a colon-separated list of absolute (or
relative to Adapter root) directories where parsers are located.


Installation of Context Broker
------------------------------

Please refer to Orion_ documentation.


Installation of the connector
-----------------------------

This component subscribes to changes at Context Broker and writes data into a
distributed filesystem storage (usually HDFS from Hadoop_). Historically the
**ngsi2cosmos** connector implementation has been used (installation details
here__), although from March 2014 this component is deprecated and a brand new
**Cygnus** implementation (installation details here__) is available.

__ ngsi2cosmos_
__ Cygnus_


Running the monitoring components
=================================

As stated before, there are a number of distributed components involved in the
monitoring. Please refer to their respective installation manuals for execution
details (this applies to probes & monitoring software, Context Broker, Hadoop,
etc.). This section focuses on NGSI Adapter specific instructions.


Running NGSI Adapter
--------------------

Once installed, there are two ways of running NGSI Adapter: manually from the
command line or as a system service. It is not recommended to mix both ways
(e.g. start it manually but using the service scripts to stop it).


As system service
~~~~~~~~~~~~~~~~~

When installed from its package distribution, a Linux service ``ngsi_adapter``
is configured (but not started). Please refer to `this document
</README.rst#Running>`_ for details.


From the command line
~~~~~~~~~~~~~~~~~~~~~

You can run the adapter just typing the following command at the installation
directory (usually ``/opt/fiware/ngsi_adapter/``):

.. code::

   $ adapter


You can use these command line options (available typing ``adapter --help``):

-l, --logLevel
   Verbosity of log messages
-H, --listenHost
   The hostname or address at which NGSI Adapter listens
-p, --listenPort
   The port number at which NGSI Adapter listens
-u, --udpEndpoints
   Optional list of UDP endpoints (host:port:parser)
-P, --parsersPath
   Colon-separated path with directories to look for parsers
-b, --brokerUrl
   The URL of the Context Broker instance to publish data to
-r, --retries
   Number of times a request to Context Broker is retried, in case of error


Sanity check procedures
=======================

These are the steps that a System Administrator will take to verify that an
installation is ready to be tested. This is therefore a preliminary set of
tests to ensure that obvious or basic malfunctioning is fixed before proceeding
to unit tests, integration tests and user validation.


End to end testing
------------------

Use the commands of the monitoring framework being used (for example, Nagios)
to reschedule some probe execution and force the generation of new monitoring
data:

- Check the logs of the framework (i.e. ``/var/log/nagios/nagios.log``) for
  a new probe execution detected by the *collector*::

   $ cat /var/log/nagios/nagios.log
   [1439283831] lvl=INFO | trans=rdPmJ/uHE62a | comp=fiware-monitoring-ngsi-event-broker | op=NGSIAdapter | msg=Request sent to http://host:1337/check_xxx?id=xxx&type=host


- Check NGSI Adapter logs for incoming requests with raw data, and for the
  corresponding updateContext() request to Context Broker::

   $ cat /var/log/ngsi_adapter/ngsi_adapter.log
   time=... | lvl=INFO | trans=rdPmJ/uHE62a | op=POST | msg=Request on resource /check_xxx with params id=xxx&type=xxx
   time=... | lvl=INFO | trans=rdPmJ/uHE62a | op=POST | msg=Response status 200 OK
   time=... | lvl=INFO | trans=rdPmJ/uHE62a | op=UpdateContext | msg=Request to ContextBroker at http://host:1026/...

- Finally, query Context Broker API to check whether entity attributes have
  been updated according to the new monitoring data (see details here__)

__ Orion_


List of Running Processes
-------------------------

A ``node`` process running the "adapter" server should be up and running, e.g.:

.. code::

   $ ps -C node -f | grep adapter
   fiware   21930     1  0 Mar28 ?        00:06:06 node /opt/fiware/ngsi_adapter/src/adapter


Alternatively, we can check if service is running, e.g.:

.. code::

   $ service ngsi_adapter status
   * ngsi_adapter is running


Network interfaces Up & Open
----------------------------

NGSI Adapter uses TCP 1337 as default port, although it can be changed using
the ``--listenPort`` command line option.

Additionally, a list of UDP listen ports may be specified by ``--udpEndpoints``
command line option.


Databases
---------

This component does not persist any data, and no database engine is needed.


Diagnosis Procedures
====================

The Diagnosis Procedures are the first steps that a System Administrator will
take to locate the source of an error in a GE. Once the nature of the error is
identified with these tests, the system admin will very often have to resort to
more concrete and specific testing to pinpoint the exact point of error and a
possible solution. Such specific testing is out of the scope of this section.


Resource availability
---------------------

Although we haven't done yet a precise profiling on NGSI Adapter, tests done in
our development and testing environment show that a host with 2 CPU cores and
4 GB RAM is fine to run server.


Remote service access
---------------------

- Probes at monitored hosts should have access to NGSI Adapter listen
  port (TCP 1337, by default)

- NGSI Adapter should have access to Context Broker listen port (TCP 1026,
  by default)

- Connector should have access to Context Broker listen port in order
  to subscribe to context changes

- Context Broker should have access to Connector callback port to notify
  changes


Resource consumption
--------------------

No issues related to resources consumption have been detected neither with
the NGSI Adapter server nor with the NGSI Event Broker loaded as a "pluggable"
module on Nagios startup.


I/O flows
---------

Figure at `installation section`__ shows the I/O flows among the different
monitoring components:

__ Installation_

- Probes send requests to NGSI Adapter with raw monitoring data, by means
  of a custom *collector* component (for example, NGSI Event Broker)

- NGSI Adapter sends request to Context Broker in terms of context
  updates of the monitored resources

- Context Broker notifies Connector with every context change

- Connector writes changes to storage


.. REFERENCES

.. _Orion: https://github.com/telefonicaid/fiware-orion/
.. _Cygnus: https://github.com/telefonicaid/fiware-cygnus/
.. _ngsi2cosmos: https://github.com/telefonicaid/fiware-livedemoapp#ngsi2cosmos
.. _NodeSource: https://github.com/nodesource/distributions/
.. _Hadoop: http://hadoop.apache.org/
.. _Nagios: http://www.nagios.org/
.. _Zabbix: http://www.zabbix.com/
.. _openNMS: http://www.opennms.org/
.. _perfSONAR: http://www.perfsonar.net/
