
/*******************************************************************************************


*******************************************************************************************/
#include <bulkio_p.h>

namespace  bulkio {


  namespace  time {


    namespace utils {

      BULKIO::PrecisionUTCTime create( const double wholeSecs, const double fractionalSecs, const bulkio::Int16 tsrc ) {

	double wsec = wholeSecs;
	double fsec = fractionalSecs;
	if ( wsec < 0.0 || fsec < 0.0 ) {
	  struct timeval tmp_time;
	  struct timezone tmp_tz;
	  gettimeofday(&tmp_time, &tmp_tz);
	  wsec = tmp_time.tv_sec;
	  fsec = tmp_time.tv_usec / 1e6;
	}
	BULKIO::PrecisionUTCTime tstamp = BULKIO::PrecisionUTCTime();
	tstamp.tcmode = tsrc;
	tstamp.tcstatus = (short)1;
	tstamp.toff = 0.0;
	tstamp.twsec = wsec;
	tstamp.tfsec = fsec;
	return tstamp;
      }

      BULKIO::PrecisionUTCTime now() {
	return create();
      }

    };

    bool DefaultComparator( const BULKIO::PrecisionUTCTime &T1, const BULKIO::PrecisionUTCTime &T2  ){
      if (T1.tcmode != T2.tcmode)
	return false;
      if (T1.tcstatus != T2.tcstatus)
	return false;
      if (T1.tfsec != T2.tfsec)
	return false;
      if (T1.toff != T2.toff)
	return false;
      if (T1.twsec != T2.twsec)
	return false;
      return true;
    }

  }  // end of timestamp namespace




} // end of bulkio namespace

