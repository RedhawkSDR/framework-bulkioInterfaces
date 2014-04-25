
#ifndef __bulkio_p_h__
#define __bulkio_p_h__

#include <queue>
#include <list>
#include <boost/make_shared.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/locks.hpp>
#include <ossie/prop_helpers.h>
#include <ossie/BULKIO/bio_runtimeStats.h>

#include "bulkio.h"
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

#define LOG_INFO(logger, expr )   if ( logger ) RH_INFO(logger, expr );
#define LOG_ERROR(logger, expr )  if ( logger ) RH_ERROR(logger, expr );
#define LOG_WARN(logger, expr )  if ( logger )  RH_WARN(logger, expr );
#define LOG_FATAL(logger, expr )  if ( logger ) RH_FATAL(logger, expr );
#define LOG_DEBUG(logger, expr )  if ( logger ) RH_DEBUG(logger, expr );
#define LOG_TRACE(logger, expr )  if ( logger ) RH_TRACE(logger, expr );

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





}   // end of namespace


#endif  // __bulkio_p_h__

