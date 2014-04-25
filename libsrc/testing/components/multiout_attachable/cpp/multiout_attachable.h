#ifndef MULTIOUT_ATTACHABLE_CPP_IMPL_H
#define MULTIOUT_ATTACHABLE_CPP_IMPL_H

#include "multiout_attachable_base.h"

class multiout_attachable_i : public multiout_attachable_base, 
                                  public bulkio::InSDDSPort::Callback,
                                  public bulkio::InVITA49Port::Callback
    
{
    typedef const std::vector<SDDSStreamDefinition_struct> SddsStreamDefs;
    typedef const std::vector<VITA49StreamDefinition_struct> Vita49StreamDefs;

    ENABLE_LOGGING
    public:
        multiout_attachable_i(const char *uuid, const char *label);
        ~multiout_attachable_i();
        int serviceFunction();

	    virtual char* attach(const BULKIO::SDDSStreamDefinition& stream, const char* userid)
	            throw (BULKIO::dataSDDS::AttachError, BULKIO::dataSDDS::StreamInputError);
	    
        virtual char* attach(const BULKIO::VITA49StreamDefinition& stream, const char* userid)
	            throw (BULKIO::dataVITA49::AttachError, BULKIO::dataVITA49::StreamInputError);

        // Applicable for both SDDS and VITA callback interface
	    virtual void detach(const char* attachId);

	    int reattaches;

	    void vita49StreamDefChanged(Vita49StreamDefs *oldValue, Vita49StreamDefs *newValue);
	    void sddsStreamDefChanged(SddsStreamDefs *oldValue, SddsStreamDefs *newValue);
};

#endif // MULTIOUT_ATTACHABLE_CPP_IMPL_H
