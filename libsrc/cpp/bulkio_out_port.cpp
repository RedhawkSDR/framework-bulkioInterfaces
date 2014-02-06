
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
      std::vector<connection_descriptor_struct >::iterator ftPtr;
      for (i = outConnections.begin(); i != outConnections.end(); ++i) {
        for (ftPtr=filterTable.begin(); ftPtr!= filterTable.end(); ftPtr++) {
          if ((ftPtr->port_name == this->name) and (ftPtr->connection_name == i->second) and (strcmp(ftPtr->stream_id.c_str(),H.streamID))){
            try {
              i->first->pushSRI(H);
            } catch(...) {
              LOG_ERROR( logger, "PUSH-SRI FAILED, PORT/CONNECTION: " << name << "/" << i->second );
            }
          }
        }
      }
      for (i = outConnections.begin(); i != outConnections.end(); ++i) {
        bool connectionListed = false;
        for (ftPtr=filterTable.begin(); ftPtr!= filterTable.end(); ftPtr++) {
          if ((ftPtr->port_name == this->name) and (ftPtr->connection_name == i->second)) {
            connectionListed = true;
            break;
          }
        }
        if (!connectionListed) {
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
          NativeSequenceType &      data,
          BULKIO::PrecisionUTCTime& T,
          bool                      EOS,
          const std::string&        streamID){

      // If there is no data to break into smaller packets, skip
      // straight to the pushPacket call and return.
      if (data.size() == 0) {
          _pushPacket(data, T, EOS, streamID);
          return;
      }

      // Multiply by some number < 1 to leave some margin for the CORBA header
      size_t maxPayloadSize    = (size_t) (bulkio::Const::MAX_TRANSFER_BYTES * .9);

      size_t numSamplesPerPush = maxPayloadSize/sizeof(data.front());

      // Determine how many sub-packets to send.
      size_t numFullPackets    = data.size()/numSamplesPerPush;
      size_t lenOfLastPacket   = data.size()%numSamplesPerPush;

      // Send all of the sub-packets of length numSamplesPerPush.
      // Always send EOS false, (the EOS of the parent packet will be sent
      // with the last sub-packet).
      bool intermediateEOS = false;
      unsigned int rowNum;
      for (rowNum = 0; rowNum < numFullPackets; rowNum++) {
          if ( (rowNum == numFullPackets -1) && (lenOfLastPacket == 0)) {
              // This is the last sub-packet.
              intermediateEOS = EOS;
          }

          NativeSequenceType  subPacket(
              data.begin() + rowNum*numSamplesPerPush,
              data.begin() + rowNum*numSamplesPerPush + numSamplesPerPush);

          _pushPacket(subPacket, T, intermediateEOS, streamID);
      }

      if (lenOfLastPacket != 0) {
          // Send the last sub-packet, whose length is less than
          // numSamplesPerPush.  Note that the EOS of the master packet is
          // sent with the last sub-packet.
          NativeSequenceType subPacket(
              data.begin() + numFullPackets*numSamplesPerPush,
              data.begin() + numFullPackets*numSamplesPerPush + lenOfLastPacket);
          _pushPacket(subPacket, T, EOS, streamID);
      }
  }


  template < typename PortTraits >
    void OutPort< PortTraits >::_pushPacket(
            NativeSequenceType &      data,
            BULKIO::PrecisionUTCTime& T,
            bool                      EOS,
            const std::string&        streamID) {

      // Make a new sequence using the data from the Iterator
      // as the data for the sequence.  The 'false' at the end is whether or not
      // CORBA is allowed to delete the buffer when the sequence is destroyed.
      PortSequenceType seq = PortSequenceType(
              data.size(),
              data.size(),
              (TransportType *)&(data[0]),
              false);
      if (active) {
      typename  ConnectionsList::iterator port;
      std::vector<connection_descriptor_struct >::iterator ftPtr;
      for (port = outConnections.begin(); port != outConnections.end(); port++) {
        for (ftPtr=filterTable.begin(); ftPtr != filterTable.end(); ftPtr++) {
            if ((ftPtr->port_name == this->name) and (ftPtr->connection_name == port->second) and (ftPtr->stream_id == streamID)) {
            try {
              port->first->pushPacket(seq, T, EOS, streamID.c_str());
              stats[(*port).second].update(data.size(), 0, EOS, streamID);
            } catch(...) {
              LOG_ERROR( logger, "PUSH-PACKET FAILED, PORT/CONNECTION: " << name << "/" << port->second );
            }
          }
        }
      }
      for (port = outConnections.begin(); port != outConnections.end(); port++) {
        bool connectionListed = false;
        for (ftPtr=filterTable.begin(); ftPtr != filterTable.end(); ftPtr++) {
          if ((ftPtr->port_name == this->name) and (ftPtr->connection_name == port->second)) {
            connectionListed = true;
          }
        }
        if (!connectionListed) {
          try {
              port->first->pushPacket(seq, T, EOS, streamID.c_str());
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
    }
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
      }
    }

    // don't want to process while command information is coming in
    SCOPED_LOCK lock(updatingPortsLock);
    _pushOversizedPacket(data, T, EOS, streamID);

    TRACE_EXIT(logger, "OutPort::pushPacket" );
  }

  /**
   * Push a packet whose payload cannot fit within the CORBA limit.
   * The packet is broken down into sub-packets and sent via multiple pushPacket
   * calls.  The EOS is set to false for all of the sub-packets, except for
   * the last sub-packet, who uses the input EOS argument.
   */
  template < typename PortTraits >
  void OutPort< PortTraits>::_pushOversizedPacket(
          const DataBufferType &    data,
          BULKIO::PrecisionUTCTime& T,
          bool                      EOS,
          const std::string&        streamID){

      // If there is no data to break into smaller packets, skip
      // straight to the pushPacket call and return.
      if (data.size() == 0) {
          _pushPacket(data, T, EOS, streamID);
          return;

      // Multiply by some number < 1 to leave some margin for the CORBA header
      size_t maxPayloadSize    = (size_t) (bulkio::Const::MAX_TRANSFER_BYTES * .9);
      size_t numSamplesPerPush = maxPayloadSize/sizeof(data.front());

      // Determine how many sub-packets to send.
      size_t numFullPackets    = data.size()/numSamplesPerPush;
      size_t lenOfLastPacket   = data.size()%numSamplesPerPush;

      // Send all of the sub-packets of length numSamplesPerPush.
      // Always send EOS false, (the EOS of the parent packet will be sent
      // with the last sub-packet).
      bool intermediateEOS = false;
      unsigned int rowNum;
      for (rowNum = 0; rowNum < numFullPackets; rowNum++) {
          if ( (rowNum == numFullPackets -1) && (lenOfLastPacket == 0)) {
              // This is the last sub-packet.
              intermediateEOS = EOS;
          }

          DataBufferType  subPacket(
              data.begin() + rowNum*numSamplesPerPush,
              data.begin() + rowNum*numSamplesPerPush + numSamplesPerPush);

          _pushPacket(subPacket, T, intermediateEOS, streamID);
      }

      if (lenOfLastPacket != 0) {
          // Send the last sub-packet, whose length is less than
          // numSamplesPerPush.  Note that the EOS of the master packet is
          // sent with the last sub-packet.
          DataBufferType subPacket(
              data.begin() + numFullPackets*numSamplesPerPush,
              data.begin() + numFullPackets*numSamplesPerPush + lenOfLastPacket);

          _pushPacket(subPacket, T, EOS, streamID);
      }
    }
  }


  template < typename PortTraits >
  void OutPort< PortTraits >::_pushPacket(
          const DataBufferType &    data,
          BULKIO::PrecisionUTCTime& T,
          bool                      EOS,
          const std::string&        streamID) {
      // Make a new sequence using the data from the Iterator
      // as the data for the sequence.  The 'false' at the end is whether or not
      // CORBA is allowed to delete the buffer when the sequence is destroyed.
      PortSequenceType seq = PortSequenceType(
              data.size(),
              data.size(),
              (TransportType *)&(data[0]),
              false);
      typename  ConnectionsList::iterator port;
      if (active) {
        std::vector<connection_descriptor_struct >::iterator ftPtr;
        for (port = outConnections.begin(); port != outConnections.end(); port++) {
          for (ftPtr=filterTable.begin(); ftPtr != filterTable.end(); ftPtr++) {
            if ((ftPtr->port_name == this->name) and (ftPtr->connection_name == port->second) and (ftPtr->stream_id == streamID)) {
              try {
                port->first->pushPacket(seq, T, EOS, streamID.c_str());
                stats[(*port).second].update(data.size(), 0, EOS, streamID);
              } catch(...) {
                LOG_ERROR( logger, "PUSH-PACKET FAILED, PORT/CONNECTION: " << name << "/" << port->second );
              }
            }
          }
        }
        for (port = outConnections.begin(); port != outConnections.end(); port++) {
          bool connectionListed = false;
          for (ftPtr=filterTable.begin(); ftPtr != filterTable.end(); ftPtr++) {
            if ((ftPtr->port_name == this->name) and (ftPtr->connection_name == port->second)) {
                connectionListed = true;
            }
          }
          if (!connectionListed) {
            try {
                port->first->pushPacket(seq, T, EOS, streamID.c_str());
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
        this->pushSRI(this->currentSRIs[streamID].first);
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
        this->pushSRI(this->currentSRIs[streamID].first);
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
