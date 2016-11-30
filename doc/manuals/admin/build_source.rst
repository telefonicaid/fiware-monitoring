=======================
 Building from sources
=======================

Monitoring GE reference distributions are CentOS 6.x and Ubuntu 12.04. This
doesn't mean that it cannot be built for other Linux distributions (actually,
it can). This section includes indications about the build process for almost
any distribution.

CentOS 6.x
==========

NGSI Adapter
------------

This component needs no compilation, as it is a server written in Node.js, so
the basic procedure consists basically on installing the ``node`` interpreter,
getting the sources and installing the required dependencies (assuming you
don't run commands as root, we use ``sudo`` for those commands that require
some special privileges):

- Install an updated ``node`` interpreter::

    $ curl -sL https://rpm.nodesource.com/setup_4.x | sudo bash -
    $ sudo yum install -y nodejs

- Install development tools::

    $ sudo yum install -y gcc-c++ make rpm-build redhat-rpm-config
    $ sudo npm install -g grunt-cli

- Get the source code from GitHub::

    $ sudo yum install -y git
    $ git clone https://github.com/telefonicaid/fiware-monitoring

- Install dependencies::

    $ cd fiware-monitoring/ngsi_adapter
    $ npm install

- (Optional but highly recommended) check coding style, run unit tests and
  get coverage::

    $ grunt lint test coverage

- At this point, we are ready to run the server manually::

    $ ./adapter

- Alternatively, we could create a package for this component, install it and
  then run the ``ngsi_adapter`` service::

    $ cd fiware-monitoring/ngsi_adapter
    $ tools/devops/package.sh
    $ sudo rpm -i fiware-monitoring-ngsi-adapter-X.Y.Z-1.noarch.rpm
    $ sudo service ngsi_adapter start


NGSI Event Broker
-----------------

This component is written in C language and requires ``autotools`` to generate
a valid Makefile to drive the build and install process.

- Install development tools::

    $ sudo yum install -y gcc-c++ make autoconf automake libtool
    $ sudo yum install -y cppunit-devel cppcheck lcov libxslt libcurl-devel wget
    $ sudo yum install -y rpm-build redhat-rpm-config
    $ sudo pip install -q gcovr

- Get Nagios 3.x sources (only some headers are actually needed)::

    $ cd fiware-monitoring/ngsi_event_broker
    $ NAGIOS_VERSION=$(awk -F= '/nagios_reqver=/ { print $2 }' configure.ac)
    $ NAGIOS_FILES=http://sourceforge.net/projects/nagios/files
    $ NAGIOS_URL=$NAGIOS_FILES/nagios-${NAGIOS_VERSION%%.*}.x/nagios-$NAGIOS_VERSION/nagios-$NAGIOS_VERSION.tar.gz/download
    $ NAGIOS_SRC_DIR=nagios
    $ wget $NAGIOS_URL -q -O nagios-${NAGIOS_VERSION}.tar.gz
    $ tar xzf nagios-${NAGIOS_VERSION}.tar.gz
    $ (cd $NAGIOS_SRC_DIR && ./configure && make nagios)

- Configure for debug build with coverage support::

    $ mkdir -p m4 && autoreconf -i
    $ ./configure --enable-gcov --with-nagios-srcdir=$NAGIOS_SRC_DIR

  Default installation directory is ``/opt/fiware/ngsi_event_broker/lib`` but
  this may be changed by adding the ``--libdir=target_libdir`` option when
  running the ``configure`` script.

- Compile and check coding style, run unit tests and get coverage (optional but
  highly recommended)::

    $ make clean lint test coverage

- Install the generated module (a dynamic library)::

    $ make install

- Alternatively, we could create a package for this component (which implies
  building and running the unit tests), and install it::

    $ cd fiware-monitoring/ngsi_event_broker
    $ tools/devops/package.sh
    $ sudo rpm -i fiware-monitoring-ngsi-event-broker-X.Y.Z-1.noarch.rpm


Ubuntu
======

The steps are the same as in CentOS, with only a few changes:

NGSI Adapter
------------

- Install an updated ``node`` interpreter::

    $ curl -sL https://deb.nodesource.com/setup_4.x | sudo bash -
    $ sudo apt-get install -y nodejs

- Install development tools::

    $ sudo apt-get install -y g++ make dpkg-dev debhelper devscripts
    $ sudo npm install -g grunt-cli


NGSI Event Broker
-----------------

- Install development tools::

    $ sudo apt-get install -y g++ build-essential make autoconf automake autotools-dev libtool
    $ sudo apt-get install -y libcppunit-dev cppcheck lcov xsltproc libcurl4-openssl-dev wget
    $ sudo apt-get install -y dpkg-dev debhelper devscripts
    $ sudo pip install -q gcovr


Other distributions
===================

Again, the steps are the same as in CentOS. We only have to pay attention to
the way to install ``node`` (see NodeSource_ for details) and to the possible
different package names of the development tools.


.. REFERENCES

.. _NodeSource: https://github.com/nodesource/distributions
