
#include <iostream>
#include "ossie/ossieSupport.h"

#include "TestLargePush.h"
int main(int argc, char* argv[])
{
    TestLargePush_i* TestLargePush_servant;
    Resource_impl::start_component(TestLargePush_servant, argc, argv);
    return 0;
}

