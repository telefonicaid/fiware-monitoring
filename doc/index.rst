==============================
 Welcome to FIWARE Monitoring
==============================

Introduction
============

FIWARE Monitoring GEri is the key component to allow incorporating monitoring
and metering mechanisms in order be able to constantly check the performance
of the system, but the architecture should be easily extended to collect data
for other required needs. Monitoring involves gathering operational data in a
running system. Collected information can be used for several purposes:

- Cloud users to track the performance of their own instances.
- SLA management, in order to check adherence to agreement terms.
- Optimization of virtual machines.

The monitoring system is used by different Cloud GEs in order to track the
status of the resources. They use gathered data to take decisions about
elasticity or for SLA management. Whenever a new resource is deployed in the
cloud, the proper monitoring probe is set up and configured to start providing
monitoring data. 

FIWARE Monitoring source code can be found here__.

__ `FIWARE Monitoring GitHub Repository`_


Documentation
=============

GitHub's README__ provides a good documentation summary, and the following
cover more advanced topics:

__ `FIWARE Monitoring GitHub README`_

.. toctree::
   :maxdepth: 1

   manuals/user/index
   manuals/admin/index


See also
========

-  `Monitoring Federation OpenStack Infrastructure`_ presentation summarises
   the development of this component as a joint task between FIWARE and XIFI
   projects.


.. REFERENCES

.. _FIWARE Monitoring GitHub Repository: https://github.com/telefonicaid/fiware-monitoring.git
.. _FIWARE Monitoring GitHub README: https://github.com/telefonicaid/fiware-monitoring/blob/master/README.rst
.. _Monitoring Federation OpenStack Infrastructure: http://www.slideshare.net/flopezaguilar/monitoring-federation-open-stack-infrastructure
