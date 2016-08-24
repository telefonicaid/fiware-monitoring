#!/bin/sh
#
# Copyright 2016 Telefónica I+D
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
# Build and execute unit tests
#
# Usage:
#     $0 [--help]
#
# Options:
#     -h, --help	show this help message
#

OPTS='h(help)'
PROG=$(basename $0)

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

# Project root at workspace
PROJECT_DIR=$PWD

# Absolute directories
LINT_REPORT_DIR=$PROJECT_DIR/report/lint
TEST_REPORT_DIR=$PROJECT_DIR/report/test
COVERAGE_REPORT_DIR=$PROJECT_DIR/report/coverage

# Properties
CFGFILE=$PROJECT_DIR/package.json
PRODUCT_INFO=$(awk '/"product"/,/\}/' $CFGFILE)
PRODUCT_AREA=$(echo "$PRODUCT_INFO" | sed -n '/"area"/ {s/.*:.*"\(.*\)".*/\1/; p; q}')
PRODUCT_NAME=$(echo "$PRODUCT_INFO" | sed -n '/"name"/ {s/.*:.*"\(.*\)".*/\1/; p; q}')
PRODUCT_RELEASE=$(echo "$PRODUCT_INFO" | sed -n '/"release"/ {s/.*:.*"\(.*\)".*/\1/; p; q}')
PROJECT_NAME=$(sed -n '/"name"/ {s/.*:.*"\(.*\)".*/\1/; p; q}' $CFGFILE)
PROJECT_DESC=$(sed -n '/"description"/ {s/.*:.*"\(.*\)".*/\1/; p; q}' $CFGFILE)
PROJECT_VERSION=$(sed -n '/"version"/ {s/.*:.*"\(.*\)".*/\1/; p; q}' $CFGFILE)
SONAR_PROJECT_NAME="Monitoring NGSI Adapter"
SONAR_PROJECT_KEY=com.telefonica.iot:monitoring-ngsi-adapter

# Install grunt
npm install -g grunt-cli

# Install development dependencies
npm install

# Generate reports
grunt lint-report test-report coverage-report

# Fix reports with paths relative to $WORKSPACE root
sed -i s:"$(readlink -f $PWD)":".":g $COVERAGE_REPORT_DIR/lcov.info

# SonarQube coverage reports
SONAR_COVERAGE_REPORT_RELATIVE_PATH=${COVERAGE_REPORT_DIR#$PROJECT_DIR/}/cobertura-coverage.xml
SONAR_LCOV_REPORT_RELATIVE_PATH=${COVERAGE_REPORT_DIR#$PROJECT_DIR/}/lcov.info

# Prepare properties file for SonarQube (workaround: awk to remove leading spaces)
awk '$1=$1' > $PROJECT_DIR/sonar-project.properties <<-EOF
	product.area.name=$PRODUCT_AREA
	product.name=$PRODUCT_NAME
	product.release=$PRODUCT_RELEASE
	sonar.projectName=$SONAR_PROJECT_NAME
	sonar.projectKey=$SONAR_PROJECT_KEY
	sonar.projectVersion=$PROJECT_VERSION
	sonar.projectDescription=$PROJECT_DESC
	sonar.language=js
	sonar.sourceEncoding=UTF-8
	sonar.sources=lib/
	sonar.tests=test/
	# coverage
	sonar.dynamicAnalysis=reuseReports
	sonar.cobertura.reportPath=$SONAR_COVERAGE_REPORT_RELATIVE_PATH
	sonar.javascript.lcov.reportPath=$SONAR_LCOV_REPORT_RELATIVE_PATH
EOF
