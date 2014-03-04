#ifndef BULKIO_MULTIOUT_FIXTURE_H
#define BULKIO_MULTIOUT_FIXTURE_H

#include <cppunit/extensions/HelperMacros.h>
#include<omniORB4/CORBA.h>
#include<log4cxx/logger.h>
#include<bulkio.h>
class Bulkio_MultiOut_Fixture : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE( Bulkio_MultiOut_Fixture );
  CPPUNIT_TEST( test_multiout_sri_filtered );
  CPPUNIT_TEST( test_multiout_sri_eos_filtered );
  CPPUNIT_TEST( test_multiout_data_filtered );
  CPPUNIT_TEST( test_multiout_data_sri_filtered );
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp();
  void tearDown();

  void test_multiout_sri_filtered();
  void test_multiout_sri_eos_filtered();
  void test_multiout_data_filtered();
  void test_multiout_data_sri_filtered();


  template < typename T > void test_port_api( T *port );

  log4cxx::LoggerPtr logger;

  CORBA::ORB_ptr orb;

  bulkio::InFloatPort *ip1;
  bulkio::InFloatPort *ip2;
  bulkio::InFloatPort *ip3;
  bulkio::InFloatPort *ip4;
  bulkio::OutFloatPort *port;
  PortableServer::ObjectId_var ip1_oid;
  PortableServer::ObjectId_var ip2_oid;
  PortableServer::ObjectId_var ip3_oid;
  PortableServer::ObjectId_var ip4_oid;
  PortableServer::ObjectId_var port_oid;

  std::vector< bulkio::connection_descriptor_struct >  desc_list;


};

#endif  // BULKIO_OutPort_FIXTURE_H
