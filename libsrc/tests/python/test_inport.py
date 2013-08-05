import random
import unittest
import sys
import time
import types
from ossie.utils import sb

def str_to_class(s):
    if s in globals() and isinstance(globals()[s], types.ClassType):
        return globals()[s]
    return None

class BaseVectorPort(unittest.TestCase):
    def __init__(self, methodName='runTest'):
        unittest.TestCase.__init__(self, methodName)
        self.cname = 'Java_Ports'
        self.pin_name = 'dataShortIn'
        self.pout_name = 'dataShortOut'
        self.sink_port_name = 'shortIn'
        

    def setUp(self):
        self.seq = range(10)

    def test_vector(self):
        dsource=sb.DataSource()
        dsink=sb.DataSink()
        test_comp=sb.Component(self.cname)
        data=range(100)
        dsource.connect(test_comp, providesPortName=self.pin_name )
        test_comp.connect(dsink, providesPortName=self.sink_port_name, usesPortName=self.pout_name)
        sb.start()
        dsource.push(data,EOS=True)
        dest_data=dsink.getData(eos_block=True)
        sb.stop()

        self.assertEqual(data, dest_data)


class TestInt8Port(BaseVectorPort):
    def __init__(self, methodName='runTest'):
        BaseVectorPort.__init__(self, methodName)
        self.cname = 'CPP_Ports'
        self.pin_name = 'dataCharIn'
        self.pout_name = 'dataCharOut'
        self.sink_port_name = 'charIn'

class Test_Java_Int16(BaseVectorPort):
    def __init__(self, methodName='runTest'):
        BaseVectorPort.__init__(self, methodName)
        self.cname = 'Java_Ports'
        self.pin_name = 'dataShortIn'
        self.pout_name = 'dataShortOut'
        self.sink_port_name = 'shortIn'


class Test_CPP_Int16(BaseVectorPort):
    def __init__(self, methodName='runTest'):
        BaseVectorPort.__init__(self, methodName)
        self.cname = 'CPP_Ports'
        self.pin_name = 'dataShortIn'
        self.pout_name = 'dataShortOut'
        self.sink_port_name = 'shortIn'

if __name__ == '__main__':
    if len(sys.argv) < 1 :
        unittest.main()
    else:
        suite = unittest.TestLoader().loadTestsFromTestCase(globals()[sys.argv[1]] ) 
        unittest.TextTestRunner(verbosity=2).run(suite)

##python -m unittest test_module1 test_module2
##python -m unittest test_module.TestClass
##python -m unittest test_module.TestClass.test_method

##You can pass in a list with any combination of module names, and fully qualified class or method names.

##You can run tests with more detail (higher verbosity) by passing in the -v flag:

##python -m unittest -v test_module

##For a list of all the command-line options:

##python -m unittest -h

