#ifndef MULTIOUT_ATTACHABLE_IMPL_BASE_H
#define MULTIOUT_ATTACHABLE_IMPL_BASE_H

#include <boost/thread.hpp>
#include <ossie/Resource_impl.h>
#include <ossie/ThreadedComponent.h>

//Need to comment this out to build locally 
//#include <bulkio/bulkio.h>
#include "bulkio.h"
#include "struct_props.h"

class multiout_attachable_base : public Resource_impl, protected ThreadedComponent
{
    public:
        multiout_attachable_base(const char *uuid, const char *label);
        ~multiout_attachable_base();

        void start() throw (CF::Resource::StartError, CORBA::SystemException);

        void stop() throw (CF::Resource::StopError, CORBA::SystemException);

        void releaseObject() throw (CF::LifeCycle::ReleaseError, CORBA::SystemException);

        void loadProperties();

    protected:
        void connectionTableChanged(const std::vector<connection_descriptor_struct>* oldValue, const std::vector<connection_descriptor_struct>* newValue);

        // Member variables exposed as properties
        callback_stats_struct callback_stats;
        std::vector<connection_descriptor_struct> connectionTable;
        std::vector<SDDSStreamDefinition_struct> SDDSStreamDefinitions;
        std::vector<VITA49StreamDefinition_struct> VITA49StreamDefinitions;
        std::vector<sdds_attachment_struct> received_sdds_attachments;
        std::vector<vita49_attachment_struct> received_vita49_attachments;

        // Ports
        bulkio::InSDDSPort *dataSDDS_in;
        bulkio::InVITA49Port *dataVITA49_in;
        bulkio::InFloatPort *dataFloat_in;
        bulkio::OutSDDSPort *dataSDDS_out;
        bulkio::OutVITA49Port *dataVITA49_out;

    private:
};
#endif // MULTIOUT_ATTACHABLE_IMPL_BASE_H
