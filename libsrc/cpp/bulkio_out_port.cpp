
/*******************************************************************************************


 *******************************************************************************************/
#include <bulkio_p.h>
#include <uuid/uuid.h>

// Suppress warnings for access to "deprecated" currentSRI member--it's the
// public access that's deprecated, not the member itself
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

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
    refreshSRI = true;
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
      bool portListed = false;
      std::vector<connection_descriptor_struct >::iterator ftPtr;
      for (i = outConnections.begin(); i != outConnections.end(); ++i) {
        for (ftPtr=filterTable.begin(); ftPtr!= filterTable.end(); ftPtr++) {

          if (ftPtr->port_name == this->name) portListed=true;

          if ( (ftPtr->port_name == this->name) and 
	       (ftPtr->connection_id == i->second) and 
	       (strcmp(ftPtr->stream_id.c_str(),H.streamID) == 0 ) ){
	    LOG_DEBUG(logger,"pushSRI - PORT:" << name << " CONNECTION:" << ftPtr->connection_id << " SRI:" << H.streamID << " Mode:" << H.mode << " XDELTA:" << 1.0/H.xdelta );  
            try {
              i->first->pushSRI(H);
            } catch(...) {
              LOG_ERROR( logger, "PUSH-SRI FAILED, PORT/CONNECTION: " << name << "/" << i->second );
            }
          }
        }
      }

      if (!portListed) {
	for (i = outConnections.begin(); i != outConnections.end(); ++i) {
	  LOG_DEBUG(logger,"pushSRI -2- PORT:" << name << " CONNECTION:" << ftPtr->connection_id << " SRI:" << H.streamID << " Mode:" << H.mode << " XDELTA:" << 1.0/H.xdelta );  
	  try {
	    i->first->pushSRI(H);
	  } catch(...) {
	    LOG_ERROR( logger, "PUSH-SRI FAILED, PORT/CONNECTION: " << name << "/" << i->second );
	  }
	}
      }
    }

    currentSRIs[std::string(H.streamID)] = std::make_pair(H,false) ;
    refreshSRI = false;

    TRACE_EXIT(logger, "OutPort::pushSRI" );
    return;
  }

  /**
   * Push a packet whose payload cannot fit within the CORBA limit.
   * The packet is broken down into sub-packets and sent via multiple pushPacket
   * calls.  The EOS is set to false for all of the sub-packets, except for
   * the last sub-packet, who uses the input EOS argument.
   */
  template < typename PortTraits >
  void OutPort< PortTraits>::_pushOversizedPacket(
          const DataBufferType &      data,
          BULKIO::PrecisionUTCTime& T,
          bool                      EOS,
          const std::string&        streamID)
  {
      // Multiply by some number < 1 to leave some margin for the CORBA header
      const size_t maxPayloadSize    = (size_t) (bulkio::Const::MaxTransferBytes() * .9);

      const size_t maxSamplesPerPush = maxPayloadSize/sizeof(TransportType);
      
      // Use const alias to point to the start of the current sub-packet's data
      const TransportType* buffer = static_cast<const TransportType*>(&data[0]);

      // Always do at least one push (may be empty), ensuring that all samples
      // are pushed
      size_t samplesRemaining = data.size();
      do {
          // Don't send more samples than are remaining
          const size_t pushSize = std::min(samplesRemaining, maxSamplesPerPush);
          samplesRemaining -= pushSize;

          // Send end-of-stream as false for all sub-packets except for the
          // last one (when there are no samples remaining after this push),
          // which gets the input EOS.
          bool packetEOS = false;
          if (samplesRemaining == 0) {
              packetEOS = EOS;
          }

          // Wrap a non-owning CORBA sequence (last argument is whether to free
          // the buffer on destruction) around this sub-packet's data
          const PortSequenceType subPacket(pushSize, pushSize, const_cast<TransportType*>(buffer), false);
          _pushPacket(subPacket, T, packetEOS, streamID);

          // Advance buffer to next sub-packet boundary
          buffer += pushSize;
      } while (samplesRemaining > 0);
  }


  template < typename PortTraits >
  void OutPort< PortTraits >::pushPacket(
          NativeSequenceType &      data,
          BULKIO::PrecisionUTCTime& T,
          bool                      EOS,
          const std::string&        streamID) {

    TRACE_ENTER(logger, "OutPort::pushPacket" );

    if (refreshSRI) {
      if (currentSRIs.find(streamID) != currentSRIs.end()) {
          pushSRI(currentSRIs[streamID].first);
      } else {
          BULKIO::StreamSRI tmpH = {1, 0.0, 1.0, 1, 0, 0.0, 0.0, 0, 0, streamID.c_str(), false, 0};
          currentSRIs[std::string(tmpH.streamID)] = std::make_pair(tmpH,false) ;
          pushSRI(currentSRIs[streamID].first);
      }
    }

    // don't want to process while command information is coming in
    SCOPED_LOCK lock(updatingPortsLock);
    _pushOversizedPacket(data, T, EOS, streamID);

    TRACE_EXIT(logger, "OutPort::pushPacket" );
  }

  template < typename PortTraits >
  void OutPort< PortTraits >::_pushPacket(
          const PortSequenceType &    data,
          BULKIO::PrecisionUTCTime& T,
          bool                      EOS,
          const std::string&        streamID) {
      typename  ConnectionsList::iterator port;
      if (active) {
	bool portListed = false;
        std::vector<connection_descriptor_struct >::iterator ftPtr;
        for (port = outConnections.begin(); port != outConnections.end(); port++) {
          for (ftPtr=filterTable.begin(); ftPtr != filterTable.end(); ftPtr++) {

	    if  (ftPtr->port_name == this->name) portListed = true;

	    if ( (ftPtr->port_name == this->name) and 
		 (ftPtr->connection_id == port->second) and 
		 (ftPtr->stream_id == streamID) ){
              try {
		port->first->pushPacket(data, T, EOS, streamID.c_str());
                stats[port->second].update(data.length(), 0, EOS, streamID);
              } catch(...) {
                LOG_ERROR( logger, "PUSH-PACKET FAILED, PORT/CONNECTION: " << name << "/" << port->second );
              }
            }
          }
        }
	if (!portListed) {
	  for (port = outConnections.begin(); port != outConnections.end(); port++) {
            try {
	      port->first->pushPacket(data, T, EOS, streamID.c_str());
	      stats[port->second].update(data.length(), 0, EOS, streamID);
            } catch(...) {
	      LOG_ERROR( logger, "PUSH-PACKET FAILED, PORT/CONNECTION: " << name << "/" << port->second );
            }
          }
        }
      }

      // if we have end of stream removed old sri
      try {
        if ( EOS ) currentSRIs.erase(streamID);
      }
      catch(...){
      }

  }
  template < typename PortTraits >
  void OutPort< PortTraits >::pushPacket(
          const DataBufferType &    data,
          BULKIO::PrecisionUTCTime& T,
          bool                      EOS,
          const std::string&        streamID) {

    TRACE_ENTER(logger, "OutPort::pushPacket" );

    if (refreshSRI) {
      if (currentSRIs.find(streamID) != currentSRIs.end()) {
        pushSRI(currentSRIs[streamID].first);
      } else {
          BULKIO::StreamSRI tmpH = {1, 0.0, 1.0, 1, 0, 0.0, 0.0, 0, 0, streamID.c_str(), false, 0};
          currentSRIs[std::string(tmpH.streamID)] = std::make_pair(tmpH,false) ;
          pushSRI(currentSRIs[streamID].first);
      }
    }

    // don't want to process while command information is coming in
    SCOPED_LOCK lock(updatingPortsLock);
    _pushOversizedPacket(data, T, EOS, streamID);

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
        if (CORBA::is_nil(port)) {
            throw CF::Port::InvalidPort(1, "Unable to narrow");
        }
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
      bool portListed = false;
      std::vector<connection_descriptor_struct >::iterator ftPtr;
      for (ftPtr=this->filterTable.begin(); ftPtr != this->filterTable.end(); ftPtr++) {
        if (ftPtr->port_name == this->name) {
          portListed = true;
          break;
        }
      }
      for (unsigned int i = 0; i < outConnections.size(); i++) {
        if (outConnections[i].second == connectionId) {
          PortSequenceType seq;
          SriMap::iterator cSRIs = currentSRIs.begin();
          BULKIO::PrecisionUTCTime tstamp = bulkio::time::utils::now();
          // send an EOS for every connection that's listed on the connection table
          for (SriMap::iterator cSRIs=currentSRIs.begin(); cSRIs!=currentSRIs.end(); cSRIs++) {
            std::string cSriSid = ossie::corba::returnString(cSRIs->second.first.streamID);
            StreamIDList aSIDs = stats[outConnections[i].second].getActiveStreamIDs();
            for (StreamIDList::iterator aSID=aSIDs.begin(); aSID!=aSIDs.end(); aSID++) {
              if (cSriSid == (*aSID)) {
                if (portListed) {
                  for (ftPtr=this->filterTable.begin(); ftPtr != this->filterTable.end(); ftPtr++) {
                    if ((ftPtr->port_name == this->name) and (ftPtr->connection_id == outConnections[i].second) and (ftPtr->stream_id == cSriSid)) {
                      try {
                        outConnections[i].first->pushPacket(seq, tstamp, true, cSriSid.c_str());
                      } catch(...) {}
                    }
                  }
                } else {
                  outConnections[i].first->pushPacket(seq, tstamp, true, cSriSid.c_str());
                }
              }
            }
          }
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
  void OutPort< PortTraits >::setLogger( LOGGER_PTR newLogger ) {
      logger = newLogger;
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
      bool portListed = false;
      typename  OutInt8Port::ConnectionsList::iterator port;
      std::vector<connection_descriptor_struct >::iterator ftPtr;
      for (port = this->outConnections.begin(); port != this->outConnections.end(); port++) {
        for (ftPtr=this->filterTable.begin(); ftPtr != this->filterTable.end(); ftPtr++) {

          if (ftPtr->port_name == this->name)  portListed = true;

          if ((ftPtr->port_name == this->name) and (ftPtr->connection_id == port->second) and (ftPtr->stream_id == streamID)) {
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
      }
      if (!portListed) {
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
      bool portListed = false;
      typename  OutInt8Port::ConnectionsList::iterator port;
      std::vector<connection_descriptor_struct >::iterator ftPtr;
      for (port = this->outConnections.begin(); port != this->outConnections.end(); port++) {
        for (ftPtr=this->filterTable.begin(); ftPtr != this->filterTable.end(); ftPtr++) {

          if (ftPtr->port_name == this->name) portListed = true;

          if ((ftPtr->port_name == this->name) and (ftPtr->connection_id == port->second) and (ftPtr->stream_id == streamID)) {
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
      }
      if (!portListed) {
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


  template < typename PortTraits >
  void OutStringPort< PortTraits >::disconnectPort(const char* connectionId)
  {
    TRACE_ENTER(this->logger, "OutStringPort::disconnectPort" );
    {
        SCOPED_LOCK lock(this->updatingPortsLock);   // don't want to process while command information is coming in
        std::vector<connection_descriptor_struct >::iterator ftPtr;
        bool portListed = false;
        for (ftPtr=this->filterTable.begin(); ftPtr != this->filterTable.end(); ftPtr++) {
            if (ftPtr->port_name == this->name) {
                portListed = true;
                break;
            }
        }
        for (unsigned int i = 0; i < this->outConnections.size(); i++) {
            if (this->outConnections[i].second == connectionId) {
                std::string data("");
                SriMap::iterator cSRIs = this->currentSRIs.begin();
                BULKIO::PrecisionUTCTime tstamp = bulkio::time::utils::now();
                // send an EOS for every connection that's listed on the connection table
                for (SriMap::iterator cSRIs=this->currentSRIs.begin(); cSRIs!=this->currentSRIs.end(); cSRIs++) {
                    std::string cSriSid = ossie::corba::returnString(cSRIs->second.first.streamID);
                    StreamIDList aSIDs = this->stats[this->outConnections[i].second].getActiveStreamIDs();
                    for (StreamIDList::iterator aSID=aSIDs.begin(); aSID!=aSIDs.end(); aSID++) {
                        if (cSriSid == (*aSID)) {
                            if (portListed) {
                                for (ftPtr=this->filterTable.begin(); ftPtr != this->filterTable.end(); ftPtr++) {
                                    if ((ftPtr->port_name == this->name) and (ftPtr->connection_id == this->outConnections[i].second) and (ftPtr->stream_id == cSriSid)) {
                                        try {
                                            this->outConnections[i].first->pushPacket(data.c_str(), tstamp, true, cSriSid.c_str());
                                        } catch(...) {}
                                    }
                                }
                            } else {
                                try {
                                    this->outConnections[i].first->pushPacket(data.c_str(), tstamp, true, cSriSid.c_str());
                                } catch(...) {}
                            }
                        }
                    }
                }
                LOG_DEBUG( this->logger, "DISCONNECT, PORT/CONNECTION: "  << this->name << "/" << connectionId );
                this->outConnections.erase(this->outConnections.begin() + i);
                this->stats.erase( this->outConnections[i].second );
                break;
            }
        }

        if (this->outConnections.size() == 0) {
            this->active = false;
        }
        this->recConnectionsRefresh = true;
    }
    if (this->_disconnectCB) (*this->_disconnectCB)(connectionId);

    TRACE_EXIT(this->logger, "OutStringPort::disconnectPort" );
  }

  template <typename PortTraits>
  void OutStringPort< PortTraits >::pushPacket( const char *data, BULKIO::PrecisionUTCTime& T, bool EOS, const std::string& streamID) {


    TRACE_ENTER(this->logger, "OutStringPort::pushPacket" );

    if (this->refreshSRI) {
      if (this->currentSRIs.find(streamID) != this->currentSRIs.end()) {
        this->pushSRI(this->currentSRIs[streamID].first);
      }
    }
    SCOPED_LOCK lock(this->updatingPortsLock);   // don't want to process while command information is coming in
    if (this->active) {
      bool portListed = false;
      typename OutPort< PortTraits >::ConnectionsList::iterator port;
      std::vector<connection_descriptor_struct >::iterator ftPtr;
      for (port = this->outConnections.begin(); port != this->outConnections.end(); port++) {
        for (ftPtr=this->filterTable.begin(); ftPtr != this->filterTable.end(); ftPtr++) {

          if (ftPtr->port_name == this->name) portListed=true;

          if ((ftPtr->port_name == this->name) and (ftPtr->connection_id == port->second) and (ftPtr->stream_id == streamID)) {
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
      }
      if (!portListed) {
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
        this->pushSRI(this->currentSRIs[streamID].first);
      }
    }
    SCOPED_LOCK lock(this->updatingPortsLock);   // don't want to process while command information is coming in
    if (this->active) {
      bool portListed = false;
      typename OutPort< PortTraits >::ConnectionsList::iterator port;
      std::vector<connection_descriptor_struct >::iterator ftPtr;
      for (port = this->outConnections.begin(); port != this->outConnections.end(); port++) {
        for (ftPtr=this->filterTable.begin(); ftPtr != this->filterTable.end(); ftPtr++) {

          if (ftPtr->port_name == this->name) portListed = true;

          if ((ftPtr->port_name == this->name) and (ftPtr->connection_id == port->second) and (ftPtr->stream_id == streamID)) {
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
      }
      if (!portListed) {
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
  void OutPort< XMLPortTraits >::disconnectPort(const char* connectionId) {
      TRACE_ENTER(this->logger, "OutPort::disconnectPort" );
    {
        SCOPED_LOCK lock(this->updatingPortsLock);   // don't want to process while command information is coming in
        std::vector<connection_descriptor_struct >::iterator ftPtr;
        bool portListed = false;
        for (ftPtr=this->filterTable.begin(); ftPtr != this->filterTable.end(); ftPtr++) {
            if (ftPtr->port_name == this->name) {
                portListed = true;
                break;
            }
        }
        for (unsigned int i = 0; i < this->outConnections.size(); i++) {
            if (this->outConnections[i].second == connectionId) {
                std::string data("");
                SriMap::iterator cSRIs = this->currentSRIs.begin();
                // send an EOS for every connection that's listed on the connection table
                for (SriMap::iterator cSRIs=this->currentSRIs.begin(); cSRIs!=this->currentSRIs.end(); cSRIs++) {
                    std::string cSriSid = ossie::corba::returnString(cSRIs->second.first.streamID);
                    StreamIDList aSIDs = this->stats[this->outConnections[i].second].getActiveStreamIDs();
                    for (StreamIDList::iterator aSID=aSIDs.begin(); aSID!=aSIDs.end(); aSID++) {
                        if (cSriSid == (*aSID)) {
                            if (portListed) {
                                for (ftPtr=this->filterTable.begin(); ftPtr != this->filterTable.end(); ftPtr++) {
                                    if ((ftPtr->port_name == this->name) and (ftPtr->connection_id == outConnections[i].second) and (ftPtr->stream_id == cSriSid)) {
                                        try {
                                            outConnections[i].first->pushPacket(data.c_str(), true, cSriSid.c_str());
                                        } catch(...) {}
                                    }
                                }
                            } else {
                                try {
                                    outConnections[i].first->pushPacket(data.c_str(), true, cSriSid.c_str());
                                } catch(...) {}
                            }
                        }
                    }
                }
                LOG_DEBUG( this->logger, "DISCONNECT, PORT/CONNECTION: "  << this->name << "/" << connectionId );
                this->outConnections.erase(this->outConnections.begin() + i);
                this->stats.erase( this->outConnections[i].second );
                break;
            }
        }
        
        if (this->outConnections.size() == 0) {
            this->active = false;
        }
        this->recConnectionsRefresh = true;
    }
    if (this->_disconnectCB) (*this->_disconnectCB)(connectionId);
    
    TRACE_EXIT(this->logger, "OutStringPort::disconnectPort" );
  }
  
  template <>
  void OutStringPort< XMLPortTraits >::disconnectPort(const char* connectionId) {
      OutPort< XMLPortTraits >::disconnectPort(connectionId);
  }
  
  template <>
  void OutPort< FilePortTraits >::disconnectPort(const char* connectionId) {
      TRACE_ENTER(this->logger, "OutPort::disconnectPort" );
      {
          SCOPED_LOCK lock(this->updatingPortsLock);   // don't want to process while command information is coming in
          BULKIO::PrecisionUTCTime tstamp = bulkio::time::utils::now();
          for (unsigned int i = 0; i < this->outConnections.size(); i++) {
              if (this->outConnections[i].second == connectionId) {
                  std::string data("");
                  SriMap::iterator cSRIs = this->currentSRIs.begin();
                  // send an EOS for every connection that's listed on the connection table
                  for (SriMap::iterator cSRIs=this->currentSRIs.begin(); cSRIs!=this->currentSRIs.end(); cSRIs++) {
                      std::string cSriSid = ossie::corba::returnString(cSRIs->second.first.streamID);
                      StreamIDList aSIDs = this->stats[this->outConnections[i].second].getActiveStreamIDs();
                      for (StreamIDList::iterator aSID=aSIDs.begin(); aSID!=aSIDs.end(); aSID++) {
                          if (cSriSid == (*aSID)) {
                              try {
                                  this->outConnections[i].first->pushPacket(data.c_str(), tstamp, true, cSriSid.c_str());
                              } catch(...) {}
                          }
                      }
                  }
                  LOG_DEBUG( this->logger, "DISCONNECT, PORT/CONNECTION: "  << this->name << "/" << connectionId );
                  this->outConnections.erase(this->outConnections.begin() + i);
                  this->stats.erase( this->outConnections[i].second );
                  break;
              }
          }
          
          if (this->outConnections.size() == 0) {
              this->active = false;
          }
          this->recConnectionsRefresh = true;
      }
      if (this->_disconnectCB) (*this->_disconnectCB)(connectionId);
      
      TRACE_EXIT(this->logger, "OutStringPort::disconnectPort" );
  }
  
  template <>
  void OutStringPort< FilePortTraits >::disconnectPort(const char* connectionId) {
      OutPort< FilePortTraits >::disconnectPort(connectionId);
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
      bool portListed = false;
      OutPort< XMLPortTraits >::ConnectionsList::iterator port;
      std::vector<connection_descriptor_struct >::iterator ftPtr;
      for (port = this->outConnections.begin(); port != this->outConnections.end(); port++) {
        for (ftPtr=filterTable.begin(); ftPtr != filterTable.end(); ftPtr++) {

	  if (ftPtr->port_name == this->name)  portListed = true;

          if ((ftPtr->port_name == this->name) and (ftPtr->connection_id == port->second) and (ftPtr->stream_id == streamID)) {
            try {
              port->first->pushPacket(data, EOS, streamID.c_str());
              this->stats[(*port).second].update(1, 0, EOS, streamID);
            } catch(...) {
              LOG_ERROR( this->logger, "PUSH-PACKET FAILED, PORT/CONNECTION: " << this->name << "/" << port->second );
            }
          }
        }
      }
      if (!portListed) {
	for (port = outConnections.begin(); port != outConnections.end(); port++) {

          try {
            port->first->pushPacket(data, EOS, streamID.c_str());
            this->stats[(*port).second].update(1, 0, EOS, streamID);
          } catch(...) {
            LOG_ERROR( logger, "PUSH-PACKET FAILED, PORT/CONNECTION: " << name << "/" << port->second );
          }
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
      bool portListed = false;
      OutPort< XMLPortTraits >::ConnectionsList::iterator port;
      std::vector<connection_descriptor_struct >::iterator ftPtr;
      for (port = this->outConnections.begin(); port != this->outConnections.end(); port++) {
        for (ftPtr=filterTable.begin(); ftPtr != filterTable.end(); ftPtr++) {
	  if (ftPtr->port_name == this->name) portListed = true;
          if ((ftPtr->port_name == this->name) and (ftPtr->connection_id == port->second) and (ftPtr->stream_id == streamID)) {
            try {
              port->first->pushPacket(data, EOS, streamID.c_str());
              this->stats[(*port).second].update(strlen(data), 0, EOS, streamID);
            } catch(...) {
              LOG_ERROR( this->logger, "PUSH-PACKET FAILED, PORT/CONNECTION: " << this->name << "/" << port->second );
            }
          }
        }
      }
      if (!portListed) {
	for (port = outConnections.begin(); port != outConnections.end(); port++) {
          try {
            port->first->pushPacket(data, EOS, streamID.c_str());
            this->stats[(*port).second].update(strlen(data), 0, EOS, streamID);
          } catch(...) {
            LOG_ERROR( logger, "PUSH-PACKET FAILED, PORT/CONNECTION: " << name << "/" << port->second );
          }
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

  // The base template for certain types cannot be fully instantiated, so explicitly
  // instantiate any methods that are not directly referenced in the subclass (e.g.,
  // setLogger).
#define TEMPLATE_INSTANTIATE_BASE(x) \
  template void OutPort<x>::setLogger(LOGGER_PTR); \
  template void OutPort<x>::setNewConnectListener( ConnectionEventListener * ); \
  template void OutPort<x>::setNewConnectListener( ConnectionEventCallbackFn ); \
  template void OutPort<x>::setNewDisconnectListener( ConnectionEventListener * ); \
  template void OutPort<x>::setNewDisconnectListener( ConnectionEventCallbackFn );

  TEMPLATE_INSTANTIATE_BASE(CharPortTraits);
  TEMPLATE_INSTANTIATE_BASE(FilePortTraits);
  TEMPLATE_INSTANTIATE_BASE(XMLPortTraits);


} // end of bulkio namespace
