#
#
#

#
# base_port
#  runs same set of tests for each type of port specified...
#

from base_ports  import *


class Test_CPP_Int8(BaseVectorPort):
    def __init__(self, methodName='runTest', ptype='Int8', cname='CPP_Ports' ):
        BaseVectorPort.__init__(self, methodName, ptype, cname )
        pass

class Test_CPP_Int16(BaseVectorPort):
    def __init__(self, methodName='runTest', ptype='Int16', cname='CPP_Ports' ):
        BaseVectorPort.__init__(self, methodName, ptype, cname ,
                                bio_in_module=bulkio.InShortPort,
                                bio_out_module=bulkio.OutShortPort )
        pass

class Test_CPP_Int32(BaseVectorPort):
    def __init__(self, methodName='runTest', ptype='Int32', cname='CPP_Ports' ):
        BaseVectorPort.__init__(self, methodName, ptype, cname, 
                                bio_in_module=bulkio.InLongPort,
                                bio_out_module=bulkio.OutLongPort )
        pass

class Test_CPP_Int64(BaseVectorPort):
    def __init__(self, methodName='runTest', ptype='Int64', cname='CPP_Ports' ):
        BaseVectorPort.__init__(self, methodName, ptype, cname,     
                                bio_in_module=bulkio.InLongLongPort,
                                bio_out_module=bulkio.OutLongLongPort )
        pass

class Test_CPP_Float(BaseVectorPort):
    def __init__(self, methodName='runTest', ptype='Float', cname='CPP_Ports' ):
        BaseVectorPort.__init__(self, methodName, ptype, cname , 
                                bio_in_module=bulkio.InFloatPort,
                                bio_out_module=bulkio.OutFloatPort )
        pass

class Test_CPP_Double(BaseVectorPort):
    def __init__(self, methodName='runTest', ptype='Double', cname='CPP_Ports' ):
        BaseVectorPort.__init__(self, methodName, ptype, cname,
                                bio_in_module=bulkio.InDoublePort,
                                bio_out_module=bulkio.OutDoublePort )
        pass

class Test_CPP_File(BaseVectorPort):
    _sample = "The quick brown fox jumped over the lazy dog"
    def __init__(self, methodName='runTest', ptype='File', cname='CPP_Ports', srcData=_sample ):
        BaseVectorPort.__init__(self, methodName, ptype, cname, 
                                bio_in_module=bulkio.InFilePort,
                                bio_out_module=bulkio.OutFilePort )
        pass


if __name__ == '__main__':
    suite = unittest.TestSuite()
    for x in [ Test_CPP_Int8, Test_CPP_Int16,  Test_CPP_Int32, Test_CPP_Int64, Test_CPP_Float, Test_CPP_Double ]:
        tests = unittest.TestLoader().loadTestsFromTestCase(x)
        suite.addTests(tests)
    unittest.TextTestRunner(verbosity=2).run(suite)

