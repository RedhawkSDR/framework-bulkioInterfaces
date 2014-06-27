

dnl Process this file with autoconf to produce a configure script.
AC_INIT(Makefile.am)
AM_INIT_AUTOMAKE(Bulkio_Test,0.1)
AM_PATH_CPPUNIT(1.9.6)
AC_PROG_CXX
AC_PROG_CC
AC_PROG_INSTALL
AC_PREFIX_DEFAULT(${OSSIEHOME})
AX_BOOST_BASE([1.41])
AX_BOOST_THREAD
AX_BOOST_SYSTEM


OSSIE_CHECK_OSSIE
OSSIE_OSSIEHOME_AS_PREFIX
PKG_CHECK_MODULES([RH_DEPS], [ossie >= 1.7 omniORB4 >= 4.0.0])

# set PKG_CONFIG_PATH to look at local xxx.pc files

export PKG_CONFIG_PATH="../../../..:../../.."
PKG_CHECK_MODULES([BIO], [ bulkio >= 1.0 bulkioInterfaces >= 1.8 ])

AC_CORBA_ORB
AC_OUTPUT(Makefile)
