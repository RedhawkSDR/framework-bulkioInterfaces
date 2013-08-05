
#ifndef __bulkio_out_port_h
#define __bulkio_out_port_h

#include <queue>
#include <list>
#include <vector>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/locks.hpp>
#include <boost/make_shared.hpp>
#include <boost/ref.hpp>

#include <bulkio_base.h>
#include <bulkio_traits.h>

namespace bulkio {

  //
  // Callback interface used by BULKIO Ports when connect/disconnect event happens
  //
  typedef void   (*ConnectionEventCallbackFn)( const char *connectionId );

  //
  // Interface definition that will be notified when a connect/disconnect event happens
  //
  class ConnectionEventListener {

    public:
      virtual void operator() ( const char *connectionId ) = 0;
      virtual ~ConnectionEventListener() {};

  };

    /**
     * Allow for member functions to receive connect/disconnect notifications
     */
    template <class T>
    class MemberConnectionEventListener : public ConnectionEventListener
    {
    public:
      typedef boost::shared_ptr< MemberConnectionEventListener< T > > SPtr;
      
      typedef void (T::*MemberFn)( const char *connectionId );

      static SPtr Create( T &target, MemberFn func ){
	return SPtr( new MemberConnectionEventListener(target, func ) );
      };

      virtual void operator() ( const char *connectionId )
      {
	(target_.*func_)(connectionId);
      }

      // Only allow PropertySet_impl to instantiate this class.
      MemberConnectionEventListener ( T& target,  MemberFn func) :
      target_(target),
	func_(func)
        {
        }
    private:
      T& target_;
      MemberFn func_;
    };

  /**
   * Wrap Callback functions as ConnectionEventListener objects
   */
  class StaticConnectionListener : public ConnectionEventListener
  {
    public:
    virtual void operator() ( const char *connectionId )
        {
            (*func_)(connectionId);
        }

    StaticConnectionListener ( ConnectionEventCallbackFn func) :
      func_(func)
        {
        }

  private:

    ConnectionEventCallbackFn func_;
  };


  //
  //  OutPort
  //
  //  Base template for data transfers between BULKIO ports.  This class is defined by 2 trait classes
  //    PortTraits - This template provides the context for the port's middleware transport classes and they base data types
  //                 passed between port objects
  //
  //
  template < typename PortTraits >
    class OutPort : public Port_Uses_base_impl, public virtual POA_BULKIO::UsesPortStatisticsProvider
  {

  public:

    typedef PortTraits                        Traits;

    //
    // Port Variable Definition
    //
    typedef typename Traits::PortVarType      PortVarType;

    //
    // BULKIO Interface Type
    //
    typedef typename Traits::PortType         PortType;

    typedef typename Traits::PortTraits       UsesPortType;

    //
    // Sequence container used during actual pushPacket call
    //
    typedef typename Traits::SequenceType     PortSequenceType;

    //
    // Data type contained in sequence container
    //
    typedef typename Traits::TransportType    TransportType;

    //
    // Data type of items passed into the pushPacket method
    //
    typedef typename Traits::NativeType       NativeType;

    // 
    // Data type of the container for passing data into the pushPacket method
    //
    typedef std::vector< NativeType >         NativeSequenceType;

    //
    // Sequence of data returned from an input port and can be passed to the output port
    //
    typedef typename Traits::DataBufferType   DataBufferType;


  public:

    //
    // OutPort Creates a uses port object for publishing data to the framework
    //
    // @param port_name name assigned to the port located in scd.xml file
    // @param connectionCB  callback that will be called when the connectPort method is called
    // @pararm disconnectDB callback that receives notification when a disconnectPort happens
    //
    OutPort(std::string port_name, 
	    ConnectionEventListener *connectCB=NULL,
	    ConnectionEventListener *disconnectCB=NULL );

    OutPort(std::string port_name, 
	    LOGGER_PTR    logger,
	    ConnectionEventListener *connectCB=NULL,
	    ConnectionEventListener *disconnectCB=NULL );

    
    //
    // virtual destructor to clean up resources
    //
    virtual ~OutPort();

    //
    //  Interface used by framework to connect/disconnect ports together and introspection of connection states
    //

    //
    // connections - Return a list of connection objects and identifiers for each connection made by connectPort
    //
    // @return ExtendedCF::UsesConnectionSequence * List of connection objects and identifiers
    //
    virtual ExtendedCF::UsesConnectionSequence * connections();

    //
    // connectPort - Called by the framework to connect this port to a Provides port object, the connection is established
    // via the association and identified by the connectionId string, no formal "type capatablity" or "bukio interface support"
    // is resolved at this time.  All data flow occurs from point A to B via the pushPacket/pushSRI interface.
    //
    // @param CORBA::Object_ptr pointer to an instance of a Provides port
    // @param connectionsId identifer for this connection, allows for external users to reference the connection association
    //
    virtual void connectPort(CORBA::Object_ptr connection, const char* connectionId);

    //
    // disconnectPort - Called by the framework to disconnect this port from the Provides port object.  The port basicall removes
    // the association to the provides port that was established with the connectionId.
    //
    // @param connectionsId identifer for this connection, allows for external users to reference the connection association
    virtual void disconnectPort(const char* connectionId);



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
    // pushSRI - called by the source component when SRI data about the stream changes, the data flow policy is this activity
    //           will occurr first before any data flows to the component.
    //
    // @param H - Incoming StreamSRI object that defines the state of the data flow portion of the stream (pushPacket)
    //
    virtual void pushSRI(const BULKIO::StreamSRI& H);

    /*
     * pushPacket
     *     maps to data<Type> BULKIO method call for passing vectors of data
     *
     *  data: sequence structure containing the payload to send out
     *  T: constant of type BULKIO::PrecisionUTCTime containing the timestamp for the outgoing data.
     *    tcmode: timecode mode
     *    tcstatus: timecode status
     *    toff: fractional sample offset
     *    twsec: J1970 GMT
     *    tfsec: fractional seconds: 0.0 to 1.0
     *  EOS: end-of-stream flag
     *  streamID: stream identifier
     */
    void pushPacket( NativeSequenceType & data, BULKIO::PrecisionUTCTime& T, bool EOS, const std::string& streamID);

    /*
     * pushPacket
     *     maps to data<Type> BULKIO method call for passing vectors of data
     *
     *  data: The sequence structure from an input port containing the payload to send out
     *  T: constant of type BULKIO::PrecisionUTCTime containing the timestamp for the outgoing data.
     *    tcmode: timecode mode
     *    tcstatus: timecode status
     *    toff: fractional sample offset
     *    twsec: J1970 GMT
     *    tfsec: fractional seconds: 0.0 to 1.0
     *  EOS: end-of-stream flag
     *  streamID: stream identifier
     */
    void pushPacket( const DataBufferType & data, BULKIO::PrecisionUTCTime& T, bool EOS, const std::string& streamID);

    //
    // statisics - returns a PortStatistics object for this uses port
    //      BULKIO::UsesPortStatisticsSequence: sequence of PortStatistics object
    //           PortStatistics
    //            portname - name of port
    //            elementsPerSecond - number of elements per second (element is based on size of port type )
    //            bitsPerSecond - number of bits per second (based on element storage size in bits)
    //            callsPerSecond - history window -1 / time between calls to this method
    //            streamIds - list of active stream id values
    //            averageQueueDepth - the average depth of the queue for this port
    //            timeSinceLastCall - time since this method as invoked and the last pushPacket happened
    //            Keyword Sequence - deprecated
    //
    // @return  BULKIO::UsesPortStatisticsSequenc - current data flow metrics collected for the port, the caller of the method
    //                                  is responsible for freeing this object
    //
    virtual BULKIO::UsesPortStatisticsSequence * statistics();

    //
    // state - returns the current state of the port as follows:
    //   BULKIO::BUSY - internal queue has reached FULL state
    //   BULKIO::IDLE - there are no items on the internal queue
    //   BULKIO::ACTIVE - there are items on the queue
    //
    // @return BULKIO::PortUsageType - current state of port
    //
    virtual BULKIO::PortUsageType state();

    //
    // turn on/off the port monitoring capability
    //
    virtual void enableStats(bool enable);

  protected:

    // ConnectionList Definition
    typedef typename  bulkio::Connections< PortVarType >::List                    _ConnectionsList;

    // Map of stream ids and statistic object
    typedef typename  std::map<std::string, linkStatistics  >    _StatsMap;

    //
    // List of Port connections and connection identifiers
    //
    _ConnectionsList                         outConnections;

    //
    // List of SRIs sent out by this port
    //
    SriMap                                   currentSRIs;


    //
    // List of connections returned by connections() method.  Used to increase efficiency when there a large amount
    // of connections for a port.
    //
    ExtendedCF::UsesConnectionSequence       recConnections;

    //
    //
    //
    bool                                     recConnectionsRefresh;

    //
    //  Set of statistical collector objects for each stream id
    //
    _StatsMap                                 stats;


    LOGGER_PTR                                logger;

  public:
    void   setLogger( LOGGER_PTR newLogger );



  private:

    boost::shared_ptr< ConnectionEventListener >    _connectCB;
    boost::shared_ptr< ConnectionEventListener >    _disconnectCB;

  };


  //
  // Character Specialization..
  //
  // This class overrides the pushPacket method to use the Int8 parameter and also overriding the PortSequence constructor to
  // use the CORBA::Char type.
  //
  // For some reason, you cannot specialize a method of a template and have the template be inherited which caused major
  // issues during compilation.  Every member variable was being reported as unknown and the _Connections type was being
  //  lost.
  //
  template < typename PortTraits >
    class OutInt8Port: public OutPort<  PortTraits >  {

  public:
    typedef PortTraits                        Traits;

    //
    // Port Variable Definition
    //
    typedef typename Traits::PortVarType      PortVarType;

    //
    // BULKIO Interface Type
    //
    typedef typename Traits::PortType         PortType;

    typedef typename Traits::PortTraits       UsesPortType;

    //
    // Sequence container used during actual pushPacket call
    //
    typedef typename Traits::SequenceType     PortSequenceType;

    //
    // Data type contained in sequence container
    //
    typedef typename Traits::TransportType    TransportType;

    // 
    // Data type of the container for passing data into the pushPacket method
    //
    typedef char*                             NativeSequenceType;

    //
    // Data type of items passed into the pushPacket method
    //
    typedef typename Traits::NativeType       NativeType;

    OutInt8Port(std::string port_name,
		ConnectionEventListener *connectCB=NULL,
		ConnectionEventListener *disconnectCB=NULL );

    OutInt8Port(std::string port_name, 
		LOGGER_PTR logger,
		ConnectionEventListener *connectCB=NULL,
		ConnectionEventListener *disconnectCB=NULL );


    virtual ~OutInt8Port() {};

    void pushPacket( std::vector< Int8 >& data, BULKIO::PrecisionUTCTime& T, bool EOS, const std::string& streamID);

  };



  //
  // OutStringPort
  //
  // This class defines the pushPacket interface for string of data. This template is use by 
  // both the File and XML port classes for pushing data downstream.  
  //
  //
  template < typename PortTraits >
    class OutStringPort : public OutPort < PortTraits > {

  public:

    typedef PortTraits                        Traits;

    //
    // Port Variable Definition
    //
    typedef typename Traits::PortVarType      PortVarType;

    //
    // BULKIO Interface Type
    //
    typedef typename Traits::PortType         PortType;

    typedef typename Traits::PortTraits       UsesPortType;

    //
    // Sequence container used during actual pushPacket call
    //
    typedef typename Traits::SequenceType     PortSequenceType;

    //
    // Data type contained in sequence container
    //
    typedef typename Traits::TransportType    TransportType;

    // 
    // Data type of the container for passing data into the pushPacket method
    //
    typedef char*                             NativeSequenceType;

    //
    // Data type of items passed into the pushPacket method
    //
    typedef typename Traits::NativeType       NativeType;


    OutStringPort( std::string pname, 
		ConnectionEventListener *connectCB=NULL,
		ConnectionEventListener *disconnectCB=NULL );


    OutStringPort(std::string port_name, 
		  LOGGER_PTR logger, 
		  ConnectionEventListener *connectCB=NULL,
		  ConnectionEventListener *disconnectCB=NULL );



    virtual ~OutStringPort() {};


    /*
     * pushPacket
     *     maps to dataFile BULKIO method call for passing strings of data 
     *
     *  data: sequence structure containing the payload to send out
     *  T: constant of type BULKIO::PrecisionUTCTime containing the timestamp for the outgoing data.
     *    tcmode: timecode mode
     *    tcstatus: timecode status
     *    toff: fractional sample offset
     *    twsec: J1970 GMT
     *    tfsec: fractional seconds: 0.0 to 1.0
     *  EOS: end-of-stream flag
     *  streamID: stream identifier
     */
    virtual void  pushPacket(const char *data, BULKIO::PrecisionUTCTime& T, bool EOS, const std::string& streamID);

    /*
     * pushPacket
     *     maps to dataXML BULKIO method call for passing strings of data
     *
     *  data: sequence structure containing the payload to send out
     *  T: constant of type BULKIO::PrecisionUTCTime containing the timestamp for the outgoing data.
     *    tcmode: timecode mode
     *    tcstatus: timecode status
     *    toff: fractional sample offset
     *    twsec: J1970 GMT
     *    tfsec: fractional seconds: 0.0 to 1.0
     *  EOS: end-of-stream flag
     *  streamID: stream identifier
     */
    virtual void  pushPacket(const char *data, bool EOS, const std::string& streamID);

  };


  /**
     Uses Port Definitions for All Bulk IO port definitions
     *
     */
  typedef OutInt8Port<  CharPortTraits >     OutCharPort;
  typedef OutPort< OctetPortTraits >         OutOctetPort;
  typedef OutOctetPort                       OutUInt8Port;
  typedef OutPort<  ShortPortTraits >        OutShortPort;
  typedef OutPort<  UShortPortTraits >       OutUShortPort;
  typedef OutShortPort                       OutInt16Port;
  typedef OutUShortPort                      OutUInt16Port;
  typedef OutPort<  LongPortTraits >         OutLongPort;
  typedef OutPort< ULongPortTraits >         OutULongPort;
  typedef OutLongPort                        OutInt32Port;
  typedef OutULongPort                       OutUInt32Port;
  typedef OutPort<  LongLongPortTraits >     OutLongLongPort;
  typedef OutPort<  ULongLongPortTraits >    OutULongLongPort;
  typedef OutLongLongPort                    OutInt64Port;
  typedef OutULongLongPort                   OutUInt64Port;
  typedef OutPort<  FloatPortTraits >        OutFloatPort;
  typedef OutPort<  DoublePortTraits >       OutDoublePort;
  typedef OutStringPort< URLPortTraits >     OutURLPort;
  typedef OutStringPort< FilePortTraits >    OutFilePort;
  typedef OutStringPort< XMLPortTraits >     OutXMLPort;


}  // end of bulkio namespace


#endif
