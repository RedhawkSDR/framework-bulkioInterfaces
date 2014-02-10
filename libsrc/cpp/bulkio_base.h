
#ifndef __bulkio_base_h
#define __bulkio_base_h

#include <queue>
#include <list>
#include <vector>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/locks.hpp>
#include <ossie/debug.h>
#include <ossie/BULKIO/bio_runtimeStats.h>
#include <ossie/BULKIO/bulkioDataTypes.h>


namespace bulkio {


  //
  // helper class for port statistics 
  //
  class  linkStatistics;

  //
  // helper class to manage queue depth and back pressure
  //
  class  queueSemaphore;

  //
  // ConnectionList container,  (allows for template typedef defs to occur)
  //
 template < typename T > class Connections {
  private:
    Connections(void) {};

  public:

    /*
     * use end point definition for more descriptive error messages
    struct EndPoint {
      T            bio_port;
      std::string  cid;
      PT           cf_port;
    };

    typedef typename std::vector< EndPoint >  List;
    */

    typedef typename std::vector< std::pair< T, std::string > >  List;

  };


  //
  // Mapping of Stream IDs to SRI objects
  //
  typedef std::map< std::string, std::pair< BULKIO::StreamSRI, bool > >  SriMap;

  //
  // Listing of Stream IDs for searching
  //
  typedef std::list < std::string >   StreamIDList;

  //
  //  Common Name for Mutex construct used by port classes
  //
  typedef boost::mutex                MUTEX;

  //
  //  Common Name for Condition construct used by port classes
  //
  typedef boost::condition_variable   CONDITION;

  //
  // Auto lock/unlock of mutex objects based on language scope
  //
  typedef boost::unique_lock< boost::mutex >   UNIQUE_LOCK;

  //
  // Auto lock/unlock of mutex objects based on language scope
  //
  typedef boost::mutex::scoped_lock           SCOPED_LOCK;


  //
  // Logging interface definition 
  //
#if HAVE_LOG4CXX
  typedef log4cxx::LoggerPtr         LOGGER_PTR;
#else
  typedef std::string         LOGGER_PTR;
#endif

  //
  //  Base Types used by Ports
  //
  typedef char                        Char;
  typedef int8_t                      Int8;
  typedef uint8_t                     UInt8;
  typedef int16_t                     Int16;
  typedef uint16_t                    UInt16;

  typedef int32_t                     Int32;
  typedef uint32_t                    UInt32;

  typedef int64_t                     Int64;
  typedef uint64_t                    UInt64;
  typedef float                       Float;
  typedef double                      Double;

  namespace Const {

    //
    // Maximum transfer size for middleware
    //
    const uint64_t  MAX_TRANSFER_BYTES =  omniORB::giopMaxMsgSize();

    //
    // Constant that defines if retrieval of data from a port's queue will NOT block
    //
    const  float    NON_BLOCKING = 0.0;

    //
    // Constant that defines if retrieval of data from a ports's queue will BLOCK
    //
    const  float    BLOCKING = -1.0;

  };


  //
  //
  // Time Stamp Helpers
  //
  //
  namespace time {

    /**
       PrecisionUTCTime object as defined by bulkio_dataTypes.idl, definition provided for information only

    //
    // Time code modes
    //
    const short TCM_OFF  = 0;
    const short TCM_CPU  = 1;
    const short TCM_ZTC  = 2;
    const short TCM_SDN  = 3;
    const short TCM_SMS  = 4;
    const short TCM_DTL  = 5;
    const short TCM_IRB  = 6;
    const short TCM_SDDS = 7;

    struct PrecisionUTCTime {
        short tcmode;        timecode mode 
        short tcstatus;      timecode status 
        double toff;         Fractional sample offset 
        double twsec;        J1970 GMT 
        double tfsec;        0.0 to 1.0 

    };

    **/

    namespace utils {

      //
      // Create a time stamp object from the provided input... 
      //
      BULKIO::PrecisionUTCTime create( const double wholeSecs=-1.0, const double fractionalSecs=-1.0, const Int16 tsrc= BULKIO::TCM_CPU  );

      //
      // Create a time stamp object from the current time of day reported by the system
      //
      BULKIO::PrecisionUTCTime now();
    };


    //
    // A default time stamp comparison method 
    //
    bool           DefaultComparator( const BULKIO::PrecisionUTCTime &a, const BULKIO::PrecisionUTCTime &b);

    //
    // Method signature for comparing time stamp objects
    //
    typedef bool  (*Compare)( const BULKIO::PrecisionUTCTime &a, const BULKIO::PrecisionUTCTime &b);

  };


  //
  // StreamSRI
  //
  // Convenience routines for building and working with StreamSRI objects
  //
  // TODO: 
  //   Convenience templates to add key/value pairs
  //
  namespace sri {

    /**
       StreamSRI object as defined by bulkio_dataTypes.idl, definition provided for information only

    struct StreamSRI {
        long hversion;     version of the StreamSRI header 
        double xstart;     start time of the stream 
        double xdelta;     delta between two samples 
        short xunits;      unit types from Platinum specification; common codes defined above 
        long subsize;      0 if the data is one dimensional; > 0 if two dimensional 
        double ystart;     start of second dimension 
        double ydelta;     delta between two samples of second dimension 
        short yunits;      unit types from Platinum specification; common codes defined above 
        short mode;        0-Scalar, 1-Complex 
        string streamID;   stream identifier 
        boolean blocking;  flag to determine whether the receiving port should exhibit back pressure
        sequence<CF::DataType> keywords;  user defined keywords 
    };
  **/
 
    //
    //  Comparator method to search for matching SRI information if "a" matches "b"
    //
    typedef bool  (*Compare)( const BULKIO::StreamSRI &a, const BULKIO::StreamSRI &b);

    //
    // Default comparator method when comparing SRI objects
    //
    // Performs a member wise comparision of a StreamSRI object. In addition to performing
    // this comparison, any additional key/value pairs will be compared. The key identifiers 
    // are compared in order, and their associated values are compared using the
    // equivalency method of the REDHAWK framework compare_anys method.
    //
    bool           DefaultComparator( const BULKIO::StreamSRI &a, const BULKIO::StreamSRI &b);

    //
    // Create a SRI object with default parameters
    //
    BULKIO::StreamSRI create( std::string sid="defStream", const double srate = 1.0, const Int16 xunits = BULKIO::UNITS_TIME, const bool blocking=false );


  };


}  // end of bulkio namespace


#endif
