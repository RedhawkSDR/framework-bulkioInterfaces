TestLargePush.jar: $(TestLargePush_jar_SOURCES)
	mkdir -p bin
	$(JAVAC) -cp $(CLASSPATH_SOFTPKG_DEP)$(OSSIE_HOME)/lib/CFInterfaces.jar:$(OSSIE_HOME)/lib/log4j-1.2.15.jar:$(OSSIE_HOME)/lib/ossie.jar:$(OSSIE_HOME)/lib/BULKIOInterfaces.jar:$(OSSIE_HOME)/lib/bulkio.jar -d bin $(TestLargePush_jar_SOURCES)
	$(JAR) cf ./TestLargePush.jar -C bin .

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

ossieName = TestLargePush
bindir = $(prefix)/dom/components/TestLargePush/java/
bin_PROGRAMS = TestLargePush.jar
TestLargePush_jar_SOURCES := $(shell find ./src -name "*.java")

xmldir = $(prefix)/dom/components/TestLargePush/
dist_xml_DATA = ../TestLargePush.scd.xml ../TestLargePush.prf.xml ../TestLargePush.spd.xml

domdir = $(prefix)/dom/components/TestLargePush/java/
dist_dom_SCRIPTS = startJava.sh
