# Package installation custom attributes
%define _name fiware-monitoring-ngsi-adapter
%define _fiware_usr fiware
%define _fiware_grp fiware
%define _fiware_dir /opt/fiware
%define _adapter_srv ngsi_adapter
%define _adapter_usr %{_fiware_usr}
%define _adapter_grp %{_fiware_grp}
%define _adapter_dir %{_fiware_dir}/%{_adapter_srv}
%define _logging_dir /var/log/%{_adapter_srv}
%define _node_req_ver %(awk '/"node":/ {split($0,v,/["~=<>]/); print v[6]}' %{_basedir}/package.json)

# Package main attributes (_topdir, _basedir, _version and _release must be given at command line)
Summary: Adapter to transform data from monitoring probes to NGSI context attributes.
URL: https://github.com/telefonicaid/fiware-monitoring/tree/master/ngsi_adapter
Name: %{_name}
Version: %{_version}
Release: %{_release}
License: Apache
Group: Applications/Engineering
Vendor: Telefónica I+D
BuildArch: noarch
# Requires: nodejs (see %pre)

%description
NGSI Adapter is a generic component to transform monitoring data from probes
to NGSI context attributes, and forward them through a NGSI Context Broker.

%prep
# Clean build root directory
if [ -d $RPM_BUILD_ROOT ]; then
	rm -rf $RPM_BUILD_ROOT
fi

%clean
rm -rf $RPM_BUILD_ROOT

%install
mkdir -p $RPM_BUILD_ROOT/%{_adapter_dir}; set +x
INCLUDE='lib|config|package.json|LICENSE|README.*|adapter$'
PATTERN='* .npmrc'
FILES=$(cd %{_basedir}; for i in $PATTERN; do echo $i; done | egrep "$INCLUDE")
for I in $FILES; do cp -R %{_basedir}/$I $RPM_BUILD_ROOT/%{_adapter_dir}; done
cp -R %{_sourcedir}/* $RPM_BUILD_ROOT
(cd $RPM_BUILD_ROOT; find . -type f -printf "/%%P\n" >> %{_topdir}/MANIFEST)
echo "FILES:"; cat %{_topdir}/MANIFEST

%files -f %{_topdir}/MANIFEST
%config /etc/sysconfig/%{_adapter_srv}

%pre
# preinst ($1 == 1)
if [ $1 -eq 1 ]; then
	# Function to compare version strings (in `x.y.z' format)
	valid_version() {
		local CUR=$1
		local REQ=$2
		printf "$CUR\n$REQ" \
		| awk '{split($0,v,"."); for (i in v) printf "%%05d ", v[i]; print}' \
		| sort | tail -1 | cat -E | fgrep -q $CUR'$'
	}

	# Function to setup EPEL and/or NodeSource repo (for the latest node.js version)
	setup_nodesource() {
		fmt --width=${COLUMNS:-$(tput cols)} 1>&2 <<-EOF

			ERROR: node.js >=v$NODE_REQ_VERSION is required. Setting up
			repositories to get the latest version ...
		EOF

		# prepare sources list configuration for the next `yum' command
		# (this requires removing the lock to allow access to the repositories configuration)
		find /var/lib/rpm -name "*.lock" -exec rm -f {} \;
		curl -sL https://rpm.nodesource.com/setup | bash - >/dev/null
		if [ $? -eq 0 ]; then fmt --width=${COLUMNS:-$(tput cols)} 1>&2 <<-EOF

			Please run \`sudo yum -y install nodejs' to install/upgrade version
			prior reinstalling this package.

			EOF
		else fmt --width=${COLUMNS:-$(tput cols)} 1>&2 <<-EOF

			Unable to setup repositories. Please install/upgrade node.js manually
			and then reinstall this package.

			EOF
		fi
	}

	NODE_REQ_VERSION=%{_node_req_ver}
	NODE_CUR_VERSION=$(node -pe 'process.versions.node' 2>/dev/null)
	if ! valid_version ${NODE_CUR_VERSION:-0.0.0} $NODE_REQ_VERSION; then
		setup_nodesource
		exit 1
	fi
	exit 0
fi

%post
# postinst ($1 == 1)
if [ $1 -eq 1 ]; then
	# actual values of installation variables
	FIWARE_USR=%{_fiware_usr}
	FIWARE_GRP=%{_fiware_grp}
	FIWARE_DIR=%{_fiware_dir}
	ADAPTER_SRV=%{_adapter_srv}
	ADAPTER_USR=%{_adapter_usr}
	ADAPTER_GRP=%{_adapter_grp}
	ADAPTER_DIR=%{_adapter_dir}
	LOGGING_DIR=%{_logging_dir}
	STATUS=0

	# create additional directories
	mkdir -p $LOGGING_DIR

	# install npm dependencies
	echo "Installing npm dependencies ..."
	cd $ADAPTER_DIR
	npm config set ca=""
	npm install --production || STATUS=1

	# check FIWARE user
	if ! getent passwd $FIWARE_USR >/dev/null; then
		groupadd --force $FIWARE_GRP
		useradd --gid $FIWARE_GRP --shell /bin/false \
		        --home-dir /nonexistent --no-create-home \
		        --comment "FIWARE" $FIWARE_USR
	fi

	# check ADAPTER user
	if ! getent passwd $ADAPTER_USR >/dev/null; then
		groupadd --force $ADAPTER_GRP
		useradd --gid $ADAPTER_GRP --shell /bin/false \
		        --home-dir /nonexistent --no-create-home \
		        --comment "FIWARE NGSI Adapter" $ADAPTER_USR
	fi

	# change ownership
	chown -R $FIWARE_USR:$FIWARE_GRP $FIWARE_DIR
	chown -R $ADAPTER_USR:$ADAPTER_GRP $ADAPTER_DIR
	chown -R $ADAPTER_USR:$ADAPTER_GRP $LOGGING_DIR

	# change file permissions
	chmod -R g+w $ADAPTER_DIR
	chmod a+x $ADAPTER_DIR/adapter

	# configure service
	chmod a+x /etc/init.d/$ADAPTER_SRV
	/sbin/chkconfig --add $ADAPTER_SRV

	# postinstall message
	if [ $STATUS -eq 0 ]; then fmt --width=${COLUMNS:-$(tput cols)} <<-EOF

		NGSI Adapter successfully installed at $ADAPTER_DIR.

		WARNING: Check configuration file /etc/sysconfig/$ADAPTER_SRV
		before starting \`$ADAPTER_SRV' service. This component DOES NOT
		rotate logging files, so consider configuring \`logrotate'
		or similar. Please read Usage section at README.rst for more
		details.

		EOF
	else fmt --width=${COLUMNS:-$(tput cols)} 1>&2 <<-EOF

		ERROR: Failed to install dependencies. Please check
		\`npm-debug.log' file for problems and then reinstall package.

		EOF
	fi

	# finalization
	exit $STATUS
fi

%postun
# uninstall ($1 == 0)
if [ $1 -eq 0 ]; then
	# remove installation directory
	rm -rf %{_adapter_dir}

	# remove FIWARE parent directory (if empty)
	[ -d %{_fiware_dir} ] && rmdir --ignore-fail-on-non-empty %{_fiware_dir}

	# remove log files
	rm -rf %{_logging_dir}
fi

%changelog
* Tue Nov 24 2015 Telefónica I+D <opensource@tid.es> 1.3.0-1
- Add option to set maximum number of simultaneous outgoing requests
- Add an option to specify a list of directories to look for parsers
- Add support to UDP requests

* Fri Aug 14 2015 Telefónica I+D <opensource@tid.es> 1.2.4-1
- Add service configuration file

* Fri Mar 27 2015 Telefónica I+D <opensource@tid.es> 1.2.3-1
- Fix problems when uninstalling package

* Fri Feb 27 2015 Telefónica I+D <opensource@tid.es> 1.2.2-1
- Add .rpm package generation
- Minor bugs resolved

* Fri Jan 30 2015 Telefónica I+D <opensource@tid.es> 1.2.1-1
- Minor bugs resolved

* Tue Oct 28 2014 Telefónica I+D <opensource@tid.es> 1.2.0-1
- Add new log format (issue #25)

* Thu Aug 28 2014 Telefónica I+D <opensource@tid.es> 1.1.1-1
- Add .deb package generation (issue #16)

* Mon Jul 14 2014 Telefónica I+D <opensource@tid.es> 1.1.0-1
- Add timestamp to entity attributes (issue #4)

* Wed Feb 05 2014 Telefónica I+D <opensource@tid.es> 1.0.1-1
- NGSI9 and NGSI10 decoupled in update requests by using append action
- Bugfixing: solved errors in update requests

* Mon Nov 25 2013 Telefónica I+D <opensource@tid.es> 1.0.0-1
- Initial release of the adapter
