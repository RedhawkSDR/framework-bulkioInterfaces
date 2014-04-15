
Java_Ports.jar: $(Java_Ports_jar_SOURCES)
	@mkdir -p bin
	$(JAVAC) -cp $(OSSIE_HOME)/lib/CFInterfaces.jar:$(OSSIE_HOME)/lib/log4j-1.2.15.jar:$(OSSIE_HOME)/lib/ossie.jar:$(bulkio_libsrc_top)/java/bulkio.jar:$(bulkio_top)/BULKIOInterfaces.jar -d bin $(Java_Ports_jar_SOURCES)
	$(JAR) cf ./Java_Ports.jar -C bin .


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

ossieName = Java_Ports
bindir = $(prefix)/dom/components/Java_Ports/java/
bin_PROGRAMS = Java_Ports.jar
Java_Ports_jar_SOURCES := $(shell find ./src -name "*.java")

##
## This is used to build against local bulkio interface library and not installed version
##
bulkio_top=../../../../..
bulkio_libsrc_top=$(bulkio_top)/libsrc

xmldir = $(prefix)/dom/components/Java_Ports/
dist_xml_DATA = ../Java_Ports.prf.xml ../Java_Ports.scd.xml ../Java_Ports.spd.xml 

domdir = $(prefix)/dom/components/Java_Ports/java/
dist_dom_SCRIPTS = startJava.sh
