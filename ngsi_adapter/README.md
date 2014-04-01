# NGSI Adapter

Generic adapter to transform monitoring data from probes to NGSI context
attributes, and forward them through a NGSI Context Broker.

## Installation

Dependencies must be resolved prior running the adapter. Be sure Node.js and npm
are installed, and then run:

    $ npm install

## Usage

Adapter runs as a standalone server listening for HTTP POST requests at the
given endpoint. Context Broker URL, adapter listen port and other parameters may
be supplied at command line, overriding default values in *config/options.js*:

    $ adapter --listenPort 1337 --brokerUrl http://{host}:{port}/

For detailed information about command line options and their defaults, please
run:

    $ adapter --help

Logging options (such as level, console, rolling files, etc.) may be
configured editing *config/logger.js* file.

## Requests

Probe raw data should be sent as body of a POST request to the adapter,
identifying the source entity being monitored in the query fields:

    POST http://{adapterEndpoint}/{probe}?id={id}&type={type}

* *{probe}* = name of the originating probe

* *{id}* = identifier of the entity being monitored

* *{type}* = type of the entity

Adapter processes requests asynchronously, indicating in the response
status code whether *{probe}* is recognized (status 200) or not (404).

In case of known probes, adapter dynamically loads a custom probe
parser that should be located at *lib/parsers/{probe}.js* and invokes
its *getContextAttrs()* method, passing probe raw data splitted into
two components: probe data and optional performance data.

## Changelog

Version 1.0.1

* NGSI9 and NGSI10 decoupled in update requests by using append action
* Bugfixing: solved errors in update requests

Version 1.0.0

* Only Nagios probes (plugins) are supported

## License

(c) 2013 Telefónica I+D, Apache License 2.0
