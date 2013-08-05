import random
import unittest
import sys
from ossie.utils import sb
import time


# remove when sandbox support for relative path works
test_dir=''

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
    
    def __init__(self, methodName='runTest', ptype='Int8', cname=None, srcData=None, cmpData=None):
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

    def runTest(self):
        ##print self.ctx
        dsource=sb.DataSource()
        dsink=sb.DataSink()
        c_spd_xml = test_dir + self.c_dir + '/' + self.c_name + '/' + self.c_name + '.spd.xml'
	print c_spd_xml
        test_comp=sb.Component( c_spd_xml, execparams=self.execparams)
        if self.seq:
            data=self.seq

        cmp_data = data
        if self.cmpData:
            cmp_data = self.cmpData

        dsource.connect(test_comp, providesPortName=self.c_inport )
        test_comp.connect(dsink, providesPortName=self.sink_inport, usesPortName=self.c_outport)
        sb.start()
        dsource.push(data,EOS=True)
        dest_data=dsink.getData(eos_block=True)
        sb.stop()

        self.assertEqual(data, dest_data)


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
