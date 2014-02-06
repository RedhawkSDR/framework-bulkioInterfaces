import random
import unittest
import sys
from ossie.utils import sb
import time
from bulkio.bulkioInterfaces.BULKIO import *

# remove when sandbox support for relative path works
test_dir='../'

def str_to_class(s):
    if s in globals() and isinstance(globals()[s], types.ClassType):
        return globals()[s]
    return None

class BaseVectorPort(unittest.TestCase):
    KEYS = ['c_name', 'c_inport', 'c_outport', 'sink_inport']
    PORT_FLOW = {
               'Int8' : [ 'dataCharIn', 'dataCharOut', 'charIn' ],
               'UInt8' : [ 'dataOctetIn', 'dataOctetOut', 'charIn' ],
               'Int16' : [ 'dataShortIn', 'dataShortOut', 'shortIn' ],
               'UInt16' : [ 'dataUShortIn', 'dataUShortOut', 'shortIn' ],
               'Int32' : [ 'dataLongIn', 'dataLongOut', 'longIn' ],
               'UInt32' : [ 'dataULongIn', 'dataULongOut', 'longIn' ],
               'Int64' : [ 'dataLongLongIn', 'dataLongLongOut', 'longlongIn' ],
               'UInt64' : [ 'dataULongLongIn', 'dataULongLongOut', 'longlongIn' ],
               'Float' : [ 'dataFloatIn', 'dataFloatOut', 'floatIn' ],
               'Double' : [ 'dataDoubleIn', 'dataDoubleOut', 'doubleIn' ],
               'File' : [ 'dataFileIn', 'dataFileOut', 'fileIn' ],
               'Xml' : [ 'dataXMLIn', 'dataXMLOut', 'xmlIn' ]
               }

    def __init__(
            self,
            methodName='runTest',
            ptype='Int8',
            cname=None,
            srcData=None,
            cmpData=None,
            bio_in_module=bulkio.InCharPort,
            bio_out_module=bulkio.OutCharPort ):
        unittest.TestCase.__init__(self, methodName)
        self.c_dir = 'components'
        self.c_name = cname
        self.ptype = ptype
        self.execparams = {}
        self.c_inport = None
        self.c_outport = None
        self.sink_inport = None
        self.srcData = srcData
        self.cmpData = cmpData
        self.ctx = dict().fromkeys(BaseVectorPort.KEYS)
        self.bio_in_module = bio_in_module
        self.bio_out_module = bio_out_module

    def getPortFlow(self, ptype='Int8' ):
        return BaseVectorPort.PORT_FLOW[ptype]

    def setContext(self, ctx=None):
        ##print "cname " + str(self.c_name) + " ptype= " + str(self.ptype)
        self.ctx[ BaseVectorPort.KEYS[0] ] = self.c_name
        self.ctx[ BaseVectorPort.KEYS[1] ] = BaseVectorPort.PORT_FLOW[self.ptype][0]
        self.ctx[ BaseVectorPort.KEYS[2] ] = BaseVectorPort.PORT_FLOW[self.ptype][1]
        self.ctx[ BaseVectorPort.KEYS[3] ] = BaseVectorPort.PORT_FLOW[self.ptype][2]
        tmp=self.ctx
        if ctx:
            tmp = ctx
        try:
            self.c_inport = tmp['c_inport']
            self.c_outport = tmp['c_outport']
            self.sink_inport = tmp['sink_inport']
        except:
            pass


    def setUp(self):
        self.setContext()
        if self.srcData:
            self.seq = self.srcData
        else:
            self.seq = range(100)

    def test_push_packet(self):
        ##print self.ctx
        dsource=sb.DataSource()
        dsink=sb.DataSink()
        c_spd_xml = test_dir + self.c_dir + '/' + self.c_name + '/' + self.c_name + '.spd.xml'
        print c_spd_xml
        test_comp=sb.launch( c_spd_xml, execparams=self.execparams)
        data=self.seq

        dsource.connect(test_comp, providesPortName=self.c_inport )
        test_comp.connect(dsink, providesPortName=self.sink_inport, usesPortName=self.c_outport)
        sb.start()
        sb.stop()

    def test_inport_api(self):
        ##print self.ctx
        c_spd_xml = test_dir + self.c_dir + '/' + self.c_name + '/' + self.c_name + '.spd.xml'
        print c_spd_xml
        test_comp=sb.launch( c_spd_xml, execparams=self.execparams)

        ##
        ## grab port from component... this is corba port
        ##
        iport = test_comp.getPort(self.c_inport)
        self.assertNotEqual(iport,None,"Cannot get Input Port")

        ps = iport._get_statistics()
        self.assertNotEqual(ps,None,"Cannot get Port Statistics")

        s = iport._get_state()
        self.assertNotEqual(s,None,"Cannot get Port State")
        self.assertEqual(s,IDLE,"Invalid Port State")

        streams = iport._get_activeSRIs()
        self.assertNotEqual(streams,None,"Cannot get Streams List")

        ##
        ## bulkio base class
        ##
        bio = self.bio_in_module("xxx")

        ps = bio._get_statistics()
        self.assertNotEqual(ps,None,"Cannot get Port Statistics")

        s = bio._get_state()
        self.assertNotEqual(s,None,"Cannot get Port State")
        self.assertEqual(s,IDLE,"Invalid Port State")

        streams = bio._get_activeSRIs()
        self.assertNotEqual(streams,None,"Cannot get Streams List")

        qed = bio.getMaxQueueDepth()
        self.assertEqual(qed,100,"Get Stream Depth Failed")

        bio.setMaxQueueDepth(22)
        qed = bio.getMaxQueueDepth()
        self.assertEqual(qed,22,"Set/Get Stream Depth Failed")

        ts = bulkio.timestamp.now()
        sri = bulkio.sri.create()
        sri.streamID = "test_port_api"
        bio.pushSRI(sri)

        data=range(50)
        bio.pushPacket(data, ts, False, "test_port_api")

        # result of getPacket
        #    DATA_BUFFER=0
        #    TIME_STAMP=1
        #    END_OF_STREAM=2
        #    STREAM_ID=3
        #    SRI=4
        #    SRI_CHG=5
        #    QUEUE_FLUSH=6
        ##      this is missing in python
        ##pkt = bio.getPacket(bulkio.const.NON_BLOCKING)
        pkt = bio.getPacket()
        self.assertNotEqual(pkt,None,"pushPacket .. getPacket Failed")
        self.assertNotEqual(pkt[bulkio.InPort.DATA_BUFFER],None,"pushPacket .. getPacket Failed")
        self.assertNotEqual(pkt[bulkio.InPort.TIME_STAMP],None,"pushPacket .. getPacket Failed")
        self.assertNotEqual(pkt[bulkio.InPort.END_OF_STREAM],None,"pushPacket .. getPacket Failed")
        self.assertNotEqual(pkt[bulkio.InPort.STREAM_ID],None,"pushPacket .. getPacket Failed")
        self.assertNotEqual(pkt[bulkio.InPort.SRI],None,"pushPacket .. getPacket Failed")
        self.assertNotEqual(pkt[bulkio.InPort.SRI_CHG],None,"pushPacket .. getPacket Failed")
        self.assertNotEqual(pkt[bulkio.InPort.QUEUE_FLUSH],None,"pushPacket .. getPacket Failed")

        pkt = bio.getPacket()
        self.assertNotEqual(pkt,None,"Second getPacket should be Empty")
        self.assertEqual(pkt[bulkio.InPort.DATA_BUFFER],None,"pushPacket .. getPacket Failed")
        self.assertEqual(pkt[bulkio.InPort.TIME_STAMP],None,"pushPacket .. getPacket Failed")
        self.assertEqual(pkt[bulkio.InPort.END_OF_STREAM],None,"pushPacket .. getPacket Failed")
        self.assertEqual(pkt[bulkio.InPort.STREAM_ID],None,"pushPacket .. getPacket Failed")
        self.assertEqual(pkt[bulkio.InPort.SRI],None,"pushPacket .. getPacket Failed")
        self.assertEqual(pkt[bulkio.InPort.SRI_CHG],None,"pushPacket .. getPacket Failed")
        self.assertEqual(pkt[bulkio.InPort.QUEUE_FLUSH],None,"pushPacket .. getPacket Failed")

        sri.streamID = "test_port_api"
        sri.mode = 1
        bio.pushSRI(sri)
        data=range(50)
        bio.pushPacket(data, ts, True, "test_port_api")
        pkt = bio.getPacket()
        self.assertNotEqual(pkt,None,"pushPacket... getPacket FAILED")
        self.assertNotEqual(pkt[bulkio.InPort.DATA_BUFFER],None,"EOS: pushPacket .. getPacket Failed")
        self.assertEqual(pkt[bulkio.InPort.END_OF_STREAM],True,"EOS: pushPacket .. getPacket EOS TEST Failed")
        self.assertEqual(pkt[bulkio.InPort.SRI].mode,1,"EOS: pushPacket .. getPacket COMPLEX MODE Failed")

        pkt = bio.getPacket()
        self.assertEqual(pkt[bulkio.InPort.DATA_BUFFER],None,"pushPacket .. getPacket EOS Failed")
        self.assertEqual(pkt[bulkio.InPort.TIME_STAMP],None,"pushPacket .. getPacket EOS Failed")
        self.assertEqual(pkt[bulkio.InPort.END_OF_STREAM],None,"pushPacket .. getPacket EOS Failed")
        self.assertEqual(pkt[bulkio.InPort.STREAM_ID],None,"pushPacket .. getPacket EOS Failed")
        self.assertEqual(pkt[bulkio.InPort.SRI],None,"pushPacket .. getPacket EOS Failed")
        self.assertEqual(pkt[bulkio.InPort.SRI_CHG],None,"pushPacket .. getPacket EOS Failed")
        self.assertEqual(pkt[bulkio.InPort.QUEUE_FLUSH],None,"pushPacket .. getPacket EOS Failed")

    def test_outport_api(self):
        ##print self.ctx
        c_spd_xml = test_dir + self.c_dir + '/' + self.c_name + '/' + self.c_name + '.spd.xml'
        print c_spd_xml
        test_comp=sb.launch( c_spd_xml, execparams=self.execparams)

        ##
        ## grab port from component... this is a corba port
        ##
        oport = test_comp.getPort(self.c_outport)
        self.assertNotEqual(oport,None,"Cannot get Output Port")

        ps = oport._get_statistics()
        self.assertNotEqual(ps,None,"Cannot get Port Statistics")

        ## Missing in Python bulkio interface port class
        ##s = oport._get_state()
        ##self.assertNotEqual(s,None,"Cannot get Port State")

        ##
        ## bulkio base class
        ##
        bio = self.bio_out_module("xxx")
        cl = bio._get_connections()
        self.assertNotEqual(cl,None,"Cannot get Connections List")
        self.assertEqual(len(cl),0,"Incorrect Connections List Length")

        ts = bulkio.timestamp.now()
        sri = bulkio.sri.create()
        sri.streamID = "test_port_api"
        bio.pushSRI(sri)

        data=range(50)
        bio.pushPacket(data, ts, False, "test_port_api")
        bio.pushPacket(data, ts, True,  "test_port_api")
        bio.pushPacket(data, ts, False, "unknown_port_api")

        ps = bio._get_statistics()
        self.assertNotEqual(ps,None,"Cannot get Port Statistics")

        cnt = len(bio.sriDict)
        self.assertEqual(cnt,0,"SRI list should be 0")

        bio.enableStats(False)



##
##class Base_CPP_Port(BaseVectorPort):
##    C_NAME='CPP_Ports'
##    def __init__(self, C_NAME, methodName='runTest', ptype='Int8', cname=C_NAME ):
##        BaseVectorPort.__init__(self, methodName, ptype, cname )
##        pass
##
##class Base_Python_Port(BaseVectorPort):
##    C_NAME='Python_Ports'
##    def __init__(self, C_NAME, methodName='runTest', ptype='Int8', cname=C_NAME ):
##        BaseVectorPort.__init__(self, methodName, ptype, cname )
##        pass
##
##class Base_Java_Port(BaseVectorPort):
##    C_NAME='Java_Ports'
##    def __init__(self, C_NAME, methodName='runTest', ptype='Int8', cname=C_NAME ):
##        BaseVectorPort.__init__(self, methodName, ptype, cname )
##        pass
