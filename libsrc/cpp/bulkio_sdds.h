
#ifndef __bulkio_sdds_h
#define __bulkio_sdds_h

#include <queue>
#include <list>
#include <vector>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/locks.hpp>

#include <bulkio_base.h>
#include <bulkio_traits.h>

namespace bulkio {

  /**

     SDDS Stream Definitions from IDL file 

     enum SDDSDataDigraph {
     SDDS_SP,  // packed bit data
     SDDS_SB,  // scalar int8_t
     SDDS_SI,  // scalar int16_t
     SDDS_SL,  // scalar int32_t
     SDDS_SX,  // scalar int64_t
     SDDS_SF,  // scalar float
     SDDS_SD,  // scalar double
     SDDS_CB,  // complex int8_t
     SDDS_CI,  // complex int16_t
     SDDS_CL,  // complex int32_t
     SDDS_CX,  // complex int64_t
     SDDS_CF,  // complex float
     SDDS_CD   // complex double
     }; 

     struct SDDSStreamDefinition {
     string        id; // a UUID to uniquely identify a stream.
     SDDSDataDigraph   dataFormat;
     string        multicastAddress;
     unsigned long vlan;
     unsigned long port;
     unsigned long sampleRate;
     boolean       timeTagValid;
     string        privateInfo; // user generated text
     };

  **/

  // 
  // SDDS Bulkio Input (sink/provides)/ Output (source/uses) definitions
  //

  class InSDDSPort : public POA_BULKIO::dataSDDS, public Port_Provides_base_impl {

  public:

    //
    // Interface to notify when an attach and detach event/request is made
    //
    class  Callback  {

    public:

      virtual char* attach(const BULKIO::SDDSStreamDefinition& stream, const char* userid)
	throw (BULKIO::dataSDDS::AttachError, BULKIO::dataSDDS::StreamInputError) = 0;

      virtual void detach(const char* attachId) = 0;

    };


    //
    // InSDDSPort
    //
    // Input SDDS port constructor. This class accepts 3 different interfaces
    //
    // Callback - Providers of this interface will be notified when attach and detach methods are called.
    // sri::Compare  - Compare method for comparing SRI object when pushSRI happens
    // time::Compare - Compare method for comparing TimeStamp objects when pushSRI happens
    //
    InSDDSPort(std::string port_name, 
	       InSDDSPort::Callback *cb = NULL,
	       bulkio::sri::Compare = bulkio::sri::DefaultComparator, 
	       bulkio::time::Compare = bulkio::time::DefaultComparator );


    InSDDSPort(std::string port_name, 
	       LOGGER_PTR    logger,
	       InSDDSPort::Callback *cb = NULL,
	       bulkio::sri::Compare = bulkio::sri::DefaultComparator, 
	       bulkio::time::Compare = bulkio::time::DefaultComparator );

    ~InSDDSPort();

  
    bool hasSriChanged () {
      return sriChanged;
    }

    //
    // Port State Interface
    //
    BULKIO::PortUsageType state();


    //
    // Port Statistics Interface
    //
    virtual BULKIO::PortStatistics* statistics();
    virtual void enableStats(bool enable);
    virtual void setBitSize(double bitSize); 
    virtual void updateStats(unsigned int elementsReceived, float queueSize, bool EOS, std::string streamID);


    //
    // updateSRI Interface
    //

    //
    // pushSRI
    //
    virtual void pushSRI(const BULKIO::StreamSRI& H, const BULKIO::PrecisionUTCTime& T);

    //
    // activeSRIs - returns a sequence of BULKIO::StreamSRI objectsPort
    //
    // @return BULKIO::StreamSRISequence - list of active SRI objects for this port
    //                                     the caller is responsible for freeing the memory returned
    //
    virtual BULKIO::StreamSRISequence* activeSRIs();

    //
    // SDDS Interface
    //

    //
    // attach
    //
    // Request an attach operation to the provided SDDS stream definition.  Each requestor is required
    // to provide a subscriber id to track attachment requests.  Upon successfull
    // completion, this method returns an attachment identifier.  This identifier is required
    // to perform the detach operation
    //
    virtual char* attach(const BULKIO::SDDSStreamDefinition& stream, const char* userid)
      throw (BULKIO::dataSDDS::AttachError, BULKIO::dataSDDS::StreamInputError);


    //
    // detach
    //
    // Process a request to detach from a SDDS stream for the provided attachment identifier.
    //
    virtual void detach(const char* attachId);

    //
    // getStreamDefinition
    //
    // @return   BULKIO::SDDSStreamDefinition Return the SDDS stream definition for this attachId
    //           NULL attachId was not found
    //
    BULKIO::SDDSStreamDefinition* getStreamDefinition(const char* attachId);

    //
    // getUser
    //
    // @return char * the user id that made the attachId request
    //         NULL attachId was not found
    //
    char* getUser(const char* attachId);

    //
    // attachedSRIs
    //
    // @return BULKIO::StreamSRISequence Returns a list of StreamSRI objects for each attached 
    //                                   stream definition. The caller is responsible for freeing
    //                                   the provided objects
    //
    BULKIO::StreamSRISequence* attachedSRIs();

    //
    // attachedStreams
    //
    // @return BULKIO::SDDSStreamSequence Returns a list of attached SDDS Stream Defintions. The caller
    //                                    is responsible for freeing the provided objects
    //
    BULKIO::SDDSStreamSequence* attachedStreams();


    //
    // attachmentIds
    //
    // @return BULKIO::StringSequence Return the current list of attachment identifiers. The caller
    //                                is responsible for freeing the provided objects
    //
    BULKIO::StringSequence* attachmentIds();


    //
    // usageState
    //
    // If the number of attached streams == 0 
    //      return BULKIO::dataSDDS::IDLE;
    // If the number of attached streams == 1
    //      return BULKIO::dataSDDS::BUSY;
    // other
    //    return BULKIO::dataSDDS::ACTIVE;
    //
    // @return   BULKIO::dataSDDS::InputUsageState return the current state of the port base on how many attach requests have been received
    //            
    BULKIO::dataSDDS::InputUsageState usageState();


  protected:

    typedef  std::map<std::string, BULKIO::SDDSStreamDefinition*>    AttachedStreams;

    typedef std::map<std::string, std::string >                      AttachedUsers;

    typedef std::map<std::string, std::pair<BULKIO::StreamSRI, BULKIO::PrecisionUTCTime> >   SriMap;

    // maps a stream ID to a pair of Stream and userID
    AttachedStreams      attachedStreamMap;

    // Holds list of uers
    AttachedUsers        attachedUsers;

    // Current set of SRI objects passed to us
    SriMap               currentHs;

    bool                 sriChanged;

    MUTEX                statUpdateLock;

    MUTEX                sriUpdateLock;

    bulkio::sri::Compare     sri_cmp;

    bulkio::time::Compare    time_cmp;

    Callback                 *attach_detach_callback;

    // statistics
    linkStatistics           *stats;


    LOGGER_PTR                                     logger;

  public:    
    
    void                                          setLogger( LOGGER_PTR logger );


  };



  // ----------------------------------------------------------------------------------------
  // SDDS Output Port
  // ----------------------------------------------------------------------------------------
  class OutSDDSPort : public Port_Uses_base_impl, public virtual POA_BULKIO::UsesPortStatisticsProvider  {

  private:

    typedef  std::vector < std::pair<BULKIO::dataSDDS_var, std::string> >                   Connections;

    typedef std::map<std::string, std::pair<BULKIO::SDDSStreamDefinition*, std::string> >   AttachedStreams;

    typedef std::map<std::string, std::pair<BULKIO::StreamSRI, BULKIO::PrecisionUTCTime> >  SriMap;

    typedef  std::map<BULKIO::dataSDDS::_var_type, std::string>                             AttachedPorts;


  public:

    // Output SDDS port constructor. This class accepts 2 different interfaces
    //
    // ConnectNotifier - Providers of this interface will be notified when connectPort event happens
    // DisconnectNotifier - Providers of this interface will be notified when disconnectPort event happens
    //
    OutSDDSPort(std::string port_name , 
		ConnectionEventListener *connectCB=NULL,
		ConnectionEventListener *disconnectCB=NULL );


    OutSDDSPort(std::string port_name , 
		LOGGER_PTR  logger,
		ConnectionEventListener *connectCB=NULL,
		ConnectionEventListener *disconnectCB=NULL );


    ~OutSDDSPort();

    //
    // Port Statistics Interface
    //
    virtual BULKIO::UsesPortStatisticsSequence * statistics();
    virtual BULKIO::PortUsageType state();
    virtual void enableStats(bool enable);
    virtual void setBitSize(double bitSize);
    virtual void updateStats(unsigned int elementsReceived, unsigned int queueSize, bool EOS, std::string streamID);

    //
    // Uses Port Interface
    //
    virtual void connectPort(CORBA::Object_ptr connection, const char* connectionId);
    virtual void disconnectPort(const char* connectionId);
    virtual ExtendedCF::UsesConnectionSequence * connections();
    
    template< typename T > inline
      void setNewConnectListener(T &target, void (T::*func)( const char *connectionId )  ) {
      _connectCB =  boost::make_shared< MemberConnectionEventListener< T > >( boost::ref(target), func );
    };

    template< typename T > inline
      void setNewConnectListener(T *target, void (T::*func)( const char *connectionId )  ) {
      _connectCB =  boost::make_shared< MemberConnectionEventListener< T > >( boost::ref(*target), func );
    };

    template< typename T > inline
      void setNewDisconnectListener(T &target, void (T::*func)( const char *connectionId )  ) {
      _disconnectCB =  boost::make_shared< MemberConnectionEventListener< T > >( boost::ref(target), func );
    };

    template< typename T > inline
      void setNewDisconnectListener(T *target, void (T::*func)( const char *connectionId )  ) {
      _disconnectCB =  boost::make_shared< MemberConnectionEventListener< T > >( boost::ref(*target), func );
    };

    //
    // Attach listener interfaces for connect and disconnect events
    //
    void   setNewConnectListener( ConnectionEventListener *newListener );
    void   setNewConnectListener( ConnectionEventCallbackFn  newListener );
    void   setNewDisconnectListener( ConnectionEventListener *newListener );
    void   setNewDisconnectListener( ConnectionEventCallbackFn  newListener );


    //
    // pushSRI to allow for insertion of SRI context into data stream
    //
    void pushSRI(const BULKIO::StreamSRI& H, const BULKIO::PrecisionUTCTime& T);


    //
    // getStreamDefinition
    //
    // @return   BULKIO::SDDSStreamDefinition Return the SDDS stream definition for this attachId
    //           NULL attachId was not found
    //
    virtual BULKIO::SDDSStreamDefinition* getStreamDefinition(const char* attachId);


    //
    // getUser
    //
    // @return char * the user id that made the attachId request
    //         NULL attachId was not found
    //
    virtual char* getUser(const char* attachId);

    //
    // usageState
    //
    // If the number of attached streams == 0 
    //      return BULKIO::dataSDDS::IDLE;
    // If the number of attached streams == 1
    //      return BULKIO::dataSDDS::BUSY;
    // other
    //    return BULKIO::dataSDDS::ACTIVE;
    //
    // @return   BULKIO::dataSDDS::InputUsageState return the current state of the port base on how many attach requests have been received
    //
    virtual BULKIO::dataSDDS::InputUsageState usageState();

    //
    // attachedStreams
    //
    // @return BULKIO::SDDSStreamSequence Returns a last SDDS Stream Defintion that was made. The caller
    //                                    is responsible for freeing the provided objects
    //
    virtual BULKIO::SDDSStreamSequence* attachedStreams();

    //
    // attachmentIds
    //
    // @return BULKIO::StringSequence Return the current list of attachment identifiers. The caller
    //                                is responsible for freeing the provided objects
    //
    virtual BULKIO::StringSequence* attachmentIds();


    //
    //  SDDS Interface
    //

    //
    // attach
    //
    // Send out a request to attach to the provided SDDS stream definition.  The end point servicing
    // this request will provide an attachment identifier that can be used by the detach request
    //    
    virtual char* attach(const BULKIO::SDDSStreamDefinition& stream, const char* userid) throw (BULKIO::dataSDDS::AttachError, BULKIO::dataSDDS::StreamInputError);

    //
    // detach
    //
    // Send a request to detach from a SDDS stream definition for the provided attachment identifier.
    //
    virtual void detach(const char* attachId );

    virtual void detach(const char* attachId, const char *connectionId );



  protected:

    typedef std::map<std::string, linkStatistics  >    _StatsMap;

    // mapping of current SRI objects to streamIDs
    SriMap  currentSRIs;

    // Connections List
    Connections   outConnections;

    // maps a stream ID to a pair of Stream and userID
    AttachedStreams   attachedGroup;

    //  list of Ports and attached stream ids
    AttachedPorts  attachedPorts;

    // track last attachment request made
    BULKIO::SDDSStreamDefinition*  lastStreamData;
    std::string  user_id;

    // connection sequence object to interface with outside world
    ExtendedCF::UsesConnectionSequence  recConnections;
    bool  recConnectionsRefresh;
  
    // list of statistic objects
    _StatsMap  stats;

    LOGGER_PTR                                     logger;

  public:    
    
    void                                          setLogger( LOGGER_PTR logger );

  private:
  

    boost::shared_ptr< ConnectionEventListener >    _connectCB;
    boost::shared_ptr< ConnectionEventListener >    _disconnectCB;

  };
 

}  // end of bulkio namespace


#endif
