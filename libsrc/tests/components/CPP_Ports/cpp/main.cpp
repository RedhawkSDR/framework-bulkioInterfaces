#include <iostream>
#include "ossie/ossieSupport.h"

#include "CPP_Ports.h"

 int main(int argc, char* argv[])
{
    CPP_Ports_i* CPP_Ports_servant;
    Resource_impl::start_component(CPP_Ports_servant, argc, argv);
}
