
==============
 NGSI Adapter
==============

Generic adapter to transform monitoring data from probes to NGSI context
attributes, and forward them through a NGSI Context Broker.


Installation
============

Please refer to `this document </README.rst#build-and-install>`_ for details.


Usage
=====

Please refer to `this document </README.rst#running>`_ for details.


Changelog
=========

Version 1.2.3

- Fix problems when uninstalling package

Version 1.2.2

- Add .rpm package generation
- Minor bugs resolved

Version 1.2.1

- Minor bugs resolved

Version 1.2.0

- Add new log format (issue #25)

Version 1.1.1

- Add .deb package generation (issue #16)

Version 1.1.0

- Add timestamp to entity attributes (issue #4)

Version 1.0.1

- NGSI9 and NGSI10 decoupled in update requests by using append action
- Bugfixing: solved errors in update requests

Version 1.0.0

- Only Nagios probes (plugins) are supported


License
=======

\(c) 2013-2015 Telefónica I+D, Apache License 2.0
