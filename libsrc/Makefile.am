###############################################################################
# C++ (via automake and libtool)
pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = bulkio.pc

## Place generated object files (.o) into the same directory as their source
## files, in order to avoid collisions when non-recursive make is used.
AUTOMAKE_OPTIONS = subdir-objects

## Additional flags to pass to aclocal when it is invoked automatically at
## make time. The ${ACLOCAL_FLAGS} variable is picked up from the environment
## to provide a way for the user to supply additional arguments.
ACLOCAL_AMFLAGS = ${ACLOCAL_FLAGS}

lib_LTLIBRARIES = libbulkio-@BULKIO_API_VERSION@.la
libbulkio_@BULKIO_API_VERSION@_la_LDFLAGS = -version-info $(BULKIO_SO_VERSION)

libbulkio_@BULKIO_API_VERSION@_la_SOURCES = \
    cpp/bulkio.cpp \
    cpp/bulkio_in_port.cpp \
    cpp/bulkio_out_port.cpp \
    cpp/bulkio_sdds_port.cpp \
    cpp/bulkio_sri_helpers.cpp \
    cpp/bulkio_time_helpers.cpp \
    cpp/bulkio_p.h

## Define the list of public header files and their install location.
library_includedir = $(includedir)/bulkio
library_include_HEADERS = cpp/bulkio.h \
	cpp/BULKIO_Interfaces.h \
	cpp/bulkio_base.h \
	cpp/bulkio_traits.h \
	cpp/bulkio_in_port.h \
	cpp/bulkio_out_port.h \
	cpp/bulkio_sdds.h \
	cpp/bulkio_compat.h

## The generated configuration header is installed in its own subdirectory of
## $(libdir).  The reason for this is that the configuration information put
## into this header file describes the target platform the installed library
## has been built for.  Thus the file must not be installed into a location
## intended for architecture-independent files, as defined by the Filesystem
## Hierarchy Standard (FHS).
## The nodist_ prefix instructs Automake to not generate rules for including
## the listed files in the distribution on 'make dist'.  Files that are listed
## in _HEADERS variables are normally included in the distribution, but the
## configuration header file is generated at configure time and should not be
## shipped with the source tarball.
#bulkio_libincludedir = $(libdir)/bulkio-$(BULKIO_API_VERSION)/include

libbulkio_@BULKIO_API_VERSION@_la_CXXFLAGS = -Wall -I./cpp -DLOGGING $(BULKIO_INF_INCLUDES) $(BOOST_CPPFLAGS) $(OMNIORB_CFLAGS) $(OSSIE_CFLAGS)

###############################################################################
# Python

all-python: build-python

build-python:
	python setup.py build

install-python:
	test -n "$(DESTDIR)" && buildroot="--root=$(DESTDIR)"; \
	  python setup.py install $$buildroot --prefix=$(prefix) --install-purelib=$(prefix)/lib/python --force

clean-python:
	python setup.py clean --all

###############################################################################
# Java

if HAVE_JAVASUPPORT

JAVA_DIR := java
JAVA_SRCDIR := $(JAVA_DIR)/src
JAVA_BINDIR := $(JAVA_DIR)/bin
CLASSPATH := $(OSSIE_HOME)/lib/CFInterfaces.jar:$(OSSIE_HOME)/lib/ossie.jar:$(OSSIE_HOME)/lib/log4j-1.2.15.jar:$(top_builddir)/BULKIOInterfaces.jar
BINARY_JAR := $(JAVA_DIR)/bulkio.jar
SOURCE_JAR := $(JAVA_DIR)/bulkio.src.jar

JAVA_SRCS := Const.java \
DataTransfer.java \
InCharPort.java \
InDoublePort.java \
InFilePort.java \
InFloatPort.java \
InInt16Port.java \
InInt32Port.java \
InInt64Port.java \
InInt8Port.java \
InLongLongPort.java \
InLongPort.java \
InOctetPort.java \
InSDDSPort.java \
InShortPort.java \
InUInt16Port.java \
InUInt32Port.java \
InUInt64Port.java \
InUInt8Port.java \
InULongLongPort.java \
InULongPort.java \
InUShortPort.java \
InXMLPort.java \
linkStatistics.java \
OutCharPort.java \
OutDoublePort.java \
OutFilePort.java \
OutFloatPort.java \
OutInt16Port.java \
OutInt32Port.java \
OutInt64Port.java \
OutInt8Port.java \
OutLongLongPort.java \
OutLongPort.java \
OutOctetPort.java \
OutSDDSPort.java \
OutShortPort.java \
OutUInt16Port.java \
OutUInt32Port.java \
OutUInt64Port.java \
OutUInt8Port.java \
OutULongLongPort.java \
OutULongPort.java \
OutUShortPort.java \
OutXMLPort.java \
queueSemaphore.java \
SizeOf.java \
SriListener.java \
ConnectionEventListener.java \
sriState.java \
sri/Comparator.java \
sri/DefaultComparator.java \
time/Comparator.java \
time/DefaultComparator.java \
time/utils.java
JAVA_SRCS_ABS := $(addprefix $(JAVA_SRCDIR)/bulkio/, $(JAVA_SRCS))

javalibdir = $(prefix)/lib
javalib_DATA = $(BINARY_JAR) $(SOURCE_JAR)

# An intermediate file (used to get from *.java to a .jar). Intermediate
# indicates it's not important in and of itself - only necessary to get to the
# .jar
.INTERMEDIATE: $(JAVA_DIR)/java.files
$(JAVA_DIR)/java.files: $(JAVA_SRCS_ABS)
	echo $(JAVA_SRCS_ABS) > $@

# The jar depends on the file list, which depends on the .java files
$(BINARY_JAR): $(JAVA_DIR)/java.files $(JAVA_DIR)/META-INF/MANIFEST.MF $(top_builddir)/BULKIOInterfaces.jar
	mkdir -p $(JAVA_BINDIR)
	$(JAVAC) -cp $(CLASSPATH) -target 1.6 -source 1.6 -d $(JAVA_BINDIR) @$(JAVA_DIR)/java.files
	$(JAR) cmf $(JAVA_DIR)/META-INF/MANIFEST.MF $(BINARY_JAR) -C $(JAVA_BINDIR) .

$(SOURCE_JAR): $(JAVA_DIR)/java.files $(JAVA_DIR)/META-INF/MANIFEST.MF.src
	$(JAR) cmf $(JAVA_DIR)/META-INF/MANIFEST.MF.src $(SOURCE_JAR) -C $(JAVA_SRCDIR) .

clean-java:
	rm -rf $(JAVA_BINDIR) $(BINARY_JAR)

else

clean-java:
	@echo "Java support is disabled"

endif

###############################################################################
# General
#

.PHONY: all-local all-python build-python install-python clean-python all-java install-java clean-java reallyclean reallyclean-java

all-local: all-python

install-exec-hook: install-python

clean-local: clean-python clean-java

reallyclean: distclean
	rm -f Makefile.in
