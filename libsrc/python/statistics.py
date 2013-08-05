
import time
import struct
from ossie.cf import CF
from omniORB import CORBA
from bulkio.bulkioInterfaces.BULKIO import PortStatistics

class InStats:
    class statPoint:
        def __init__(self):
            self.elements = 0
            self.queueSize = 0.0
            self.secs = 0.0

    def __init__(self, name, element_type ):
        self.enabled = True
        self.flushTime = None
        self.historyWindow = 10
        self.receivedStatistics = []
        self.name = name
        self.receivedStatistics_idx = 0
        self.bitSize = struct.calcsize(element_type) * 8
        self.activeStreamIDs = []
        for i in range(self.historyWindow):
            self.receivedStatistics.append(self.statPoint())
        self.runningStats = None

    def setEnabled(self, enableStats):
        self.enabled = enableStats

    def setBitSize(self, bitSize ):
        self.bitSize = bitSize

    def update(self, elementsReceived, queueSize, EOS, streamID, flush=False):
        if not self.enabled:
            return

        self.receivedStatistics[self.receivedStatistics_idx].elements = elementsReceived
        self.receivedStatistics[self.receivedStatistics_idx].queueSize = queueSize
        self.receivedStatistics[self.receivedStatistics_idx].secs = time.time()
        self.receivedStatistics_idx += 1
        self.receivedStatistics_idx = self.receivedStatistics_idx%self.historyWindow
        if flush:
            self.flushTime = self.receivedStatistics[self.receivedStatistics_idx].secs

        if EOS :
            try:
                self.activeStreamIDs.remove(streamID)
            except:
                pass
        else:
            if streamID not in self.activeStreamIDs:
                self.activeStreamIDs.append( streamID )

            

    def retrieve(self):
        if not self.enabled:
            return None

        self.runningStats = PortStatistics(portName=self.name, averageQueueDepth=-1, elementsPerSecond=-1, bitsPerSecond=-1, callsPerSecond=-1, streamIDs=[], timeSinceLastCall=-1, keywords=[])

        listPtr = (self.receivedStatistics_idx + 1) % self.historyWindow    # don't count the first set of data, since we're looking at change in time rather than absolute time
        frontTime = self.receivedStatistics[(self.receivedStatistics_idx - 1) % self.historyWindow].secs
        backTime = self.receivedStatistics[self.receivedStatistics_idx].secs
        totalData = 0.0
        queueSize = 0.0
        while (listPtr != self.receivedStatistics_idx):
            totalData += self.receivedStatistics[listPtr].elements
            queueSize += self.receivedStatistics[listPtr].queueSize
            listPtr += 1
            listPtr = listPtr%self.historyWindow

        # copy stream ids used 
        streamIDs = []
        for sid in self.activeStreamIDs:
            streamIDs.append(sid)

        receivedSize = len(self.receivedStatistics)
        currentTime = time.time()
        totalTime = currentTime - backTime
        if totalTime == 0:
            totalTime = 1e6
        self.runningStats.bitsPerSecond = (totalData * self.bitSize) / totalTime
        self.runningStats.elementsPerSecond = totalData / totalTime
        self.runningStats.averageQueueDepth = queueSize / receivedSize
        self.runningStats.callsPerSecond = float((receivedSize - 1)) / totalTime
        self.runningStats.streamIDs = streamIDs
        self.runningStats.timeSinceLastCall = currentTime - frontTime
        if not self.flushTime == None:
            flushTotalTime = currentTime - self.flushTime
            self.runningStats.keywords = [CF.DataType(id="timeSinceLastFlush", value=CORBA.Any(CORBA.TC_double, flushTotalTime))]

        return self.runningStats



class OutStats:
    class statPoint:
        def __init__(self):
            self.elements = 0
            self.queueSize = 0.0
            self.secs = 0.0
            self.streamID = ""

    def __init__(self, name, element_type ):
        self.enabled = True
        self.bitSize = struct.calcsize(element_type) * 8
        self.historyWindow = 10
        self.receivedStatistics = {}
        self.name = name
        self.receivedStatistics_idx = {}
        self.activeStreamIDs = []

    def setEnabled(self, enableStats):
        self.enabled = enableStats

    def update(self, elementsReceived, queueSize, EOS, streamID, connectionId):
        if not self.enabled:
            return

        if self.receivedStatistics.has_key(connectionId):
            self.receivedStatistics[connectionId][self.receivedStatistics_idx[connectionId]].elements = elementsReceived
            self.receivedStatistics[connectionId][self.receivedStatistics_idx[connectionId]].queueSize = queueSize
            self.receivedStatistics[connectionId][self.receivedStatistics_idx[connectionId]].secs = time.time()
            self.receivedStatistics[connectionId][self.receivedStatistics_idx[connectionId]].streamID = streamID
            self.receivedStatistics_idx[connectionId] += 1
            self.receivedStatistics_idx[connectionId] = self.receivedStatistics_idx[connectionId]%self.historyWindow
        else:
            self.receivedStatistics[connectionId] = []
            self.receivedStatistics_idx[connectionId] = 0
            for i in range(self.historyWindow):
                self.receivedStatistics[connectionId].append(self.statPoint())
            self.receivedStatistics[connectionId][self.receivedStatistics_idx[connectionId]].elements = elementsReceived
            self.receivedStatistics[connectionId][self.receivedStatistics_idx[connectionId]].queueSize = queueSize
            self.receivedStatistics[connectionId][self.receivedStatistics_idx[connectionId]].secs = time.time()
            self.receivedStatistics[connectionId][self.receivedStatistics_idx[connectionId]].streamID = streamID
            self.receivedStatistics_idx[connectionId] += 1
            self.receivedStatistics_idx[connectionId] = self.receivedStatistics_idx[connectionId] % self.historyWindow

    def retrieve(self):
        if not self.enabled:
            return

        retVal = []
        for entry in self.receivedStatistics:
            runningStats = BULKIO.PortStatistics(portName=self.name,averageQueueDepth=-1,elementsPerSecond=-1,bitsPerSecond=-1,callsPerSecond=-1,streamIDs=[],timeSinceLastCall=-1,keywords=[])

            listPtr = (self.receivedStatistics_idx[entry] + 1) % self.historyWindow    # don't count the first set of data, since we're looking at change in time rather than absolute time
            frontTime = self.receivedStatistics[entry][(self.receivedStatistics_idx[entry] - 1) % self.historyWindow].secs
            backTime = self.receivedStatistics[entry][self.receivedStatistics_idx[entry]].secs
            totalData = 0.0
            queueSize = 0.0
            streamIDs = []
            while (listPtr != self.receivedStatistics_idx[entry]):
                totalData += self.receivedStatistics[entry][listPtr].elements
                queueSize += self.receivedStatistics[entry][listPtr].queueSize
                streamIDptr = 0
                foundstreamID = False
                while (streamIDptr != len(streamIDs)):
                    if (streamIDs[streamIDptr] == self.receivedStatistics[entry][listPtr].streamID):
                        foundstreamID = True
                        break
                    streamIDptr += 1
                if (not foundstreamID):
                    streamIDs.append(self.receivedStatistics[entry][listPtr].streamID)
                listPtr += 1
                listPtr = listPtr % self.historyWindow

            currentTime = time.time()
            totalTime = currentTime - backTime
            if totalTime == 0:
                totalTime = 1e6
            receivedSize = len(self.receivedStatistics[entry])
            runningStats.bitsPerSecond = (totalData * self.bitSize) / totalTime
            runningStats.elementsPerSecond = totalData/totalTime
            runningStats.averageQueueDepth = queueSize / receivedSize
            runningStats.callsPerSecond = float((receivedSize - 1)) / totalTime
            runningStats.streamIDs = streamIDs
            runningStats.timeSinceLastCall = currentTime - frontTime
            usesPortStat = BULKIO.UsesPortStatistics(connectionId=entry, statistics=runningStats)
            retVal.append(usesPortStat)
        return retVal
