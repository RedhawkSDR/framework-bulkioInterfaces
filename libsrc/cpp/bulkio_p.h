
#ifndef __bulkio_p_h__
#define __bulkio_p_h__

#include <queue>
#include <list>
#include <boost/make_shared.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/locks.hpp>
#include <ossie/prop_helpers.h>
#include <BULKIO/bio_runtimeStats.h>

#include <bulkio.h>
#define CORBA_MAX_TRANSFER_BYTES omniORB::giopMaxMsgSize()

#ifdef LOGGING
#undef LOG_INFO
#undef LOG_ERROR
#undef LOG_WARN
#undef LOG_FATAL
#undef LOG_DEBUG
#undef LOG_TRACE
#undef TRACE_ENTER
#undef TRACE_EXIT

#define LOG_INFO(logger, expr )   if ( logger ) LOG4CXX_INFO(logger, expr );
#define LOG_ERROR(logger, expr )  if ( logger ) LOG4CXX_ERROR(logger, expr );
#define LOG_WARN(logger, expr )  if ( logger ) LOG4CXX_WARN(logger, expr );
#define LOG_FATAL(logger, expr )  if ( logger ) LOG4CXX_FATAL(logger, expr );
#define LOG_DEBUG(logger, expr )  if ( logger ) LOG4CXX_DEBUG(logger, expr );
#define LOG_TRACE(logger, expr )  if ( logger ) LOG4CXX_TRACE(logger, expr );

#ifdef TRACE_ENABLE
#define TRACE_ENTER(logger, method)						\
    LOG_TRACE(logger, "ENTER bulkio::" << method << " [" << __FILE__ << ":" << __LINE__ << "]")
#define TRACE_EXIT(logger, method)						\
    LOG_TRACE(logger, "EXIT bulkio::" << method << " [" << __FILE__ << ":" << __LINE__ << "]")
#else
#define TRACE_ENTER(logger, method )
#define TRACE_EXIT(logger, method )
#endif

#else
#define LOG_INFO(logger, expr )  
#define LOG_ERROR(logger, expr ) 
#define LOG_WARN(logger, expr )  
#define LOG_FATAL(logger, expr ) 
#define LOG_DEBUG(logger, expr ) 
#define LOG_TRACE(logger, expr ) 
#define TRACE_ENTER(logger, method )
#define TRACE_EXIT(logger, method )

#endif


namespace bulkio    {

  //
  // used for boost shared pointer instantion when user
  // supplied callback is provided
  //
  struct null_deleter
  {
    void operator()(void const *) const
    {
    }
  };


  class queueSemaphore {

  public:
    queueSemaphore(unsigned int initialMaxValue);

    void release();

    void setMaxValue(unsigned int newMaxValue);

    unsigned int getMaxValue(void);

    void setCurrValue(unsigned int newValue);

    void incr();

    void decr();

  private:
    unsigned int maxValue;
    unsigned int currValue;
    MUTEX mutex;
    CONDITION condition;

  };

  class linkStatistics
  {
  public:

    linkStatistics( std::string &portName, const int nbytes=1 );

    linkStatistics();

    virtual ~linkStatistics() {};

    virtual void setEnabled(bool enableStats);

    virtual void setBitSize( double bitSize );

    virtual void update(unsigned int elementsReceived, float queueSize, bool EOS, const std::string &streamID, bool flush = false);

    virtual BULKIO::PortStatistics retrieve();


  protected:

    struct statPoint {
      unsigned int elements;
      float queueSize;
      double secs;
      double usecs;
    };

    std::string  portName;      
    bool enabled;
    int  nbytes;
    double bitSize;
    BULKIO::PortStatistics runningStats;
    std::vector< statPoint > receivedStatistics;
    StreamIDList activeStreamIDs;
    unsigned long historyWindow;
    int receivedStatistics_idx;

    double flush_sec;                   // track time since last queue flush happened
    double flush_usec;                  // track time since last queue flush happened

  };


}   // end of namespace


#endif  // __bulkio_p_h__

