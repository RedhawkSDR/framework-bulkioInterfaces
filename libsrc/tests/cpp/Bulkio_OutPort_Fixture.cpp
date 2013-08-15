#include "Bulkio_OutPort_Fixture.h"
#include "bulkio.h"
#include<log4cxx/logger.h>
#include<log4cxx/propertyconfigurator.h>
#include <log4cxx/logstring.h>
#include <log4cxx/patternlayout.h>
#include <log4cxx/consoleappender.h>
#include <log4cxx/logmanager.h>


// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( Bulkio_OutPort_Fixture );


void 
Bulkio_OutPort_Fixture::setUp()
{
   logger = log4cxx::Logger::getLogger("BulkioOutPort");
   logger->setLevel( log4cxx::Level::getTrace());
}


void 
Bulkio_OutPort_Fixture::tearDown()
{
}

template< typename T>
void  Bulkio_OutPort_Fixture::test_port_api( T *port  ) {

  ExtendedCF::UsesConnectionSequence *clist = port->connections();
  CPPUNIT_ASSERT( clist != NULL );
  delete clist;

  CORBA::Object_ptr  p;
  port->connectPort( p, "connection_1");

  port->disconnectPort( "connection_1");

  BULKIO::StreamSRI sri;
  port->pushSRI( sri );

  typename T::NativeSequenceType v;
  BULKIO::PrecisionUTCTime TS;
  port->pushPacket( v, TS, false, "test_port_api" );

  port->pushPacket( v, TS, true, "test_port_api" );

  port->pushPacket( v, TS, true, "unknown_stream_id" );

  BULKIO::UsesPortStatisticsSequence *stats = port->statistics();
  CPPUNIT_ASSERT( stats != NULL );
  delete stats;

  BULKIO::PortUsageType rt = port->state();
  CPPUNIT_ASSERT( rt == BULKIO::IDLE );


 typename T::ConnectionsList cl =  port->_getConnections();
  std::string sid="none";
  int cnt= port->currentSRIs.count(sid);
  CPPUNIT_ASSERT( cnt == 0 );

  port->enableStats( false );

}


template< >
void  Bulkio_OutPort_Fixture::test_port_api( bulkio::OutCharPort *port  ) {

  ExtendedCF::UsesConnectionSequence *clist = port->connections();
  CPPUNIT_ASSERT( clist != NULL );
  delete clist;

  CORBA::Object_ptr  p;
  port->connectPort( p, "connection_1");

  port->disconnectPort( "connection_1");

  BULKIO::StreamSRI sri;
  port->pushSRI( sri );

  std::vector< bulkio::OutCharPort::NativeType > v;
  BULKIO::PrecisionUTCTime TS;
  port->pushPacket( v, TS, false, "test_port_api" );


  std::vector< bulkio::Char > v1;
  port->pushPacket( v1, TS, false, "test_port_api" );

  BULKIO::UsesPortStatisticsSequence *stats = port->statistics();
  CPPUNIT_ASSERT( stats != NULL );
  delete stats;

    BULKIO::PortUsageType rt = port->state();
    CPPUNIT_ASSERT( rt == BULKIO::IDLE );

    port->enableStats( false );

}


template< >
void  Bulkio_OutPort_Fixture::test_port_api( bulkio::OutFilePort *port  ) {

  ExtendedCF::UsesConnectionSequence *clist = port->connections();
  CPPUNIT_ASSERT( clist != NULL );
  delete clist;

  CORBA::Object_ptr  p;
  port->connectPort( p, "connection_1");

  port->disconnectPort( "connection_1");

  BULKIO::StreamSRI sri;
  port->pushSRI( sri );

  bulkio::OutFilePort::NativeSequenceType v;
  BULKIO::PrecisionUTCTime TS;
  port->pushPacket( v, TS, false, "test_port_api" );

  port->pushPacket( v, TS, true, "test_port_api" );

  BULKIO::UsesPortStatisticsSequence *stats = port->statistics();
  CPPUNIT_ASSERT( stats != NULL );
  delete stats;

  BULKIO::PortUsageType rt = port->state();
  CPPUNIT_ASSERT( rt == BULKIO::IDLE );

  port->enableStats( false );

}



template< >
void  Bulkio_OutPort_Fixture::test_port_api( bulkio::OutXMLPort *port  ) {

  ExtendedCF::UsesConnectionSequence *clist = port->connections();
  CPPUNIT_ASSERT( clist != NULL );
  delete clist;

  CORBA::Object_ptr  p;
  port->connectPort( p, "connection_1");

  port->disconnectPort( "connection_1");

  BULKIO::StreamSRI sri;
  port->pushSRI( sri );

  bulkio::OutXMLPort::NativeSequenceType v;
  BULKIO::PrecisionUTCTime TS;
  port->pushPacket( v, false, "test_port_api" );

  port->pushPacket( v, TS, true, "test_port_api" );

  BULKIO::UsesPortStatisticsSequence *stats = port->statistics();
  CPPUNIT_ASSERT( stats != NULL );
  delete stats;

  BULKIO::PortUsageType rt = port->state();
  CPPUNIT_ASSERT( rt == BULKIO::IDLE );

  port->enableStats( false );

}


template< >
void  Bulkio_OutPort_Fixture::test_port_api( bulkio::OutSDDSPort *port  ) {

  ExtendedCF::UsesConnectionSequence *clist = port->connections();
  CPPUNIT_ASSERT( clist != NULL );
  delete clist;

  CORBA::Object_ptr  p;
  port->connectPort( p, "connection_1");

  port->disconnectPort( "connection_1");

  BULKIO::StreamSRI sri;
  BULKIO::PrecisionUTCTime TS;
  port->pushSRI( sri, TS );

  BULKIO::UsesPortStatisticsSequence *stats = port->statistics();
  CPPUNIT_ASSERT( stats != NULL );
  delete stats;

  BULKIO::PortUsageType rt = port->state();
  CPPUNIT_ASSERT( rt == BULKIO::IDLE );

  port->enableStats( false );

  // create a connection
  port->connectPort( p, "connection_1");
  port->enableStats( true );
  port->setBitSize(10);
  port->updateStats( 12, 1, false, "stream1");

  stats = port->statistics();
  CPPUNIT_ASSERT( stats != NULL );
  int slen =   stats->length();
  std::cout << " slen :" << slen << std::endl;
  CPPUNIT_ASSERT( slen == 1 ) ;
  CPPUNIT_ASSERT( strcmp((*stats)[0].connectionId, "connection_1") == 0 );
  delete stats;


}





void 
Bulkio_OutPort_Fixture::test_create_int8()
{
  bulkio::OutCharPort *port = new bulkio::OutCharPort("test", logger );
  CPPUNIT_ASSERT( port != NULL );
}

void 
Bulkio_OutPort_Fixture::test_int8()
{
  bulkio::OutCharPort *port = new bulkio::OutCharPort("test", logger );
  CPPUNIT_ASSERT( port != NULL );

  test_port_api( port );

  CPPUNIT_ASSERT_NO_THROW( port );
}


void 
Bulkio_OutPort_Fixture::test_create_int16()
{
  bulkio::OutInt16Port *port = new bulkio::OutInt16Port("test", logger );
  CPPUNIT_ASSERT( port != NULL );
}


void 
Bulkio_OutPort_Fixture::test_int16()
{
  bulkio::OutInt16Port *port = new bulkio::OutInt16Port("test", logger );
  CPPUNIT_ASSERT( port != NULL );

  test_port_api( port );

  CPPUNIT_ASSERT_NO_THROW( port );
}


void 
Bulkio_OutPort_Fixture::test_create_int32()
{
  bulkio::OutInt32Port *port = new bulkio::OutInt32Port("test", logger );
  CPPUNIT_ASSERT( port != NULL );
}

void 
Bulkio_OutPort_Fixture::test_int32()
{
  bulkio::OutInt32Port *port = new bulkio::OutInt32Port("test", logger );
  CPPUNIT_ASSERT( port != NULL );

  test_port_api( port );

  CPPUNIT_ASSERT_NO_THROW( port );
}



void 
Bulkio_OutPort_Fixture::test_create_int64()
{
  bulkio::OutInt64Port *port = new bulkio::OutInt64Port("test", logger );
  CPPUNIT_ASSERT( port != NULL );
}

void 
Bulkio_OutPort_Fixture::test_int64()
{
  bulkio::OutInt64Port *port = new bulkio::OutInt64Port("test", logger );
  CPPUNIT_ASSERT( port != NULL );

  test_port_api( port );

  CPPUNIT_ASSERT_NO_THROW( port );
}



void 
Bulkio_OutPort_Fixture::test_create_uint8()
{
  bulkio::OutOctetPort *port = new bulkio::OutOctetPort("test", logger );
  CPPUNIT_ASSERT( port != NULL );
}

void 
Bulkio_OutPort_Fixture::test_create_uint16()
{
  bulkio::OutUInt16Port *port = new bulkio::OutUInt16Port("test", logger );
  CPPUNIT_ASSERT( port != NULL );
}

void 
Bulkio_OutPort_Fixture::test_uint16()
{
  bulkio::OutUInt16Port *port = new bulkio::OutUInt16Port("test", logger );
  CPPUNIT_ASSERT( port != NULL );

  test_port_api( port );

  CPPUNIT_ASSERT_NO_THROW( port );
}


void 
Bulkio_OutPort_Fixture::test_create_uint32()
{
  bulkio::OutUInt32Port *port = new bulkio::OutUInt32Port("test", logger );
  CPPUNIT_ASSERT( port != NULL );
}

void 
Bulkio_OutPort_Fixture::test_uint32()
{
  bulkio::OutUInt32Port *port = new bulkio::OutUInt32Port("test", logger );
  CPPUNIT_ASSERT( port != NULL );

  test_port_api( port );

  CPPUNIT_ASSERT_NO_THROW( port );
}


void 
Bulkio_OutPort_Fixture::test_create_uint64()
{
  bulkio::OutUInt64Port *port = new bulkio::OutUInt64Port("test", logger );
  CPPUNIT_ASSERT( port != NULL );
}

void 
Bulkio_OutPort_Fixture::test_uint64()
{
  bulkio::OutUInt64Port *port = new bulkio::OutUInt64Port("test", logger );
  CPPUNIT_ASSERT( port != NULL );

  test_port_api( port );

  CPPUNIT_ASSERT_NO_THROW( port );
}


void 
Bulkio_OutPort_Fixture::test_create_float()
{
  bulkio::OutFloatPort *port = new bulkio::OutFloatPort("test", logger );
  CPPUNIT_ASSERT( port != NULL );
}

void 
Bulkio_OutPort_Fixture::test_create_double()
{
  bulkio::OutDoublePort *port = new bulkio::OutDoublePort("test", logger );
  CPPUNIT_ASSERT( port != NULL );
}


void 
Bulkio_OutPort_Fixture::test_create_file()
{
  bulkio::OutFilePort *port = new bulkio::OutFilePort("test", logger );
  CPPUNIT_ASSERT( port != NULL );
}

void 
Bulkio_OutPort_Fixture::test_file()
{
  bulkio::OutFilePort *port = new bulkio::OutFilePort("test", logger );
  CPPUNIT_ASSERT( port != NULL );

  test_port_api( port );

  CPPUNIT_ASSERT_NO_THROW( port );
}


void 
Bulkio_OutPort_Fixture::test_create_xml()
{
  bulkio::OutXMLPort *port = new bulkio::OutXMLPort("test", logger );
  CPPUNIT_ASSERT( port != NULL );
}




void 
Bulkio_OutPort_Fixture::test_xml()
{
  bulkio::OutXMLPort *port = new bulkio::OutXMLPort("test", logger );
  CPPUNIT_ASSERT( port != NULL );

  test_port_api( port );

  CPPUNIT_ASSERT_NO_THROW( port );
}


void 
Bulkio_OutPort_Fixture::test_create_sdds()
{
  bulkio::OutSDDSPort *port = new bulkio::OutSDDSPort("test", logger);
  CPPUNIT_ASSERT( port != NULL );
}




void 
Bulkio_OutPort_Fixture::test_sdds()
{
  bulkio::OutSDDSPort *port = new bulkio::OutSDDSPort("test", logger );
  CPPUNIT_ASSERT( port != NULL );

  test_port_api( port );

  CPPUNIT_ASSERT_NO_THROW( port );
}
