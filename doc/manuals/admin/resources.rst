=======================
 Resources & I/O Flows
=======================

Resources recommendations
=========================

Although we haven't done yet a precise profiling on the NGSI Adapter component
of this Monitoring GE, tests done in our development and testing environment
show that a host with 2 CPU cores and 4 GB RAM is fine to run server.

Resources consumption
=====================

No issues related to resources consumption have been detected neither with
the NGSI Adapter server nor with the NGSI Event Broker loaded as a "pluggable"
module on Nagios startup.

I/O flows
=========

Figure below shows the I/O flows among the different monitoring components:

.. figure:: /doc/resources/Monitoring_IO_Flows.png
   :alt: Monitoring I/O flows.

- The probes gather raw monitoring data, and a Collector (for Nagios, this is
  *NGSI Event Broker*) send requests to *NGSI Adapter* with such data
- NGSI Adapter sends request to *Context Broker* in terms of context updates
  of the monitored resources
- Context Broker notifies a *Connector* (for example, Cygnus) with every change
  in the context of entities
- The connector writes changes to storage
