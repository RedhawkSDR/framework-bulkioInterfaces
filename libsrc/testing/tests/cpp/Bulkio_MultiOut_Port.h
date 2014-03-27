#ifndef BULKIO_MULTIOUT_PORT_H
#define BULKIO_MULTIOUT_PORT_H

#include <cppunit/extensions/HelperMacros.h>
#include<omniORB4/CORBA.h>
#include<log4cxx/logger.h>
#include<bulkio.h>

template< typename OUT_PORT, typename IN_PORT >
class Bulkio_MultiOut_Port : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE( Bulkio_MultiOut_Port );
  CPPUNIT_TEST( test_multiout_sri_filtered ); 
  CPPUNIT_TEST( test_multiout_sri_eos_filtered );
  CPPUNIT_TEST( test_multiout_data_filtered );
  CPPUNIT_TEST( test_multiout_data_sri_filtered );
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp();
  void tearDown();

  virtual void test_multiout_sri_filtered();
  virtual void test_multiout_sri_eos_filtered();
  virtual void test_multiout_data_filtered();
  virtual void test_multiout_data_sri_filtered();

  template < typename T > void test_port_api( T *port );

  log4cxx::LoggerPtr logger;

  CORBA::ORB_ptr orb;

  std::string  lname;

  IN_PORT *ip1; 
  IN_PORT *ip2;
  IN_PORT *ip3;
  IN_PORT *ip4;
  OUT_PORT *port;
  PortableServer::ObjectId_var ip1_oid;
  PortableServer::ObjectId_var ip2_oid;
  PortableServer::ObjectId_var ip3_oid; 
  PortableServer::ObjectId_var ip4_oid;
  PortableServer::ObjectId_var port_oid;

  std::vector< bulkio::connection_descriptor_struct >  desc_list;


};


typedef Bulkio_MultiOut_Port< bulkio::OutOctetPort, bulkio::InUInt8Port >   MultiOutUInt8;
typedef Bulkio_MultiOut_Port< bulkio::OutInt16Port, bulkio::InInt16Port >   MultiOutInt16;
typedef Bulkio_MultiOut_Port< bulkio::OutUInt16Port, bulkio::InUInt16Port >   MultiOutUInt16;
typedef Bulkio_MultiOut_Port< bulkio::OutInt32Port, bulkio::InInt32Port >   MultiOutInt32;
typedef Bulkio_MultiOut_Port< bulkio::OutUInt32Port, bulkio::InUInt32Port >   MultiOutUInt32;
typedef Bulkio_MultiOut_Port< bulkio::OutInt64Port, bulkio::InInt64Port >   MultiOutInt64;
typedef Bulkio_MultiOut_Port< bulkio::OutUInt64Port, bulkio::InUInt64Port >   MultiOutUInt64;
typedef Bulkio_MultiOut_Port< bulkio::OutDoublePort, bulkio::InDoublePort >   MultiOutDouble;
typedef Bulkio_MultiOut_Port< bulkio::OutFloatPort, bulkio::InFloatPort >   MultiOutFloat;


#define DEF_TEST( IP, OP, NAME ) class MultiOut##IP##_Port : public  Bulkio_MultiOut_Port< bulkio::Out##OP##Port, bulkio::In##IP##Port > \
{ \
  CPPUNIT_TEST_SUITE( MultiOut##IP##_Port ); \
  CPPUNIT_TEST( test_multiout_sri_filtered ); \
  CPPUNIT_TEST( test_multiout_sri_eos_filtered ); \
  CPPUNIT_TEST( test_multiout_data_filtered ); \
  CPPUNIT_TEST( test_multiout_data_sri_filtered ); \
  CPPUNIT_TEST_SUITE_END(); \
public: \
\
 MultiOut##IP##_Port() : MultiOut##IP (){ \
    this->lname=#NAME; \
  }; \
};


DEF_TEST( UInt8, Octet, UINT8 );
DEF_TEST( Int16, Int16, INT16 );
DEF_TEST( UInt16, UInt16, UINT16 );
DEF_TEST( Int32, Int32, INT32 );
DEF_TEST( UInt32, UInt32, UINT32 );
DEF_TEST( Int64, Int64, INT64 );
DEF_TEST( UInt64, UInt64, UINT64 );
DEF_TEST( Double, Double, DOUBLE );
DEF_TEST( Float, Float, FLOAT );

#endif  // BULKIO_OutPort_PORT_H
