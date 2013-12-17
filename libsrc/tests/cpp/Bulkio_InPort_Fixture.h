#ifndef BULKIO_INPORT_FIXTURE_H
#define BULKIO_INPORT_FIXTURE_H

#include <cppunit/extensions/HelperMacros.h>
#include<log4cxx/logger.h>

class Bulkio_InPort_Fixture : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE( Bulkio_InPort_Fixture );
  CPPUNIT_TEST( test_create_int8 );
  CPPUNIT_TEST( test_int8 );
  CPPUNIT_TEST( test_create_int16 );
  CPPUNIT_TEST( test_int16 );
  CPPUNIT_TEST( test_create_int32);
  CPPUNIT_TEST( test_int32 );
  CPPUNIT_TEST( test_create_int64);
  CPPUNIT_TEST( test_int64 );
  CPPUNIT_TEST( test_create_uint8 );
  CPPUNIT_TEST( test_uint8 );
  CPPUNIT_TEST( test_create_uint16 );
  CPPUNIT_TEST( test_uint16 );
  CPPUNIT_TEST( test_create_uint32);
  CPPUNIT_TEST( test_uint32 );
  CPPUNIT_TEST( test_create_uint64);
  CPPUNIT_TEST( test_uint64 );
  CPPUNIT_TEST( test_create_float );
  CPPUNIT_TEST( test_create_double );
  CPPUNIT_TEST( test_create_file );
  CPPUNIT_TEST( test_file );
  CPPUNIT_TEST( test_create_xml );
  CPPUNIT_TEST( test_xml );
  CPPUNIT_TEST( test_create_sdds );
  CPPUNIT_TEST( test_sdds );
  CPPUNIT_TEST( test_subclass );
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp();
  void tearDown();

  void test_create_int8();
  void test_int8();
  void test_create_int16();
  void test_int16();
  void test_create_int32();
  void test_int32();
  void test_create_int64();
  void test_int64();
  void test_create_uint8();
  void test_uint8();
  void test_create_uint16();
  void test_uint16();
  void test_create_uint32();
  void test_uint32();
  void test_create_uint64();
  void test_uint64();
  void test_create_float();
  void test_create_double();
  void test_create_file();
  void test_file();
  void test_create_xml();
  void test_xml();
  void test_create_sdds();
  void test_sdds();
  void test_subclass();

  template < typename T > void test_port_api( T *port );

  log4cxx::LoggerPtr logger;
};

#endif  // BULKIO_InPort_FIXTURE_H
