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
# Release a version of this component and generate .deb or .rpm package
#
# Usage:
#     $0 [--help]
#
# Options:
#     -h, --help	show this help message
#

OPTS='h(help)'
NAME=$(basename $0)

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
[ -z "$OPTERR" -a -n "$*" ] && OPTERR="Too many arguments"
[ -n "$OPTERR" ] && {
	[ "$OPTERR" != "$OPTHLP" ] && OPTERR="${OPTERR}\nTry \`$NAME --help'"
	TAB=4; LEN=$(echo "$OPTERR" | awk -F'\t' '/\t.+\t/ {print $2}' | wc -L)
	TABSTOPS=$TAB,$(((2+LEN/TAB)*TAB)); WIDTH=${COLUMNS:-$(tput cols)}
	printf "$OPTERR" | tr -s '\t' | expand -t$TABSTOPS | fmt -$WIDTH -s 1>&2
	exit 1
}

# Function to create a Debian package
create_debian_package() {
	local package dpkg_files dpkg_dir=$BASEDIR/..
	cd $BASEDIR && cp -r $PROGDIR/files/debian .
	dpkg-buildpackage -b -rfakeroot -D -us -uc \
	&& dpkg_files=$(ls -t ../*.deb ../*.changes 2>/dev/null | head -2) \
	&& package=$(expr "$dpkg_files" : ".*/\(.*\.deb\)") \
	&& mv -f $dpkg_files $BASEDIR \
	&& printf "\n%s successfully created.\n\n" $(readlink -f $package)
	[ -d ./debian ] && rm -rf ./debian
}

# Function to obtain GNU/Linux distro (set variable $1; OS_DISTRO if not given)
get_linux_distro() {
	local retvar=${1:-OS_DISTRO}
	local distro
	if [ -r /etc/redhat-release ]; then
		# RedHat/CentOS/Fedora
		distro=$(cat /etc/redhat-release)
	elif [ -r /etc/lsb-release -a -r /etc/issue.net ]; then
		# Ubuntu
		distro=$(cat /etc/issue.net)
	elif [ -r /etc/debian_version -a -r /etc/issue.net ]; then
		# Debian
		distro=$(cat /etc/issue.net)
	fi
	[ -z "$distro" ] && return 1
	eval $retvar=\"$distro\"
}

# Main
PROGDIR=$(readlink -f $(dirname $0))
BASEDIR=$(readlink -f $PROGDIR/../..)
if ! get_linux_distro OS_DISTRO; then
	echo "Could not get GNU/Linux distribution" 1>&2
	exit 2
elif [ $(expr "$OS_DISTRO" : 'Ubuntu.*\|Debian.*') -ne 0 ]; then
	create_debian_package
else
	echo "Unsupported GNU/Linux distribution" 1>&2
	exit 3
fi
