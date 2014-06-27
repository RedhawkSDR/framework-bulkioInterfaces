# Rules for the test code (use `make check` to execute)
TESTS = Bulkio
check_PROGRAMS = $(TESTS)
bulkio_top=../../../../
bulkio_libsrc_top=$(bulkio_top)/libsrc
Bulkio_SOURCES = Bulkio.cpp Bulkio_Helper_Fixture.cpp Bulkio_InPort_Fixture.cpp Bulkio_OutPort_Fixture.cpp Bulkio_MultiOut_Port.cpp
Bulkio_CXXFLAGS = $(CPPUNIT_CFLAGS) -I$(bulkio_libsrc_top)/cpp  -I$(bulkio_top)/src/cpp -I$(bulkio_top)/src/cpp/ossie  $(BOOST_CPPFLAGS) $(RH_DEPS_CFLAGS)
Bulkio_LDADD = -L$(bulkio_libsrc_top)/.libs -L$(bulkio_top)/.libs -lbulkio-1.10 -lbulkioInterfaces $(BOOST_SYSTEM_LIB) $(RH_DEPS_LIBS) $(CPPUNIT_LIBS) -llog4cxx 
