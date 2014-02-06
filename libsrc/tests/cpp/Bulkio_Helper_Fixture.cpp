#include "Bulkio_Helper_Fixture.h"
#include "bulkio.h"

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( Bulkio_Helper_Fixture );


void 
Bulkio_Helper_Fixture::setUp()
{
}


void 
Bulkio_Helper_Fixture::tearDown()
{
}


void 
Bulkio_Helper_Fixture::test_sri_create()
{
  BULKIO::StreamSRI sri = bulkio::sri::create();
}


void 
Bulkio_Helper_Fixture::test_sri_compare()
{
  BULKIO::StreamSRI A = bulkio::sri::create();
  BULKIO::StreamSRI B = bulkio::sri::create();
  BULKIO::StreamSRI C = bulkio::sri::create();

  C.streamID = std::string("No Match").c_str();

  CPPUNIT_ASSERT( bulkio::sri::DefaultComparator(A , B) == true );
  CPPUNIT_ASSERT( bulkio::sri::DefaultComparator(A , C) == false );

}

void 
Bulkio_Helper_Fixture::test_time_now()
{
  BULKIO::PrecisionUTCTime  T = bulkio::time::utils::now();
}

void 
Bulkio_Helper_Fixture::test_time_create()
{
  const double wsec = 100.0;
  const double fsec = 0.125;
  BULKIO::PrecisionUTCTime  T = bulkio::time::utils::create(100.0, 0.125);
  
  CPPUNIT_ASSERT( T.twsec == wsec );
  CPPUNIT_ASSERT( T.tfsec == fsec );
}

void 
Bulkio_Helper_Fixture::test_time_compare()
{
  BULKIO::PrecisionUTCTime  A = bulkio::time::utils::create();
  BULKIO::PrecisionUTCTime  B = A;
  // Wait a brief time to ensure that "now" has changed
  usleep(100);
  BULKIO::PrecisionUTCTime  C = bulkio::time::utils::create();

  CPPUNIT_ASSERT( bulkio::time::DefaultComparator(A , B) == true );
  CPPUNIT_ASSERT( bulkio::time::DefaultComparator(A , C) == false );

}


