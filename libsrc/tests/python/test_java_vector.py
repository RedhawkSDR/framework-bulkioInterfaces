
from base_ports  import *


class Test_Java_Int8(BaseVectorPort):
    def __init__(self, methodName='runTest', ptype='Int8', cname='Java_Ports' ):
        BaseVectorPort.__init__(self, methodName, ptype, cname )
        pass

class Test_Java_Int16(BaseVectorPort):
    def __init__(self, methodName='runTest', ptype='Int16', cname='Java_Ports' ):
        BaseVectorPort.__init__(self, methodName, ptype, cname )
        pass

class Test_Java_Int32(BaseVectorPort):
    def __init__(self, methodName='runTest', ptype='Int32', cname='Java_Ports' ):
        BaseVectorPort.__init__(self, methodName, ptype, cname )
        pass

class Test_Java_Int64(BaseVectorPort):
    def __init__(self, methodName='runTest', ptype='Int64', cname='Java_Ports' ):
        BaseVectorPort.__init__(self, methodName, ptype, cname )
        pass

class Test_Java_Float(BaseVectorPort):
    def __init__(self, methodName='runTest', ptype='Float', cname='Java_Ports' ):
        BaseVectorPort.__init__(self, methodName, ptype, cname )
        pass

class Test_Java_Double(BaseVectorPort):
    def __init__(self, methodName='runTest', ptype='Double', cname='Java_Ports' ):
        BaseVectorPort.__init__(self, methodName, ptype, cname )
        pass


if __name__ == '__main__':
    suite = unittest.TestSuite()
    for x in [ Test_Java_Int8, Test_Java_Int16,  Test_Java_Int32, Test_Java_Int64, Test_Java_Float, Test_Java_Double ]:
        tests = unittest.TestLoader().loadTestsFromTestCase(x)
        suite.addTests(tests)
    unittest.TextTestRunner(verbosity=2).run(suite)



##python -m unittest test_module1 test_module2
##python -m unittest test_module.TestClass
##python -m unittest test_module.TestClass.test_method
##You can pass in a list with any combination of module names, and fully qualified class or method names.
##You can run tests with more detail (higher verbosity) by passing in the -v flag:
##python -m unittest -v test_module
##For a list of all the command-line options:
##python -m unittest -h

