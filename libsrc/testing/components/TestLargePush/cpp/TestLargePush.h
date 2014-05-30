#ifndef TESTLARGEPUSH_IMPL_H
#define TESTLARGEPUSH_IMPL_H

#include "TestLargePush_base.h"

class TestLargePush_i : public TestLargePush_base
{
    ENABLE_LOGGING
    public:
        TestLargePush_i(const char *uuid, const char *label);
        ~TestLargePush_i();
        int serviceFunction();
        void serviceAsBuffers();
        void serviceAsVectors();
};

#endif
