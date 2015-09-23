**# Welcome to FIWARE Monitoring.**

Monitoring GE - TID Implementation is the key component to allow incorporating monitoring and metering mechanisms 
in order be able to constantly check the performance of the system, but the architecture should be easily extended 
to collect data for other required needs. Monitoring involves gathering operational data in a running system. 
Collected information can be used for several purposes:

- Cloud users to track the performance of their own instances.
- SLA management, in order to check adherence to agreement terms.
- Optimization of virtual machines.

The monitoring system is used by different Cloud GEs in order to track the status of the resources. They use 
gathered data to take decisions about elasticity or for SLA management. Whenever a new resource is deployed 
in the cloud, the proper monitoring probe is set up and configured to start providing monitoring data. 

`Github's <README.rst>`_ provides a good documentation summary.
`The User and Programmers Guide <doc/manuals/user/README.rst>`_  and `the Installation and Administration Guide <doc/manuals/admin/README.rst>`_ cover more advanced topics.
