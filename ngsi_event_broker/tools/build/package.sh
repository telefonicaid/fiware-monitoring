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
# Generate a package (.deb or .rpm) for this component, automatically including
# new changelog entries taken from descriptions of the pull requests been merged
# since last release.
#
# Usage:
#     $0 [--interactive] [--release [--push]]
#     $0 [--help]
#
# Options:
#     -i, --interactive	edit changelog interactively before packaging
#     -r, --release	new release (default revision=1) instead of a snapshot
#     -p, --push	commit and push updated changelog (only for releases)
#     -h, --help	show this help message
#

OPTS='i(interactive)r(release)p(push)h(help)'
PROG=$(basename $0)

# Command line options
INTERACTIVE=
RELEASE=
PUSH=

# Command line processing
OPTERR=
OPTSTR=$(echo :-:$OPTS | sed 's/([a-zA-Z0-9]*)//g')
OPTHLP=$(sed -n '20,/^$/ { s/$0/'$PROG'/; s/^#[ ]\?//p }' $0)
while getopts $OPTSTR OPT; do while [ -z "$OPTERR" ]; do
case $OPT in
'i')	INTERACTIVE=true;;
'r')	RELEASE=true;;
'p')	PUSH=true; [ -n "$RELEASE" ] || OPTERR="Only releases allow push.";;
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

# Packaging properties
BRANCH=$(git rev-parse --abbrev-ref HEAD)
COMMIT=$(git rev-parse --short HEAD)
PROGDIR=$(readlink -f $(dirname $0))
BASEDIR=$(readlink -f $PROGDIR/../..)
CFGFILE=$BASEDIR/configure.ac
REPOURL=$(sed -n '/ARTIFACTS/ {s/.*\[.*\].*\[\(.*\)\].*/\1/; p}' $CFGFILE)
PACKAGE=$(sed -n '/AC_INIT/ {s/.*\[PRODUCT_NAME-\(.*\)].*/\1/; p}' $CFGFILE)
VERSION=$(sed -n '/AC_INIT/ {s/.*,[ \t]*\(.*\))/\1/; p}' $CFGFILE)
REVISION=1

# Function to print changelog entries since last release
print_changelog_update() {
	local latest_tag=$(git tag -l | tail -1)
	local branch_types="hardening\|feature\|bug\|hotfix"
	local branch_regex=".*\($branch_types\)\/\([A-Z_-]*[0-9]*\).*"
	git log --merges --grep=pull --reverse --format=format:'%s: %b' \
		$latest_tag..HEAD \
	| sed 's/'$branch_regex': \(.*\)/\3 (\1 #\2)/' \
	| sed 's/\(.*\) (.* #)/\1/' && echo
}

# Function to write changelog entries in MarkDown syntax (for GitHub releases)
write_changelog_markdown() {
	local changelog=$1
	local pkgfile=$2
	local pkgtype=${pkgfile##*.}
	local markdown=${pkgfile%.*}.changelog.md
	local component=$(echo $PACKAGE | tr '-' '_')
	local artifacts=$REPOURL/$pkgtype
	cat > $markdown <<-EOF
		#### Changelog
		$(cat $changelog)

		#### Components
		- $component version $VERSION ([download])

		[download]:
		$artifacts
		"FIWARE Artifacts Repository"
	EOF
	printf "%s successfully created.\n\n" $markdown
}

# Function to change product release
bump_product_release() {
	local cur_release=$(sed -n '/\[PRODUCT_RELEASE\]/ {
		s/.*\[.*\].*\[\(.*\)\].*/\1/; p}' $CFGFILE)
	local new_release=$1
	[ "$cur_release" != "$new_release" ] && bumpversion \
		--no-commit --no-tag --allow-dirty \
		--current-version=$cur_release --new-version=$new_release \
		--message='New product release {new_version}' \
		--search='"release": "{current_version}"' \
		--replace='"release": "{new_version}"' \
		patch $CFGFILE
}

# Function to commit & push or revert updated files when required (and allowed)
git_process_updated_files() {
	local files=$1
	local push=$PUSH
	local is_release_branch=$(expr "$BRANCH" : "release/\(.*\)")
	local release_number=${BRANCH#release/}
	local commit_message="Update changelog"

	# Check whether current branch (for push) is named as 'release/*'
	if [ -n "$push" -a -z "$is_release_branch" ]; then
		printf "\nWARNING: Not on a 'release/*' branch, cannot push\n\n"
		unset push
	fi

	# Update product release
	if [ -n "$is_release_branch" ]; then
		bump_product_release $release_number
		commit_message="${commit_message} and product release"
		files="$files $CFGFILE"
	fi

	# Commit & push changes, or revert
	if [ -n "$push" ]; then
		git add $files
		git commit -m "${commit_message}"
		git push origin $BRANCH
	else
		git checkout -- $files
	fi
}

# Function to create a DEB package
create_deb_package() {
	local pkgfile dpkg_files comment
	local changelog=$PROGDIR/files/debian/changelog
	local tmpfile=$(mktemp --tmpdir changelog_XXXXXX)
	local pkgdist=$(head -1 $changelog | awk -F'[ ;]' '{printf $3}')
	local pkgversion=$(head -1 $changelog | awk -F'[()]' '{printf $2}')
	local pkgrevision=$(expr "$pkgversion" : "$VERSION-\([0-9]*\)" '|' 0)
	local pkgsnapshot=$(date '+%Y%m%d')git$COMMIT
	local deboptions

	# Update changelog
	cd $PROGDIR/files
	pkgversion=$VERSION-$((pkgrevision + 1))
	print_changelog_update | sed 's/^/- /' > $tmpfile
	[ -n "$INTERACTIVE" ] && ${EDITOR:-vi} $tmpfile
	deboptions="--newversion $pkgversion"
	sed 's/^- //' $tmpfile | while read comment; do
		debchange $deboptions --controlmaint "$comment"
		deboptions=--append
	done
	[ -n "$RELEASE" ] && EDITOR=true debchange --release --controlmaint \
		--no-force-save-on-release --distribution=${pkgdist:-unstable}

	# Copy auxiliary files and commit/revert changelog modifications
	cp -r $PROGDIR/files/debian $BASEDIR
	git_process_updated_files $changelog

	# Set package snapshot/release version number
	[ -n "$RELEASE" ] || pkgversion=$VERSION-$pkgrevision.$pkgsnapshot
	sed -i "s/($VERSION-.*)/($pkgversion)/" $BASEDIR/debian/changelog

	# Create package
	cd $BASEDIR
	dpkg-buildpackage -b -rfakeroot -D -us -uc \
	&& dpkg_files=$(ls -t ../*.deb ../*.changes 2>/dev/null | head -2) \
	&& pkgfile=$(readlink -f $(expr "$dpkg_files" : ".*/\(.*\.deb\)")) \
	&& mv -f $dpkg_files $BASEDIR \
	&& printf "\n%s successfully created.\n\n" $pkgfile

	# Create a changelog for GitHub
	[ -n "$RELEASE" ] && write_changelog_markdown $tmpfile $pkgfile

	# Cleanup
	rm -f $tmpfile
	[ -d ./debian ] && rm -rf ./debian
}

# Function to create a RPM package
create_rpm_package() {
	local pkgfile user rpmbuild_file
	local topdir=$BASEDIR/redhat
	local rpmspec=$PROGDIR/files/redhat/SPECS/*.spec
	local tmpfile=$(mktemp --tmpdir changelog_XXXXXX)
	local pkgversion=$(awk '/^\*/ {printf $NF; exit}' $rpmspec)
	local pkgrevision=$(expr "$pkgversion" : "$VERSION-\([0-9]*\)" '|' 0)
	local pkgsnapshot=$(date '+%Y%m%d')git$COMMIT
	local pkgrelease

	# Update changelog
	print_changelog_update | sed 's/^/- /' > $tmpfile
	[ -n "$INTERACTIVE" ] && ${EDITOR:-vi} $tmpfile
	user=$(rpmdev-packager || awk -F': ' '/Packager:/ {printf $2}' $rpmspec)
	rpmdev-bumpspec --comment="$(cat $tmpfile)" \
		--userstring="$user" $rpmspec 2>/dev/null \
		&& sed -i 's/- \(%{_version}-%{_release}\)\.[0-9]*/\1/' $rpmspec

	# Copy auxiliary files and commit/revert changelog modifications
	cp -r $PROGDIR/files/redhat $BASEDIR
	git_process_updated_files $rpmspec

	# Set package version and release
	pkgversion=$VERSION
	pkgrelease=$pkgrevision.$pkgsnapshot
	[ -n "$RELEASE" ] && pkgrelease=$((pkgrevision + 1))

	# Create package
	cd $topdir
	rpmbuild -bb SPECS/*.spec \
	         --define "_topdir $topdir" \
	         --define "_basedir $BASEDIR" \
	         --define "_version $pkgversion" \
	         --define "_release $pkgrelease" \
	&& rpmbuild_file=$(find RPMS/ -name *.rpm) \
	&& pkgfile=$(basename $rpmbuild_file) \
	&& mv -f $rpmbuild_file $BASEDIR \
	&& printf "\n%s successfully created.\n\n" $BASEDIR/$pkgfile
	cd $BASEDIR

	# Create a changelog for GitHub
	[ -n "$RELEASE" ] && write_changelog_markdown $tmpfile $BASEDIR/$pkgfile

	# Cleanup
	rm -f $tmpfile
	[ -d $topdir ] && rm -rf $topdir
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
if ! get_linux_distro OS_DISTRO; then
	echo "Could not get GNU/Linux distribution" 1>&2
	exit 2
elif [ $(expr "$OS_DISTRO" : 'Ubuntu.*\|Debian.*') -ne 0 ]; then
	create_deb_package
elif [ $(expr "$OS_DISTRO" : 'CentOS.*\|RedHat.*') -ne 0 ]; then
	create_rpm_package
else
	echo "Unsupported GNU/Linux distribution" 1>&2
	exit 3
fi
