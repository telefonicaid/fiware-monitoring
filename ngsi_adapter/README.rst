NGSI Adapter
____________


Generic adapter to transform monitoring data from probes to NGSI context
attributes, and forward them through a NGSI Context Broker.


Installation
============

Adapter is distributed as a Ubuntu (.deb) or CentOS (.rpm) package. Assuming
FIWARE package repositories are configured, just use the proper tool (such as
``apt-get`` or ``yum``) to install ``fiware-monitoring-ngsi-adapter`` package.
These distributions are currently supported:

-  Ubuntu 12.04 LTS
-  CentOS 6.3

During installation process, Node.js engine version is checked and package
dependencies are resolved using ``npm`` tool. Upon successful installation,
a Linux service ``ngsi_adapter`` is created.


Usage
=====

Adapter runs as a standalone server listening for HTTP POST requests at the
given endpoint. To start the adapter as service, just type:

.. code::

   $ sudo service ngsi_adapter start


We can also start the adapter manually from the command line:

.. code::

   $ cd {installation_path}
   $ adapter


Context Broker URL, adapter listen port, logging level and other parameters may
be supplied as command line arguments, overriding the default values defined
in *config/options.js*:

.. code::

   $ adapter --listenPort 1337 --brokerUrl http://{host}:{port}/


For detailed information about command line options and their defaults, please
run:

.. code::

   $ adapter --help


To modify the parameters to run the adapter as a service, please set variable
``DAEMON_ARGS`` in */etc/init.d/ngsi\_adapter* script.


Requests
========

Probe raw data should be sent as body of a POST request to the adapter,
identifying the source entity being monitored in the query fields (for
further information, see `docs.fiwaremonitoring.apiary.io`__):

__ http://docs.fiwaremonitoring.apiary.io/

.. code::

   POST http://{adapter_endpoint}/{probe}?id={id}&type={type}

-  *{probe}* = name of the originating probe
-  *{id}* = identifier of the entity being monitored
-  *{type}* = type of the entity

Adapter processes requests asynchronously, indicating in the response status
code whether *{probe}* is recognized (status 200) or not (404).

In case of known probes, adapter dynamically loads a custom probe parser
that should be located at *lib/parsers/{probe}.js* and invokes its
*getContextAttrs()* method, passing probe raw data as attribute(s) of
an object *EntityData*.


Changelog
=========

Version 1.2.3

-  Fix problems when uninstalling package

Version 1.2.2

-  Add .rpm package generation
-  Minor bugs resolved

Version 1.2.1

-  Minor bugs resolved

Version 1.2.0

-  Add new log format (issue #25)

Version 1.1.1

-  Add .deb package generation (issue #16)

Version 1.1.0

-  Add timestamp to entity attributes (issue #4)

Version 1.0.1

-  NGSI9 and NGSI10 decoupled in update requests by using append action
-  Bugfixing: solved errors in update requests

Version 1.0.0

-  Only Nagios probes (plugins) are supported


License
=======

\(c) 2013-2015 Telefónica I+D, Apache License 2.0
