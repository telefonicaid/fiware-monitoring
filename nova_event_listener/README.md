# OpenStack Nova Event Listener

AMQP client listening for Nova [compute.instance][event_ref] notification events
to register newly created instances as entities in a NGSI Context Broker.

## Installation

Dependencies must be resolved prior running the listener. Be sure Python and pip
are installed, and then run:

    $ pip install -r requirements.txt

## Usage

Listener runs as a standalone process listening to AMQP queues, which responds
to instance creation by sending a request to Context Broker (whose URL must be
given at command line or in configuration file *conf/nova_event_listener.cfg*)

The requests to Context Broker are asynchronously processed by [Celery][celery]
using the local AMQP as tasks queue. Therefore, a Celery worker must be started
in addition to event listener:

    $ celery --app=nova_event_listener worker --loglevel=info &
    $ nova_event_listener.py --brokerUrl http://{host}:{port}/

For detailed information about command line options and default values,
please run:

    $ nova_event_listener.py --help

Logging options (such as level, console, rolling files, etc.) may be
configured editing *conf/nova_event_listener.cfg* file.


## Entities

New instances created by OpenStack Nova service are registered as NGSI entities,
with the following considerations:

* *{entityId}* = *{regionId}*:*{instanceUUID}*

* *{entityType}* = "vm"

## Changelog

Version 1.0.0

* Initial release of the component

## License

(c) 2013 Telefónica I+D, Apache License 2.0

[event_ref]:
https://wiki.openstack.org/wiki/SystemUsageData
"Nova notification system events"

[celery]:
http://www.celeryproject.org/
"Celery: Distributed Task Queue"
