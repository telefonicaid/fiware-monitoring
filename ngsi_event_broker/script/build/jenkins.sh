#!/bin/sh
#
# Copyright 2013-2014 Telefónica I+D
# All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License"); you may
# not use this file except in compliance with the License. You may obtain
# a copy of the License at
#
#         http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
# WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
# License for the specific language governing permissions and limitations
# under the License.
#

#
# Support script for this component within a Jenkins CI job
#
# Usage:
#     $0 build|release
#     $0 --help
#
# Options:
#     -h, --help	show this help message
#
# Actions:
#     build		build, generate reports and publish to SonarQube
#     release		generate distribution package
#

OPTS='h(help)'
NAME=$(basename $0)

# Command line options
ACTION=

# Process command line
OPTERR=
OPTSTR=$(echo :-:$OPTS | sed 's/([a-zA-Z0-9]*)//g')
OPTHLP=$(sed -n '20,/^$/ { s/$0/'$NAME'/; s/^#[ ]\?//p }' $0)
while getopts $OPTSTR OPT; do while [ -z "$OPTERR" ]; do
case $OPT in
'h')	OPTERR="$OPTHLP";;
'?')	OPTERR="Unknown option -$OPTARG";;
':')	OPTERR="Missing value for option -$OPTARG";;
'-')	OPTLONG="${OPTARG%=*}";
	OPT=$(expr $OPTS : ".*\(.\)($OPTLONG):.*" '|' '?');
	if [ "$OPT" = '?' ]; then
		OPT=$(expr $OPTS : ".*\(.\)($OPTLONG).*" '|' '?')
		OPTARG=-$OPTLONG
	else
		OPTARG=$(echo =$OPTARG | cut -d= -f3)
		[ -z "$OPTARG" ] && { OPTARG=-$OPTLONG; OPT=':'; }
	fi;
	continue;;
esac; break; done; done
shift $(expr $OPTIND - 1)
ACTION=$(expr "$1" : "^\(build\|release\)$") && shift
[ -z "$OPTERR" -a -z "$ACTION" ] && OPTERR="Missing or invalid action"
[ -z "$OPTERR" -a -n "$*" ] && OPTERR="Too many arguments"
[ -n "$OPTERR" ] && {
	[ "$OPTERR" != "$OPTHLP" ] && OPTERR="${OPTERR}\nTry \`$NAME --help'"
	TAB=4; LEN=$(echo "$OPTERR" | awk -F'\t' '/ .+\t/ {print $1}' | wc -L)
	TABSTOPS=$TAB,$(((2+LEN/TAB)*TAB)); WIDTH=${COLUMNS:-$(tput cols)}
	printf "$OPTERR" | tr -s '\t' | expand -t$TABSTOPS | fmt -$WIDTH -s 1>&2
	exit 1
}

# Main
if [ -z "$WORKSPACE" ]; then
	printf "Variable WORKSPACE not set\n" 1>&2
	exit 2
fi

# Project root at Jenkins workspace
PROJECT_DIR=$WORKSPACE/ngsi_event_broker

# Absolute directories
PROJECT_SRC_DIR=$PROJECT_DIR/src
LINT_REPORT_DIR=$PROJECT_DIR/report/cppcheck
TEST_REPORT_DIR=$PROJECT_DIR/report/test
COVERAGE_REPORT_DIR=$PROJECT_DIR/report/coverage
COVERAGE_SITE_DIR=$PROJECT_DIR/site/coverage/lcov-report

# Properties
PRODUCT_AREA=$(sed -n '/\[PRODUCT_AREA\]/ {s/.*\[.*\].*\[\(.*\)\].*/\1/; p}' $PROJECT_DIR/configure.ac)
PRODUCT_NAME=$(sed -n '/\[PRODUCT_NAME\]/ {s/.*\[.*\].*\[\(.*\)\].*/\1/; p}' $PROJECT_DIR/configure.ac)
PRODUCT_RELEASE=$(sed -n '/\[PRODUCT_RELEASE\]/ {s/.*\[.*\].*\[\(.*\)\].*/\1/; p}' $PROJECT_DIR/configure.ac)
PROJECT_NAME=$(sed -n '/AC_INIT/ {s/.*\[PRODUCT_NAME-\(.*\)].*/\1/; p}' $PROJECT_DIR/configure.ac)
PROJECT_VERSION=$(sed -n '/AC_INIT/ {s/.*,[ \t]*\(.*\))/\1/; p}' $PROJECT_DIR/configure.ac)
SONAR_PROJECT_NAME="Monitoring NGSI Event Broker"
SONAR_PROJECT_KEY=com.telefonica.iot:monitoring-ngsi-event-broker

# Dependencies
RPM_DEPENDENCIES="wget gcc-c++ make autoconf automake libtool cppunit-devel libcurl-devel"
DEB_DEPENDENCIES="wget g++ build-essential autoconf automake autotools-dev libtool libcppunit-dev libcurl4-openssl-dev"
NAGIOS_SRC_DIR=$PROJECT_DIR/nagios
NAGIOS_INC_DIR=$PROJECT_DIR/$(awk -F= '/nagios_incdir=/ { print $2 }' $PROJECT_DIR/configure.ac)
NAGIOS_VERSION=$(awk -F= '/nagios_reqver=/ { print $2 }' $PROJECT_DIR/configure.ac)
NAGIOS_FILES=http://sourceforge.net/projects/nagios/files
NAGIOS_URL=$NAGIOS_FILES/nagios-${NAGIOS_VERSION%%.*}.x/nagios-$NAGIOS_VERSION/nagios-$NAGIOS_VERSION.tar.gz/download

# Change to project directory
cd $PROJECT_DIR

# Perform action
case $ACTION in
build)
	# Install development dependencies
	if test -r /etc/redhat-release; then
		# CentOS
		sudo yum -y -q install $RPM_DEPENDENCIES lcov libxslt cppcheck
	else
		# Ubuntu
		sudo apt-get -y -q install $DEB_DEPENDENCIES lcov xsltproc cppcheck
	fi
	sudo pip install -q gcovr
	if ! test -d $NAGIOS_SRC_DIR; then
		wget $NAGIOS_URL -q -O nagios-${NAGIOS_VERSION}.tar.gz
		tar xzf nagios-${NAGIOS_VERSION}.tar.gz
		(cd nagios && ./configure && make nagios)
	fi

	# Configure for debug build with coverage support
	mkdir -p m4 && autoreconf -i
	./configure --enable-gcov --with-nagios-srcdir=$NAGIOS_SRC_DIR

	# Compile and generate reports
	make clean lint-report test-report coverage-report

	# Copy reports with paths relative to $WORKSPACE root
	sed 's#\./src/#ngsi_event_broker/src/#' $COVERAGE_REPORT_DIR/lcov.info > $PROJECT_DIR/jenkins-lcov.info
	sed 's#filename="src/#filename="ngsi_event_broker/src/#' $COVERAGE_REPORT_DIR/cobertura-coverage.xml > $PROJECT_DIR/jenkins-cobertura-coverage.xml

	# Get include directories for sonar-cxx plugin
	C_CXX_INCLUDE_DIRS=$(cpp -x c++ -v 2>&1 /dev/null | sed -n '/include <\.\.\.>/,/End/ { p;}' | tail -n +2 | head -n -1 | tr -d ' ' | tr '\n ' ',')
	SONAR_INCLUDE_DIRS="${NAGIOS_INC_DIR},${C_CXX_INCLUDE_DIRS%,}"

	# Prepare properties file for SonarQube (awk to remove leading spaces)
	awk '$1=$1' > $PROJECT_DIR/sonar-project.properties <<-EOF
		product.area.name=$PRODUCT_AREA
		product.name=$PRODUCT_NAME
		product.release=$PRODUCT_RELEASE
		sonar.projectName=$SONAR_PROJECT_NAME
		sonar.projectKey=$SONAR_PROJECT_KEY
		sonar.projectVersion=$PROJECT_VERSION
		sonar.language=c++
		sonar.sourceEncoding=UTF-8
		sonar.sources=src/
		sonar.tests=test/
		sonar.test.exclusions=**/*.c
		sonar.cxx.includeDirectories=$SONAR_INCLUDE_DIRS
		sonar.cxx.cppcheck.reportPath=report/cppcheck/cppcheck-result.xml
		sonar.cxx.xunit.reportPath=report/test/TEST-xunit-*.xml
		sonar.cxx.coverage.reportPath=report/coverage/cobertura-coverage.xml
	EOF

	# Workaround if using sonar-cxx plugin version <=0.9
	# sed -i 's#includeDirectories#include_directories#' $PROJECT_DIR/sonar-project.properties

	# Workaround for metrics_runner.sh to detect current pwd as metrics dir
	sed '/function getMetricsDir/,/}/ c\
	function getMetricsDir() { \
		local currDir=$(readlink -f "$PWD")/ \
		local workDir=$(readlink -f "$WORKSPACE")/ \
		local metricsDir=${currDir#$workDir} \
		echo $metricsDir \
	}' $(which metrics_runner.sh) > ./metrics_runner.sh
	chmod a+x ./metrics_runner.sh

	# Generate metrics in SonarQube
	export DEBUG_METRICS=FALSE
	./metrics_runner.sh
	;;

release)
	# Install development and package generation dependencies
	if test -r /etc/redhat-release; then
		# CentOS
		sudo yum -y -q install $RPM_DEPENDENCIES redhat-rpm-config
	else
		# Ubuntu
		sudo apt-get -y -q install $DEB_DEPENDENCIES dpkg-dev debhelper
	fi
	if ! test -d $NAGIOS_SRC_DIR; then
		wget $NAGIOS_URL -q -O nagios-${NAGIOS_VERSION}.tar.gz
		tar xzf nagios-${NAGIOS_VERSION}.tar.gz
		(cd nagios && ./configure && make nagios)
	fi

	# Configure for release build
	mkdir -p m4 && autoreconf -i
	./configure --with-nagios-srcdir=$NAGIOS_SRC_DIR

	# Generate source distribution and package
	make clean dist
	script/build/release.sh
	;;
esac
