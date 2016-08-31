#!/bin/sh
#
# Copyright 2013-2016 Telefónica I+D
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
# Generate a package (.deb or .rpm) for this component. Unless active branch is
# "master", a snapshot including current date and git hash is created.
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

# Common properties
PROGDIR=$(readlink -f $(dirname $0))
BASEDIR=$(readlink -f $PROGDIR/../..)
CUR_COMMIT=$(git rev-parse --short HEAD)
CUR_BRANCH=$(git rev-parse --abbrev-ref HEAD)
IS_RELEASE=

# Package properties
CFGFILE=$BASEDIR/configure.ac
VERSION=$(sed -n '/AC_INIT/ {s/.*,[ \t]*\(.*\))/\1/; p}' $CFGFILE)

# Function to enable a release package depending on current branch
check_release() {
	if [ "$CUR_BRANCH" = "master" ]; then
		IS_RELEASE=true
	fi
}

# Function to create a .deb package
create_deb_package() {
	local pkgfile dpkg_files comment
	local pkgchangelog=$PROGDIR/files/debian/changelog
	local pkgversion=$(head -1 $pkgchangelog | awk -F'[()]' '{printf $2}')
	local pkgrevision=$(expr "$pkgversion" : "$VERSION-\([0-9]*\)" '|' 0)
	local pkgsnapshot=$(date '+%Y%m%d')git$CUR_COMMIT

	# Make a copy of files for package generation
	cp -r $PROGDIR/files/debian $BASEDIR
	cd $BASEDIR

	# Set snapshot changelog entries when appropriate
	if [ -z "$IS_RELEASE" ]; then
		comment="New snapshot"
		pkgversion=$VERSION-$pkgrevision.$pkgsnapshot
		debchange --newversion $pkgversion --controlmaint "$comment"
	fi

	# Create package
	dpkg-buildpackage -b -rfakeroot -D -us -uc \
	&& dpkg_files=$(ls -t ../*.deb ../*.changes 2>/dev/null | head -2) \
	&& pkgfile=$(readlink -f $(expr "$dpkg_files" : ".*/\(.*\.deb\)")) \
	&& mv -f $dpkg_files $BASEDIR \
	&& printf "\n%s successfully created.\n\n" $pkgfile

	# Cleanup
	[ -d ./debian ] && rm -rf ./debian
}

# Function to create a RPM package
create_rpm_package() {
	local pkgfile rpmbuild_file comment user
	local topdir=$BASEDIR/redhat
	local pkgspec=$PROGDIR/files/redhat/SPECS/*.spec
	local pkgversion=$(awk '/^\*/ {printf $NF; exit}' $pkgspec)
	local pkgrevision=$(expr "$pkgversion" : "$VERSION-\([0-9]*\)" '|' 0)
	local pkgsnapshot=$(date '+%Y%m%d')git$CUR_COMMIT
	local pkgrelease=$pkgrevision

	# Make a copy of files for package generation
	cp -r $PROGDIR/files/redhat $BASEDIR

	# Set snapshot changelog entries when appropriate
	if [ -z "$IS_RELEASE" ]; then
		comment="New snapshot"
		user=$(rpmdev-packager)
		user=${user:-$(awk -F': ' '/Packager:/ {printf $2}' $pkgspec)}
		pkgspec=$BASEDIR/files/redhat/SPECS/*.spec
		pkgrelease=$pkgrevision.$pkgsnapshot
		rpmdev-bumpspec -c "$comment" -u "$user" $pkgspec 2>/dev/null
		sed -i 's/- \(%{_version}-%{_release}\)\.[0-9]*/\1/' $pkgspec
	fi

	# Create package
	cd $topdir
	pkgversion=$VERSION
	rpmbuild -bb SPECS/*.spec \
	         --define "_topdir $topdir" \
	         --define "_basedir $BASEDIR" \
	         --define "_builddir $BASEDIR" \
	         --define "_version $pkgversion" \
	         --define "_release $pkgrelease" \
	&& rpmbuild_file=$(find RPMS/ -name *.rpm) \
	&& pkgfile=$(basename $rpmbuild_file) \
	&& mv -f $rpmbuild_file $BASEDIR \
	&& printf "\n%s successfully created.\n\n" $BASEDIR/$pkgfile

	# Cleanup
	[ -d $topdir ] && rm -rf $topdir
}

# Function to get GNU/Linux distro and invoke the function to create the package
create_package() {
	if [ -r /etc/redhat-release ]; then
		# RedHat/CentOS/Fedora
		create_rpm_package
	elif [ -r /etc/lsb-release -a -r /etc/issue.net ]; then
		# Ubuntu
		create_deb_package
	elif [ -r /etc/debian_version -a -r /etc/issue.net ]; then
		# Debian
		create_deb_package
	else
		printf "Unsupported GNU/Linux distribution\n" 1>&2
		return 1
	fi
}

# Main
check_release
create_package
