# NGSI Event Broker

Nagios event broker ([NEB][NEB_ref]) module to forward plugin data to
[NGSI Adapter][NGSI_Adapter_ref]. Currently, the broker is particularized for
[XIFI][XIFI_ref] monitoring:

* *ngsi_event_broker_xifi* to process plugin executions for XIFI

## Installation

As the module is an architecture-dependent compiled shared object,
first we'll get sources either from this repository or downloading a
[source code distribution][src_dist_ref].

The first option requires *autotools* and *libtool* to be installed, in order
to generate configuration script

    $ mkdir m4
    $ autoreconf --install

Once configuration script is generated/downloaded, follow these steps:

    $ ./configure
    $ make
    $ make check
    $ sudo make install

Last step will try to copy generated shared object to the Nagios library
directory, thus requiring sudoer privileges. Installation directory will
usually be `/usr/lib/nagios` or `/usr/lib64/nagios`.

## Usage

Stop Nagios service and edit configuration file at `/etc/nagios/nagios.cfg`
to add new broker module. The id of the [region][region_ref] that current
infrastructure belongs to and the URL of NGSI Adapter must be supplied as
arguments:

    event_broker_options=-1
    broker_module=/path/ngsi_event_broker_xifi.so -r region -u http://host:port

Finally, start Nagios service. Check log files for module initialization (may
fail for missing arguments, for example). Also check that requests are sent to
adapter server in response to plugin executions. Requests will include some
query string parameters:

* SNMP monitoring:

    `http://host:port/check_snmp?id=region:ifaddr/ifport&type=interface`

* Host service monitoring:

    `http://host:port/check_xxxx?id=region:hostname:servname&type=host_service`

* Other plugins executed locally:

    `http://host:port/check_xxxx?id=region:localaddr&type=host`

* Other plugins executed remotely via NRPE:

    `http://host:port/check_xxxx?id=region:nrpeaddr&type=vm`

## Changelog

Version 1.3.0

* Included "host_service" monitoring

Version 1.2.0

* Unification into a single _xifi broker

Version 1.1.0

* Broker splitted into _snmp and _host
* IP address as unique identifier (within region) for hosts and vms
* Added region as argument
* Added NRPE support

Version 1.0.1

* Added regions support (value retrieved from a metadata key named "region")

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

[src_dist_ref]:
https://forge.fi-ware.org/frs/download.php/1101/ngsi_event_broker-1.3.0.src.tar.gz
"NGSI Event Broker source distribution package"

[region_ref]:
http://docs.openstack.org/glossary/content/glossary.html#region
"OpenStack Glossary: Region"

[XIFI_ref]:
https://www.fi-xifi.eu/home.html
"XIFI Project"
