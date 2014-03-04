AC_INIT(Java_Ports, 1.0.0)
AM_INIT_AUTOMAKE(nostdinc)

AC_CORBA_ORB
OSSIE_CHECK_OSSIE
OSSIE_SDRROOT_AS_PREFIX

export PKG_CONFIG_PATH="$PKG_CONFIG_PATH:/usr/local/lib/pkgconfig"
PKG_CHECK_MODULES([OSSIE], [ossie >= 1.8])

AC_CHECK_PROG([IDLJ], [idlj], [idlj], [AC_MSG_ERROR([cannot find idlj program])])
AC_CHECK_PROG([JAVAC], [javac], [javac], [AC_MSG_ERROR([cannot find Java compiler])])
AC_CHECK_PROG([JAR], [jar], [jar], [AC_MSG_ERROR([cannot find jar program])])

AC_PATH_PROG(JAVAC, javac, [not found], [${JAVA_HOME}/bin])
AC_PATH_PROG(JAVAH, javah, [not found], [${JAVA_HOME}/bin])
AC_PATH_PROG(JAVA, java, [not found], [${JAVA_HOME}/bin])
AC_PATH_PROG(JAR, jar, [not found], [${JAVA_HOME}/bin])
AC_SUBST(JAVAC)
AC_SUBST(JAVAH)
AC_SUBST(JAR)
AC_SUBST(JAVA)

#PKG_CHECK_MODULES([INTERFACEDEPS], [bulkio >= 1.0 bulkioInterfaces >= 1.8])

AC_CONFIG_FILES(Makefile)
AC_OUTPUT
