#!/usr/bin/env python
#
#
# AUTO-GENERATED
#
# Source: TestLargePush.spd.xml
from ossie.resource import Resource, start_component
import logging

from TestLargePush_base import *

class TestLargePush_i(TestLargePush_base):
    """<DESCRIPTION GOES HERE>"""
    def initialize(self):
        """
        This is called by the framework immediately after your component registers with the NameService.

        In general, you should add customization here and not in the __init__ constructor.  If you have 
        a custom port implementation you can override the specific implementation here with a statement
        similar to the following:
          self.some_port = MyPortImplementation()
        """
        TestLargePush_base.initialize(self)

    def process(self):
        outData     = [0 for x in range(self.numSamples)]
        T           = bulkio.timestamp.create()
        EOS         = True
        streamID    = "test"

        sri         = bulkio.sri.create(streamID)
        sri.xdelta  = 0.001

        self.port_dataFloat.pushSRI(sri)
        self.port_dataFloat.pushPacket(     outData, T, EOS, streamID)
        self.port_dataDouble.pushSRI(sri)
        self.port_dataDouble.pushPacket(    outData, T, EOS, streamID)
        self.port_dataShort.pushSRI(sri)
        self.port_dataShort.pushPacket(     outData, T, EOS, streamID)
        self.port_dataUshort.pushSRI(sri)
        self.port_dataUshort.pushPacket(    outData, T, EOS, streamID)
        self.port_dataLong.pushSRI(sri)
        self.port_dataLong.pushPacket(      outData, T, EOS, streamID)
        self.port_dataUlong.pushSRI(sri)
        self.port_dataUlong.pushPacket(     outData, T, EOS, streamID)
        self.port_dataLongLong.pushSRI(sri)
        self.port_dataLongLong.pushPacket(  outData, T, EOS, streamID)
        self.port_dataUlongLong.pushSRI(sri)
        self.port_dataUlongLong.pushPacket( outData, T, EOS, streamID)

        return FINISH

if __name__ == '__main__':
    logging.getLogger().setLevel(logging.WARN)
    logging.debug("Starting Component")
    start_component(TestLargePush_i)

