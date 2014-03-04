import time

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


