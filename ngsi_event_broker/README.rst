===================
 NGSI Event Broker
===================

Nagios event broker (NEB_) module to forward plugin data to `NGSI Adapter
<../ngsi_adapter/README.rst>`_. Current release comprises two customizations:

- *ngsi\_event\_broker\_fiware* to process plugin executions for `FIWARE Lab`_
  monitoring.
- *ngsi\_event\_broker\_xifi* used in XIFI_ project.


Installation
============

The module is an architecture-dependent compiled shared object distributed as
a single library bundled in an Ubuntu (.deb) or CentOS (.rpm) package.

Please refer to `this document </README.rst#build-and-install>`_ for details.


Usage
=====

Nagios should be instructed to load this module on startup. First, stop Nagios
service and then edit configuration file at ``/etc/nagios/nagios.cfg`` to add
the new broker module with its arguments: the id of the region__ (i.e. domain)
that monitored resources belong to, and the endpoint of NGSI Adapter component
to request:

__ `OpenStack region`_

.. code::

   event_broker_options=-1
   broker_module=/path/ngsi_event_broker_fiware.so -r region -u http://host:port

The module will use such information given as arguments together with data taken
from the `Nagios service definition`_ to issue a request to NGSI Adapter. In
many cases, service definitions need no modifications and the broker just works
transparently once Nagios is restarted. But there are some scenarios requiring
slight changes in those service definitions (see below).

Once main configuration file and service definitions have been reviewed, then
start Nagios service. Check log files for module initialization (may fail for
missing arguments, for example). Also check that requests are sent to Adapter
server in response to plugin executions.


Service definitions
-------------------

Assuming this Nagios host definition:

.. code::

   define host{
       use                     linux-server
       host_name               my_host_name
       alias                   linux_server
       address                 192.168.0.2
       }

then a typical Nagios service definition would look like this:

.. code::

   define service{
       use                     generic-service
       host_name               my_host_name
       service_description     my_service_description
       check_command           check_name!arguments
       ...
       }

Depending on the entities being monitored (thus depending on the kind of plugins
used), some of these data items are taken and some additional may be required.
Requests to NGSI Adapter issued by this broker will all follow the pattern
``http://{host}:{port}/{check_name}?id={region}:{uniqueid}&type={type}``, where:

-  ``http://{host}:{port}`` is the endpoint taken from broker arguments
-  ``{check_name}`` is taken from Nagios command specified at service definition
   (for plugins executed remotely via NRPE, it will be taken from the arguments
   of ``check_nrpe``)
-  ``{region}`` is taken from broker arguments
-  ``{uniqueid}`` is taken from service definition, and may depend on the
   command
-  ``{type}`` is also taken from service definition, and may also depend on
   the command

For *GEri global instance monitoring* there are no restrictions on the command
names and the plugins to be used. The ``{uniqueid}`` will result from the
concatenation of the ``host_name`` and ``service_description``  defined in
the corresponding Nagios service, while ``{type}`` must be explicitly given
with a custom variable ``_entity_type`` in the service definition (or using
templates, as follows):

.. code::

   define service{
       use                     generic-service
       name                    fiware-ge-service
       _entity_type            ge
       }

   define service{
       use                     fiware-ge-service
       host_name               my_host_name
       service_description     my_service_description
       check_command           check_name!arguments
       ...
       }


Changelog
=========

Please refer to `FIWARE Monitoring releases changelog`_.


License
=======

\(c) 2013-2016 Telefónica I+D, Apache License 2.0


.. REFERENCES

.. _NEB: http://nagios.sourceforge.net/download/contrib/documentation/misc/NEB%202x%20Module%20API.pdf
.. _Nagios service definition: http://nagios.sourceforge.net/docs/3_0/objectdefinitions.html#service
.. _OpenStack region: http://docs.openstack.org/glossary/content/glossary.html#region
.. _FIWARE Monitoring releases changelog: https://github.com/telefonicaid/fiware-monitoring/releases
.. _FIWARE Lab: https://www.fiware.org/lab/
.. _XIFI: https://www.fi-xifi.eu/home.html
