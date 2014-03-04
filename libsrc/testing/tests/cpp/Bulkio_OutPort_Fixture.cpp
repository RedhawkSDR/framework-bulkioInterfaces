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


class MyOutFloatPort : public bulkio::OutFloatPort {

public:

  MyOutFloatPort( std::string pname, bulkio::LOGGER_PTR logger ) :
    bulkio::OutFloatPort( pname, logger ) {};


  void pushPacket( bulkio::OutFloatPort::NativeSequenceType & data, BULKIO::PrecisionUTCTime& T, bool EOS, const std::string& streamID) {

    stats[streamID].update( 1, 1.0, false, "testing" );
    bulkio::OutFloatPort::pushPacket( data, T, EOS, streamID );
  }

};

// Global connection/disconnection callbacks
static void port_connected( const char* connectionId ) {

}

static void port_disconnected( const char* connectionId ) {

}


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

  LOG4CXX_INFO(logger, "Running tests port:" << port->getName() );

  ExtendedCF::UsesConnectionSequence *clist = port->connections();
  CPPUNIT_ASSERT( clist != NULL );
  delete clist;

  port->setNewConnectListener(&port_connected);
  port->setNewDisconnectListener(&port_disconnected);

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

  port->setLogger(logger);
}


template< >
void  Bulkio_OutPort_Fixture::test_port_api( bulkio::OutCharPort *port  ) {

  ExtendedCF::UsesConnectionSequence *clist = port->connections();
  CPPUNIT_ASSERT( clist != NULL );
  delete clist;

  port->setNewConnectListener(&port_connected);
  port->setNewDisconnectListener(&port_disconnected);

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

    port->setLogger(logger);
}


template< >
void  Bulkio_OutPort_Fixture::test_port_api( bulkio::OutFilePort *port  ) {

  ExtendedCF::UsesConnectionSequence *clist = port->connections();
  CPPUNIT_ASSERT( clist != NULL );
  delete clist;

  port->setNewConnectListener(&port_connected);
  port->setNewDisconnectListener(&port_disconnected);

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

  port->setLogger(logger);
}



template< >
void  Bulkio_OutPort_Fixture::test_port_api( bulkio::OutXMLPort *port  ) {

  ExtendedCF::UsesConnectionSequence *clist = port->connections();
  CPPUNIT_ASSERT( clist != NULL );
  delete clist;

  port->setNewConnectListener(&port_connected);
  port->setNewDisconnectListener(&port_disconnected);

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

  port->setLogger(logger);
}


template< >
void  Bulkio_OutPort_Fixture::test_port_api( bulkio::OutSDDSPort *port  ) {

  ExtendedCF::UsesConnectionSequence *clist = port->connections();
  CPPUNIT_ASSERT( clist != NULL );
  delete clist;

  port->setNewConnectListener(&port_connected);
  port->setNewDisconnectListener(&port_disconnected);

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

  port->setLogger(logger);
}





void
Bulkio_OutPort_Fixture::test_create_int8()
{
  bulkio::OutCharPort *port = new bulkio::OutCharPort("test_int8", logger );
  CPPUNIT_ASSERT( port != NULL );
}

void
Bulkio_OutPort_Fixture::test_int8()
{
  bulkio::OutCharPort *port = new bulkio::OutCharPort("test_api_int8", logger );
  CPPUNIT_ASSERT( port != NULL );

  test_port_api( port );

  CPPUNIT_ASSERT_NO_THROW( port );
}


void
Bulkio_OutPort_Fixture::test_create_int16()
{
  bulkio::OutInt16Port *port = new bulkio::OutInt16Port("test_ctor_int16", logger );
  CPPUNIT_ASSERT( port != NULL );
}


void
Bulkio_OutPort_Fixture::test_int16()
{
  bulkio::OutInt16Port *port = new bulkio::OutInt16Port("test_api_int16", logger );
  CPPUNIT_ASSERT( port != NULL );

  test_port_api( port );

  CPPUNIT_ASSERT_NO_THROW( port );
}


void
Bulkio_OutPort_Fixture::test_create_int32()
{
  bulkio::OutInt32Port *port = new bulkio::OutInt32Port("test_ctor_int32", logger );
  CPPUNIT_ASSERT( port != NULL );
}

void
Bulkio_OutPort_Fixture::test_int32()
{
  bulkio::OutInt32Port *port = new bulkio::OutInt32Port("test_api_int32", logger );
  CPPUNIT_ASSERT( port != NULL );

  test_port_api( port );

  CPPUNIT_ASSERT_NO_THROW( port );
}



void
Bulkio_OutPort_Fixture::test_create_int64()
{
  bulkio::OutInt64Port *port = new bulkio::OutInt64Port("test_ctor_int64", logger );
  CPPUNIT_ASSERT( port != NULL );
}

void
Bulkio_OutPort_Fixture::test_int64()
{
  bulkio::OutInt64Port *port = new bulkio::OutInt64Port("test_api_int64", logger );
  CPPUNIT_ASSERT( port != NULL );

  test_port_api( port );

  CPPUNIT_ASSERT_NO_THROW( port );
}



void
Bulkio_OutPort_Fixture::test_create_uint8()
{
  bulkio::OutOctetPort *port = new bulkio::OutOctetPort("test_ctor_uint8", logger );
  CPPUNIT_ASSERT( port != NULL );
}

void
Bulkio_OutPort_Fixture::test_create_uint16()
{
  bulkio::OutUInt16Port *port = new bulkio::OutUInt16Port("test_ctor_uint16", logger );
  CPPUNIT_ASSERT( port != NULL );
}

void
Bulkio_OutPort_Fixture::test_uint16()
{
  bulkio::OutUInt16Port *port = new bulkio::OutUInt16Port("test_api_uint16", logger );
  CPPUNIT_ASSERT( port != NULL );

  test_port_api( port );

  CPPUNIT_ASSERT_NO_THROW( port );
}


void
Bulkio_OutPort_Fixture::test_create_uint32()
{
  bulkio::OutUInt32Port *port = new bulkio::OutUInt32Port("test_ctor_uint32", logger );
  CPPUNIT_ASSERT( port != NULL );
}

void
Bulkio_OutPort_Fixture::test_uint32()
{
  bulkio::OutUInt32Port *port = new bulkio::OutUInt32Port("test_api_uint32", logger );
  CPPUNIT_ASSERT( port != NULL );

  test_port_api( port );

  CPPUNIT_ASSERT_NO_THROW( port );
}


void
Bulkio_OutPort_Fixture::test_create_uint64()
{
  bulkio::OutUInt64Port *port = new bulkio::OutUInt64Port("test_ctor_uint64", logger );
  CPPUNIT_ASSERT( port != NULL );
}

void
Bulkio_OutPort_Fixture::test_uint64()
{
  bulkio::OutUInt64Port *port = new bulkio::OutUInt64Port("test_api_uint64", logger );
  CPPUNIT_ASSERT( port != NULL );

  test_port_api( port );

  CPPUNIT_ASSERT_NO_THROW( port );
}


void
Bulkio_OutPort_Fixture::test_create_float()
{
  bulkio::OutFloatPort *port = new bulkio::OutFloatPort("test_ctor_float", logger );
  CPPUNIT_ASSERT( port != NULL );
}

void
Bulkio_OutPort_Fixture::test_create_double()
{
  bulkio::OutDoublePort *port = new bulkio::OutDoublePort("test_ctor_double", logger );
  CPPUNIT_ASSERT( port != NULL );
}


void
Bulkio_OutPort_Fixture::test_create_file()
{
  bulkio::OutFilePort *port = new bulkio::OutFilePort("test_ctor_file", logger );
  CPPUNIT_ASSERT( port != NULL );
}

void
Bulkio_OutPort_Fixture::test_file()
{
  bulkio::OutFilePort *port = new bulkio::OutFilePort("test_api_file", logger );
  CPPUNIT_ASSERT( port != NULL );

  test_port_api( port );

  CPPUNIT_ASSERT_NO_THROW( port );
}


void
Bulkio_OutPort_Fixture::test_create_xml()
{
  bulkio::OutXMLPort *port = new bulkio::OutXMLPort("test_ctor_xml", logger );
  CPPUNIT_ASSERT( port != NULL );
}




void
Bulkio_OutPort_Fixture::test_xml()
{
  bulkio::OutXMLPort *port = new bulkio::OutXMLPort("test_api_xml", logger );
  CPPUNIT_ASSERT( port != NULL );

  test_port_api( port );

  CPPUNIT_ASSERT_NO_THROW( port );
}


void
Bulkio_OutPort_Fixture::test_create_sdds()
{
  bulkio::OutSDDSPort *port = new bulkio::OutSDDSPort("test_ctor_sdds", logger);
  CPPUNIT_ASSERT( port != NULL );
}




void
Bulkio_OutPort_Fixture::test_sdds()
{
  bulkio::OutSDDSPort *port = new bulkio::OutSDDSPort("test_api_sdds", logger );
  CPPUNIT_ASSERT( port != NULL );

  test_port_api( port );

  CPPUNIT_ASSERT_NO_THROW( port );
}



void
Bulkio_OutPort_Fixture::test_subclass()
{
  bulkio::OutFloatPort *port = new MyOutFloatPort("test_api_subclass", logger );
  CPPUNIT_ASSERT( port != NULL );

  test_port_api( port );

  CPPUNIT_ASSERT_NO_THROW( port );
}

