#include "bulkio.h"
#include<log4cxx/logger.h>
#include<log4cxx/propertyconfigurator.h>
#include <log4cxx/logstring.h>
#include <log4cxx/patternlayout.h>
#include <log4cxx/consoleappender.h>
#include <log4cxx/logmanager.h>
#include  "Bulkio_MultiOut_Port.h"

template < typename OUT_PORT, typename IN_PORT >
void Bulkio_MultiOut_Port< OUT_PORT, IN_PORT >::setUp()
{
  logger = log4cxx::Logger::getLogger("Bulkio-MultiOutPort-" + lname );
  logger->setLevel( log4cxx::Level::getInfo());
  orb = ossie::corba::CorbaInit(0,NULL);

  LOG4CXX_INFO(logger, "Setup - Multiout Create Ports Table " );

  ip1 = new IN_PORT("sink_1", logger );
  ip1_oid = ossie::corba::RootPOA()->activate_object(ip1);
  ip2 = new IN_PORT("sink_2", logger );
  ip2_oid = ossie::corba::RootPOA()->activate_object(ip2);
  ip3 = new IN_PORT("sink_3", logger );
  ip3_oid = ossie::corba::RootPOA()->activate_object(ip3);
  ip4 = new IN_PORT("sink_4", logger );
  ip4_oid = ossie::corba::RootPOA()->activate_object(ip4);
  port = new OUT_PORT("multiout_source", logger );
  port_oid = ossie::corba::RootPOA()->activate_object(port);

  desc_list.clear();
  LOG4CXX_INFO(logger, "Setup - Multiout Connection Table " );
  bulkio::connection_descriptor_struct desc;
  desc.connection_id = "connection_1";
  desc.stream_id = "stream-1-1";
  desc.port_name = "multiout_source";
  desc_list.push_back(desc);
  desc.connection_id = "connection_1";
  desc.stream_id = "stream-1-2";
  desc.port_name = "multiout_source";
  desc_list.push_back(desc);
  desc.connection_id = "connection_1";
  desc.stream_id = "stream-1-3";
  desc.port_name = "multiout_source";
  desc_list.push_back(desc);
  desc.connection_id = "connection_2";
  desc.stream_id = "stream-2-1";
  desc.port_name = "multiout_source";
  desc_list.push_back(desc);
  desc.connection_id = "connection_2";
  desc.stream_id = "stream-2-2";
  desc.port_name = "multiout_source";
  desc_list.push_back(desc);
  desc.connection_id = "connection_2";
  desc.stream_id = "stream-2-3";
  desc.port_name = "multiout_source";
  desc_list.push_back(desc);
  desc.connection_id = "connection_3";
  desc.stream_id = "stream-3-1";
  desc.port_name = "multiout_source";
  desc_list.push_back(desc);
  desc.connection_id = "connection_3";
  desc.stream_id = "stream-3-2";
  desc.port_name = "multiout_source";
  desc_list.push_back(desc);
  desc.connection_id = "connection_3";
  desc.stream_id = "stream-3-3";
  desc.port_name = "multiout_source";
  desc_list.push_back(desc);
  desc.connection_id = "connection_4";
  desc.stream_id = "stream-4-1";
  desc.port_name = "multiout_source";
  desc_list.push_back(desc);

}


template < typename OUT_PORT, typename IN_PORT >
void Bulkio_MultiOut_Port< OUT_PORT, IN_PORT >::tearDown()
{

  LOG4CXX_INFO(logger, "TearDown - Deactivate Servants " );
  ossie::corba::RootPOA()->deactivate_object(ip1_oid);
  ossie::corba::RootPOA()->deactivate_object(ip2_oid);
  ossie::corba::RootPOA()->deactivate_object(ip3_oid);
  ossie::corba::RootPOA()->deactivate_object(ip4_oid);
  ossie::corba::RootPOA()->deactivate_object(port_oid);

  LOG4CXX_INFO(logger, "TearDown - Shutdown the ORB " );
  //orb->shutdown(1);
}

//
// test_multiout_sri_filtered()
//
//   Test pushing out SRI to a single port and ensure other ports did not receive the SRI data
//

template < typename OUT_PORT, typename IN_PORT >
void  Bulkio_MultiOut_Port< OUT_PORT, IN_PORT >::test_multiout_sri_filtered( ) {

  LOG4CXX_INFO(logger, "Multiout SRI Filtered - BEGIN " );

  ExtendedCF::UsesConnectionSequence *clist = port->connections();
  CPPUNIT_ASSERT( clist != NULL );
  delete clist;

  LOG4CXX_INFO(logger, "Multiout SRI Filtered - Create Connections and Filter list " );
  port->connectPort( ip1->_this(), "connection_1");
  port->connectPort( ip2->_this(), "connection_2");
  port->connectPort( ip3->_this(), "connection_3");
  port->connectPort( ip4->_this(), "connection_4");
  port->updateConnectionFilter( desc_list );

  //
  // Push SRI for IP1
  //

  std::string  filter_stream_id( "stream-1-1" );
  double srate=11.0;
  double xdelta = 1.0/srate;
  BULKIO::StreamSRI sri;
  BULKIO::PrecisionUTCTime TS = bulkio::time::utils::now();
  typename OUT_PORT::NativeSequenceType v(91);
  sri = bulkio::sri::create( filter_stream_id, srate);
  port->pushSRI( sri );

  BULKIO::StreamSRISequence  *streams = ip1->activeSRIs();
  CPPUNIT_ASSERT( streams != NULL );
  CPPUNIT_ASSERT( streams->length() == 1 );
  delete streams;

  streams = ip2->activeSRIs();
  CPPUNIT_ASSERT_MESSAGE( "Multiout SRI Filtered - Port 2 Stream Failed", streams != NULL );
  CPPUNIT_ASSERT_MESSAGE( "Multiout SRI Filtered - Port 2 SRI was Received, Failed", streams->length() == 0 );
  delete streams;

  streams = ip3->activeSRIs();
  CPPUNIT_ASSERT_MESSAGE( "Multiout SRI Filtered - Port 3, Stream Failed", streams != NULL );
  CPPUNIT_ASSERT_MESSAGE( "Multiout SRI Filtered - Port 3, SRI was Received, Failed", streams->length() == 0 );
  delete streams;

  streams = ip4->activeSRIs();
  CPPUNIT_ASSERT_MESSAGE( "Multiout SRI Filtered - Port 4, Stream Failed", streams != NULL );
  CPPUNIT_ASSERT_MESSAGE( "Multiout SRI Filtered  - Port 4, SRI was Received, Failed", streams->length() == 0 );
  delete streams;

}



//
// test_multiout_sri_eos_filtered()
//
// Test pushing out SRI to each port and ensure other ports did not receive the SRI data,
// then terminate the data flow for each stream with EOS and then check each ports
// active SRI list is empty
//
template < typename OUT_PORT, typename IN_PORT >
void  Bulkio_MultiOut_Port< OUT_PORT, IN_PORT >::test_multiout_sri_eos_filtered( ) {

  LOG4CXX_INFO(logger, "Multiout SRI Filtered - BEGIN " );

  ExtendedCF::UsesConnectionSequence *clist = port->connections();
  CPPUNIT_ASSERT( clist != NULL );
  delete clist;

  LOG4CXX_INFO(logger, "Multiout SRI Filtered - Create Connections and Filter list " );
  port->connectPort( ip1->_this(), "connection_1");
  port->connectPort( ip2->_this(), "connection_2");
  port->connectPort( ip3->_this(), "connection_3");
  port->connectPort( ip4->_this(), "connection_4");
  port->updateConnectionFilter( desc_list );

  //
  // Push SRI for IP1
  //

  std::string  filter_stream_id( "stream-1-1" );
  double srate=11.0;
  double xdelta = 1.0/srate;
  BULKIO::StreamSRI sri;
  BULKIO::PrecisionUTCTime TS = bulkio::time::utils::now();
  typename OUT_PORT::NativeSequenceType v(0);
  sri = bulkio::sri::create( filter_stream_id, srate);
  port->pushSRI( sri );

  BULKIO::StreamSRISequence  *streams = ip1->activeSRIs();
  CPPUNIT_ASSERT( streams != NULL );
  CPPUNIT_ASSERT( streams->length() == 1 );
  BULKIO::StreamSRI asri;
  asri=(*streams)[0];  
  CPPUNIT_ASSERT_MESSAGE( "activeSRIs - StreamID Mismatch", strcmp( asri.streamID, filter_stream_id.c_str() ) == 0 );
  CPPUNIT_ASSERT_MESSAGE( "activeSRIs - SRI Mismatch:", asri.mode == 0 ) ;
  delete streams;

  streams = ip2->activeSRIs();
  CPPUNIT_ASSERT_MESSAGE( "Multiout SRI Filtered - Port 2 Stream Failed", streams != NULL );
  CPPUNIT_ASSERT_MESSAGE( "Multiout SRI Filtered - Port 2 SRI was Received, Failed", streams->length() == 0 );
  delete streams;

  streams = ip3->activeSRIs();
  CPPUNIT_ASSERT_MESSAGE( "Multiout SRI Filtered - Port 3, Stream Failed", streams != NULL );
  CPPUNIT_ASSERT_MESSAGE( "Multiout SRI Filtered - Port 3, SRI was Received, Failed", streams->length() == 0 );
  delete streams;

  streams = ip4->activeSRIs();
  CPPUNIT_ASSERT_MESSAGE( "Multiout SRI Filtered - Port 4, Stream Failed", streams != NULL );
  CPPUNIT_ASSERT_MESSAGE( "Multiout SRI Filtered  - Port 4, SRI was Received, Failed", streams->length() == 0 );
  delete streams;


  //
  // Push SRI for IP2
  //
  filter_stream_id =  "stream-2-1";
  srate=22.0;
  xdelta = 1.0/srate;
  TS = bulkio::time::utils::now();
  LOG4CXX_INFO(logger, "Multiout SRI Filter - sid:" << filter_stream_id );
  sri = bulkio::sri::create( filter_stream_id, srate);
  port->pushSRI( sri );

  streams = ip1->activeSRIs();
  CPPUNIT_ASSERT_MESSAGE( "Multiout SRI Filtered - Port 1 Stream Failed", streams != NULL );
  CPPUNIT_ASSERT_MESSAGE( "Multiout SRI Filtered - Port 1 StreamsLength, Failed", streams->length() == 1 );
  asri=(*streams)[0];  
  CPPUNIT_ASSERT_MESSAGE( "activeSRIs - StreamID Mismatch", strcmp( asri.streamID, "stream-1-1" ) == 0 );
  CPPUNIT_ASSERT_MESSAGE( "activeSRIs - SRI Mismatch:", asri.mode == 0 ) ;
  delete streams;

  streams = ip2->activeSRIs();
  CPPUNIT_ASSERT( streams != NULL );
  CPPUNIT_ASSERT( streams->length() == 1 );
  asri=(*streams)[0];  
  CPPUNIT_ASSERT_MESSAGE( "activeSRIs - StreamID Mismatch", strcmp( asri.streamID, "stream-2-1" ) == 0 );
  CPPUNIT_ASSERT_MESSAGE( "activeSRIs - SRI Mismatch:", asri.mode == 0 ) ;
  delete streams;

  streams = ip3->activeSRIs();
  CPPUNIT_ASSERT_MESSAGE( "Multiout SRI Filtered - Port 3, Stream Failed", streams != NULL );
  CPPUNIT_ASSERT_MESSAGE( "Multiout SRI Filtered - Port 3, SRI was Received, Failed", streams->length() == 0 );
  delete streams;

  streams = ip4->activeSRIs();
  CPPUNIT_ASSERT_MESSAGE( "Multiout SRI Filtered - Port 4, Stream Failed", streams != NULL );
  CPPUNIT_ASSERT_MESSAGE( "Multiout SRI Filtered  - Port 4, SRI was Received, Failed", streams->length() == 0 );
  delete streams;


  //
  // Push SRI for IP3
  //
  filter_stream_id =  "stream-3-1";
  srate=33.0;
  xdelta = 1.0/srate;
  TS = bulkio::time::utils::now();
  LOG4CXX_INFO(logger, "Multiout SRI Filter - sid:" << filter_stream_id );
  sri = bulkio::sri::create( filter_stream_id, srate);
  port->pushSRI( sri );

  streams = ip1->activeSRIs();
  CPPUNIT_ASSERT_MESSAGE( "Multiout SRI Filtered - Port 1 Stream Failed", streams != NULL );
  CPPUNIT_ASSERT_MESSAGE( "Multiout SRI Filtered - Port 1 StreamsLength, Failed", streams->length() == 1 );
  asri=(*streams)[0];  
  CPPUNIT_ASSERT_MESSAGE( "activeSRIs - StreamID Mismatch", strcmp( asri.streamID, "stream-1-1" ) == 0 );
  CPPUNIT_ASSERT_MESSAGE( "activeSRIs - SRI Mismatch:", asri.mode == 0 ) ;
  delete streams;

  streams = ip2->activeSRIs();
  CPPUNIT_ASSERT_MESSAGE( "Multiout SRI Filtered - Port 2, Stream Failed", streams != NULL );
  CPPUNIT_ASSERT_MESSAGE( "Multiout SRI Filtered - Port 2 StreamsLength, Failed", streams->length() == 1 );
  asri=(*streams)[0];  
  CPPUNIT_ASSERT_MESSAGE( "activeSRIs - StreamID Mismatch", strcmp( asri.streamID, "stream-2-1" ) == 0 );
  CPPUNIT_ASSERT_MESSAGE( "activeSRIs - SRI Mismatch:", asri.mode == 0 ) ;
  delete streams;


  streams = ip3->activeSRIs();
  CPPUNIT_ASSERT( streams != NULL );
  CPPUNIT_ASSERT( streams->length() == 1 );
  asri=(*streams)[0];  
  CPPUNIT_ASSERT_MESSAGE( "activeSRIs - StreamID Mismatch", strcmp( asri.streamID, "stream-3-1" ) == 0 );
  CPPUNIT_ASSERT_MESSAGE( "activeSRIs - SRI Mismatch:", asri.mode == 0 ) ;
  delete streams;

  streams = ip4->activeSRIs();
  CPPUNIT_ASSERT_MESSAGE( "Multiout SRI Filtered - Port 4, Stream Failed", streams != NULL );
  CPPUNIT_ASSERT_MESSAGE( "Multiout SRI Filtered  - Port 4, SRI was Received, Failed", streams->length() == 0 );
  delete streams;

  //
  // Push SRI for IP4
  //
  filter_stream_id =  "stream-4-1";
  srate=44.0;
  xdelta = 1.0/srate;
  TS = bulkio::time::utils::now();
  LOG4CXX_INFO(logger, "Multiout SRI Filter - sid:" << filter_stream_id );
  sri = bulkio::sri::create( filter_stream_id, srate);
  port->pushSRI( sri );

  streams = ip1->activeSRIs();
  CPPUNIT_ASSERT_MESSAGE( "Multiout SRI Filtered - Port 1 Stream Failed", streams != NULL );
  CPPUNIT_ASSERT_MESSAGE( "Multiout SRI Filtered - Port 1 StreamsLength, Failed", streams->length() == 1 );
  asri=(*streams)[0];  
  CPPUNIT_ASSERT_MESSAGE( "activeSRIs - StreamID Mismatch", strcmp( asri.streamID, "stream-1-1" ) == 0 );
  CPPUNIT_ASSERT_MESSAGE( "activeSRIs - SRI Mismatch:", asri.mode == 0 ) ;
  delete streams;

  streams = ip2->activeSRIs();
  CPPUNIT_ASSERT_MESSAGE( "Multiout SRI Filtered - Port 2, Stream Failed", streams != NULL );
  CPPUNIT_ASSERT_MESSAGE( "Multiout SRI Filtered - Port 2 StreamsLength, Failed", streams->length() == 1 );
  asri=(*streams)[0];  
  CPPUNIT_ASSERT_MESSAGE( "activeSRIs - StreamID Mismatch", strcmp( asri.streamID, "stream-2-1" ) == 0 );
  CPPUNIT_ASSERT_MESSAGE( "activeSRIs - SRI Mismatch:", asri.mode == 0 ) ;
  delete streams;

  streams = ip3->activeSRIs();
  CPPUNIT_ASSERT_MESSAGE( "Multiout SRI Filtered - Port 3, Stream Failed", streams != NULL );
  CPPUNIT_ASSERT_MESSAGE( "Multiout SRI Filtered - Port 3 StreamsLength, Failed", streams->length() == 1 );
  asri=(*streams)[0];  
  CPPUNIT_ASSERT_MESSAGE( "activeSRIs - StreamID Mismatch", strcmp( asri.streamID, "stream-3-1" ) == 0 );
  CPPUNIT_ASSERT_MESSAGE( "activeSRIs - SRI Mismatch:", asri.mode == 0 ) ;
  delete streams;

  streams = ip4->activeSRIs();
  CPPUNIT_ASSERT( streams != NULL );
  CPPUNIT_ASSERT( streams->length() == 1 );
  asri=(*streams)[0];  
  CPPUNIT_ASSERT_MESSAGE( "activeSRIs - StreamID Mismatch", strcmp( asri.streamID, "stream-4-1" ) == 0 );
  CPPUNIT_ASSERT_MESSAGE( "activeSRIs - SRI Mismatch:", asri.mode == 0 ) ;
  delete streams;

  //
  // Send EOS downstream and check activeSRIs
  //
  filter_stream_id = "stream-1-1";
  port->pushPacket( v, TS, true, filter_stream_id );

  typename IN_PORT::dataTransfer *pkt;
  pkt  = ip1->getPacket(bulkio::Const::NON_BLOCKING );;
  CPPUNIT_ASSERT_MESSAGE( "getPacket - PKT was empty", pkt != NULL );
  CPPUNIT_ASSERT_MESSAGE( "getPacket - StreamID Mismatch", strcmp( pkt->SRI.streamID, filter_stream_id.c_str() ) == 0 );
  CPPUNIT_ASSERT_MESSAGE( "getPacket - EOS Mismatch:" , pkt->EOS == 1) ;

  filter_stream_id = "stream-2-1";
  port->pushPacket( v, TS, true, filter_stream_id );
  pkt  = ip2->getPacket(bulkio::Const::NON_BLOCKING );;
  CPPUNIT_ASSERT_MESSAGE( "getPacket - PKT was empty", pkt != NULL );
  CPPUNIT_ASSERT_MESSAGE( "getPacket - StreamID Mismatch", strcmp( pkt->SRI.streamID, filter_stream_id.c_str() ) == 0 );
  CPPUNIT_ASSERT_MESSAGE( "getPacket - EOS Mismatch:" , pkt->EOS == 1) ;

  filter_stream_id = "stream-3-1";
  port->pushPacket( v, TS, true, filter_stream_id );
  pkt  = ip3->getPacket(bulkio::Const::NON_BLOCKING );;
  CPPUNIT_ASSERT_MESSAGE( "getPacket - PKT was empty", pkt != NULL );
  CPPUNIT_ASSERT_MESSAGE( "getPacket - StreamID Mismatch", strcmp( pkt->SRI.streamID, filter_stream_id.c_str() ) == 0 );
  CPPUNIT_ASSERT_MESSAGE( "getPacket - EOS Mismatch:" , pkt->EOS == 1) ;

  filter_stream_id = "stream-4-1";
  port->pushPacket( v, TS, true, filter_stream_id );
  pkt  = ip4->getPacket(bulkio::Const::NON_BLOCKING );;
  CPPUNIT_ASSERT_MESSAGE( "getPacket - PKT was empty", pkt != NULL );
  CPPUNIT_ASSERT_MESSAGE( "getPacket - StreamID Mismatch", strcmp( pkt->SRI.streamID, filter_stream_id.c_str() ) == 0 );
  CPPUNIT_ASSERT_MESSAGE( "getPacket - EOS Mismatch:" , pkt->EOS == 1) ;

  streams = ip1->activeSRIs();
  CPPUNIT_ASSERT_MESSAGE( "Multiout SRI Filtered - Port 1 Stream Failed", streams != NULL );
  CPPUNIT_ASSERT_MESSAGE( "Multiout SRI Filtered - Port 1 SRI was Received, Failed", streams->length() == 0 );
  delete streams;
  streams = ip2->activeSRIs();
  CPPUNIT_ASSERT_MESSAGE( "Multiout SRI Filtered - Port 2 Stream Failed", streams != NULL );
  CPPUNIT_ASSERT_MESSAGE( "Multiout SRI Filtered - Port 2 SRI was Received, Failed", streams->length() == 0 );
  delete streams;
  streams = ip3->activeSRIs();
  CPPUNIT_ASSERT_MESSAGE( "Multiout SRI Filtered - Port 3 Stream Failed", streams != NULL );
  CPPUNIT_ASSERT_MESSAGE( "Multiout SRI Filtered - Port 3 SRI was Received, Failed", streams->length() == 0 );
  delete streams;
  streams = ip3->activeSRIs();
  CPPUNIT_ASSERT_MESSAGE( "Multiout SRI Filtered - Port 4 Stream Failed", streams != NULL );
  CPPUNIT_ASSERT_MESSAGE( "Multiout SRI Filtered - Port 4 SRI was Received, Failed", streams->length() == 0 );
  delete streams;

}


template < typename OUT_PORT, typename IN_PORT >
void  Bulkio_MultiOut_Port< OUT_PORT, IN_PORT >::test_multiout_data_filtered( ) {

  LOG4CXX_INFO(logger, "Multiout Data Filter - 1 stream id , 4 independent consumers" );

  LOG4CXX_INFO(logger, "Multiout Data Filter - setup connections" );
  port->connectPort( ip1->_this(), "connection_1");
  port->connectPort( ip2->_this(), "connection_2");
  port->connectPort( ip3->_this(), "connection_3");
  port->connectPort( ip4->_this(), "connection_4");

  ExtendedCF::UsesConnectionSequence *clist = port->connections();
  CPPUNIT_ASSERT( clist != NULL );
  LOG4CXX_INFO(logger, "Multiout Data Filter - Check connections:" << clist->length() );
  CPPUNIT_ASSERT( clist->length() == 4 );
  delete clist;

  port->updateConnectionFilter( desc_list );

  //
  //  Test Filter for IP1
  //

  std::string  filter_stream_id( "stream-1-1" );
  double srate=11.0;
  double xdelta = 1.0/srate;
  BULKIO::StreamSRI sri;
  BULKIO::PrecisionUTCTime TS = bulkio::time::utils::now();
  LOG4CXX_INFO(logger, "Multiout Data Filter - Pushing vector to consumers,  sid:" << filter_stream_id );
  sri = bulkio::sri::create( filter_stream_id, srate);
  port->pushSRI( sri );

  typename OUT_PORT::NativeSequenceType v(91);
  port->pushPacket( v, TS, false, filter_stream_id );

  // check all the consumers to see if they got the correct packet
  typename IN_PORT::dataTransfer *pkt ;
  pkt  = ip1->getPacket(bulkio::Const::NON_BLOCKING );
  CPPUNIT_ASSERT_MESSAGE( "getPacket - PKT was empty", pkt != NULL );
  CPPUNIT_ASSERT_MESSAGE( "getPacket - StreamID Mismatch", strcmp( pkt->SRI.streamID, filter_stream_id.c_str() ) == 0 );
  CPPUNIT_ASSERT_MESSAGE( "getPacket - EOS Mismatch:" , pkt->EOS == 0 ) ;
  CPPUNIT_ASSERT_MESSAGE( "getPacket - SRI Mismatch:", pkt->SRI.mode == 0 ) ;
  CPPUNIT_ASSERT_MESSAGE( "getPacket - Data Length:",  pkt->dataBuffer.size() == 91 ) ;
  if ( pkt ) delete pkt;

  //
  // make sure others did not get data ip2, ip3, ip4
  //
  pkt  = ip2->getPacket(bulkio::Const::NON_BLOCKING );
  CPPUNIT_ASSERT_MESSAGE( "getPacket - PKT was NOT EMPTY", pkt == NULL );
  if ( pkt ) delete pkt;

  pkt  = ip3->getPacket(bulkio::Const::NON_BLOCKING );
  CPPUNIT_ASSERT_MESSAGE( "getPacket - PKT was NOT EMPTY", pkt == NULL );
  if ( pkt ) delete pkt;

  pkt  = ip4->getPacket(bulkio::Const::NON_BLOCKING );
  CPPUNIT_ASSERT_MESSAGE( "getPacket - PKT was NOT EMPTY", pkt == NULL );
  if ( pkt ) delete pkt;

  //
  //  Test Filter for IP2
  //
  filter_stream_id =  "stream-2-1";
  srate=22.0;
  xdelta = 1.0/srate;
  TS = bulkio::time::utils::now();
  LOG4CXX_INFO(logger, "Multiout Data Filter - Pushing vector to consumers,  sid:" << filter_stream_id );
  sri = bulkio::sri::create( filter_stream_id, srate);
  port->pushSRI( sri );
 
  port->pushPacket( v, TS, false, filter_stream_id );

  // check all the consumers to see if they got the correct packet
  pkt  = ip1->getPacket(bulkio::Const::NON_BLOCKING );
  CPPUNIT_ASSERT_MESSAGE( "getPacket - PKT was NOT EMPTY", pkt == NULL );
  if ( pkt ) delete pkt;

  //
  // make sure others did not get data ip1, ip3, ip4
  //
  pkt  = ip2->getPacket(bulkio::Const::NON_BLOCKING );
  CPPUNIT_ASSERT_MESSAGE( "getPacket - PKT was empty", pkt != NULL );
  CPPUNIT_ASSERT_MESSAGE( "getPacket - StreamID Mismatch", strcmp( pkt->SRI.streamID, filter_stream_id.c_str() ) == 0 );
  CPPUNIT_ASSERT_MESSAGE( "getPacket - EOS Mismatch:" , pkt->EOS == 0 ) ;
  CPPUNIT_ASSERT_MESSAGE( "getPacket - SRI Mismatch:", pkt->SRI.mode == 0 ) ;
  CPPUNIT_ASSERT_MESSAGE( "getPacket - Data Length:",  pkt->dataBuffer.size() == 91 ) ;
  if ( pkt ) delete pkt;

  pkt  = ip3->getPacket(bulkio::Const::NON_BLOCKING );
  CPPUNIT_ASSERT_MESSAGE( "getPacket - PKT was NOT EMPTY", pkt == NULL );
  if ( pkt ) delete pkt;

  pkt  = ip4->getPacket(bulkio::Const::NON_BLOCKING );
  CPPUNIT_ASSERT_MESSAGE( "getPacket - PKT was NOT EMPTY", pkt == NULL );
  if ( pkt ) delete pkt;

  //
  //  Test Filter for IP3
  //
  filter_stream_id =  "stream-3-1";
  srate=33.0;
  xdelta = 1.0/srate;
  TS = bulkio::time::utils::now();
  LOG4CXX_INFO(logger, "Multiout Data Filter - Pushing vector to consumers,  sid:" << filter_stream_id );
  sri = bulkio::sri::create( filter_stream_id, srate);
  port->pushSRI( sri );
 
  port->pushPacket( v, TS, false, filter_stream_id );

  // check all the consumers to see if they got the correct packet
  pkt  = ip1->getPacket(bulkio::Const::NON_BLOCKING );
  CPPUNIT_ASSERT_MESSAGE( "getPacket - PKT was NOT EMPTY", pkt == NULL );
  if ( pkt ) delete pkt;

  //
  // make sure others did not get data ip1, ip3, ip4
  //
  pkt  = ip2->getPacket(bulkio::Const::NON_BLOCKING );
  CPPUNIT_ASSERT_MESSAGE( "getPacket - PKT was NOT EMPTY", pkt == NULL );
  if ( pkt ) delete pkt;  

  pkt  = ip3->getPacket(bulkio::Const::NON_BLOCKING );
  CPPUNIT_ASSERT_MESSAGE( "getPacket - PKT was empty", pkt != NULL );
  CPPUNIT_ASSERT_MESSAGE( "getPacket - StreamID Mismatch", strcmp( pkt->SRI.streamID, filter_stream_id.c_str() ) == 0 );
  CPPUNIT_ASSERT_MESSAGE( "getPacket - EOS Mismatch:" , pkt->EOS == 0 ) ;
  CPPUNIT_ASSERT_MESSAGE( "getPacket - SRI Mismatch:", pkt->SRI.mode == 0 ) ;
  CPPUNIT_ASSERT_MESSAGE( "getPacket - Data Length:",  pkt->dataBuffer.size() == 91 ) ;
  if ( pkt ) delete pkt;


  pkt  = ip4->getPacket(bulkio::Const::NON_BLOCKING );
  CPPUNIT_ASSERT_MESSAGE( "getPacket - PKT was NOT EMPTY", pkt == NULL );
  if ( pkt ) delete pkt;

  //
  //  Test Filter for IP4
  //
  filter_stream_id =  "stream-4-1";
  srate = 44.0;
  xdelta = 1.0/srate;
  TS = bulkio::time::utils::now();
  LOG4CXX_INFO(logger, "Multiout Data Filter - Pushing vector to consumers,  sid:" << filter_stream_id );
  sri = bulkio::sri::create( filter_stream_id, srate);
  port->pushSRI( sri );
 
  port->pushPacket( v, TS, false, filter_stream_id );

  // check all the consumers to see if they got the correct packet
  pkt  = ip1->getPacket(bulkio::Const::NON_BLOCKING );
  CPPUNIT_ASSERT_MESSAGE( "getPacket - PKT was NOT EMPTY", pkt == NULL );
  if ( pkt ) delete pkt;

  //
  // make sure others did not get data ip1, ip3, ip4
  //
  pkt  = ip2->getPacket(bulkio::Const::NON_BLOCKING );
  CPPUNIT_ASSERT_MESSAGE( "getPacket - PKT was NOT EMPTY", pkt == NULL );
  if ( pkt ) delete pkt;  

  pkt  = ip3->getPacket(bulkio::Const::NON_BLOCKING );
  CPPUNIT_ASSERT_MESSAGE( "getPacket - PKT was NOT EMPTY", pkt == NULL );
  if ( pkt ) delete pkt;


  pkt  = ip4->getPacket(bulkio::Const::NON_BLOCKING );
  CPPUNIT_ASSERT_MESSAGE( "getPacket - PKT was empty", pkt != NULL );
  CPPUNIT_ASSERT_MESSAGE( "getPacket - StreamID Mismatch", strcmp( pkt->SRI.streamID, filter_stream_id.c_str() ) == 0 );
  CPPUNIT_ASSERT_MESSAGE( "getPacket - EOS Mismatch:" , pkt->EOS == 0 ) ;
  CPPUNIT_ASSERT_MESSAGE( "getPacket - SRI Mismatch:", pkt->SRI.mode == 0 ) ;
  CPPUNIT_ASSERT_MESSAGE( "getPacket - Data Length:",  pkt->dataBuffer.size() == 91 ) ;
  if ( pkt ) delete pkt;

}


//
// test_multiout_data_sri_filtered( )
//
// Test pushPacket data operations on each port do not affect the other port's state
//
template < typename OUT_PORT, typename IN_PORT >
void  Bulkio_MultiOut_Port< OUT_PORT, IN_PORT >::test_multiout_data_sri_filtered( ) {

  LOG4CXX_INFO(logger, "Multiout Data/SRI Filter - 1 stream id , 4 independent consumers" );

  LOG4CXX_INFO(logger, "Multiout Data Filter - setup connections" );
  port->connectPort( ip1->_this(), "connection_1");
  port->connectPort( ip2->_this(), "connection_2");
  port->connectPort( ip3->_this(), "connection_3");
  port->connectPort( ip4->_this(), "connection_4");

  ExtendedCF::UsesConnectionSequence *clist = port->connections();
  CPPUNIT_ASSERT( clist != NULL );
  LOG4CXX_INFO(logger, "Multiout Data Filter - Check connections:" << clist->length() );
  CPPUNIT_ASSERT( clist->length() == 4 );
  delete clist;

  port->updateConnectionFilter( desc_list );

  //
  //  Test Filter for IP1
  //

  std::string  filter_stream_id( "stream-1-1" );
  double srate=11.0;
  double xdelta = 1.0/srate;
  BULKIO::StreamSRI sri;
  BULKIO::PrecisionUTCTime TS = bulkio::time::utils::now();
  LOG4CXX_INFO(logger, "Multiout Data Filter - Pushing vector to consumers,  sid:" << filter_stream_id );
  sri = bulkio::sri::create( filter_stream_id, srate);
  port->pushSRI( sri );

  typename OUT_PORT::NativeSequenceType v(91);
  port->pushPacket( v, TS, false, filter_stream_id );

  // check all the consumers to see if they got the correct packet
  typename IN_PORT::dataTransfer *pkt ;
  pkt  = ip1->getPacket(bulkio::Const::NON_BLOCKING );
  LOG4CXX_INFO(logger, "Multiout Data Filter - " << pkt->SRI.streamID << " exp:" << filter_stream_id );
  CPPUNIT_ASSERT_MESSAGE( "getPacket - PKT was empty", pkt != NULL );
  CPPUNIT_ASSERT_MESSAGE( "getPacket - StreamID Mismatch", strcmp( pkt->SRI.streamID, filter_stream_id.c_str() ) == 0 );
  CPPUNIT_ASSERT_MESSAGE( "getPacket - EOS Mismatch:" , pkt->EOS == 0 ) ;
  CPPUNIT_ASSERT_MESSAGE( "getPacket - SRI Mismatch:", pkt->SRI.mode == 0 ) ;
  CPPUNIT_ASSERT_MESSAGE( "getPacket - Data Length:",  pkt->dataBuffer.size() == 91 ) ;
  if ( pkt ) delete pkt;

  //
  // make sure others did not get data ip2, ip3, ip4
  //
  pkt  = ip2->getPacket(bulkio::Const::NON_BLOCKING );
  CPPUNIT_ASSERT_MESSAGE( "getPacket - PKT was NOT EMPTY", pkt == NULL );
  if ( pkt ) delete pkt;

  pkt  = ip3->getPacket(bulkio::Const::NON_BLOCKING );
  CPPUNIT_ASSERT_MESSAGE( "getPacket - PKT was NOT EMPTY", pkt == NULL );
  if ( pkt ) delete pkt;

  pkt  = ip4->getPacket(bulkio::Const::NON_BLOCKING );
  CPPUNIT_ASSERT_MESSAGE( "getPacket - PKT was NOT EMPTY", pkt == NULL );
  if ( pkt ) delete pkt;

  //
  //  Test Filter for IP2
  //
  filter_stream_id =  "stream-2-1";
  srate=22.0;
  xdelta = 1.0/srate;
  TS = bulkio::time::utils::now();
  LOG4CXX_INFO(logger, "Multiout Data Filter - Pushing vector to consumers,  sid:" << filter_stream_id );
  sri = bulkio::sri::create( filter_stream_id, srate);
  port->pushSRI( sri );
 
  port->pushPacket( v, TS, false, filter_stream_id );

  // check all the consumers to see if they got the correct packet
  pkt  = ip1->getPacket(bulkio::Const::NON_BLOCKING );
  CPPUNIT_ASSERT_MESSAGE( "getPacket - PKT was NOT EMPTY", pkt == NULL );
  if ( pkt ) delete pkt;

  //
  // make sure others did not get data ip1, ip3, ip4
  //
  pkt  = ip2->getPacket(bulkio::Const::NON_BLOCKING );
  LOG4CXX_INFO(logger, "Multiout Data Filter - " << pkt->SRI.streamID << " exp:" << filter_stream_id );
  CPPUNIT_ASSERT_MESSAGE( "getPacket - PKT was empty", pkt != NULL );
  CPPUNIT_ASSERT_MESSAGE( "getPacket - StreamID Mismatch", strcmp( pkt->SRI.streamID, filter_stream_id.c_str() ) == 0 );
  CPPUNIT_ASSERT_MESSAGE( "getPacket - EOS Mismatch:" , pkt->EOS == 0 ) ;
  CPPUNIT_ASSERT_MESSAGE( "getPacket - SRI Mismatch:", pkt->SRI.mode == 0 ) ;
  CPPUNIT_ASSERT_DOUBLES_EQUAL( xdelta,  pkt->SRI.xdelta, 0.01 ) ;
  CPPUNIT_ASSERT_MESSAGE( "getPacket - Data Length:",  pkt->dataBuffer.size() == 91 ) ;
  if ( pkt ) delete pkt;

  pkt  = ip3->getPacket(bulkio::Const::NON_BLOCKING );
  CPPUNIT_ASSERT_MESSAGE( "getPacket - PKT was NOT EMPTY", pkt == NULL );
  if ( pkt ) delete pkt;

  pkt  = ip4->getPacket(bulkio::Const::NON_BLOCKING );
  CPPUNIT_ASSERT_MESSAGE( "getPacket - PKT was NOT EMPTY", pkt == NULL );
  if ( pkt ) delete pkt;

  //
  //  Test Filter for IP3
  //
  filter_stream_id =  "stream-3-1";
  srate=33.0;
  xdelta = 1.0/srate;
  TS = bulkio::time::utils::now();
  LOG4CXX_INFO(logger, "Multiout Data Filter - Pushing vector to consumers,  sid:" << filter_stream_id );
  sri = bulkio::sri::create( filter_stream_id, srate);
  port->pushSRI( sri );
 
  port->pushPacket( v, TS, false, filter_stream_id );

  // check all the consumers to see if they got the correct packet
  pkt  = ip1->getPacket(bulkio::Const::NON_BLOCKING );
  CPPUNIT_ASSERT_MESSAGE( "getPacket - PKT was NOT EMPTY", pkt == NULL );
  if ( pkt ) delete pkt;

  //
  // make sure others did not get data ip1, ip3, ip4
  //
  pkt  = ip2->getPacket(bulkio::Const::NON_BLOCKING );
  CPPUNIT_ASSERT_MESSAGE( "getPacket - PKT was NOT EMPTY", pkt == NULL );
  if ( pkt ) delete pkt;  

  pkt  = ip3->getPacket(bulkio::Const::NON_BLOCKING );
  LOG4CXX_INFO(logger, "Multiout Data Filter - " << pkt->SRI.streamID << " exp:" << filter_stream_id );
  CPPUNIT_ASSERT_MESSAGE( "getPacket - PKT was empty", pkt != NULL );
  CPPUNIT_ASSERT_MESSAGE( "getPacket - StreamID Mismatch", strcmp( pkt->SRI.streamID, filter_stream_id.c_str() ) == 0 );
  CPPUNIT_ASSERT_MESSAGE( "getPacket - EOS Mismatch:" , pkt->EOS == 0 ) ;
  CPPUNIT_ASSERT_MESSAGE( "getPacket - SRI Mismatch:", pkt->SRI.mode == 0 ) ;
  CPPUNIT_ASSERT_DOUBLES_EQUAL( xdelta,  pkt->SRI.xdelta, 0.01 ) ;
  CPPUNIT_ASSERT_MESSAGE( "getPacket - Data Length:",  pkt->dataBuffer.size() == 91 ) ;
  if ( pkt ) delete pkt;

  pkt  = ip4->getPacket(bulkio::Const::NON_BLOCKING );
  CPPUNIT_ASSERT_MESSAGE( "getPacket - PKT was NOT EMPTY", pkt == NULL );
  if ( pkt ) delete pkt;

  //
  //  Test Filter for IP4
  //
  filter_stream_id =  "stream-4-1";
  srate=44.0;
  xdelta = 1.0/srate;
  TS = bulkio::time::utils::now();
  LOG4CXX_INFO(logger, "Multiout Data Filter - Pushing vector to consumers,  sid:" << filter_stream_id );
  sri = bulkio::sri::create( filter_stream_id, srate);
  port->pushSRI( sri );
 
  port->pushPacket( v, TS, false, filter_stream_id );

  // check all the consumers to see if they got the correct packet
  pkt  = ip1->getPacket(bulkio::Const::NON_BLOCKING );
  CPPUNIT_ASSERT_MESSAGE( "getPacket - PKT was NOT EMPTY", pkt == NULL );
  if ( pkt ) delete pkt;

  //
  // make sure others did not get data ip1, ip3, ip4
  //
  pkt  = ip2->getPacket(bulkio::Const::NON_BLOCKING );
  CPPUNIT_ASSERT_MESSAGE( "getPacket - PKT was NOT EMPTY", pkt == NULL );
  if ( pkt ) delete pkt;  

  pkt  = ip3->getPacket(bulkio::Const::NON_BLOCKING );
  CPPUNIT_ASSERT_MESSAGE( "getPacket - PKT was NOT EMPTY", pkt == NULL );
  if ( pkt ) delete pkt;

  pkt  = ip4->getPacket(bulkio::Const::NON_BLOCKING );
  LOG4CXX_INFO(logger, "Multiout Data Filter - " << pkt->SRI.streamID << " exp:" << filter_stream_id );
  CPPUNIT_ASSERT_MESSAGE( "getPacket - PKT was empty", pkt != NULL );
  CPPUNIT_ASSERT_MESSAGE( "getPacket - StreamID Mismatch", strcmp( pkt->SRI.streamID, filter_stream_id.c_str() ) == 0 );
  CPPUNIT_ASSERT_MESSAGE( "getPacket - EOS Mismatch:", pkt->EOS == 0 ) ;
  CPPUNIT_ASSERT_MESSAGE( "getPacket - SRI Mismatch:", pkt->SRI.mode == 0 ) ;
  CPPUNIT_ASSERT_DOUBLES_EQUAL( xdelta,  pkt->SRI.xdelta, 0.01 ) ;
  CPPUNIT_ASSERT_MESSAGE( "getPacket - Data Length:",  pkt->dataBuffer.size() == 91 ) ;
  if ( pkt ) delete pkt;

}



// Registers the fixture into the 'registry'
// this also worked sans type name in output CPPUNIT_TEST_SUITE_REGISTRATION( MultiOutUInt8 );
CPPUNIT_TEST_SUITE_REGISTRATION( MultiOutUInt8_Port );
CPPUNIT_TEST_SUITE_REGISTRATION( MultiOutInt16_Port );
CPPUNIT_TEST_SUITE_REGISTRATION( MultiOutUInt16_Port );
CPPUNIT_TEST_SUITE_REGISTRATION( MultiOutInt32_Port );
CPPUNIT_TEST_SUITE_REGISTRATION( MultiOutUInt32_Port );
CPPUNIT_TEST_SUITE_REGISTRATION( MultiOutInt64_Port );
CPPUNIT_TEST_SUITE_REGISTRATION( MultiOutUInt64_Port );
CPPUNIT_TEST_SUITE_REGISTRATION( MultiOutDouble_Port );
CPPUNIT_TEST_SUITE_REGISTRATION( MultiOutFloat_Port );

//template class Bulkio_MultiOut_Port< bulkio::OutCharPort, bulkio::InInt8Port >;
template class Bulkio_MultiOut_Port< bulkio::OutOctetPort, bulkio::InUInt8Port >;
template class Bulkio_MultiOut_Port< bulkio::OutInt16Port, bulkio::InInt16Port >;
template class Bulkio_MultiOut_Port< bulkio::OutUInt16Port, bulkio::InUInt16Port >;
template class Bulkio_MultiOut_Port< bulkio::OutInt32Port, bulkio::InInt32Port >;
template class Bulkio_MultiOut_Port< bulkio::OutUInt32Port, bulkio::InUInt32Port >;
template class Bulkio_MultiOut_Port< bulkio::OutInt64Port, bulkio::InInt64Port >;
template class Bulkio_MultiOut_Port< bulkio::OutUInt64Port, bulkio::InUInt64Port >;
template class Bulkio_MultiOut_Port< bulkio::OutDoublePort, bulkio::InDoublePort >;
template class Bulkio_MultiOut_Port< bulkio::OutFloatPort, bulkio::InFloatPort >;

