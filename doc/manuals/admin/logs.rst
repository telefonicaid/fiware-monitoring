======
 Logs
======

The log system has been re-worked in release `4.1.1`__. This section describes
its main characteristics.

__ `FIWARE Monitoring release 4.1.1`_


Log file
========

The *NGSI Adapter* component of Monitoring GE writes logs, when running as a
service, to the file ``/var/log/ngsi_adapter/ngsi_adapter.log`` (if started
manually from command line, logs are written to standard output).

The *NGSI Event Broker* component is a module integrated into the *Nagios*
framework and its logs are written to the file ``/var/log/nagios/nagios.log``
(or any other defined by Nagios configuration).


Log format
==========

The log format is designed to be processed by tools like Splunk_ or Fluentd_.

Each line in the log file is composed by several key-value fields, separated
by the pipe character (``|``). Example:

::

    time=2015-08-01T08:00:00.511Z | lvl=INFO | trans=ci2627bx00000b42g8m2pxw3z | op=POST | msg=Request on resource /check_xxx with params id=xxx&type=xxx
    time=2015-08-01T08:00:00.675Z | lvl=INFO | trans=ci2627bx00000b42g8m2pxw3z | op=POST | msg=Response status 200 OK
    time=2015-08-01T08:00:00.922Z | lvl=INFO | trans=ci2627bx00000b42g8m2pxw3z | op=UpdateContext | msg=Request to ContextBroker at http://host:1026/...


These are the different fields found in each line:

- **time**. A timestamp corresponding to the moment in which the log line was
  generated.

- **lvl (level)**. One of the following:

  * INFO: This level designates informational messages that highlight the
    progress of the component.
  * WARNING: This level designates potentially harmful situations. There is
    a minor problem that should be fixed.
  * ERROR: This level designates error events. There is a severe problem that
    should be fixed.
  * FATAL: This level designates very severe error events that will presumably
    lead the application to abort. The process can no longer work.
  * DEBUG: This level designates fine-grained informational events that are
    most useful to debug an application.

- **trans (transaction id)**. Can be either "N/A" (for log messages "out of
  transaction", as the ones corresponding to startup) or a unique string id.

- **op (operation)**. The function in the source code that generated the log
  message. This information is useful for developers only.

- **msg (message)**. The actual log message.


Log rotation
============

The system administrator **must** configure some log rotation mechanism, or
otherwise the log file size will increase indefinitely. We recommend using
logrotate_.

Depending on your expected work load, you would need to adjust the rotation
parameters.


.. REFERENCES

.. _Splunk: http://www.splunk.com/
.. _Fluentd: http://www.fluentd.org/
.. _logrotate: http://linux.die.net/man/8/logrotate
.. _FIWARE Monitoring release 4.1.1: https://github.com/telefonicaid/fiware-monitoring/releases/tag/v4.1.1
