#include "Bulkio_InPort_Fixture.h"
#include "bulkio.h"

#include<log4cxx/logger.h>
#include<log4cxx/propertyconfigurator.h>
#include <log4cxx/logstring.h>
#include <log4cxx/patternlayout.h>
#include <log4cxx/consoleappender.h>
#include <log4cxx/logmanager.h>

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( Bulkio_InPort_Fixture );


void 
Bulkio_InPort_Fixture::setUp()
{
   logger = log4cxx::Logger::getLogger("BulkioInPort");
   logger->setLevel( log4cxx::Level::getTrace());
}


void 
Bulkio_InPort_Fixture::tearDown()
{
}

template< typename T>
void  Bulkio_InPort_Fixture::test_port_api( T *port  ) {

  BULKIO::PortStatistics *stats = port->statistics();
  CPPUNIT_ASSERT( stats != NULL );
  delete stats;

  BULKIO::PortUsageType rt = port->state();
  CPPUNIT_ASSERT( rt == BULKIO::IDLE );

  BULKIO::StreamSRISequence  *streams = port->activeSRIs();
  CPPUNIT_ASSERT( streams != NULL );
  delete streams;

  int tmp = port->getMaxQueueDepth();
  CPPUNIT_ASSERT( tmp == 100 );

  tmp = port->getCurrentQueueDepth();
  CPPUNIT_ASSERT( tmp == 0 );

  port->setMaxQueueDepth(22);
  tmp = port->getMaxQueueDepth();
  CPPUNIT_ASSERT( tmp == 22 );

  // check that port queue is empty
  typename T::dataTransfer *pkt  = port->getPacket(bulkio::Const::NON_BLOCKING );
  CPPUNIT_ASSERT( pkt == NULL );

  BULKIO::StreamSRI sri;
  port->pushSRI( sri );

  streams = port->activeSRIs();
  CPPUNIT_ASSERT( streams != NULL );
  CPPUNIT_ASSERT( streams->length() == 1 );
  delete streams;

  typename T::PortSequenceType v;
  BULKIO::PrecisionUTCTime TS;
  port->pushPacket( v, TS, false, "test_port_api" );

  // grab off packet
  pkt  = port->getPacket(bulkio::Const::NON_BLOCKING );
  CPPUNIT_ASSERT( pkt != NULL );
  delete pkt;

  port->enableStats( false );

  port->block();

  port->unblock();


}

template<>
void  Bulkio_InPort_Fixture::test_port_api( bulkio::InFilePort *port  ) {

  BULKIO::PortStatistics *stats = port->statistics();
  CPPUNIT_ASSERT( stats != NULL );
  delete stats;

  BULKIO::PortUsageType rt = port->state();
  CPPUNIT_ASSERT( rt == BULKIO::IDLE );

  BULKIO::StreamSRISequence  *streams = port->activeSRIs();
  CPPUNIT_ASSERT( streams != NULL );
  delete streams;

  int tmp = port->getMaxQueueDepth();
  CPPUNIT_ASSERT( tmp == 100 );

  tmp = port->getCurrentQueueDepth();
  CPPUNIT_ASSERT( tmp == 0 );

  port->setMaxQueueDepth(22);
  tmp = port->getMaxQueueDepth();
  CPPUNIT_ASSERT( tmp == 22 );

  // check that port queue is empty
  bulkio::InFilePort::dataTransfer *pkt  = port->getPacket(bulkio::Const::NON_BLOCKING );
  CPPUNIT_ASSERT( pkt == NULL );

  BULKIO::StreamSRI sri;
  port->pushSRI( sri );

  streams = port->activeSRIs();
  CPPUNIT_ASSERT( streams != NULL );
  CPPUNIT_ASSERT( streams->length() == 1 );
  delete streams;

  bulkio::InFilePort::PortSequenceType v = new bulkio::InFilePort::TransportType[1];
  BULKIO::PrecisionUTCTime TS;
  port->pushPacket( v, TS, false, "test_port_api" );

  // grab off packet
  pkt  = port->getPacket(bulkio::Const::NON_BLOCKING );
  CPPUNIT_ASSERT( pkt != NULL );
  delete pkt;

  port->enableStats( false );

  port->block();

  port->unblock();


}


template<>
void  Bulkio_InPort_Fixture::test_port_api( bulkio::InXMLPort *port  ) {

  BULKIO::PortStatistics *stats = port->statistics();
  CPPUNIT_ASSERT( stats != NULL );
  delete stats;

  BULKIO::PortUsageType rt = port->state();
  CPPUNIT_ASSERT( rt == BULKIO::IDLE );

  BULKIO::StreamSRISequence  *streams = port->activeSRIs();
  CPPUNIT_ASSERT( streams != NULL );
  delete streams;

  int tmp = port->getMaxQueueDepth();
  CPPUNIT_ASSERT( tmp == 100 );

  tmp = port->getCurrentQueueDepth();
  CPPUNIT_ASSERT( tmp == 0 );

  port->setMaxQueueDepth(22);
  tmp = port->getMaxQueueDepth();
  CPPUNIT_ASSERT( tmp == 22 );

  // check that port queue is empty
  bulkio::InXMLPort::dataTransfer *pkt  = port->getPacket(bulkio::Const::NON_BLOCKING );
  CPPUNIT_ASSERT( pkt == NULL );

  BULKIO::StreamSRI sri;
  port->pushSRI( sri );

  streams = port->activeSRIs();
  CPPUNIT_ASSERT( streams != NULL );
  CPPUNIT_ASSERT( streams->length() == 1 );
  delete streams;

  bulkio::InXMLPort::PortSequenceType v = new bulkio::InXMLPort::TransportType[1];
  BULKIO::PrecisionUTCTime TS;
  port->pushPacket( v, TS, false, "test_port_api" );

  // grab off packet
  pkt  = port->getPacket(bulkio::Const::NON_BLOCKING );
  CPPUNIT_ASSERT( pkt != NULL );
  delete pkt;

  port->enableStats( false );

  port->block();

  port->unblock();


}


template<>
void  Bulkio_InPort_Fixture::test_port_api( bulkio::InSDDSPort *port  ) {

  BULKIO::PortStatistics *stats = port->statistics();
  CPPUNIT_ASSERT( stats != NULL );
  delete stats;

  BULKIO::PortUsageType rt = port->state();
  CPPUNIT_ASSERT( rt == BULKIO::IDLE );

  BULKIO::StreamSRISequence  *streams = port->activeSRIs();
  CPPUNIT_ASSERT( streams != NULL );
  delete streams;

  BULKIO::StreamSRI sri;
  BULKIO::PrecisionUTCTime TS;
  port->pushSRI( sri, TS );

  streams = port->activeSRIs();
  CPPUNIT_ASSERT( streams != NULL );
  CPPUNIT_ASSERT( streams->length() == 1 );
  delete streams;


  BULKIO::SDDSStreamDefinition sdef;
  sdef.id = "test_sdds_id";
  sdef.dataFormat = BULKIO::SDDS_SB;
  sdef.multicastAddress = "1.1.1.1";
  sdef.vlan = 1234;
  sdef.port = 5678;
  char *aid = port->attach( sdef, "test_sdds_port_api" );
  CPPUNIT_ASSERT( aid != NULL );

  BULKIO::SDDSStreamSequence  *sss = port->attachedStreams();
  CPPUNIT_ASSERT( sss != NULL );
  CPPUNIT_ASSERT( sss->length() == 1 );
  std::string paddr;
  paddr = (*sss)[0].multicastAddress;
  std::cout << "port address " << paddr << std::endl;
  
  CPPUNIT_ASSERT( strcmp( paddr.c_str(), "1.1.1.1") == 0  );
  delete sss;

  char *uid = port->getUser(aid);
  CPPUNIT_ASSERT( uid != NULL );
  std::cout << "user id " << uid << std::endl;
  CPPUNIT_ASSERT( strcmp( uid, "test_sdds_port_api" ) == 0 );


  port->detach( aid );

  sss = port->attachedStreams();
  CPPUNIT_ASSERT( sss != NULL );
  CPPUNIT_ASSERT( sss->length() == 0 );
  delete sss;

  port->enableStats( false );


}




void 
Bulkio_InPort_Fixture::test_create_int8()
{
  bulkio::InInt8Port *port = new bulkio::InInt8Port("test", logger );
  CPPUNIT_ASSERT( port != NULL );

  CPPUNIT_ASSERT_NO_THROW( port );
}


void 
Bulkio_InPort_Fixture::test_int8()
{
  bulkio::InInt8Port *port = new bulkio::InInt8Port("test", logger );
  CPPUNIT_ASSERT( port != NULL );

  test_port_api( port );

  CPPUNIT_ASSERT_NO_THROW( port );

}

void 
Bulkio_InPort_Fixture::test_create_int16()
{
  bulkio::InInt16Port *port = new bulkio::InInt16Port("test");
  CPPUNIT_ASSERT( port != NULL );

  CPPUNIT_ASSERT_NO_THROW( port );
}


void 
Bulkio_InPort_Fixture::test_int16()
{
  bulkio::InInt16Port *port = new bulkio::InInt16Port("test");
  CPPUNIT_ASSERT( port != NULL );

  test_port_api( port );

  CPPUNIT_ASSERT_NO_THROW( port );

}

void 
Bulkio_InPort_Fixture::test_create_int32()
{
  bulkio::InInt32Port *port = new bulkio::InInt32Port("test");
  CPPUNIT_ASSERT( port != NULL );
}

void 
Bulkio_InPort_Fixture::test_int32()
{
  bulkio::InInt32Port *port = new bulkio::InInt32Port("test");
  CPPUNIT_ASSERT( port != NULL );

  test_port_api( port );

  CPPUNIT_ASSERT_NO_THROW( port );

}


void 
Bulkio_InPort_Fixture::test_create_int64()
{
  bulkio::InInt64Port *port = new bulkio::InInt64Port("test");
  CPPUNIT_ASSERT( port != NULL );
}


void 
Bulkio_InPort_Fixture::test_int64()
{
  bulkio::InInt64Port *port = new bulkio::InInt64Port("test");
  CPPUNIT_ASSERT( port != NULL );

  test_port_api( port );

  CPPUNIT_ASSERT_NO_THROW( port );

}


void 
Bulkio_InPort_Fixture::test_create_uint8()
{
  bulkio::InUInt8Port *port = new bulkio::InUInt8Port("test");
  CPPUNIT_ASSERT( port != NULL );
}

void 
Bulkio_InPort_Fixture::test_uint8()
{
  bulkio::InUInt8Port *port = new bulkio::InUInt8Port("test");
  CPPUNIT_ASSERT( port != NULL );

  test_port_api( port );

  CPPUNIT_ASSERT_NO_THROW( port );

}

void 
Bulkio_InPort_Fixture::test_create_uint16()
{
  bulkio::InUInt16Port *port = new bulkio::InUInt16Port("test");
  CPPUNIT_ASSERT( port != NULL );
}

void 
Bulkio_InPort_Fixture::test_uint16()
{
  bulkio::InUInt16Port *port = new bulkio::InUInt16Port("test");
  CPPUNIT_ASSERT( port != NULL );

  test_port_api( port );

  CPPUNIT_ASSERT_NO_THROW( port );

}


void 
Bulkio_InPort_Fixture::test_create_uint32()
{
  bulkio::InUInt32Port *port = new bulkio::InUInt32Port("test");
  CPPUNIT_ASSERT( port != NULL );
}

void 
Bulkio_InPort_Fixture::test_uint32()
{
  bulkio::InUInt32Port *port = new bulkio::InUInt32Port("test");
  CPPUNIT_ASSERT( port != NULL );

  test_port_api( port );

  CPPUNIT_ASSERT_NO_THROW( port );

}


void 
Bulkio_InPort_Fixture::test_create_uint64()
{
  bulkio::InUInt64Port *port = new bulkio::InUInt64Port("test");
  CPPUNIT_ASSERT( port != NULL );
}

void 
Bulkio_InPort_Fixture::test_uint64()
{
  bulkio::InUInt64Port *port = new bulkio::InUInt64Port("test");
  CPPUNIT_ASSERT( port != NULL );

  test_port_api( port );

  CPPUNIT_ASSERT_NO_THROW( port );

}



void 
Bulkio_InPort_Fixture::test_create_float()
{
  bulkio::InFloatPort *port = new bulkio::InFloatPort("test");
  CPPUNIT_ASSERT( port != NULL );
}


void 
Bulkio_InPort_Fixture::test_create_double()
{
  bulkio::InDoublePort *port = new bulkio::InDoublePort("test");
  CPPUNIT_ASSERT( port != NULL );
}



void 
Bulkio_InPort_Fixture::test_create_file()
{
  bulkio::InFilePort *port = new bulkio::InFilePort("test", logger );
  CPPUNIT_ASSERT( port != NULL );
}


void 
Bulkio_InPort_Fixture::test_file()
{
  bulkio::InFilePort *port = new bulkio::InFilePort("test", logger );
  CPPUNIT_ASSERT( port != NULL );

  test_port_api( port );

  CPPUNIT_ASSERT_NO_THROW( port );

}


void 
Bulkio_InPort_Fixture::test_create_xml()
{
  bulkio::InXMLPort *port = new bulkio::InXMLPort("test", logger );
  CPPUNIT_ASSERT( port != NULL );
}



void 
Bulkio_InPort_Fixture::test_xml()
{
  bulkio::InXMLPort *port = new bulkio::InXMLPort("test", logger );
  CPPUNIT_ASSERT( port != NULL );

  test_port_api( port );

  CPPUNIT_ASSERT_NO_THROW( port );

}


void 
Bulkio_InPort_Fixture::test_create_sdds()
{
  bulkio::InSDDSPort *port = new bulkio::InSDDSPort("test", logger );
  CPPUNIT_ASSERT( port != NULL );
}



void 
Bulkio_InPort_Fixture::test_sdds()
{
  bulkio::InSDDSPort *port = new bulkio::InSDDSPort("test", logger );
  CPPUNIT_ASSERT( port != NULL );

  test_port_api( port );

  CPPUNIT_ASSERT_NO_THROW( port );

}

