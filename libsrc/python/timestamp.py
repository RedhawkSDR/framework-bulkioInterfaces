import time
import copy

try:
    from bulkio.bulkioInterfaces import BULKIO, BULKIO__POA
except:
    pass

def now():
    """
    Generates a BULKIO.PrecisionUTCTime object using the current 
    CPU time that you can use in the pushPacket call
    """
    ts = time.time()
    return BULKIO.PrecisionUTCTime(BULKIO.TCM_CPU,
                                   BULKIO.TCS_VALID, 0.0,
                                   int(ts), ts - int(ts))

def notSet():
    """
    Generates a BULKIO.PrecisionUTCTime object with zero time 
    and an invalid flag. This is used by the automatic EOS
    """
    return BULKIO.PrecisionUTCTime(BULKIO.TCM_OFF,
                                   BULKIO.TCS_INVALID, 0.0,
                                   0.0, 0.0)

def cpuTimeStamp():
    return now()

def create( whole_secs=-1.0, fractional_secs=-1.0, tsrc=BULKIO.TCM_CPU ):
    """
    Generates a BULKIO.PrecisionUTCTime object using the current 
    CPU time that you can use in the pushPacket call
    """
    wsec = whole_secs;
    fsec = fractional_secs;
    if wsec < 0.0 and fsec < 0.0 :
        ts=time.time()
        wsec=int(ts)
        fsec = ts-int(ts)

    return BULKIO.PrecisionUTCTime(tsrc,
                                   BULKIO.TCS_VALID, 0.0,
                                   wsec, fsec )

def compare(T1, T2):
    """
    Will compare two BULKIO.PrecisionUTCTime objects and return True
    if they are both equal, and false otherwise
    """
    if not T1 or not T2:
        return False

    if T1.tcmode != T2.tcmode:
        return False
    if T1.tcstatus != T2.tcstatus:
        return False
    if T1.tfsec != T2.tfsec:
        return False
    if T1.toff != T2.toff:
        return False
    if T1.twsec != T2.twsec:
        return False
    return True

def addSampleOffset(T, numSamples=0, xdelta=0.0):
    tstamp = copy.deepcopy(T)
    tstamp.twsec += int(numSamples*xdelta)
    tstamp.tfsec += numSamples*xdelta - int(numSamples*xdelta)
    if tstamp.tfsec >= 1.0:
        tstamp.twsec += 1
        tstamp.tfsec -= 1.0
    return tstamp
