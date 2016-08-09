# Package installation custom attributes
%define _name fiware-monitoring-ngsi-event-broker
%define _fiware_usr fiware
%define _fiware_grp fiware
%define _fiware_dir /opt/fiware
%define _broker_usr %{_fiware_usr}
%define _broker_grp %{_fiware_grp}
%define _broker_dir %{_fiware_dir}/ngsi_event_broker

# Package main attributes (_topdir, _builddir, _basedir, _version and _release must be given at command line)
Summary: Nagios event broker (NEB) module to forward plugin data to NGSI Adapter.
URL: https://github.com/telefonicaid/fiware-monitoring/tree/master/ngsi_event_broker
Name: %{_name}
Version: %{_version}
Release: %{_release}%{?dist}.1
License: Apache
Group: Applications/Engineering
Vendor: Telefónica I+D
Packager: Telefónica I+D <opensource@tid.es>
Requires: libstdc++, libcurl
BuildRequires: wget, gcc-c++, make, autoconf, automake, libtool, cppunit-devel, libcurl-devel

%description
The module will intercept results from any plugin executed by Nagios. Then, it
will use information given as module arguments together with data taken from the
Nagios service definitions to issue a request to NGSI Adapter. In many cases,
the service definitions need no further modifications and the broker just works
transparently once Nagios is restarted.

%prep
# Clean build root directory
if [ -d $RPM_BUILD_ROOT ]; then
	rm -rf $RPM_BUILD_ROOT
fi

# Check for ./configure status
set +x
if [ ! -x %{_basedir}/configure -o ! -r %{_basedir}/config.status ]; then
	%{error:Please run `./configure' first}
	exit 1
fi

%clean
rm -rf $RPM_BUILD_ROOT

%build
SRCDIR=$(awk -F\" '/NAGIOS_SRCDIR/ {print $4}' %{_basedir}/config.status)
./configure --with-nagios-srcdir=$SRCDIR --libdir=%{_broker_dir}/lib

%install
make DESTDIR=$RPM_BUILD_ROOT clean install
set +x
(cd $RPM_BUILD_ROOT; find . -name "*.so" -printf "/%%P\n" >> %{_topdir}/MANIFEST)
echo "FILES:"; cat %{_topdir}/MANIFEST

%files -f %{_topdir}/MANIFEST
%doc README.rst

%post
# postinst ($1 == 1)
if [ $1 -eq 1 ]; then
	# actual values of installation variables
	FIWARE_USR=%{_fiware_usr}
	FIWARE_GRP=%{_fiware_grp}
	FIWARE_DIR=%{_fiware_dir}
	BROKER_USR=%{_broker_usr}
	BROKER_GRP=%{_broker_grp}
	BROKER_DIR=%{_broker_dir}

	# check FIWARE user
	if ! getent passwd $FIWARE_USR >/dev/null; then
		groupadd --force $FIWARE_GRP
		useradd --gid $FIWARE_GRP --shell /bin/false \
		        --home-dir /nonexistent --no-create-home \
		        --comment "FIWARE" $FIWARE_USR
	fi

	# check BROKER user
	if ! getent passwd $BROKER_USR >/dev/null; then
		groupadd --force $BROKER_GRP
		useradd --gid $BROKER_GRP --shell /bin/false \
		        --home-dir /nonexistent --no-create-home \
		        --comment "FIWARE NGSI Event Broker" $BROKER_USR
	fi

	# change ownership
	chown -R $FIWARE_USR:$FIWARE_GRP $FIWARE_DIR
	chown -R $BROKER_USR:$BROKER_GRP $BROKER_DIR

	# change file permissions
	chmod -R g+w $BROKER_DIR
	find $BROKER_DIR -name "*.so" -exec chmod a+x {} \;

	# postinstall message
	fmt --width=${COLUMNS:-$(tput cols)} <<-EOF

		NGSI Event Broker installed at $BROKER_DIR.

		This library is intended to be used as a Nagios module, so
		further configuration steps are required. Please read Usage
		section at /usr/share/doc/%{_name}-%{_version}/README.rst
		for more details.

	EOF
fi

%postun
# uninstall ($1 == 0)
if [ $1 -eq 0 ]; then
	# remove installation directory
	rm -rf %{_broker_dir}

	# remove FIWARE parent directory (if empty)
	[ -d %{_fiware_dir} ] && rmdir --ignore-fail-on-non-empty %{_fiware_dir}
fi

%changelog
* Tue Aug 09 2016 Telefónica I+D <opensource@tid.es> %{_version}-%{_release}
- New Nagios Event Broker to monitor FIWARE GEri global instances

* Fri Mar 27 2015 Telefónica I+D <opensource@tid.es> 1.4.3-1
- Fix problems when uninstalling package

* Fri Feb 27 2015 Telefónica I+D <opensource@tid.es> 1.4.2-1
- Add .rpm package generation
- Minor bugs resolved

* Fri Jan 30 2015 Telefónica I+D <opensource@tid.es> 1.4.1-1
- Minor bugs resolved

* Tue Oct 28 2014 Telefónica I+D <opensource@tid.es> 1.4.0-1
- Include new log format (issue #25)

* Mon Jul 14 2014 Telefónica I+D <opensource@tid.es> 1.3.1-1
- Include .deb package generation
- Fix error in argument parser

* Tue Apr 01 2014 Telefónica I+D <opensource@tid.es> 1.3.0-1
- Include "host_service" monitoring

* Wed Mar 12 2014 Telefónica I+D <opensource@tid.es> 1.2.0-1
- Unification into a single _xifi broker

* Wed Feb 05 2014 Telefónica I+D <opensource@tid.es> 1.1.0-1
- Broker splitted into _snmp and _host
- IP address as unique identifier (within region) for hosts and vms
- Add region as argument
- Add NRPE support

* Tue Jan 14 2014 Telefónica I+D <opensource@tid.es> 1.0.1-1
- Add regions support (value retrieved from a metadata key named "region")

* Mon Nov 25 2013 Telefónica I+D <opensource@tid.es> 1.0.0-1
- Initial release of the module
