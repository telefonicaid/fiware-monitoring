FIWARE Monitoring GE
____________________


| |Build Status| |Coverage Status|


Description
===========

This generic enabler allows gathering data from heterogeneous monitoring and
metering systems in order to constantly check the status and performance of the
cloud infrastructure.

Collected information can be used for several purposes:

-  Cloud users to track the performance of their own instances.
-  SLA management, in order to check adherence to agreement terms.
-  Optimization of virtual machines.

For more information, please refer to the `documentation <doc/README.rst>`_ and,
additionally, to the description of this GE in `FIWARE Catalogue`__.

__ `FIWARE Catalogue - Monitoring GE`_


Components
==========

`NGSI Adapter <ngsi_adapter/README.rst>`_
   Generic adapter to transform monitoring data from probes to NGSI context
   attributes.

`NGSI Event Broker <ngsi_event_broker/README.rst>`_
   Nagios module to forward plugin data to NGSI Adapter.


Releases
========

`FIWARE 4.2.3`_

-  NGSI Event Broker version 1.4.3
-  NGSI Adapter version 1.2.3

`FIWARE 4.2.2`_

-  NGSI Event Broker version 1.4.2
-  NGSI Adapter version 1.2.2

`FIWARE 4.2.1`_

-  NGSI Event Broker version 1.4.1
-  NGSI Adapter version 1.2.1

`FIWARE 4.1.1`_

-  NGSI Event Broker version 1.4.0
-  NGSI Adapter version 1.2.0

`FIWARE 3.5.2`_

-  NGSI Event Broker version 1.3.1
-  NGSI Adapter version 1.1.1

`FIWARE 3.5.1`_

-  NGSI Event Broker version 1.3.1
-  NGSI Adapter version 1.1.0

`FIWARE 3.3.3`_

-  NGSI Event Broker version 1.3.0
-  NGSI Adapter version 1.0.1

`FIWARE 3.3.2`_

-  NGSI Event Broker version 1.1.0
-  NGSI Adapter version 1.0.1


License
=======

\(c) 2013-2015 Telefónica I+D, Apache License 2.0


.. IMAGES

.. |Build Status| image:: https://travis-ci.org/telefonicaid/fiware-monitoring.svg?branch=develop
   :target: https://travis-ci.org/telefonicaid/fiware-monitoring
.. |Coverage Status| image:: https://coveralls.io/repos/telefonicaid/fiware-monitoring/badge.png?branch=develop
   :target: https://coveralls.io/r/telefonicaid/fiware-monitoring


.. REFERENCES

.. _FIWARE Catalogue - Monitoring GE: http://catalogue.fiware.org/enablers/monitoring-ge-tid-implementation
.. _FIWARE 4.2.3: https://forge.fiware.org/frs/?group_id=7&release_id=576#cloud-monitoring-4-2-3-title-content
.. _FIWARE 4.2.2: https://forge.fiware.org/frs/?group_id=7&release_id=571#cloud-monitoring-4-2-2-title-content
.. _FIWARE 4.2.1: https://forge.fiware.org/frs/?group_id=7&release_id=563#cloud-monitoring-4-2-1-title-content
.. _FIWARE 4.1.1: https://forge.fiware.org/frs/?group_id=7&release_id=544#cloud-monitoring-4-1-1-title-content
.. _FIWARE 3.5.2: https://forge.fiware.org/frs/?group_id=7&release_id=529#cloud-monitoring-3-5-2-title-content
.. _FIWARE 3.5.1: https://forge.fiware.org/frs/?group_id=7&release_id=506#cloud-monitoring-3-5-1-title-content
.. _FIWARE 3.3.3: https://forge.fiware.org/frs/?group_id=7&release_id=419#cloud-monitoring-3-3-3-title-content
.. _FIWARE 3.3.2: https://forge.fiware.org/frs/?group_id=23&release_id=399#cloud-monitoring-3-3-2-title-content
