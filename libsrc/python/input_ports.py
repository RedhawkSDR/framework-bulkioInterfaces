

import threading
import Queue
import copy
import time

from ossie.utils import uuid
from bulkio.statistics import InStats
from bulkio import sri

from bulkio.bulkioInterfaces import BULKIO, BULKIO__POA #@UnusedImport 


class InPort:
    DATA_BUFFER=0
    TIME_STAMP=1
    END_OF_STREAM=2
    STREAM_ID=3
    SRI=4
    SRI_CHG=5
    QUEUE_FLUSH=6
    _TYPE_ = 'c'
    def __init__(self, name, logger=None, maxsize=10, sriCompare=sri.compare, newStreamCallback=None, PortTransferType=_TYPE_ ):
        self.name = name
        self.logger = logger
        self.queue = Queue.Queue(maxsize)
        self.port_lock = threading.Lock()
        self.stats =  InStats(name, PortTransferType)
        self.blocking = False
        self.sri_cmp = sriCompare
        self.newStreamCallback = newStreamCallback
        self.sriDict = {} # key=streamID, value=StreamSRI

        _cmpMsg  = "DEFAULT"
        _sriMsg  = "EMPTY"
        if sriCompare != sri.compare:
            _cmpMsg  = "USER_DEFINED"
        if newStreamCallback:
            _sriMsg  = "USER_DEFINED"

        if self.logger:
            self.logger.debug( "bulkio::InPort CTOR port:" + str(name) + 
                          " Blocking/MaxInputQueueSize " + str(self.blocking) + "/"  + str(maxsize) + 
                          " SriCompare/NewStreamCallback " +  _cmpMsg + "/" + _sriMsg );

    def enableStats(self, enabled):
        self.stats.setEnabled(enabled)

    def _get_statistics(self):
        self.port_lock.acquire()
        recStat = self.stats.retrieve()
        self.port_lock.release()
        return recStat

    def _get_state(self):
        self.port_lock.acquire()
        if self.queue.full():
            self.port_lock.release()
            return BULKIO.BUSY
        elif self.queue.empty():
            self.port_lock.release()
            return BULKIO.IDLE
        else:
            self.port_lock.release()
            return BULKIO.ACTIVE
        self.port_lock.release()
        return BULKIO.BUSY

    def _get_activeSRIs(self):
        self.port_lock.acquire()
        activeSRIs = [self.sriDict[entry][0] for entry in self.sriDict]
        self.port_lock.release()
        return activeSRIs

    def getCurrentQueueDepth(self):
        self.port_lock.acquire()
        depth = self.queue.qsize()
        self.port_lock.release()
        return depth

    def getMaxQueueDepth(self):
        self.port_lock.acquire()
        depth = self.queue.maxsize
        self.port_lock.release()
        return depth
        
    #set to -1 for infinite queue
    def setMaxQueueDepth(self, newDepth):
        self.port_lock.acquire()
        self.queue.maxsize = int(newDepth)
        self.port_lock.release()

    def pushSRI(self, H):
        
        if self.logger:
            self.logger.trace( "bulkio::InPort pushSRI ENTER (port=" + str(name) +")" )

        self.port_lock.acquire()
        if H.streamID not in self.sriDict:
            if self.logger:
                self.logger.debug( "pushSRI PORT:" + str(name) + " NEW SRI:" + str(H.streamID) )
            if self.newStreamCallback:
                self.newStreamCallback( H ) 
            self.sriDict[H.streamID] = (copy.deepcopy(H), True)
            if H.blocking:
                self.blocking = True
        else:
            sri, sriChanged = self.sriDict[H.streamID]
            if self.sri_cmp:
                if not self.sri_cmp(sri, H):
                    self.sriDict[H.streamID] = (copy.deepcopy(H), True)
                    if H.blocking:
                        self.blocking = True
        self.port_lock.release()

        if self.logger:
            self.logger.trace( "bulkio::InPort pushSRI EXIT (port=" + str(name) +")" )


    def pushPacket(self, data, T, EOS, streamID):

        if self.logger:
            self.logger.trace( "bulkio::InPort pushPacket ENTER (port=" + str(name) +")" )

        self.port_lock.acquire()
        if self.queue.maxsize == 0:
            self.port_lock.release()
            if self.logger:
                self.logger.trace( "bulkio::InPort pushPacket EXIT (port=" + str(name) +")" )
            return
        packet = None
        try:
            sri = BULKIO.StreamSRI(1, 0.0, 1.0, 1, 0, 0.0, 0.0, 0, 0, streamID, False, [])
            sriChanged = False
            if self.sriDict.has_key(streamID):
                sri, sriChanged = self.sriDict[streamID]
                self.sriDict[streamID] = (sri, False)

            if self.blocking:
                packet = (data, T, EOS, streamID, copy.deepcopy(sri), sriChanged, False)
                self.stats.update(len(data), float(self.queue.qsize()) / float(self.queue.maxsize), streamID, False)
                self.queue.put(packet)
            else:
                sriChangedHappened = False
                if self.queue.full():
                    try:
                        if self.logger:
                            self.logger.debug( "bulkio::InPort pushPacket PURGE INPUT QUEUE (SIZE=" + 
                                          str(len(self.queue.queue)) + ")"  )

                        self.queue.mutex.acquire()
                        while len(self.queue.queue) != 0:
                            data, T, EOS, streamID, sri, sriChanged, inputQueueFlushed = self.queue.queue.pop()
                            if sriChanged:
                                sriChangedHappened = True
                                self.queue.queue.clear()
                        self.queue.mutex.release()
                    except Queue.Empty:
                        self.queue.mutex.release()
                    if sriChangedHappened:
                        sriChanged = True
                    packet = (data, T, EOS, streamID, copy.deepcopy(sri), sriChanged, True)
                    self.stats.update(len(data), float(self.queue.qsize()) / float(self.queue.maxsize), EOS, streamID, True)
                else:
                    packet = (data, T, EOS, streamID, copy.deepcopy(sri), sriChanged, False)
                    self.stats.update(len(data), float(self.queue.qsize()) / float(self.queue.maxsize), EOS, streamID, False)

                if self.logger:
                    self.logger.trace( "bulkio::InPort pushPacket NEW Packet (QUEUE=" + 
                                       str(len(self.queue.queue)) + ")" )
                self.queue.put(packet)
        finally:
            self.port_lock.release()

        if self.logger:
            self.logger.trace( "bulkio::InPort pushPacket EXIT (port=" + str(name) +")" )
    
    def getPacket(self):

        if self.logger:
            self.logger.trace( "bulkio::InPort getPacket ENTER (port=" + str(name) +")" )

        try:
            data, T, EOS, streamID, sri, sriChanged, inputQueueFlushed = self.queue.get(block=False)
            
            if EOS: 
                if self.sriDict.has_key(streamID):
                    (a,b) = self.sriDict.pop(streamID)
                    if sri.blocking:
                        stillBlock = False
                        for _sri, _sriChanged in self.sriDict.values():
                            if _sri.blocking:
                                stillBlock = True
                                break
                        if not stillBlock:
                            self.blocking = False
            return (data, T, EOS, streamID, sri, sriChanged, inputQueueFlushed)
        except Queue.Empty:
            return None, None, None, None, None, None, None

        if self.logger:
            self.logger.trace( "bulkio::InPort getPacket EXIT (port=" + str(name) +")" )



class InCharPort(InPort, BULKIO__POA.dataChar):
    _TYPE_ = 'c'
    def __init__(self, name, logger=None, maxsize=10, sriCompare=sri.compare, newStreamCallback=None ):
        InPort.__init__(self, name, logger, maxsize, sriCompare, newStreamCallback, InCharPort._TYPE_ )

class InOctetPort(InPort, BULKIO__POA.dataOctet):
    _TYPE_ = 'B'
    def __init__(self, name, logger=None, maxsize=10, sriCompare=sri.compare, newStreamCallback=None ):
        InPort.__init__(self, name, logger, maxsize, sriCompare, newStreamCallback, InOctetPort._TYPE_ )

class InShortPort(InPort, BULKIO__POA.dataShort):
    _TYPE_ = 'h'
    def __init__(self, name, logger=None, maxsize=10, sriCompare=sri.compare, newStreamCallback=None ):
        InPort.__init__(self, name, logger, maxsize, sriCompare, newStreamCallback, InShortPort._TYPE_ )

class InUShortPort(InPort, BULKIO__POA.dataUshort):
    _TYPE_ = 'H'
    def __init__(self, name, logger=None, maxsize=10, sriCompare=sri.compare, newStreamCallback=None ):
        InPort.__init__(self, name, logger, maxsize, sriCompare, newStreamCallback, InUShortPort._TYPE_ )

class InLongPort(InPort, BULKIO__POA.dataLong):
    _TYPE_ = 'i'
    def __init__(self, name, logger=None, maxsize=10, sriCompare=sri.compare, newStreamCallback=None ):
        InPort.__init__(self, name, logger, maxsize, sriCompare, newStreamCallback, InLongPort._TYPE_ )

class InULongPort(InPort, BULKIO__POA.dataUlong):
    _TYPE_ = 'I'
    def __init__(self, name, logger=None, maxsize=10, sriCompare=sri.compare, newStreamCallback=None ):
        InPort.__init__(self, name, logger, maxsize, sriCompare, newStreamCallback, InULongPort._TYPE_ )

class InLongLongPort(InPort, BULKIO__POA.dataLongLong):
    _TYPE_ = 'q'
    def __init__(self, name, logger=None, maxsize=10, sriCompare=sri.compare, newStreamCallback=None ):
        InPort.__init__(self, name, logger, maxsize, sriCompare, newStreamCallback, InLongLongPort._TYPE_ )


class InULongLongPort(InPort, BULKIO__POA.dataUlongLong):
    _TYPE_ = 'Q'
    def __init__(self, name, logger=None, maxsize=10, sriCompare=sri.compare, newStreamCallback=None ):
        InPort.__init__(self, name, logger, maxsize, sriCompare, newStreamCallback, InULongLongPort._TYPE_ )


class InFloatPort(InPort, BULKIO__POA.dataFloat):
    _TYPE_ = 'f'
    def __init__(self, name, logger=None, maxsize=10, sriCompare=sri.compare, newStreamCallback=None ):
        InPort.__init__(self, name, logger, maxsize, sriCompare, newStreamCallback, InFloatPort._TYPE_ )


class InDoublePort(InPort, BULKIO__POA.dataDouble):
    _TYPE_ = 'd'
    def __init__(self, name, logger=None, maxsize=10, sriCompare=sri.compare, newStreamCallback=None ):
        InPort.__init__(self, name, logger, maxsize, sriCompare, newStreamCallback, InDoublePort._TYPE_ )



class InFilePort(InPort, BULKIO__POA.dataFile):
    _TYPE_ = 'd'
    def __init__(self, name, logger=None, maxsize=10, sriCompare=sri.compare, newStreamCallback=None ):
        InPort.__init__(self, name, logger, maxsize, sriCompare, newStreamCallback, InFilePort._TYPE_ )


    def pushPacket(self, URL, T, EOS, streamID):

        if self.logger:
            self.logger.trace( "bulkio::InFilePort pushPacket ENTER (port=" + str(name) +")" )

        self.port_lock.acquire()
        if self.queue.maxsize == 0:
            self.port_lock.release()
            if self.logger:
                self.logger.trace( "bulkio::InFilePort pushPacket EXIT (port=" + str(name) +")" )
            return
        packet = None
        try:
            sri = BULKIO.StreamSRI(1, 0.0, 1.0, 1, 0, 0.0, 0.0, 0, 0, streamID, False, [])
            sriChanged = False
            if self.sriDict.has_key(streamID):
                sri, sriChanged = self.sriDict[streamID]
                self.sriDict[streamID] = (sri, False)

            if self.blocking:
                packet = (URL, T, EOS, streamID, copy.deepcopy(sri), sriChanged, False)
                self.stats.update(1, float(self.queue.qsize()) / float(self.queue.maxsize), EOS,streamID, False)
                self.queue.put(packet)
            else:
                sriChangedHappened = False
                if self.queue.full():
                    try:
                        if self.logger:
                            self.logger.debug( "bulkio::InFilePort pushPacket PURGE INPUT QUEUE (SIZE=" + 
                                          str(len(self.queue.queue)) + ")"  )

                        self.queue.mutex.acquire()
                        while len(self.queue.queue) != 0:
                            URL, T, EOS, streamID, sri, sriChanged, inputQueueFlushed = self.queue.queue.pop()
                            if sriChanged:
                                sriChangedHappened = True
                                self.queue.queue.clear()
                        self.queue.mutex.release()
                    except Queue.Empty:
                        self.queue.mutex.release()
                    if sriChangedHappened:
                        sriChanged = True
                    packet = (URL, T, EOS, streamID, copy.deepcopy(sri), sriChanged, True)
                    self.stats.update(1, float(self.queue.qsize()) / float(self.queue.maxsize), EOS, streamID, True)
                else:
                    packet = (URL, T, EOS, streamID, copy.deepcopy(sri), sriChanged, False)
                    self.stats.update(len(URL), float(self.queue.qsize()) / float(self.queue.maxsize), streamID, False)

                if self.logger:
                    self.logger.trace( "bulkio::InXMLPort pushPacket NEW Packet (QUEUE=" + 
                                       str(len(self.queue.queue)) + ")" )
                self.queue.put(packet)
        finally:
            self.port_lock.release()

        if self.logger:
            self.logger.trace( "bulkio::InFilePort pushPacket EXIT (port=" + str(name) +")" )


class InXMLPort(InPort, BULKIO__POA.dataXML):
    _TYPE_ = 'd'
    def __init__(self, name, logger=None, maxsize=10, sriCompare=sri.compare, newStreamCallback=None ):
        InPort.__init__(self, name, logger, maxsize, sriCompare, newStreamCallback, InXMLPort._TYPE_ )


    def pushPacket(self, xml_string, EOS, streamID):

        if self.logger:
            self.logger.trace( "bulkio::InXMLPort pushPacket ENTER (port=" + str(name) +")" )
            
        self.port_lock.acquire()
        if self.queue.maxsize == 0:
            self.port_lock.release()
            if self.logger:
                self.logger.trace( "bulkio::InXMLPort pushPacket EXIT (port=" + str(name) +")" )
            return
        packet = None
        try:
            sri = BULKIO.StreamSRI(1, 0.0, 1.0, 1, 0, 0.0, 0.0, 0, 0, streamID, False, [])
            sriChanged = False
            if self.sriDict.has_key(streamID):
                sri, sriChanged = self.sriDict[streamID]
                self.sriDict[streamID] = (sri, False)

            if self.blocking:
                packet = (xml_string, None, EOS, streamID, copy.deepcopy(sri), sriChanged, False)
                self.stats.update(len(xml_string), float(self.queue.qsize()) / float(self.queue.maxsize), streamID, False)
                self.queue.put(packet)
            else:
                sriChangedHappened = False
                if self.queue.full():
                    try: 
                        if self.logger:
                            self.logger.debug( "bulkio::InXMLPort pushPacket PURGE INPUT QUEUE (SIZE=" + 
                                          str(len(self.queue.queue)) + ")"  )
                        self.queue.mutex.acquire()
                        while len(self.queue.queue) != 0:
                            xml_string, T, EOS, streamID, sri, sriChanged, inputQueueFlushed = self.queue.queue.pop()
                            if sriChanged:
                                sriChangedHappened = True
                                self.queue.queue.clear()
                        self.queue.mutex.release()
                    except Queue.Empty:
                        self.queue.mutex.release()
                    if sriChangedHappened:
                        sriChanged = True
                    packet = (xml_string, None, EOS, streamID, copy.deepcopy(sri), sriChanged, True)
                    self.stats.update(len(xml_string), float(self.queue.qsize()) / float(self.queue.maxsize), EOS, streamID, True)
                else:
                    packet = (xml_string, None, EOS, streamID, copy.deepcopy(sri), sriChanged, False)
                    self.stats.update(len(xml_string), float(self.queue.qsize()) / float(self.queue.maxsize), EOS, streamID, False)

                if self.logger:
                    self.logger.trace( "bulkio::InXMLPort pushPacket NEW Packet (QUEUE=" + 
                                       str(len(self.queue.queue)) + ")" )
                self.queue.put(packet)
        finally:
            self.port_lock.release()

        if self.logger:
            self.logger.trace( "bulkio::InXMLPort pushPacket EXIT (port=" + str(name) +")" )
    

class InSDDSPort(BULKIO__POA.dataSDDS):
    _TYPE_='b'
    def __init__(self, name, logger=None, callback = None, sriCmp=None, timeCmp=None, PortType = _TYPE_ ):
        self.name = name
        self.logger = logger
        self.sri = None
        self.port_lock = threading.Lock()
        self._attachedStreams = {} # key=attach_id, value = (streamDef, userid) 
        self.stats = InStats(name, PortType )
        self.sriDict = {} # key=streamID, value=(StreamSRI, PrecisionUTCTime)
        self.callback = callback
        self.sri_cmp = sriCmp
        self.time_cmp = timeCmp
        self.sriChanged = False
        try:
            self._attach_cb = getattr(callback, "attach")
            if not callable(self._attach_cb):
                self._attach_cb = None
        except AttributeError:
            self._attach_cb = None
        try:
            self._detach_cb = getattr(callback, "detach")
            if not callable(self._detach_cb):
                self._attach_cb = None
        except AttributeError:
            self._detach_cb = None

        if self.logger:
            self.logger.debug("bulkio::InSDDSPort CTOR port:" + str(name) )

    def setBitSize(self, bitSize):
        self.stats.setBitSize(bitSize)

    def enableStats(self, enabled):
        self.stats.setEnabled(enabled)
        
    def updateStats(self, elementsReceived, queueSize, streamID):
        self.port_lock.acquire()
        self.stats.update(elementsReceived, queueSize, streamID)
        self.port_lock.release()

    def _get_statistics(self):
        self.port_lock.acquire()
        recStat = self.stats.retrieve()
        self.port_lock.release()
        return recStat

    def _get_state(self):
        if len(self._attachedStreams.values()) == 0:
            return BULKIO.IDLE
        # default behavior is to limit to one connection
        elif len(self._attachedStreams.values()) == 1:
            return BULKIO.BUSY
        else:
            return BULKIO.ACTIVE

    def _get_attachedSRIs(self):
        sris = []
        for entry in self.sriDict:
            sris.append(copy.deepcopy(self.sriDict[entry]))
        self.port_lock.release()
        return sris

    def _get_usageState(self):
        if len(self._attachedStreams.values()) == 0:
            return BULKIO.dataSDDS.IDLE
        # default behavior is to limit to one connection
        elif len(self._attachedStreams.values()) == 1:
            return BULKIO.dataSDDS.BUSY
        else:
            return BULKIO.dataSDDS.ACTIVE

    def _get_attachedStreams(self):
        return [x[0] for x in self._attachedStreams.values()]

    def _get_attachmentIds(self):
        return self._attachedStreams.keys()

    def attach(self, streamDef, userid):

        if self.logger:
            self.logger.trace("bulkio::InSDDSPort attach ENTER  (port=" + str(name) +")" )
            self.logger.debug("SDDS PORT, ATTACH REQUEST, STREAM/USER"  + str(streamDef) + '/' + str(userid))

        if self._get_usageState() == BULKIO.dataSDDS.BUSY:
            if self.logger:
                self.logger.error("SDDS PORT, No Capacity to satisfy request. STREAM/USER"  + str(streamDef) + '/' + str(userid))
            raise BULKIO.dataSDDS.AttachError("No capacity")

        #
        # Allocate capacities here if applicable
        #

        # The attachment succeeded so generate a attachId
        attachId = None
        try:
            if self.logger:
                self.logger.debug("SDDS PORT: CALLING ATTACH CALLBACK, STREAM/USER"  + str(streamDef) + '/' + str(userid) )
            if self._attach_cb != None:
                attachId = self._attach_cb(streamDef, userid)
        except Exception, e:
            if self.logger:
                self.logger.error("SDDS PORT: ATTACH CALLBACK EXCEPTION : " + str(e) + " STREAM/USER"  + str(streamDef) + '/' + str(userid) )
            raise BULKIO.dataSDDS.AttachError(str(e))
        
        if attachId == None:
            attachId = str(uuid.uuid4())

        self._attachedStreams[attachId] = (streamDef, userid)

        if self.logger:
            self.logger.debug("SDDS PORT, ATTACH COMPLETED,  ID:" + str(attachId) + " STREAM/USER: " + str(streamDef) + '/' + str(userid))
            self.logger.trace("bulkio::InSDDSPort attach EXIT (port=" + str(name) +")" )
            
        return attachId

    def detach(self, attachId):

        if self.logger:
            self.logger.trace("bulkio::InSDDSPort detach ENTER (port=" + str(name) +")" )
            self.logger.debug("SDDS PORT, DETACH REQUESTED, ID:" + str(attachId) )

        if not self._attachedStreams.has_key(attachId):

            if self.logger:
                self.logger.debug("SDDS PORT, DETACH UNKNOWN ID:" + str(attachId) )

            if attachId:
                raise BULKIO.dataSDDS.DetachError("Stream %s not attached" % str(attachId))
            else:
                raise BULKIO.dataSDDS.DetachError("Cannot detach Unkown ID")

        attachedStreamDef, refcnf = self._attachedStreams[attachId]

        #
        # Deallocate capacity here if applicable
        #
        try:
            if self.logger:
                self.logger.debug("SDDS PORT, CALLING DETACH CALLBACK, ID:" + str(attachId) )

            if self._detach_cb != None:
                self._detach_cb(attachId)
        except Exception, e:
            if self.logger:
                self.logger.error("SDDS PORT, DETACH CALLBACK EXCEPTION: " + str(e) )
            raise BULKIO.dataSDDS.DetachError(str(e))

        # Remove the attachment from our list
        del self._attachedStreams[attachId]

        if self.logger:
            self.logger.debug("SDDS PORT, DETACH SUCCESS, ID:" + str(attachId) )
            self.logger.trace("bulkio::InSDDSPort detach EXIT (port=" + str(name) +")" )

    def getStreamDefinition(self, attachId):
        try:
            return self._attachedStreams[attachId][0]
        except KeyError:
            raise BULKIO.dataSDDS.StreamInputError("Stream %s not attached" % attachId)

    def getUser(self, attachId):
        try:
            return self._attachedStreams[attachId][1]
        except KeyError:
            raise BULKIO.dataSDDS.StreamInputError("Stream %s not attached" % attachId)

    def _get_activeSRIs(self):
        self.port_lock.acquire()
        activeSRIs = [self.sriDict[entry][0] for entry in self.sriDict]
        self.port_lock.release()
        return activeSRIs

    def pushSRI(self, H, T):

        if self.logger:
            self.logger.trace("bulkio::InSDDSPort pushSRI ENTER (port=" + str(name) +")" )

        self.port_lock.acquire()
        if H.streamID not in self.sriDict:
            if self.newStreamCallback:
                self.newStreamCallback( H ) 
            self.sriDict[H.streamID] = (copy.deepcopy(H), copy.deepcopy(T))
        else:
            cur_H, cur_T = self.sriDict[H.streamID]
            s_same = False
            if self.sri_cmp:
                s_same = self.sri_cmp(cur_H, H)
            
            t_same = False
            if self.time_cmp:
                t_same = self.time_cmp(cur_T, T)
                
            self.sriChanged = ( s_same == False )  or  ( t_same == False )
            self.sriDict[H.streamID] = (copy.deepcopy(H), copy.deepcopy(T))
        self.port_lock.release()

        if self.logger:
            self.logger.trace("bulkio::InSDDSPort pushSRI EXIT (port=" + str(name) +")" )
