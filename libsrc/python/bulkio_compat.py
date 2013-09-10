import warnings
warnings.warn("Using deprecated syntax for port variables",DeprecationWarning)
from bulkio import *
class PortBULKIODataCharIn_i(InCharPort):
    def __init__(self,parent,name,maxsize=100):
        InCharPort.__init__(self,name,maxsize=maxsize)

class PortBULKIODataDoubleIn_i(InDoublePort):
    def __init__(self,parent,name,maxsize=100):
        InDoublePort.__init__(self,name,maxsize=maxsize)

class PortBULKIODataFloatIn_i(InFloatPort):
    def __init__(self,parent,name,maxsize=100):
        InFloatPort.__init__(self,name,maxsize=maxsize)

class PortBULKIODataLongIn_i(InLongPort):
    def __init__(self,parent,name,maxsize=100):
        InLongPort.__init__(self,name,maxsize=maxsize)

class PortBULKIODataLongLongIn_i(InLongLongPort):
    def __init__(self,parent,name,maxsize=100):
        InLongLongPort.__init__(self,name,maxsize=maxsize)

class PortBULKIODataOctetIn_i(InOctetPort):
    def __init__(self,parent,name,maxsize=100):
        InOctetPort.__init__(self,name,maxsize=maxsize)

class PortBULKIODataShortIn_i(InShortPort):
    def __init__(self,parent,name,maxsize=100):
        InShortPort.__init__(self,name,maxsize=maxsize)

class PortBULKIODataUlongIn_i(InULongPort):
    def __init__(self,parent,name,maxsize=100):
        InULongPort.__init__(self,name,maxsize=maxsize)

class PortBULKIODataUlongLongIn_i(InULongLongPort):
    def __init__(self,parent,name,maxsize=100):
        InULongLongPort.__init__(self,name,maxsize=maxsize)

class PortBULKIODataFileIn_i(InFilePort):
    def __init__(self,parent,name,maxsize=100):
        InFilePort.__init__(self,name,maxsize=maxsize)

class PortBULKIODataSDDSIn_i(InSDDSPort):
    def __init__(self,parent,name,maxsize=100):
        InSDDSPort.__init__(self,name,maxsize=maxsize)

class PortBULKIODataXMLIn_i(InXMLPort):
    def __init__(self,parent,name,maxsize=100):
        InXMLPort.__init__(self,name,maxsize=maxsize)

class PortBULKIODataUshortIn_i(InUShortPort):
    def __init__(self,parent,name,maxsize=100):
        InUShortPort.__init__(self,name,maxsize=maxsize)

class PortBULKIODataCharOut_i(OutCharPort):
    def __init__(self,parent,name):
        OutCharPort.__init__(self,name)

class PortBULKIODataDoubleOut_i(OutDoublePort):
    def __init__(self,parent,name):
        OutDoublePort.__init__(self,name)

class PortBULKIODataFloatOut_i(OutFloatPort):
    def __init__(self,parent,name):
        OutFloatPort.__init__(self,name)

class PortBULKIODataLongOut_i(OutLongPort):
    def __init__(self,parent,name):
        OutLongPort.__init__(self,name)

class PortBULKIODataLongLongOut_i(OutLongLongPort):
    def __init__(self,parent,name):
        OutLongLongPort.__init__(self,name)

class PortBULKIODataOctetOut_i(OutOctetPort):
    def __init__(self,parent,name):
        OutOctetPort.__init__(self,name)

class PortBULKIODataShortOut_i(OutShortPort):
    def __init__(self,parent,name):
        OutShortPort.__init__(self,name)

class PortBULKIODataUlongOut_i(OutULongPort):
    def __init__(self,parent,name):
        OutULongPort.__init__(self,name)

class PortBULKIODataUlongLongOut_i(OutULongLongPort):
    def __init__(self,parent,name):
        OutULongLongPort.__init__(self,name)

class PortBULKIODataUshortOut_i(OutUShortPort):
    def __init__(self,parent,name):
        OutUShortPort.__init__(self,name)

class PortBULKIODataFileOut_i(OutFilePort):
    def __init__(self,parent,name):
        OutFilePort.__init__(self,name)

class PortBULKIODataSDDSOut_i(OutSDDSPort):
    def __init__(self,parent,name):
        OutSDDSPort.__init__(self,name)

class PortBULKIODataXMLOut_i(OutXMLPort):
    def __init__(self,parent,name):
        OutXMLPort.__init__(self,name)
