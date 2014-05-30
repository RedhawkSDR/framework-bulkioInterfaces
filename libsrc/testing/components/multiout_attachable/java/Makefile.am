multiout_attachable_jar_CLASSPATH = $(CLASSPATH_SOFTPKG_DEP)$(OSSIE_HOME)/lib/CFInterfaces.jar:$(OSSIE_HOME)/lib/log4j-1.2.15.jar:$(OSSIE_HOME)/lib/ossie.jar:$(bulkio_libsrc_top)/bulkio.jar:$(bulkio_top)/BULKIOInterfaces.jar

multiout_attachable.jar: $(multiout_attachable_jar_SOURCES)
	mkdir -p bin
	$(JAVAC) -cp $(multiout_attachable_jar_CLASSPATH) -d bin $(multiout_attachable_jar_SOURCES)
	$(JAR) cf ./multiout_attachable.jar -C bin .

clean-local:
	rm -rf bin

distclean-local:
	rm -f config.*
	rm -rf autom4te.cache
	rm -f acinclude.m4
	rm -f aclocal.m4
	rm -f configure
	rm -f depcomp
	rm -f install-sh
	rm -f ltmain.sh
	rm -f Makefile.in
	rm -f missing
	rm -rf .deps

ossieName = multiout_attachable
bindir = $(prefix)/dom/components/multiout_attachable/java/
bin_PROGRAMS = multiout_attachable.jar
multiout_attachable_jar_SOURCES := $(shell find ./src -name "*.java")

##
## This is used to build against local bulkio interface library and not installed version
##
bulkio_top=../../../../..
bulkio_libsrc_top=$(bulkio_top)/libsrc

xmldir = $(prefix)/dom/components/multiout_attachable/
dist_xml_DATA = ../multiout_attachable.scd.xml ../multiout_attachable.prf.xml ../multiout_attachable.spd.xml

domdir = $(prefix)/dom/components/multiout_attachable/java/
dist_dom_SCRIPTS = startJava.sh
