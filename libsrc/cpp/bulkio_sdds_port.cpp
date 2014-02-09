
/*******************************************************************************************


*******************************************************************************************/
#include <bulkio_p.h>
#include <uuid/uuid.h>


namespace  bulkio {


  std::string _uuid_gen() {
    uuid_t id;
    uuid_generate(id);

    // Per the man page, UUID strings are 36 characters plus the '\0' terminator.
    char _uid[37];
    uuid_unparse(id, _uid);
    std::string ret;
    ret=_uid;
    return ret;
  }

  // ----------------------------------------------------------------------------------------
  // InSDDSPort
  // ----------------------------------------------------------------------------------------
  InSDDSPort::InSDDSPort(std::string port_name, 
			 InSDDSPort::Callback *attach_detach_cb,
			 bulkio::sri::Compare  sriCmp,
			 bulkio::time::Compare timeCmp):
    Port_Provides_base_impl(port_name),
    attach_detach_callback(attach_detach_cb),
    sri_cmp(sriCmp),
    time_cmp(timeCmp),
    sriChanged(false)
  {
    stats = new linkStatistics(port_name);
  }


  InSDDSPort::InSDDSPort(std::string port_name, 
			 LOGGER_PTR  logger,
			 InSDDSPort::Callback *attach_detach_cb,
			 bulkio::sri::Compare  sriCmp,
			 bulkio::time::Compare timeCmp):
    Port_Provides_base_impl(port_name),
    attach_detach_callback(attach_detach_cb),
    sri_cmp(sriCmp),
    time_cmp(timeCmp),
    sriChanged(false),
    logger(logger)
  {
    stats = new linkStatistics(port_name);

    LOG_DEBUG( logger, "bulkio::InSDDSPort CTOR port:" << name );

  }



  InSDDSPort::~InSDDSPort()
  {
    if (stats) delete stats;
  }

  void   InSDDSPort::setLogger( LOGGER_PTR newLogger ) 
  {
    logger = newLogger;
  }

  //
  // Port Statistics Interface
  //
  void InSDDSPort::enableStats(bool enable)  
  {
    if (stats) stats->setEnabled(enable);
  }

  void InSDDSPort::setBitSize(double bitSize) {
    if (stats) stats->setBitSize(bitSize);
  }

  void InSDDSPort::updateStats(unsigned int elementsReceived, float queueSize, bool EOS, std::string streamID) {
    boost::mutex::scoped_lock lock(statUpdateLock);
    if (stats) stats->update(elementsReceived, queueSize, EOS, streamID );
  }


  BULKIO::PortStatistics * InSDDSPort::statistics()
  {
    boost::mutex::scoped_lock lock(statUpdateLock);
    if (stats) {
      BULKIO::PortStatistics_var recStat = new BULKIO::PortStatistics(stats->retrieve());
      return recStat._retn();
    }
    return NULL;
  }


  BULKIO::PortUsageType InSDDSPort::state()
  {
    if (attachedStreamMap.size() == 0) {
      return BULKIO::IDLE;
    } else if (attachedStreamMap.size() == 1) {
      return BULKIO::BUSY;
    } else {
      return BULKIO::ACTIVE;
    }
  }


  //
  // updateSRI Interface
  //

  BULKIO::StreamSRISequence * InSDDSPort::activeSRIs()
  {
    SCOPED_LOCK lock(sriUpdateLock);
    BULKIO::StreamSRISequence seq_rtn;
    SriMap::iterator currH;
    int i = 0;
    for (currH = currentHs.begin(); currH != currentHs.end(); currH++) {
      i++;
      seq_rtn.length(i);
      seq_rtn[i-1] = currH->second.first;
    }
    BULKIO::StreamSRISequence_var retSRI = new BULKIO::StreamSRISequence(seq_rtn);

    // NOTE: You must delete the object that this function returns!
    return retSRI._retn();
  }


  void InSDDSPort::pushSRI(const BULKIO::StreamSRI& H, const BULKIO::PrecisionUTCTime& T)
  {
    TRACE_ENTER(logger );

    boost::mutex::scoped_lock lock(sriUpdateLock);
    bool foundSRI = false;
    BULKIO::StreamSRI tmpH = H;
    SriMap::iterator sriIter;

    sriIter = currentHs.begin();
    while (sriIter != currentHs.end()) {
      if (strcmp(H.streamID, (*sriIter).first.c_str()) == 0) {
	foundSRI = true;
	break;
      }
      sriIter++;
    }
    if (!foundSRI) {
      currentHs.insert(std::make_pair(CORBA::string_dup(H.streamID), std::make_pair(H, T)));
      sriChanged = true;
    } else {
      bool schanged = false;
      if ( sri_cmp != NULL ) {
	schanged = sri_cmp( (*sriIter).second.first, H );
      }
      bool tchanged = false;
      if ( time_cmp != NULL ) {
	tchanged = time_cmp( (*sriIter).second.second, T );
      }
      sriChanged = !schanged || !tchanged;
  
  
      (*sriIter).second = std::make_pair(H, T);
    }

    TRACE_EXIT(logger );
  }


  //
  // SDDS Interface
  //


  char* InSDDSPort::attach(const BULKIO::SDDSStreamDefinition& stream, const char* userid) 
    throw (BULKIO::dataSDDS::AttachError, BULKIO::dataSDDS::StreamInputError)
  {

    TRACE_ENTER(logger );

    LOG_DEBUG( logger, "SDDS PORT: ATTACH REQUEST, STREAM/USER: " << stream.id <<  "/" << userid );

    std::string attachId("");

    if ( attach_detach_callback ) {
      try {
	LOG_DEBUG( logger, "SDDS PORT: CALLING ATTACH CALLBACK, STREAM/USER: " << stream.id <<  "/" << userid );
	attachId = attach_detach_callback->attach(stream, userid);
      }
      catch(...) {
	LOG_ERROR( logger, "SDDS PORT: ATTACH CALLBACK EXCEPTION, STREAM/USER: " << stream.id <<  "/" << userid );
	throw BULKIO::dataSDDS::AttachError("Callback Failed.");
      }
    }
    if ( attachId.size() == 0 ) {
      attachId = _uuid_gen();
    }

    attachedStreamMap.insert(std::make_pair(attachId, new BULKIO::SDDSStreamDefinition(stream)));
    attachedUsers.insert(std::make_pair(attachId, std::string(userid)));

    LOG_DEBUG( logger, "SDDS PORT, ATTACH COMPLETED, ID:" << attachId << 
	       " STREAM/USER" << stream.id <<  "/" << userid );

    TRACE_EXIT(logger );
    return CORBA::string_dup(attachId.c_str());
  }


  void InSDDSPort::detach(const char* attachId)
  {
    TRACE_ENTER(logger );
    LOG_DEBUG( logger, "SDDS PORT: DETACH REQUESTED,  ID:" << attachId   );
    if ( attach_detach_callback )  {

      try {
	LOG_DEBUG( logger, "SDDS PORT: CALLING DETACH CALLBACK, ID:" << attachId );
	attach_detach_callback->detach(attachId);
      }
      catch(...) {
	LOG_ERROR( logger, "SDDS PORT: DETACH CALLBACK EXCEPTION ID:" << attachId  );
	throw BULKIO::dataSDDS::DetachError();
      }
    }

    //
    // remove item from attachStreamMap
    //
    try {
      AttachedStreams::iterator itr = attachedStreamMap.find(attachId );
      if ( itr != attachedStreamMap.end() )
	delete itr->second;

      attachedStreamMap.erase(attachId);
      attachedUsers.erase(attachId);
    }
    catch(...) {
      throw BULKIO::dataSDDS::DetachError("Unknown Attach ID");
    }

    LOG_DEBUG( logger, "SDDS PORT: DETACH SUCCESS,  ID:" << attachId  );

    TRACE_EXIT(logger );

  }


  BULKIO::StreamSRISequence * InSDDSPort::attachedSRIs()
  {
    boost::mutex::scoped_lock lock(sriUpdateLock);
    BULKIO::StreamSRISequence_var sris = new BULKIO::StreamSRISequence();
    sris->length(currentHs.size());
    SriMap::iterator sriIter;
    unsigned int idx = 0;

    sriIter = currentHs.begin();
    while (sriIter != currentHs.end()) {
      sris[idx++] = (*sriIter).second.first;
      sriIter++;
    }
    return sris._retn();
  }


  BULKIO::SDDSStreamDefinition* InSDDSPort::getStreamDefinition(const char* attachId)
  {
    AttachedStreams::iterator portIter2;
    portIter2 = attachedStreamMap.begin();
    // use: attachedPorts[(*portIter).first] :instead
    while (portIter2 != attachedStreamMap.end()) {
      if (strcmp((*portIter2).first.c_str(), attachId) == 0) {
	return (*portIter2).second;
      }
      portIter2++;
    }
    return NULL;
  }


  BULKIO::SDDSStreamSequence* InSDDSPort::attachedStreams()
  {
    BULKIO::SDDSStreamSequence* seq = new BULKIO::SDDSStreamSequence();
    seq->length(attachedStreamMap.size());
    AttachedStreams::iterator portIter2;
    portIter2 = attachedStreamMap.begin();
    unsigned int i = 0;
    while (portIter2 != attachedStreamMap.end()) {
      (*seq)[i++] = *((*portIter2).second);
      portIter2++;
    }
    return seq;
  }

  char* InSDDSPort::getUser(const char* attachId)
  {
    AttachedUsers::iterator portIter2;
    portIter2 = attachedUsers.begin();
    while (portIter2 != attachedUsers.end()) {
      if (strcmp((*portIter2).first.c_str(), attachId) == 0) {
	return CORBA::string_dup((*portIter2).second.c_str());
      }
      portIter2++;
    }
    return NULL;
  }

  BULKIO::dataSDDS::InputUsageState InSDDSPort::usageState()
  {
    if (attachedStreamMap.size() == 0) {
      return BULKIO::dataSDDS::IDLE;
    } else if (attachedStreamMap.size() == 1) {
      return BULKIO::dataSDDS::BUSY;
    } else {
      return BULKIO::dataSDDS::ACTIVE;
    }
  }

  BULKIO::StringSequence* InSDDSPort::attachmentIds()
  {
    BULKIO::StringSequence* seq = new BULKIO::StringSequence();
    seq->length(attachedStreamMap.size());
    AttachedStreams::iterator portIter2;
    portIter2 = attachedStreamMap.begin();
    unsigned int i = 0;
    while (portIter2 != attachedStreamMap.end()) {
      (*seq)[i++] = CORBA::string_dup((*portIter2).first.c_str());
      portIter2++;
    }
    return seq;
  }



  //
  // SDDS Output Port Implementation
  //
  OutSDDSPort::OutSDDSPort( std::string port_name, 
			    ConnectionEventListener *connectCB, 
			    ConnectionEventListener *disconnectCB) :
    Port_Uses_base_impl(port_name),
    _connectCB(),
    _disconnectCB()
  {
    lastStreamData = NULL;
    recConnectionsRefresh = false;
    recConnections.length(0);
  }


  OutSDDSPort::OutSDDSPort(std::string port_name, 
			   LOGGER_PTR  logger,
			   ConnectionEventListener *connectCB, 
			   ConnectionEventListener *disconnectCB) :

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

    lastStreamData = NULL;
    recConnectionsRefresh = false;
    recConnections.length(0);

    LOG_DEBUG( logger, "bulkio::OutSDDSPort::CTOR port:" << name );
  }


  OutSDDSPort::~OutSDDSPort()
  {
  }


  BULKIO::UsesPortStatisticsSequence * OutSDDSPort::statistics()
  {
    boost::mutex::scoped_lock lock(updatingPortsLock);
    BULKIO::UsesPortStatisticsSequence_var recStat = new BULKIO::UsesPortStatisticsSequence();
    recStat->length(outConnections.size());
    for (unsigned int i = 0; i < outConnections.size(); i++) {
      recStat[i].connectionId = CORBA::string_dup(outConnections[i].second.c_str());
      recStat[i].statistics = stats[outConnections[i].second].retrieve();
    }
    return recStat._retn();
  }

  BULKIO::PortUsageType OutSDDSPort::state()
  {
    boost::mutex::scoped_lock lock(updatingPortsLock);
    if (outConnections.size() > 0) {
      return BULKIO::ACTIVE;
    } else {
      return BULKIO::IDLE;
    }

    return BULKIO::BUSY;
  }

  void OutSDDSPort::enableStats(bool enable)
  {
    for (unsigned int i = 0; i < outConnections.size(); i++) {
      stats[outConnections[i].second].setEnabled(enable);
    }
  }

  void OutSDDSPort::setBitSize(double bitSize)
  {
    for (unsigned int i = 0; i < outConnections.size(); i++) {
      stats[outConnections[i].second].setBitSize(bitSize);
    }
  }

  void OutSDDSPort:: updateStats(unsigned int elementsReceived, unsigned int queueSize, bool EOS, std::string streamID)
  {
    for (unsigned int i = 0; i < outConnections.size(); i++) {
      stats[outConnections[i].second].update(elementsReceived, queueSize, EOS, streamID);
    }
  }


  void  OutSDDSPort::connectPort(CORBA::Object_ptr connection, const char* connectionId)
  {
    TRACE_ENTER(logger );

    {
      boost::mutex::scoped_lock lock(updatingPortsLock);   // don't want to process while command information is coming in
      BULKIO::dataSDDS_var port;
      try{
	port = BULKIO::dataSDDS::_narrow(connection);
      }
      catch(...) {
	LOG_ERROR( logger, "CONNECT FAILED: UNABLE TO NARROW ENDPOINT,  USES PORT:" << name );
	throw CF::Port::InvalidPort(1, "Unable to narrow");
      }
      if (lastStreamData != NULL) {
	// TODO - use the username instead
	std::string attachId = port->attach(*lastStreamData, user_id.c_str());
	attachedGroup.insert(std::make_pair(attachId, std::make_pair(lastStreamData, user_id)));
	attachedPorts.insert(std::make_pair(port, attachId));
      }
      outConnections.push_back(std::make_pair(port, connectionId));
      active = true;
      recConnectionsRefresh = true;
      refreshSRI = true;    
      LOG_DEBUG( logger, "CONNECTION ESTABLISHED,  PORT/CONNECTION_ID:" << name << "/" << connectionId );
    }

    if ( _connectCB ) (*_connectCB)(connectionId);

    TRACE_EXIT(logger );
  }

  void  OutSDDSPort::disconnectPort(const char* connectionId)
  {
    {
      boost::mutex::scoped_lock lock(updatingPortsLock);   // don't want to process while command information is coming in
      for (unsigned int i = 0; i < outConnections.size(); i++) {
	if (outConnections[i].second == connectionId) {
	  LOG_DEBUG( logger, "DISCONNECT, PORT/CONNECTION: "  << name << "/" << connectionId );
	  if (attachedPorts.find(outConnections[i].first) != attachedPorts.end()) {
	    try {
	      outConnections[i].first->detach(attachedPorts[outConnections[i].first].c_str());
	    }
	    catch(...) {
	      LOG_ERROR(logger," Unable to detach for CONNECTION: " << connectionId );
	    }
	      
	  }
	  outConnections.erase(outConnections.begin() + i);
	  break;
	}
      }

      if (outConnections.size() == 0) {
	active = false;
      }
      recConnectionsRefresh = true;
    }
    if ( _disconnectCB ) (*_disconnectCB)(connectionId);

  }

  ExtendedCF::UsesConnectionSequence *  OutSDDSPort::connections() 
  {
    boost::mutex::scoped_lock lock(updatingPortsLock);   // don't want to process while command information is coming in
    if (recConnectionsRefresh) {
      recConnections.length(outConnections.size());
      for (unsigned int i = 0; i < outConnections.size(); i++) {
	recConnections[i].connectionId = CORBA::string_dup(outConnections[i].second.c_str());
	recConnections[i].port = CORBA::Object::_duplicate(outConnections[i].first);
      }
      recConnectionsRefresh = false;
    }
    ExtendedCF::UsesConnectionSequence_var retVal = new ExtendedCF::UsesConnectionSequence(recConnections);
    return retVal._retn();
  }


  BULKIO::SDDSStreamDefinition*  OutSDDSPort::getStreamDefinition(const char* attachId) 
  {
    AttachedStreams::iterator groupIter;
    groupIter = attachedGroup.begin();

    while (groupIter != attachedGroup.end()) {
      if (strcmp((*groupIter).first.c_str(), attachId) == 0) {
	return (*groupIter).second.first;
      }
      groupIter++;
    }
    return NULL;
  }

  char* OutSDDSPort::getUser(const char* attachId)
  {
    AttachedStreams::iterator groupIter;
    groupIter = attachedGroup.begin();
    while (groupIter != attachedGroup.end()) {
      if (strcmp((*groupIter).first.c_str(), attachId) == 0) {
	return CORBA::string_dup((*groupIter).second.second.c_str());
      }
      groupIter++;
    }
    return NULL;
  }

  BULKIO::dataSDDS::InputUsageState OutSDDSPort::usageState()
  {

    if (attachedGroup.size() == 0) {
      return BULKIO::dataSDDS::IDLE;
    } else if (attachedGroup.size() == 1) {
      return BULKIO::dataSDDS::BUSY;
    } else {
      return BULKIO::dataSDDS::ACTIVE;
    }
  }

  BULKIO::SDDSStreamSequence* OutSDDSPort::attachedStreams()
  {
    BULKIO::SDDSStreamSequence* seq = new BULKIO::SDDSStreamSequence();
    seq->length(1);
    (*seq)[0] = *lastStreamData;
    return seq;
  }

  BULKIO::StringSequence* OutSDDSPort::attachmentIds() 
  {
    BULKIO::StringSequence* seq = new BULKIO::StringSequence();
    seq->length(attachedGroup.size());
    AttachedStreams::iterator groupIter;
    groupIter = attachedGroup.begin();
    unsigned int i = 0;
    while (groupIter != attachedGroup.end()) {
      (*seq)[i++] = CORBA::string_dup((*groupIter).first.c_str());
      groupIter++;
    }

    return seq;
  }


  char* OutSDDSPort::attach(const BULKIO::SDDSStreamDefinition& stream, const char* userid) 
    throw (BULKIO::dataSDDS::AttachError, BULKIO::dataSDDS::StreamInputError)
  {
    TRACE_ENTER(logger );

    boost::mutex::scoped_lock lock(updatingPortsLock);
    std::string attachId;
    user_id = userid;
    AttachedPorts::iterator portIter;
    BULKIO::dataSDDS::_var_type port = NULL;
    lastStreamData = new BULKIO::SDDSStreamDefinition(stream);

    // detach all existing ports (???)
    portIter = attachedPorts.begin();
    while (portIter != attachedPorts.end()) {
      try {
	port = (*portIter).first;
	port->detach(attachedPorts[port].c_str());
	attachedGroup.erase((*portIter).second);
	portIter++;
      }
      catch(...) {
	LOG_ERROR(logger, "Detach failed for ATTACH ID: " << attachedPorts[port].c_str() );
      }
    }

    Connections::iterator portIter2 = outConnections.begin();
    while (portIter2 != outConnections.end()) {
      port = (*portIter2).first;
      attachId = port->attach(stream, user_id.c_str());

      attachedGroup.insert(std::make_pair(attachId, std::make_pair(lastStreamData, user_id)));
      attachedPorts[port] = attachId;
      portIter2++;
    }

    LOG_DEBUG(logger, "SDDS PORT: ATTACH COMPLETD ID:" << attachId << " NAME(user-id):" << user_id );
    

    TRACE_EXIT(logger );
    return CORBA::string_dup(attachId.c_str());
  }

  void OutSDDSPort::detach(const char* attachId, const char* connectionId)
  {
    TRACE_ENTER(logger );
    boost::mutex::scoped_lock lock(updatingPortsLock);
    Connections::iterator portIter = outConnections.begin();
    AttachedPorts::iterator portIter2;
    while (portIter != outConnections.end()) {
      portIter2 = attachedPorts.begin();
      if (!strcmp(connectionId, (*portIter).second.c_str())) {
	while (portIter2 != attachedPorts.end()) {
	  if ((*portIter2).first == (*portIter).first) {
	    portIter->first->detach(attachedPorts[(*portIter).first].c_str());
	    LOG_DEBUG(logger, "SDDS PORT: DETACH COMPLETD ID:" << attachId  );
	    return;
	  }
	  portIter2++;
	}
      }
      portIter++;
    }
    TRACE_EXIT(logger );
  }


  void OutSDDSPort::detach(const char* attachId )
  {
    TRACE_ENTER(logger );
    boost::mutex::scoped_lock lock(updatingPortsLock);
    AttachedPorts::iterator port = attachedPorts.begin();

    // look for each port object based on attachId and call detach
    while ( port != attachedPorts.end()) {    
      if ( strcmp( port->second.c_str(), attachId ) == 0 ) {
	if ( port->first ) {
	  port->first->detach( port->second.c_str());
	  LOG_DEBUG(logger, "SDDS PORT: DETACH COMPLETD ID:" << attachId  );
	}
      }
      port++;
    }
    TRACE_EXIT(logger );
  }

  /*
   * pushSRI
   *     description: send out SRI describing the data payload
   *
   *  H: structure of type BULKIO::StreamSRI with the SRI for this stream
   *    hversion
   *    xstart: start time of the stream
   *    xdelta: delta between two samples
   *    xunits: unit types from Platinum specification
   *    subsize: 0 if the data is one-dimensional
   *    ystart
   *    ydelta
   *    yunits: unit types from Platinum specification
   *    mode: 0-scalar, 1-complex
   *    streamID: stream identifier
   *    sequence<CF::DataType> keywords: unconstrained sequence of key-value pairs for additional description
   *
   *  T: structure of type BULKIO::PrecisionUTCTime with the Time for this stream
   *    tcmode: timecode mode
   *    tcstatus: timecode status
   *    toff: Fractional sample offset
   *    twsec
   *    tfsec
   */
  void OutSDDSPort::pushSRI(const BULKIO::StreamSRI& H, const BULKIO::PrecisionUTCTime& T)
  {
    TRACE_ENTER(logger );

    boost::mutex::scoped_lock lock(updatingPortsLock);   // don't want to process while command information is coming in

    if (active) {
      Connections::iterator i;
      for (i = outConnections.begin(); i != outConnections.end(); ++i) {
	try {
	  i->first->pushSRI(H, T);
	} catch(...) {
	  LOG_ERROR( logger, "PUSH-SRI FAILED, PORT/CONNECTION: " << name << "/" << i->second );
	}
      }
    }

    currentSRIs[std::string(H.streamID)] = std::make_pair(H, T);
    refreshSRI = false;

    TRACE_EXIT(logger );
    return;
  }


} // end of bulkio namespace
