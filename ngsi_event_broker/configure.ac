# Autoconf initialization.
AC_INIT(ngsi_event_broker, 1.1.0)
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
   prefix="/usr"
fi
if test "$exec_prefix" = "NONE"; then
   exec_prefix=$prefix
fi
AC_CANONICAL_HOST
AS_CASE([$host], [x86_64*],
   [libdir=$exec_prefix/lib64/nagios],
   [libdir=$exec_prefix/lib/nagios])

# Configuration arguments.
AC_ARG_WITH(nagios-srcdir,
   AC_HELP_STRING([--with-nagios-srcdir=DIR],
   [Sets path to Nagios sources @<:@default=/usr/local/src/nagios@:>@], 30),
   nagios_srcdir=$withval,
   nagios_srcdir=/usr/local/src/nagios)
CPPFLAGS="$CPPFLAGS -I$nagios_srcdir/include"
AC_SUBST([NAGIOS_SRCDIR],[$nagios_srcdir])

# Library version spec (major:minor:revision).
VERSION_SPEC=$(echo $VERSION | sed 's/\./:/g')
AC_SUBST([VERSION_SPEC])

# Package specification attributes.
PACKAGE_DISTNAME=$PACKAGE
AC_SUBST([PACKAGE_DISTNAME])

# Checks for programs.
AC_PROG_CC
AC_PROG_CXX
if test "$CXX" = "no" || test "$CXX:$GXX" = "g++:"; then
   AC_MSG_ERROR([no C++ compiler found])
fi
ifdef([AM_PATH_CPPUNIT],
   [AM_PATH_CPPUNIT(1.11.0)],
   [AC_MSG_ERROR([please install CppUnit for tests (requires autoreconf)])])

# Checks for packages and dependencies.
AC_CACHE_CHECK([for Nagios sources],
   [my_cv_nagios_sources],
   [AC_PREPROC_IFELSE(
   [AC_LANG_SOURCE([[#include "nebmodules.h"]])],
   my_cv_nagios_sources=$nagios_srcdir, my_cv_nagios_sources=no)])
if test "$my_cv_nagios_sources" = "no"; then
   AC_MSG_ERROR([cannot find sources (use --with-nagios-srcdir option)])
fi

# Checks for header files.
AC_CACHE_CHECK([for Nagios headers],
   [my_cv_nagios_headers],
   [AC_PREPROC_IFELSE(
   [AC_LANG_SOURCE([[#include "nagios.h"]])],
   my_cv_nagios_headers=yes, my_cv_nagios_headers=no)])
if test "$my_cv_nagios_headers" = "no"; then
   AC_MSG_ERROR([cannot find headers (run make nagios on $nagios_srcdir)])
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

AC_CACHE_CHECK([for yajl],
   [my_cv_yajl_library],
   [AC_PREPROC_IFELSE(
   [AC_LANG_SOURCE([[#include "yajl/yajl_common.h"]])],
   my_cv_yajl_library=yes, my_cv_yajl_library=no)])
if test "$my_cv_yajl_library" = "no"; then
   AC_MSG_ERROR([yajl development files must be installed])
fi

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_TYPES([yajl_parser_config],
   [my_cv_yajl_v1=yes], [my_cv_yajl_v1=no],
   [[#include "yajl/yajl_parse.h"]])
if test "$my_cv_yajl_v1" = "no"; then
   AC_MSG_ERROR([yajl 1.0.x must be installed])
fi

# Checks for library functions.

AC_OUTPUT
