# Autoconf initialization.
m4_define([DESCRIPTION], [Nagios module to forward plugin data to NGSI Adapter])
m4_define([PRODUCT_NAME], [fiware-monitoring])
m4_define([PRODUCT_AREA], [iotplatform])
m4_define([PRODUCT_RELEASE], [5.2.1])
m4_define([ARTIFACTS_URL], [http://repositories.testbed.fiware.org/repo])
AC_INIT([PRODUCT_NAME-ngsi-event-broker], 1.4.4)
AC_LANG(C)
if test -z "$CFLAGS"; then
   # prevent configure from assigning default flags
   CFLAGS=""
fi

# Automake initialization.
AM_INIT_AUTOMAKE([foreign])

# Libtool initialization.
LT_INIT([shared disable-static pic-only])

# Source files and directories.
AC_CONFIG_MACRO_DIR(m4)
AM_CONFIG_HEADER(src/config.h)
AC_CONFIG_FILES([Makefile src/Makefile test/Makefile])
AC_CONFIG_COMMANDS([cppunit],
   [sed -i 's/\(tests="\)\(test\)\(s\?"\)/\1suite\3/g' test/Makefile])

# Installation directory (same as Nagios libraries).
if test "$prefix" = "NONE"; then
   prefix="/opt/fiware"
fi
if test "$exec_prefix" = "NONE"; then
   exec_prefix=$prefix
fi
if test "$libdir" = '${exec_prefix}/lib'; then
   AC_CANONICAL_HOST
   AS_CASE([$host], [x86_64*],
      [libdir=$exec_prefix/ngsi_event_broker/lib],
      [libdir=$exec_prefix/ngsi_event_broker/lib])
fi

# Optional features.
AC_ARG_ENABLE(gcov,
   AC_HELP_STRING([--enable-gcov],
   [Turn on gcov coverage testing (GCC only).]),
   [if test "$enableval" = "yes"; then
       if test "$GCC" = "yes"; then
          AC_MSG_NOTICE([enabled gcov coverage])
          CFLAGS="$CFLAGS -fprofile-arcs -ftest-coverage -g -UNDEBUG"
          LDFLAGS="$LDFLAGS -coverage"
       else
          AC_MSG_ERROR([--enable-gcov requires GCC.])
       fi
    fi])
AM_CONDITIONAL([GCOV_ENABLED], [test "$enableval" = "yes"])

# Configuration arguments.
AC_ARG_WITH(nagios-srcdir,
   AC_HELP_STRING([--with-nagios-srcdir=DIR],
   [Sets path to Nagios sources @<:@default=/usr/local/src/nagios@:>@], 30),
   nagios_srcdir=$(readlink -f $withval),
   nagios_srcdir=/usr/local/src/nagios)
AC_SUBST([NAGIOS_SRCDIR],[$nagios_srcdir])
if test -d $nagios_srcdir/include; then
   # Current and required Nagios versions
   nagios_common=$nagios_srcdir/include/common.h
   nagios_curver=$(awk -F'"' '/PROGRAM_VERSION/ {print $2}' $nagios_common)
   nagios_reqver=3.4.1
   # Directory where Nagios headers are copied
   nagios_incdir=contrib/nagios/include
   mkdir -p $nagios_incdir
   # Avoid including <syslog.h> in Nagios headers copy
   cp $nagios_srcdir/include/*.h $nagios_incdir
   sed -i 's/#define HAVE_SYSLOG_H 1/#undef HAVE_SYSLOG_H/' $nagios_incdir/config.h
   CPPFLAGS="$CPPFLAGS -I$(readlink -f $nagios_incdir)"
fi

# Library version spec (major:minor:revision).
VERSION_SPEC=$(echo $PACKAGE_VERSION | sed 's/\./:/g')
AC_SUBST([VERSION_SPEC])

# Package specification attributes.
PACKAGE=$PACKAGE_NAME
PACKAGE_TARNAME=$PACKAGE_NAME

# Checks for programs.
AC_PROG_CC
AC_PROG_CXX
if test "$CXX" = "no" || test "$CXX:$GXX" = "g++:"; then
   AC_MSG_ERROR([no C++ compiler found])
fi
ifdef([AM_PATH_CPPUNIT], [AM_PATH_CPPUNIT(1.11.0)])
if test "$CPPUNIT_CONFIG" = "no"; then
   AC_MSG_ERROR([please install CppUnit for tests])
elif test "$cppunit_version_proper" != "1"; then
   AC_MSG_ERROR([please check CppUnit version])
fi

# Checks for packages and dependencies.
AC_CACHE_CHECK([for Nagios sources],
   [my_cv_nagios_sources],
   [AC_PREPROC_IFELSE(
   [AC_LANG_SOURCE([[#include "nebmodules.h"]])],
   my_cv_nagios_sources=$nagios_srcdir, my_cv_nagios_sources=no)])
if test "$my_cv_nagios_sources" = "no"; then
   AC_MSG_ERROR([cannot find sources (use --with-nagios-srcdir option)])
elif test $(printf "$nagios_reqver\n$nagios_curver" | sort -V | head -1) != $nagios_reqver; then
   AC_MSG_ERROR([Nagios sources of version $nagios_reqver or greater required])
fi

# Checks for header files.
AC_CACHE_CHECK([for Nagios headers],
   [my_cv_nagios_headers],
   [AC_PREPROC_IFELSE(
   [AC_LANG_SOURCE([[#include "nagios.h"]])],
   my_cv_nagios_headers=yes, my_cv_nagios_headers=no)])
if test "$my_cv_nagios_headers" = "no"; then
   AC_MSG_ERROR([cannot find headers (run 'make nagios' on $nagios_srcdir)])
fi

# Checks for libraries.
AC_CACHE_CHECK([for libcURL],
   [my_cv_libcurl],
   [AC_PREPROC_IFELSE(
   [AC_LANG_SOURCE([[#include "curl/curl.h"]])],
   my_cv_libcurl=yes, my_cv_libcurl=no)])
if test "$my_cv_libcurl" = "no"; then
   AC_MSG_ERROR([libcURL development files must be installed])
fi

# Checks for library functions.

# Checks for host configuration.
AC_CACHE_CHECK([/etc/hosts for localhost address],
   [my_cv_localhost_address],
   [my_cv_localhost_address=$(hostname -i 2>/dev/null)])
if test "$my_cv_localhost_address" = ""; then
   AC_MSG_ERROR([unknown IP address for host '$HOSTNAME'])
fi

AC_OUTPUT
