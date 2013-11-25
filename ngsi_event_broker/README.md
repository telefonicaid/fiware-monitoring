# NGSI Event Broker

Nagios event broker ([NEB][NEB_ref]) module to forward plugin data
to [NGSI Adapter][NGSI_Adapter_ref].

## Usage

As the module is an architecture-dependent compiled shared object, from
source code distribution these steps should be followed (assuming autotools
and libtool are installed):

    $ autoreconf --install
    $ ./configure
    $ make
    $ make check
    $ sudo make install

Last step will try to copy generated shared object to the Nagios library
directory, thus requiring sudoer privileges. Installation directory will
usually be */usr/lib/nagios* or */usr/lib64/nagios*.

Stop Nagios service and edit configuration file at */etc/nagios/nagios.cfg*
to add a new broker module:

    event_broker_options=-1
    broker_module={path}/ngsi_event_broker.so http://{host}:{port}

Pay attention to the module argument: the endpoint of the NGSI Adapter
to forward plugin data to, usually listening at a local port.

Finally, start Nagios service. Check in log files that Nagios loads the module
and that adapter server requests are sent in response to plugin executions.

## Changelog

Version 1.0.0

* Initial release of the module

## License

(c) 2013 Telefónica I+D, Apache License 2.0

[NEB_ref]:
http://nagios.sourceforge.net/download/contrib/documentation/misc/NEB%202x%20Module%20API.pdf
"The Nagios Event Broker API"

[NGSI_Adapter_ref]:
https://github.com/Fiware/fiware-monitoring/tree/master/ngsi_adapter
"NGSI Adapter"
