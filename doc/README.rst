==========
 Overview
==========


What you get
============

Monitoring GE - FIWARE Implementation is the key component to allow
incorporating monitoring and metering mechanisms in order be able to
constantly check the performance of the system, but the architecture should
be easily extended to collect data for other required needs. Monitoring
involves gathering operational data in a running system. Collected information
can be used for several purposes:

- Cloud users to track the performance of their own instances.
- SLA management, in order to check adherence to agreement terms.
- Optimization of virtual machines.

The monitoring system is used by different Cloud GEs in order to track the
status of the resources. They use gathered data to take decisions about
elasticity or for SLA management. Whenever a new resource is deployed in the
cloud, the proper monitoring probe is set up and configured to start providing
monitoring data.


Why to get it
=============

Monitoring GE - FIWARE Implementation is the monitoring platform to be used
in the FIWARE Cloud ecosystem in order to monitoring and metering virtual
resources. This middleware unify the monitoring and metering solution,
providing the following advantages:

- **Full FIWARE integrated solution**

  This component is integrated with the architecture deployed in the FIWARE
  and you do not need to do extra work in order to integrate the solution with
  the rest of Generic Enabler implementation.

- **Non-intrusiveness on resource functionality and performance**

  The Monitoring system not affect the rest of resource functionality nor
  performance.

- **Deal with metric heterogeneity**

  The Monitoring system deals with different kind of metrics (infrastructure,
  KPI, applications and product metrics), different virtualization
  technologies, different products, applications, etc.

- **Scalability in monitored resources**

  The system scales to large numbers of monitored nodes and resources.

- **Service Aggregation**

  The framework aggregates the monitoring information collected at
  application/service level, which means that it aggregates metrics from
  virtual machines or hardware resources at service level.


Documentation
=============

- `User and Programmers Guide <manuals/user/README.rst>`_
- `Installation and Administration Guide <manuals/admin/README.rst>`_


See also
========

- `Monitoring Federation Infrastructure`_

  This presentation summarises the development of this component as a joint
  task between FIWARE and XIFI projects.


.. REFERENCES

.. _Monitoring Federation Infrastructure: http://www.slideshare.net/flopezaguilar/monitoring-federation-open-stack-infrastructure
