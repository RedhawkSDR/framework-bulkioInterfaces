
/*******************************************************************************************


*******************************************************************************************/
#include <bulkio_p.h>
#include <uuid/uuid.h>


namespace  bulkio {


  /**
     OutPort Constructor
  
     Accepts connect/disconnect interfaces for notification when these events occur
  */

  template < typename PortTraits >
  OutPort< PortTraits >::OutPort(std::string port_name,  
				 LOGGER_PTR logger, 
				 ConnectionEventListener *connectCB, 
				 ConnectionEventListener *disconnectCB ) :
    Port_Uses_base_impl(port_name),
    logger(logger),
    _connectCB(),
    _disconnectCB()
  {

    if ( connectCB ) {
      _connectCB = boost::shared_ptr< ConnectionEventListener >( connectCB, null_deleter() );
    }

    if ( disconnectCB ) {
      _disconnectCB = boost::shared_ptr< ConnectionEventListener >( disconnectCB, null_deleter() );
    }


    recConnectionsRefresh = false;
    recConnections.length(0);

    LOG_DEBUG( logger, "bulkio::OutPort::CTOR port:" << name );

  }


  template < typename PortTraits >
  OutPort< PortTraits >::OutPort(std::string port_name,  
				 ConnectionEventListener *connectCB, 
				 ConnectionEventListener *disconnectCB ) :
    Port_Uses_base_impl(port_name),
    _connectCB(),
    _disconnectCB()
  {

    if ( connectCB ) {
      _connectCB = boost::shared_ptr< ConnectionEventListener >( connectCB, null_deleter() );
    }

    if ( disconnectCB ) {
      _disconnectCB = boost::shared_ptr< ConnectionEventListener >( disconnectCB, null_deleter() );
    }

    recConnectionsRefresh = false;
    recConnections.length(0);

  }

  template < typename PortTraits >
  OutPort< PortTraits >::~OutPort(){

  }


  template < typename PortTraits >
  void   OutPort< PortTraits >::setNewConnectListener( ConnectionEventListener *newListener ) {
    _connectCB =  boost::shared_ptr< ConnectionEventListener >(newListener, null_deleter());
  }

  template < typename PortTraits >
  void   OutPort< PortTraits >::setNewConnectListener( ConnectionEventCallbackFn  newListener ) {
    _connectCB =  boost::make_shared< StaticConnectionListener >( newListener );
  }

  template < typename PortTraits >
  void   OutPort< PortTraits >::setNewDisconnectListener( ConnectionEventListener *newListener ) {
    _disconnectCB =  boost::shared_ptr< ConnectionEventListener >(newListener, null_deleter());
  }

  template < typename PortTraits >
  void   OutPort< PortTraits >::setNewDisconnectListener( ConnectionEventCallbackFn  newListener ) {
    _disconnectCB =  boost::make_shared< StaticConnectionListener >( newListener );
  }



  template < typename PortTraits >
  void OutPort< PortTraits >::pushSRI(const BULKIO::StreamSRI& H) {

  
    TRACE_ENTER(logger, "OutPort::pushSRI" );


    typename ConnectionsList::iterator i;

    SCOPED_LOCK lock(updatingPortsLock);   // don't want to process while command information is coming in

    if (active) {
      for (i = outConnections.begin(); i != outConnections.end(); ++i) {
	try {
	  i->first->pushSRI(H);
	} catch(...) {
	  LOG_ERROR( logger, "PUSH-SRI FAILED, PORT/CONNECTION: " << name << "/" << i->second );
	}
      }
    }

    currentSRIs[std::string(H.streamID)] = std::make_pair(H,false) ;
    refreshSRI = false;

    TRACE_EXIT(logger, "OutPort::pushSRI" );
    return;
  }


  template < typename PortTraits >
  void OutPort< PortTraits >::pushPacket( NativeSequenceType & data, BULKIO::PrecisionUTCTime& T, bool EOS, const std::string& streamID) {

    TRACE_ENTER(logger, "OutPort::pushPacket" );

    if (refreshSRI) {
      if (currentSRIs.find(streamID) != currentSRIs.end()) {
	pushSRI(currentSRIs[streamID].first);
      }
    }
    SCOPED_LOCK lock(updatingPortsLock);   
    // don't want to process while command information is coming in
    // Magic is below, make a new sequence using the data from the Iterator
    // as the data for the sequence.  The 'false' at the end is whether or not
    // CORBA is allowed to delete the buffer when the sequence is destroyed.
    PortSequenceType seq = PortSequenceType(data.size(), data.size(), (TransportType *)&(data[0]), false);
    if (active) {
      typename  ConnectionsList::iterator port;
      for (port = outConnections.begin(); port != outConnections.end(); port++) {
	try {
	  port->first->pushPacket(seq, T, EOS, streamID.c_str());
	  stats[(*port).second].update(data.size(), 0, EOS, streamID);
	} catch(...) {
	  LOG_ERROR( logger, "PUSH-PACKET FAILED, PORT/CONNECTION: " << name << "/" << port->second );
	}
      }
    }
    
    // if we have end of stream removed old sri
    try {
      if ( EOS ) currentSRIs.erase(streamID);
    }
    catch(...){
    }


    TRACE_EXIT(logger, "OutPort::pushPacket" );
  }


  template < typename PortTraits >
  void OutPort< PortTraits >::pushPacket( const DataBufferType & data, BULKIO::PrecisionUTCTime& T, bool EOS, const std::string& streamID) {

    TRACE_ENTER(logger, "OutPort::pushPacket" );

    if (refreshSRI) {
      if (currentSRIs.find(streamID) != currentSRIs.end()) {
	pushSRI(currentSRIs[streamID].first);
      }
    }
    SCOPED_LOCK lock(updatingPortsLock);   // don't want to process while command information is coming in
    // Magic is below, make a new sequence using the data from the Iterator
    // as the data for the sequence.  The 'false' at the end is whether or not
    // CORBA is allowed to delete the buffer when the sequence is destroyed.
    PortSequenceType seq = PortSequenceType(data.size(), data.size(), (TransportType *)&(data[0]), false);
    if (active) {
      typename  ConnectionsList::iterator port;
      for (port = outConnections.begin(); port != outConnections.end(); port++) {
	try {
	  port->first->pushPacket(seq, T, EOS, streamID.c_str());
	  if ( stats.count((*port).second) == 0 ) {
	    stats.insert( std::pair< std::string, linkStatistics>((*port).second, linkStatistics( name, sizeof(TransportType) ) ) );
	  }
	  stats[(*port).second].update(data.size(), 0, EOS, streamID);
	} catch(...) {
	  LOG_ERROR( logger, "bulkio::OutPort PUSH-PACKET FAILED, PORT/NECTION: " << name << "/" << port->second );
	}
      }
    }

    // if we have end of stream removed old sri
    try {
      if ( EOS ) currentSRIs.erase(streamID);
    }
    catch(...){
    }


    TRACE_EXIT(logger, "OutPort::pushPacket" );
  }




  template < typename PortTraits >
  BULKIO::UsesPortStatisticsSequence *  OutPort< PortTraits >::statistics()
  {
    SCOPED_LOCK   lock(updatingPortsLock);
    BULKIO::UsesPortStatisticsSequence_var recStat = new BULKIO::UsesPortStatisticsSequence();
    recStat->length(outConnections.size());
    for (unsigned int i = 0; i < outConnections.size(); i++) {
      recStat[i].connectionId = CORBA::string_dup(outConnections[i].second.c_str());
      recStat[i].statistics = stats[outConnections[i].second].retrieve();
    }
    return recStat._retn();
  }

  template < typename PortTraits >
  BULKIO::PortUsageType OutPort< PortTraits >::state()
  {
    SCOPED_LOCK lock(updatingPortsLock);
    if (outConnections.size() > 0) {
      return BULKIO::ACTIVE;
    } else {
      return BULKIO::IDLE;
    }

    return BULKIO::BUSY;
  }

  template < typename PortTraits >
  void OutPort< PortTraits >::enableStats(bool enable)
  {
    for (unsigned int i = 0; i < outConnections.size(); i++) {
      stats[outConnections[i].second].setEnabled(enable);
    }
  }


  template < typename PortTraits >
  ExtendedCF::UsesConnectionSequence * OutPort< PortTraits >::connections()
  {
    SCOPED_LOCK lock(updatingPortsLock);   // don't want to process while command information is coming in
    if (recConnectionsRefresh) {
      recConnections.length(outConnections.size());
      for (unsigned int i = 0; i < outConnections.size(); i++) {
	recConnections[i].connectionId = CORBA::string_dup(outConnections[i].second.c_str());
	recConnections[i].port = CORBA::Object::_duplicate(outConnections[i].first);
      }
      recConnectionsRefresh = false;
    }
    ExtendedCF::UsesConnectionSequence_var retVal = new ExtendedCF::UsesConnectionSequence(recConnections);
    // NOTE: You must delete the object that this function returns!
    return retVal._retn();
  }

  template < typename PortTraits >
  void OutPort< PortTraits >::connectPort(CORBA::Object_ptr connection, const char* connectionId)
  {
    TRACE_ENTER(logger, "OutPort::connectPort" );
    {
      SCOPED_LOCK lock(updatingPortsLock);   // don't want to process while command information is coming in
      PortVarType port;
      try {
	port = PortType::_narrow(connection);
      }
      catch(...) {
	LOG_ERROR( logger, "CONNECT FAILED: UNABLE TO NARROW ENDPOINT,  USES PORT:" << name );
	throw CF::Port::InvalidPort(1, "Unable to narrow");
      }
      outConnections.push_back(std::make_pair(port, connectionId));
      active = true;
      recConnectionsRefresh = true;
      refreshSRI = true;

      LOG_DEBUG( logger, "CONNECTION ESTABLISHED,  PORT/CONNECTION_ID:" << name << "/" << connectionId );
    }
    if (_connectCB) (*_connectCB)(connectionId);

    TRACE_EXIT(logger, "OutPort::connectPort" );
  }


  template < typename PortTraits >
  void OutPort< PortTraits >::disconnectPort(const char* connectionId)
  {
    TRACE_ENTER(logger, "OutPort::disconnectPort" );
    {
      SCOPED_LOCK lock(updatingPortsLock);   // don't want to process while command information is coming in
      for (unsigned int i = 0; i < outConnections.size(); i++) {
        if (outConnections[i].second == connectionId) {
	  LOG_DEBUG( logger, "DISCONNECT, PORT/CONNECTION: "  << name << "/" << connectionId );
	  outConnections.erase(outConnections.begin() + i);
	  stats.erase( outConnections[i].second );
	  break;
        }
      }

      if (outConnections.size() == 0) {
        active = false;
      }
      recConnectionsRefresh = true;
    }
    if (_disconnectCB) (*_disconnectCB)(connectionId);

    TRACE_EXIT(logger, "OutPort::disconnectPort" );
  }

  template < typename PortTraits >
  SriMap  OutPort< PortTraits >::getCurrentSRI() 
  {
      SCOPED_LOCK lock(updatingPortsLock);   // restrict access till method completes
      return currentSRIs;
  }

  template < typename PortTraits >
  typename OutPort< PortTraits >::ConnectionsList  OutPort< PortTraits >::getConnections()
  {
      SCOPED_LOCK lock(updatingPortsLock);   // restrict access till method completes
      return outConnections;
  }


  template < typename PortTraits >
  OutInt8Port< PortTraits >::OutInt8Port( std::string name, 
					  ConnectionEventListener *connectCB,
					  ConnectionEventListener *disconnectCB ):
    OutPort < PortTraits >(name,connectCB, disconnectCB )
  {

  }


  template < typename PortTraits >
  OutInt8Port< PortTraits >::OutInt8Port( std::string name, 
					  LOGGER_PTR logger, 
					  ConnectionEventListener *connectCB,
					  ConnectionEventListener *disconnectCB ) :
    OutPort < PortTraits >(name, logger, connectCB, disconnectCB )
  {

  }


  template <typename PortTraits>
  void OutInt8Port< PortTraits >::pushPacket( std::vector< Int8 >& data, BULKIO::PrecisionUTCTime& T, bool EOS, const std::string& streamID) {

    TRACE_ENTER(this->logger, "OutInt8Port::pushPacket" );

    if (  this->refreshSRI) {
      if (this->currentSRIs.find(streamID) != this->currentSRIs.end()) {
	this->pushSRI(this->currentSRIs[streamID].first);
      }
    }
    SCOPED_LOCK lock(this->updatingPortsLock);   // don't want to process while command information is coming in
    // Magic is below, make a new sequence using the data from the Iterator
    // as the data for the sequence.  The 'false' at the end is whether or not
    // CORBA is allowed to delete the buffer when the sequence is destroyed.
    PortTypes::CharSequence seq = PortTypes::CharSequence(data.size(), data.size(), (CORBA::Char *)&(data[0]), false);
    if (this->active) {
      typename  OutInt8Port::ConnectionsList::iterator port;
      for (port = this->outConnections.begin(); port != this->outConnections.end(); port++) {
	try {
	  port->first->pushPacket(seq, T, EOS, streamID.c_str());
	  if ( this->stats.count((*port).second) == 0 ) {
	    this->stats.insert( std::pair< std::string, linkStatistics>((*port).second, linkStatistics( this->name, sizeof(Int8) ) ) );
	  }

	  this->stats[(*port).second].update(data.size(), 0, 0, streamID);
	} catch(...) {
	  LOG_ERROR( this->logger, "PUSH-PACKET FAILED, PORT/CONNECTION: " << this->name << "/" << port->second );
	}
      }
    }

    // if we have end of stream removed old sri
    try {
      if ( EOS ) this->currentSRIs.erase(streamID);
    }
    catch(...){
    }


    TRACE_EXIT(this->logger, "OutInt8Port::pushPacket" );
  }


  template <typename PortTraits>
  void OutInt8Port< PortTraits >::pushPacket( std::vector< Char >& data, BULKIO::PrecisionUTCTime& T, bool EOS, const std::string& streamID) {

    TRACE_ENTER(this->logger, "OutInt8Port::pushPacket" );

    if (  this->refreshSRI) {
      if (this->currentSRIs.find(streamID) != this->currentSRIs.end()) {
	this->pushSRI(this->currentSRIs[streamID].first);
      }
    }
    SCOPED_LOCK lock(this->updatingPortsLock);   // don't want to process while command information is coming in
    // Magic is below, make a new sequence using the data from the Iterator
    // as the data for the sequence.  The 'false' at the end is whether or not
    // CORBA is allowed to delete the buffer when the sequence is destroyed.
    PortTypes::CharSequence seq = PortTypes::CharSequence(data.size(), data.size(), (CORBA::Char *)&(data[0]), false);
    if (this->active) {
      typename  OutInt8Port::ConnectionsList::iterator port;
      for (port = this->outConnections.begin(); port != this->outConnections.end(); port++) {
	try {
	  port->first->pushPacket(seq, T, EOS, streamID.c_str());
	  if ( this->stats.count((*port).second) == 0 ) {
	    this->stats.insert( std::pair< std::string, linkStatistics>((*port).second, linkStatistics( this->name, sizeof(Int8) ) ) );
	  }

	  this->stats[(*port).second].update(data.size(), 0, 0, streamID);
	} catch(...) {
	  LOG_ERROR( this->logger, "PUSH-PACKET FAILED, PORT/CONNECTION: " << this->name << "/" << port->second );
	}
      }
    }

    // if we have end of stream removed old sri
    try {
      if ( EOS ) this->currentSRIs.erase(streamID);
    }
    catch(...){
    }


    TRACE_EXIT(this->logger, "OutInt8Port::pushPacket" );
  }



  template <typename PortTraits>
  OutStringPort< PortTraits >::OutStringPort ( std::string name, 
					       ConnectionEventListener *connectCB,
					       ConnectionEventListener *disconnectCB ) :
    OutPort < PortTraits >(name,connectCB, disconnectCB )
  {

  }


  template <typename PortTraits>
  OutStringPort< PortTraits >::OutStringPort( std::string name, 
					      LOGGER_PTR logger,
					      ConnectionEventListener *connectCB,
					      ConnectionEventListener *disconnectCB ) :
    OutPort < PortTraits >(name,logger,connectCB, disconnectCB )
  {

  }


  template <typename PortTraits>
  void OutStringPort< PortTraits >::pushPacket( const char *data, BULKIO::PrecisionUTCTime& T, bool EOS, const std::string& streamID) {


    TRACE_ENTER(this->logger, "OutStringPort::pushPacket" );

    if (this->refreshSRI) {
      if (this->currentSRIs.find(streamID) != this->currentSRIs.end()) {
	pushSRI(this->currentSRIs[streamID].first);
      }
    }
    SCOPED_LOCK lock(this->updatingPortsLock);   // don't want to process while command information is coming in
    if (this->active) {
      typename OutPort< PortTraits >::ConnectionsList::iterator port;
      for (port = this->outConnections.begin(); port != this->outConnections.end(); port++) {
	try {
	  port->first->pushPacket(data, T, EOS, streamID.c_str());
	  if ( this->stats.count((*port).second) == 0 ) {
	    this->stats.insert( std::pair< std::string, linkStatistics>((*port).second, linkStatistics( this->name, sizeof(char) ) ) );
	  }
	  this->stats[(*port).second].update(1, 0, EOS, streamID);
	} catch(...) {
	  LOG_ERROR( this->logger, "PUSH-PACKET FAILED, PORT/CONNECTION: " << this->name << "/" << port->second );
	}
      }
    }

    // if we have end of stream removed old sri
    try {
      if ( EOS ) this->currentSRIs.erase(streamID);
    }
    catch(...){
    }


    TRACE_EXIT(this->logger, "OutStringPort::pushPacket" );

  }


  template <typename PortTraits>
  void OutStringPort< PortTraits >::pushPacket( const char *data, bool EOS, const std::string& streamID) {

    TRACE_ENTER(this->logger, "OutStringPort::pushPacket" );

    if (this->refreshSRI) {
      if (this->currentSRIs.find(streamID) != this->currentSRIs.end()) {
	pushSRI(this->currentSRIs[streamID].first);
      }
    }
    SCOPED_LOCK lock(this->updatingPortsLock);   // don't want to process while command information is coming in
    if (this->active) {
      typename OutPort< PortTraits >::ConnectionsList::iterator port;
      for (port = this->outConnections.begin(); port != this->outConnections.end(); port++) {
	try {
	  BULKIO::PrecisionUTCTime tstamp = bulkio::time::utils::now();
	  port->first->pushPacket(data, tstamp, EOS, streamID.c_str());
	  if ( this->stats.count((*port).second) == 0 ) {
	    this->stats.insert( std::pair< std::string, linkStatistics>((*port).second, linkStatistics( this->name, sizeof(char) ) ) );
	  }
	  this->stats[(*port).second].update(1, 0, EOS, streamID);
	} catch(...) {
	  LOG_ERROR( this->logger, "PUSH-PACKET FAILED, PORT/CONNECTION: " << this->name << "/" << port->second );
	}
      }
    }

    // if we have end of stream removed old sri
    try {
      if ( EOS ) this->currentSRIs.erase(streamID);
    }
    catch(...){
    }

    TRACE_EXIT(this->logger, "OutStringPort::pushPacket" );

  }



  template <>
  void OutStringPort< XMLPortTraits >::pushPacket( const char *data, BULKIO::PrecisionUTCTime& T, bool EOS, const std::string& streamID) {

    TRACE_ENTER(this->logger, "OutStringPort::pushPacket" );
    if (this->refreshSRI) {
      if (this->currentSRIs.find(streamID) != this->currentSRIs.end()) {
	pushSRI(this->currentSRIs[streamID].first);
      }
    }
    SCOPED_LOCK lock(this->updatingPortsLock);   // don't want to process while command information is coming in
    if (this->active) {
      OutPort< XMLPortTraits >::ConnectionsList::iterator port;
      for (port = this->outConnections.begin(); port != this->outConnections.end(); port++) {
	try {
	  port->first->pushPacket(data, EOS, streamID.c_str());
	  this->stats[(*port).second].update(1, 0, EOS, streamID);
	} catch(...) {
	  LOG_ERROR( this->logger, "PUSH-PACKET FAILED, PORT/CONNECTION: " << this->name << "/" << port->second );
	}
      }
    }

    // if we have end of stream removed old sri
    try {
      if ( EOS ) this->currentSRIs.erase(streamID);
    }
    catch(...){
    }

    TRACE_EXIT(this->logger, "OutStringPort::pushPacket" );

  }

  template <>
  void OutStringPort<  XMLPortTraits  >::pushPacket( const char *data, bool EOS, const std::string& streamID) {

    TRACE_ENTER(this->logger, "OutStringPort::pushPacket" );
    if (this->refreshSRI) {
      if (this->currentSRIs.find(streamID) != this->currentSRIs.end()) {
	pushSRI(this->currentSRIs[streamID].first);
      }
    }
    SCOPED_LOCK lock(this->updatingPortsLock);   // don't want to process while command information is coming in
    if (this->active) {
      OutPort< XMLPortTraits >::ConnectionsList::iterator port;
      for (port = this->outConnections.begin(); port != this->outConnections.end(); port++) {
	try {
	  port->first->pushPacket(data, EOS, streamID.c_str());
	  this->stats[(*port).second].update(strlen(data), 0, EOS, streamID);
	} catch(...) {
	  LOG_ERROR( this->logger, "PUSH-PACKET FAILED, PORT/CONNECTION: " << this->name << "/" << port->second );
	}
      }
    }

    // if we have end of stream removed old sri
    try {
      if ( EOS ) this->currentSRIs.erase(streamID);
    }
    catch(...){
    }


    TRACE_EXIT(this->logger, "OutStringPort::pushPacket" );
  }


  //
  // Required for Template Instantion for the compilation unit.
  // Note: we only define those valid types for which Bulkio IDL is defined. Users wanting to
  // inherit this functionality will be unable to since they cannot instantiate and
  // link against the template.
  //

  template class OutInt8Port<  CharPortTraits >;
  template class OutPort<  OctetPortTraits >;
  template class OutPort<  ShortPortTraits >;
  template class OutPort<  UShortPortTraits >;
  template class OutPort<  LongPortTraits >;
  template class OutPort<  ULongPortTraits >;
  template class OutPort<  LongLongPortTraits >;
  template class OutPort<  ULongLongPortTraits >;
  template class OutPort<  FloatPortTraits >;
  template class OutPort<  DoublePortTraits >;
  template class OutStringPort< FilePortTraits >;
  template class OutStringPort<  XMLPortTraits >;



} // end of bulkio namespace
