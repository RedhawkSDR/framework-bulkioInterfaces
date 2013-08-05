# Rules for the test code (use `make check` to execute)
TESTS = Bulkio
check_PROGRAMS = $(TESTS)
bulkio_top=../../../
bulkio_libsrc_top=$(bulkio_top)/libsrc
Bulkio_SOURCES = Bulkio.cpp Bulkio_Helper_Fixture.cpp Bulkio_InPort_Fixture.cpp Bulkio_OutPort_Fixture.cpp
Bulkio_CXXFLAGS = $(CPPUNIT_CFLAGS) -I$(bulkio_libsrc_top)/cpp  -I$(bulkio_top)/src/cpp -I$(bulkio_top)/src/cpp/ossie  $(BOOST_CPPFLAGS) $(RH_DEPS_CFLAGS)
Bulkio_LDADD = $(RH_DEPS_LIBS)
Bulkio_LDFLAGS = $(CPPUNIT_LIBS) -L$(bulkio_libsrc_top)/.libs -L$(bulkio_top)/.libs 
Bulkio_LDFLAGS += -ldl  -lbulkio-1.0 -lbulkioInterfaces
