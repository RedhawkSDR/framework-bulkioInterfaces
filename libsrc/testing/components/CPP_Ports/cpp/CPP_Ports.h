
#ifndef CPP_PORTS_IMPL_H
#define CPP_PORTS_IMPL_H

#include "CPP_Ports_base.h"

class CPP_Ports_i;

class CPP_Ports_i : public CPP_Ports_base
{

    ENABLE_LOGGING
    public:
        CPP_Ports_i(const char *uuid, const char *label);
        ~CPP_Ports_i();
        int serviceFunction();
        void initialize() throw (CF::LifeCycle::InitializeError, CORBA::SystemException);

 public:
    void newStreamCallback( BULKIO::StreamSRI &sri ) {
    }
};

#endif
