#ifndef BULKIO_HELPER_FIXTURE_H
#define BULKIO_HELPER_FIXTURE_H

#include <cppunit/extensions/HelperMacros.h>

class Bulkio_Helper_Fixture : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE( Bulkio_Helper_Fixture );
  CPPUNIT_TEST( test_sri_create );
  CPPUNIT_TEST( test_sri_compare );
  CPPUNIT_TEST( test_time_now );
  CPPUNIT_TEST( test_time_create );
  CPPUNIT_TEST( test_time_compare );
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp();
  void tearDown();

  void test_sri_create();
  void test_sri_compare();

  void test_time_now();
  void test_time_create();
  void test_time_compare();
};

#endif  // BULKIO_HELPER_FIXTURE_H
