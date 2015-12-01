#!/bin/sh
#
# Copyright 2013-2015 Telefónica I+D
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
#     $0 build|package
#     $0 --help
#
# Options:
#     -h, --help	show this help message
#
# Actions:
#     build		build, generate reports and publish to SonarQube
#     package		generate distribution package
#
# Environment:
#     JOB_URL		full URL for this build job
#     WORKSPACE		absolute path for build job's workspace
#

OPTS='h(help)'
PROG=$(basename $0)

# Command line options
ACTION=

# Command line processing
OPTERR=
OPTSTR=$(echo :-:$OPTS | sed 's/([a-zA-Z0-9]*)//g')
OPTHLP=$(sed -n '20,/^$/ { s/$0/'$PROG'/; s/^#[ ]\?//p }' $0)
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
ACTION=$(expr "$1" : "^\(build\|package\)$") && shift
[ -z "$OPTERR" -a -z "$ACTION" ] && OPTERR="Valid action required as argument"
[ -z "$OPTERR" -a -n "$*" ] && OPTERR="Too many arguments"
[ -n "$OPTERR" ] && {
	PREAMBLE=$(printf "$OPTHLP" | sed -n '0,/^Usage:/ p' | head -n -1)
	USAGE="Usage:\n"$(printf "$OPTHLP" | sed '0,/^Usage:/ d')"\n\n"
	TAB=4; LEN=$(echo "$USAGE" | awk -F'\t' '/ .+\t/ {print $1}' | wc -L)
	TABSTOPS=$TAB,$(((LEN/TAB+1)*TAB)); WIDTH=${COLUMNS:-$(tput cols)}
	[ "$OPTERR" != "$OPTHLP" ] && PREAMBLE="ERROR: $OPTERR"
	printf "$PREAMBLE\n\n" | fmt -$WIDTH 1>&2
	printf "$USAGE" | tr -s '\t' | expand -t$TABSTOPS | fmt -$WIDTH -s 1>&2
	exit 1
}

# Main
if [ -z "$JOB_URL" -o -z "$WORKSPACE" ]; then
	printf "Jenkins variables JOB_URL and WORKSPACE are required\n" 1>&2
	exit 2
fi

# Absolute path of metrics-queue-consumer's workspace
METRICS_WORKSPACE=$(readlink -f $WORKSPACE/../../metrics-queue-consumer/workspace/workspace)

# Project root at Jenkins workspace
PROJECT_DIR=$WORKSPACE/ngsi_event_broker

# Absolute directories
PROJECT_SRC_DIR=$PROJECT_DIR/src
LINT_REPORT_DIR=$PROJECT_DIR/report/cppcheck
TEST_REPORT_DIR=$PROJECT_DIR/report/test
COVERAGE_REPORT_DIR=$PROJECT_DIR/report/coverage

# Properties
CFGFILE=$PROJECT_DIR/configure.ac
PRODUCT_AREA=$(sed -n '/\[PRODUCT_AREA\]/ {s/.*\[.*\].*\[\(.*\)\].*/\1/; p}' $CFGFILE)
PRODUCT_NAME=$(sed -n '/\[PRODUCT_NAME\]/ {s/.*\[.*\].*\[\(.*\)\].*/\1/; p}' $CFGFILE)
PRODUCT_RELEASE=$(sed -n '/\[PRODUCT_RELEASE\]/ {s/.*\[.*\].*\[\(.*\)\].*/\1/; p}' $CFGFILE)
PROJECT_DESC=$(sed -n '/\[DESCRIPTION\]/ {s/.*\[.*\].*\[\(.*\)\].*/\1/; p}' $CFGFILE)
PROJECT_NAME=$(sed -n '/AC_INIT/ {s/.*\[PRODUCT_NAME-\(.*\)].*/\1/; p}' $CFGFILE)
PROJECT_VERSION=$(sed -n '/AC_INIT/ {s/.*,[ \t]*\(.*\))/\1/; p}' $CFGFILE)
SONAR_PROJECT_NAME="Monitoring NGSI Event Broker"
SONAR_PROJECT_KEY=com.telefonica.iot:monitoring-ngsi-event-broker

# Dependencies
RPM_BUILD_DEPENDENCIES="wget gcc-c++ make autoconf automake libtool cppunit-devel libcurl-devel"
DEB_BUILD_DEPENDENCIES="wget g++ build-essential autoconf automake autotools-dev libtool libcppunit-dev libcurl4-openssl-dev"
NAGIOS_SRC_DIR=$PROJECT_DIR/nagios
NAGIOS_INC_DIR=$PROJECT_DIR/$(awk -F= '/nagios_incdir=/ { print $2 }' $CFGFILE)
NAGIOS_VERSION=$(awk -F= '/nagios_reqver=/ { print $2 }' $CFGFILE)
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
		sudo yum -y -q install $RPM_BUILD_DEPENDENCIES lcov libxslt cppcheck
	else
		# Ubuntu
		sudo apt-get -y -q install $DEB_BUILD_DEPENDENCIES lcov xsltproc cppcheck
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

	# Fix reports with paths relative to $WORKSPACE root
	sed -i 's#\./src/#ngsi_event_broker/src/#' $COVERAGE_REPORT_DIR/lcov.info
	sed -i 's#filename="src/#filename="ngsi_event_broker/src/#' $COVERAGE_REPORT_DIR/cobertura-coverage.xml

	# SonarQube lint report with absolute metrics-queue-consumer paths
	SONAR_LINT_REPORT_PATH=$LINT_REPORT_DIR/sonar-cppcheck-result.xml
	SONAR_LINT_REPORT_RELATIVE_PATH=${SONAR_LINT_REPORT_PATH#$PROJECT_DIR/}
	sed 's#file="#&'$METRICS_WORKSPACE/ngsi_event_broker/'#' $LINT_REPORT_DIR/cppcheck-result.xml \
	> $SONAR_LINT_REPORT_PATH

	# SonarQube test reports (no need to change paths)
	SONAR_TEST_REPORT_RELATIVE_PATH="${TEST_REPORT_DIR#$PROJECT_DIR/}/TEST-xunit-*.xml"

	# SonarQube coverage report with absolute metrics-queue-consumer paths
	SONAR_COVERAGE_REPORT_PATH=$COVERAGE_REPORT_DIR/sonar-cobertura-coverage.xml
	SONAR_COVERAGE_REPORT_RELATIVE_PATH=${SONAR_COVERAGE_REPORT_PATH#$PROJECT_DIR/}
	sed 's#filename="#&'$METRICS_WORKSPACE/'#' $COVERAGE_REPORT_DIR/cobertura-coverage.xml \
	> $SONAR_COVERAGE_REPORT_PATH

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
		sonar.projectDescription=$PROJECT_DESC
		sonar.language=c++
		sonar.sourceEncoding=UTF-8
		sonar.sources=src/
		sonar.tests=test/
		sonar.test.exclusions=**/*.c
		sonar.cxx.includeDirectories=$SONAR_INCLUDE_DIRS
		sonar.cxx.cppcheck.reportPath=$SONAR_LINT_REPORT_RELATIVE_PATH
		sonar.cxx.xunit.reportPath=$SONAR_TEST_REPORT_RELATIVE_PATH
		sonar.cxx.coverage.reportPath=$SONAR_COVERAGE_REPORT_RELATIVE_PATH
	EOF

	# Generate metrics in SonarQube
	export DEBUG_METRICS=FALSE
	metrics_runner.sh
	;;

package)
	# Install development and package generation dependencies
	if test -r /etc/redhat-release; then
		# CentOS
		sudo yum -y -q install $RPM_BUILD_DEPENDENCIES rpm-build rpmdevtools redhat-rpm-config
	else
		# Ubuntu
		sudo apt-get -y -q install $DEB_BUILD_DEPENDENCIES dpkg-dev debhelper devscripts
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
	tools/build/package.sh
	;;
esac
