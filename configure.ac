#
# This file is protected by Copyright. Please refer to the COPYRIGHT file 
# distributed with this source distribution.
# 
# This file is part of REDHAWK core.
# 
# REDHAWK core is free software: you can redistribute it and/or modify it under 
# the terms of the GNU Lesser General Public License as published by the Free 
# Software Foundation, either version 3 of the License, or (at your option) any 
# later version.
# 
# REDHAWK core is distributed in the hope that it will be useful, but WITHOUT 
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS 
# FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
# details.
# 
# You should have received a copy of the GNU Lesser General Public License 
# along with this program.  If not, see http://www.gnu.org/licenses/.
#

AC_INIT(bulkioInterfaces, 1.8.3)
AC_SUBST([LIBBULKIOINTERFACES_VERSION_INFO], [2:0:0])

AC_PREFIX_DEFAULT(${OSSIEHOME})

AM_INIT_AUTOMAKE(nostdinc)
AC_PROG_CC
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_LIBTOOL

AM_PATH_PYTHON([2.3])

OSSIE_CHECK_OSSIE
OSSIE_OSSIEHOME_AS_PREFIX
OSSIE_PYTHON_INSTALL_SCHEME

AC_CORBA_ORB
AC_CHECK_PROG([IDL], [omniidl], [omniidl], [no])
if test "$IDL" = no; then
  AC_MSG_ERROR([cannot find omniidl program])
fi
AC_LANG_PUSH([C++])
PKG_CHECK_MODULES([OMNIORB], [omniORB4 >= 4.0.0])
PKG_CHECK_MODULES(OSSIE, ossie >= 1.7.0,,exit)

# Optionally include java support
AC_ARG_ENABLE([java], AS_HELP_STRING([--disable-java], [Disable framework java support]))

HAVE_JAVASUPPORT=no
if test "x$enable_java" != "xno"; then
  # JAVA_HOME is required to locate the JNI headers
  AC_ARG_VAR([JAVA_HOME], [Java Development Kit (JDK) location])
  AC_MSG_CHECKING([for JAVA_HOME])
  if test -z "$JAVA_HOME"; then
    AC_MSG_RESULT([no])
  else
    AC_MSG_RESULT([$JAVA_HOME])
    # Put JAVA_HOME at the front of the path to locate Java tools
    export PATH="${JAVA_HOME}/bin:${PATH}"
  fi
  AC_SUBST(JAVA_HOME)

  AC_CHECK_PROG([IDLJ], [idlj], [idlj], [no])
  AC_CHECK_PROG([JAVAC], [javac], [javac], [no])
  AC_CHECK_PROG([JAR], [jar], [jar], [no])
  if test "$IDLJ" = no -o "$JAVAC" = no -o "$JAR" = no; then
    AC_MSG_WARN([Java support won't be built due to missing dependencies])
  else
    # Verify that we can include the JNI header; the platform-specific location is
    # assumed to be Linux.
    saved_CPPFLAGS="$CPPFLAGS"
    JNI_CPPFLAGS="-I$JAVA_HOME/include -I$JAVA_HOME/include/linux"
    CPPFLAGS="$JNI_CPPFLAGS"
    AC_CHECK_HEADER([jni.h], [HAVE_JNI_H="yes"])
    CPPFLAGS="$saved_CPPFLAGS"
  fi

  if test -n "$HAVE_JNI_H"; then
    # The omnijni package must be available to generate JNI stubs and skeletons.
    AC_CHECK_PYMODULE([ossie.omnijni], [have_omnijni="yes"])
  fi

  if test -n "$have_omnijni"; then
    # Verify that we can include the CF JNI headers.
    saved_CPPFLAGS="$CPPFLAGS"
    CPPFLAGS="$JNI_CPPFLAGS $OSSIE_CFLAGS"
    AC_CHECK_HEADER([CF/jni_cf.h], [HAVE_JNI_CF_H="yes"])
    CPPFLAGS="$saved_CPPFLAGS"
  fi

  if test -n "HAVE_JNI_CF_H"; then
    # Finally, check for the CF JNI module.
    AC_MSG_CHECKING([for ossiecfjni dynamic library])
    ossie_libdir=`pkg-config --variable=libdir ossie`
    if test -f "${ossie_libdir}/libossiecfjni.so"; then
      AC_MSG_RESULT([yes])
      JNI_LIBS="-L$ossie_libdir -lossiecfjni"
      HAVE_JAVASUPPORT="yes"
    else
      AC_MSG_RESULT([no])
    fi
  fi
  AC_SUBST(JNI_LIBS)
  AC_SUBST(JNI_CPPFLAGS)
fi
AC_MSG_CHECKING([for framework java support])
AC_MSG_RESULT($HAVE_JAVASUPPORT)
AM_CONDITIONAL(HAVE_JAVASUPPORT, test $HAVE_JAVASUPPORT = yes)
# End optional java support

AC_CONFIG_FILES(Makefile jni/Makefile)
AC_OUTPUT
