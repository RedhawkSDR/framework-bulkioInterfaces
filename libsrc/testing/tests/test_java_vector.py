
from base_ports  import *


class Test_Java_Int8(BaseVectorPort):
    def __init__(self, methodName='runTest', ptype='Int8', cname='Java_Ports' ):
        BaseVectorPort.__init__(self, methodName, ptype, cname )
        pass

class Test_Java_Int16(BaseVectorPort):
    def __init__(self, methodName='runTest', ptype='Int16', cname='Java_Ports' ):
        BaseVectorPort.__init__(self, methodName, ptype, cname, 
                                bio_in_module=bulkio.InShortPort,
                                bio_out_module=bulkio.OutShortPort )
        pass

class Test_Java_Int32(BaseVectorPort):
    def __init__(self, methodName='runTest', ptype='Int32', cname='Java_Ports' ):
        BaseVectorPort.__init__(self, methodName, ptype, cname, 
                                bio_in_module=bulkio.InLongPort,
                                bio_out_module=bulkio.OutLongPort )
        pass

class Test_Java_Int64(BaseVectorPort):
    def __init__(self, methodName='runTest', ptype='Int64', cname='Java_Ports' ):
        BaseVectorPort.__init__(self, methodName, ptype, cname, 
                                bio_in_module=bulkio.InLongLongPort,
                                bio_out_module=bulkio.OutLongLongPort )
        pass

class Test_Java_Float(BaseVectorPort):
    def __init__(self, methodName='runTest', ptype='Float', cname='Java_Ports' ):
        BaseVectorPort.__init__(self, methodName, ptype, cname, 
                                bio_in_module=bulkio.InFloatPort,
                                bio_out_module=bulkio.OutFloatPort )
        pass

class Test_Java_Double(BaseVectorPort):
    def __init__(self, methodName='runTest', ptype='Double', cname='Java_Ports' ):
        BaseVectorPort.__init__(self, methodName, ptype, cname, 
                                bio_in_module=bulkio.InDoublePort,
                                bio_out_module=bulkio.OutDoublePort )
        pass


if __name__ == '__main__':
    suite = unittest.TestSuite()
    for x in [ Test_Java_Int8, Test_Java_Int16,  Test_Java_Int32, Test_Java_Int64, Test_Java_Float, Test_Java_Double ]:
        tests = unittest.TestLoader().loadTestsFromTestCase(x)
        suite.addTests(tests)
    unittest.TextTestRunner(verbosity=2).run(suite)

