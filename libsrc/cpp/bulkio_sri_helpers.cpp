
/*******************************************************************************************


*******************************************************************************************/
#include <bulkio_p.h>

namespace  bulkio {

namespace  sri {
  
bool DefaultComparator( const BULKIO::StreamSRI &SRI_1, const BULKIO::StreamSRI &SRI_2){
    if (SRI_1.hversion != SRI_2.hversion)
        return false;
    if (SRI_1.xstart != SRI_2.xstart)
        return false;
    if (SRI_1.xdelta != SRI_2.xdelta)
        return false;
    if (SRI_1.xunits != SRI_2.xunits)
        return false;
    if (SRI_1.subsize != SRI_2.subsize)
        return false;
    if (SRI_1.ystart != SRI_2.ystart)
        return false;
    if (SRI_1.ydelta != SRI_2.ydelta)
        return false;
    if (SRI_1.yunits != SRI_2.yunits)
        return false;
    if (SRI_1.mode != SRI_2.mode)
        return false;
    if (strcmp(SRI_1.streamID, SRI_2.streamID) != 0)
        return false;
    if (SRI_1.keywords.length() != SRI_2.keywords.length())
        return false;
    std::string action = "eq";
    for (unsigned int i=0; i<SRI_1.keywords.length(); i++) {
        if (strcmp(SRI_1.keywords[i].id, SRI_2.keywords[i].id)) {
            return false;
        }
        CORBA::TypeCode_var SRI_1_type = SRI_1.keywords[i].value.type();
        CORBA::TypeCode_var SRI_2_type = SRI_2.keywords[i].value.type();
        if (!SRI_1_type->equivalent(SRI_2_type)) {
            return false;
        }
        if (!ossie::compare_anys(SRI_1.keywords[i].value, SRI_2.keywords[i].value, action)) {
            return false;
        }
    }
    return true;
}

  BULKIO::StreamSRI create( std::string sid, const double srate , const Int16 xunits, const bool blocking ) {

    BULKIO::StreamSRI sri;
    sri.hversion = 1;
    sri.xstart = 0.0;
    if ( srate <= 0.0 )
        sri.xdelta =  1.0;
    else
        sri.xdelta = 1/srate;
    sri.xunits = xunits;
    sri.subsize = 0;
    sri.ystart = 0.0;
    sri.ydelta = 0.0;
    sri.yunits = BULKIO::UNITS_NONE;
    sri.mode = 0;
    sri.blocking=blocking;
    sri.streamID = sid.c_str();
    return sri;

}


}


} // end of bulkio namespace

