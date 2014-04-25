#include <iostream>
#include "ossie/ossieSupport.h"

#include "multiout_attachable.h"
int main(int argc, char* argv[])
{
    multiout_attachable_i* multiout_attachable_servant;
    Resource_impl::start_component(multiout_attachable_servant, argc, argv);
    return 0;
}

