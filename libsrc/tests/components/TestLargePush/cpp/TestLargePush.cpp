/**************************************************************************

    This is the component code. This file contains the child class where
    custom functionality can be added to the component. Custom
    functionality to the base class can be extended here. Access to
    the ports can also be done from this class

**************************************************************************/

#include "TestLargePush.h"

PREPARE_LOGGING(TestLargePush_i)

TestLargePush_i::TestLargePush_i(const char *uuid, const char *label) :
    TestLargePush_base(uuid, label)
{
}

TestLargePush_i::~TestLargePush_i()
{
}

int TestLargePush_i::serviceFunction()
{

    BULKIO::PrecisionUTCTime timestamp = BULKIO::PrecisionUTCTime();
    std::string streamID = "test";
    bool EOS = true;

    std::vector<float> outputDataFloat;
    outputDataFloat.resize(numSamples);
    dataFloat->pushPacket(
            outputDataFloat,    /* data vector */
            timestamp,          /* timestamp   */
            EOS,                /* EOS         */
            streamID);          /* streamID    */


    std::vector<double> outputDataDouble;
    outputDataDouble.resize(numSamples);
    dataDouble->pushPacket(
            outputDataDouble,    /* data vector */
            timestamp,          /* timestamp   */
            EOS,                /* EOS         */
            streamID);          /* streamID    */

    std::vector<unsigned short> outputDataUshort;
    outputDataUshort.resize(numSamples);
    dataUshort->pushPacket(
            outputDataUshort,    /* data vector */
            timestamp,          /* timestamp   */
            EOS,                /* EOS         */
            streamID);          /* streamID    */

    std::vector<CORBA::ULong> outputDataUlong;
    outputDataUlong.resize(numSamples);
    dataUlong->pushPacket(
            outputDataUlong,    /* data vector */
            timestamp,          /* timestamp   */
            EOS,                /* EOS         */
            streamID);          /* streamID    */

    std::vector<CORBA::Long> outputDataLong;
    outputDataLong.resize(numSamples);
    dataLong->pushPacket(
            outputDataLong,    /* data vector */
            timestamp,          /* timestamp   */
            EOS,                /* EOS         */
            streamID);          /* streamID    */

    std::vector<CORBA::ULongLong> outputDataUlongLong;
    outputDataUlongLong.resize(numSamples);
    dataUlongLong->pushPacket(
            outputDataUlongLong,/* data vector */
            timestamp,          /* timestamp   */
            EOS,                /* EOS         */
            streamID);          /* streamID    */

    std::vector<CORBA::LongLong> outputDataLongLong;
    outputDataLongLong.resize(numSamples);
    dataLongLong->pushPacket(
            outputDataLongLong,    /* data vector */
            timestamp,          /* timestamp   */
            EOS,                /* EOS         */
            streamID);          /* streamID    */

    std::vector<short> outputDataShort;
    outputDataShort.resize(numSamples);
    dataShort->pushPacket(
            outputDataShort,    /* data vector */
            timestamp,          /* timestamp   */
            EOS,                /* EOS         */
            streamID);          /* streamID    */

    std::vector<CORBA::Octet> outputDataOctet;
    outputDataOctet.resize(numSamples);
    dataOctet->pushPacket(
            outputDataOctet,    /* data vector */
            timestamp,          /* timestamp   */
            EOS,                /* EOS         */
            streamID);          /* streamID    */

    return FINISH;
}
