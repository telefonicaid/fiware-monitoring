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
# Update version strings using bumpversion package (run 'pip' to install)
#
# Usage:  .bumpversion.sh [bumpversion options ...]
# See:    https://pypi.python.org/pypi/bumpversion
#


# Files to be updated
BUMPVERSION_CONF=$0
ADAPTER_CONF=ngsi_adapter/src/package.json
BROKER_CONF=ngsi_event_broker/configure.ac

# Additional bumpversion options
BUMPVERSION_OPTS="$*"

# Check whether pip tool is installed
which pip >/dev/null || {
	printf "Cannot find pip command\n\n" 1>&2
	exit 1
}

# Check whether bumpversion package is installed
which bumpversion >/dev/null || {
	printf "Please run \`pip install bumpversion' to use this tool\n\n" 1>&2
	exit 1
}

# Ask for new FIWARE release
CUR_RELEASE=4.3.2
PROMPT="Please specify new FIWARE release (current is $CUR_RELEASE): "
read -p "$PROMPT" NEW_RELEASE
test -n "$(expr "$NEW_RELEASE" : '\([0-9]\+\.[0-9]\+\.[0-9]\+\)$')" || {
	printf "ERROR: A release number x.y.z must be supplied\n\n" 1>&2
	exit 2
}

# Ask for new NGSI Adapter version
CUR_ADAPTER_VER=$(awk -F'"' '/"version"/ {print $4}' $ADAPTER_CONF)
PROMPT="Please specify new NGSI Adapter version (current is $CUR_ADAPTER_VER): "
read -p "$PROMPT" NEW_ADAPTER_VER
test -n "$(expr "$NEW_ADAPTER_VER" : '\([0-9]\+\.[0-9]\+\.[0-9]\+\)$')" || {
	printf "ERROR: A version number x.y.z must be supplied\n\n" 1>&2
	exit 2
}

# Ask for new Event Broker version
CUR_BROKER_VER=$(sed -n '/AC_INIT/ {s/.*,[ \t]*\(.*\))/\1/; p}' $BROKER_CONF)
PROMPT="Please specify new Event Broker version (current is $CUR_BROKER_VER): "
read -p "$PROMPT" NEW_BROKER_VER
test -n "$(expr "$NEW_BROKER_VER" : '\([0-9]\+\.[0-9]\+\.[0-9]\+\)$')" || {
	printf "ERROR: A version number x.y.z must be supplied\n\n" 1>&2
	exit 2
}

# Abort on error
set -e

# Update release in bumpversion configuration (this file)
[ "$NEW_RELEASE" != "$CUR_RELEASE" ] && bumpversion --commit --no-tag \
	--current-version=$CUR_RELEASE --new-version=$NEW_RELEASE \
	--message='Bump product release: {current_version} -> {new_version}' \
	--search='CUR_RELEASE={current_version}' \
	--replace='CUR_RELEASE={new_version}' \
	$BUMPVERSION_OPTS patch $BUMPVERSION_CONF

# Update release and version in NGSI Adapter configuration files
[ "$NEW_RELEASE" != "$CUR_RELEASE" ] && bumpversion --commit --no-tag \
	--current-version=$CUR_RELEASE --new-version=$NEW_RELEASE \
	--message='Bump product release: {current_version} -> {new_version}' \
	--search='"release": "{current_version}"' \
	--replace='"release": "{new_version}"' \
	$BUMPVERSION_OPTS patch $ADAPTER_CONF

[ "$NEW_ADAPTER_VER" != "$CUR_ADAPTER_VER" ] && bumpversion --commit --no-tag \
	--current-version=$CUR_ADAPTER_VER --new-version=$NEW_ADAPTER_VER \
	--message='Bump component version: {current_version} -> {new_version}' \
	--search='"version": "{current_version}"' \
	--replace='"version": "{new_version}"' \
	$BUMPVERSION_OPTS patch $ADAPTER_CONF

# Update release and version in Event Broker configuration files
[ "$NEW_RELEASE" != "$CUR_RELEASE" ] && bumpversion --commit --no-tag \
	--current-version=$CUR_RELEASE --new-version=$NEW_RELEASE \
	--message='Bump product release: {current_version} -> {new_version}' \
	--search='[PRODUCT_RELEASE], [{current_version}]' \
	--replace='[PRODUCT_RELEASE], [{new_version}]' \
	$BUMPVERSION_OPTS patch $BROKER_CONF

[ "$NEW_BROKER_VER" != "$CUR_BROKER_VER" ] && bumpversion --commit --no-tag \
	--current-version=$CUR_BROKER_VER --new-version=$NEW_BROKER_VER \
	--message='Bump component version: {current_version} -> {new_version}' \
	--search='AC_INIT([PRODUCT_NAME-ngsi-event-broker], {current_version})'\
	--replace='AC_INIT([PRODUCT_NAME-ngsi-event-broker], {new_version})' \
	$BUMPVERSION_OPTS patch $BROKER_CONF
