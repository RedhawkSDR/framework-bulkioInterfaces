
import threading
import copy
import time
import sys
import struct

from ossie.cf    import ExtendedCF
from ossie.cf.CF    import Port
from ossie.utils import uuid
from bulkio.statistics import OutStats
from bulkio import sri
from bulkio import timestamp
from bulkio.bulkioInterfaces import BULKIO, BULKIO__POA 
from bulkio.const import MAX_TRANSFER_BYTES


class connection_descriptor_struct:
    def __init__( self, pname=None, conn_name=None, stream_id=None ): 
        self.port_name=pname
        self.connection_id=conn_name
        self.stream_id = stream_id

class OutPort (BULKIO__POA.UsesPortStatisticsProvider ):
    
    class SriMapStruct:
        def __init__( self, sri=None, connections=None, time=None): 
            self.sri=sri
            self.connections = connections #set of connection ID strings that have received this SRI
            self.time=time

    TRANSFER_TYPE='c'
    def __init__(self, name, PortTypeClass, PortTransferType=TRANSFER_TYPE, logger=None, noData=[] ):
        self.name = name
        self.logger = logger
        self.PortType = PortTypeClass
        self.PortTransferType=PortTransferType
        self.outConnections = {} # key=connectionId,  value=port
        self.stats = OutStats(self.name, PortTransferType )
        self.port_lock = threading.Lock()
        self.sriDict = {} # key=streamID  value=SriMapStruct
        self.filterTable = []
        self.noData = noData

        # Determine maximum transfer size in advance
        byteSize = 1
        if self.PortTransferType:
            byteSize = struct.calcsize(PortTransferType)
        # Multiply by some number < 1 to leave some margin for the CORBA header
        self.maxSamplesPerPush = int(MAX_TRANSFER_BYTES*.9)/byteSize

        if self.logger:
            self.logger.debug('bulkio::OutPort CTOR port:' + str(self.name))
            

    def connectPort(self, connection, connectionId):

        if self.logger:
            self.logger.trace('bulkio::OutPort  connectPort ENTER ')

        self.port_lock.acquire()
        try:
           try:
              port = connection._narrow(self.PortType)
              if port == None:
                  raise Port.InvalidPort(1, "Invalid Port for Connection ID:" + str(connectionId) )
              self.outConnections[str(connectionId)] = port

              if self.logger:
                  self.logger.debug('bulkio::OutPort  CONNECT PORT:' + str(self.name) + ' CONNECTION:' + str(connectionId) )
              
           except:
              if self.logger:
                  self.logger.error('bulkio::OutPort  CONNECT PORT:' + str(self.name) + ' PORT FAILED NARROW')
              raise Port.InvalidPort(1, "Invalid Port for Connection ID:" + str(connectionId) )
        finally:
            self.port_lock.release()

        if self.logger:
            self.logger.trace('bulkio::OutPort  connectPort EXIT ')
            
    def disconnectPort(self, connectionId):
        if self.logger:
            self.logger.trace('bulkio::OutPort  disconnectPort ENTER ')
        self.port_lock.acquire()
        connId = str(connectionId)
        portListed = False
        for filt in self.filterTable:
            if filt.port_name == self.name:
                portList = True
                break
        for streamid in self.sriDict.keys():
            sid = str(streamid)
            if portListed:
                for filt in self.filterTable:
                    if self.name == filt.port_name and sid == filt.stream_id and connId == filt.connection_name:
                        self.outConnections[connId].pushPacket(self.noData, timestamp.now(), True, sid)
            else:
                self.outConnections[connId].pushPacket(self.noData, timestamp.now(), True, sid)
        try:
            self.outConnections.pop(connId, None)
            for key in self.sriDict.keys():
                # if connID exist in set, remove it, otherwise do nothing (that is what discard does)
                self.sriDict[key].connections.discard(connId)
            if self.logger:
                self.logger.debug( "bulkio::OutPort DISCONNECT PORT:" + str(self.name) + " CONNECTION:" + str(connId) )
                self.logger.trace( "bulkio::OutPort DISCONNECT PORT:" + str(self.name) + " updated sriDict" + str(sriDict) )
        finally:
            self.port_lock.release()

        if self.logger:
            self.logger.trace('bulkio::OutPort  disconnectPort EXIT ')


    def enableStats(self, enabled):
        self.stats.setEnabled(enabled)

    def setBitSize(self, bitSize):
        self.stats.setBitSize(bitSize)
        
    def _get_connections(self):
        currentConnections = []
        self.port_lock.acquire()
        for id_, port in self.outConnections.items():
            currentConnections.append(ExtendedCF.UsesConnection(id_, port))
        self.port_lock.release()
        return currentConnections

    def _get_statistics(self):
        self.port_lock.acquire()
        recStat = self.stats.retrieve()
        self.port_lock.release()
        return recStat

    def _get_state(self):
        self.port_lock.acquire()
        numberOutgoingConnections = len(self.outConnections)
        self.port_lock.release()
        if numberOutgoingConnections == 0:
            return BULKIO.IDLE
        else:
            return BULKIO.ACTIVE
        return BULKIO.BUSY

    def _get_activeSRIs(self):
        self.port_lock.acquire()
        sris = []
        for entry in self.sriDict:
            sris.append(copy.deepcopy(self.sriDict[entry].sri))
        self.port_lock.release()
        return sris
    
    def updateConnectionFilter(self, _filterTable):
        self.port_lock.acquire()
        if _filterTable == None :
            _filterTable = []
        self.filterTable = _filterTable
        self.port_lock.release()

    def pushSRI(self, H):
        if self.logger:
            self.logger.trace('bulkio::OutPort pushSRI ENTER ')
        self.port_lock.acquire()
        self.sriDict[H.streamID] = OutPort.SriMapStruct(sri=copy.deepcopy(H), connections=set()) 
        try:
            portListed = False
            for connId, port in self.outConnections.items():
                for ftPtr in self.filterTable:

                    # check if port was listed in connection filter table
                    if ftPtr.port_name == self.name:
                        portListed = True

                    if (ftPtr.port_name == self.name) and (ftPtr.connection_id == connId) and (ftPtr.stream_id == H.streamID):
                        try:
                            if port != None:
                                port.pushSRI(H)
                                self.sriDict[H.streamID].connections.add(connId)
                        except Exception:
                            if self.logger:
                                self.logger.error("The call to pushSRI failed on port %s connection %s instance %s", self.name, connId, port)

            if not portListed:
                for connId, port in self.outConnections.items():
                    try:
                        if port != None:
                            port.pushSRI(H)
                            self.sriDict[H.streamID].connections.add(connId)
                    except Exception:
                        if self.logger:
                            self.logger.error("The call to pushSRI failed on port %s connection %s instance %s", self.name, connId, port)

        finally:
            self.port_lock.release()
        if self.logger:
            self.logger.trace('bulkio::OutPort  pushSRI EXIT ')

    def _pushOversizedPacket(self, data, T, EOS, streamID):
        # If there is no need to break data into smaller packets, skip straight
        # to the pushPacket call and return.
        if len(data) <= self.maxSamplesPerPush:
            self._pushPacket(data, T, EOS, streamID);
            return

        # Push sub-packets maxSamplesPerPush at a time
        for start in xrange(0, len(data), self.maxSamplesPerPush):
            # The end index of the packet may exceed the length of the data;
            # the Python slice operator will clamp it to the actual end
            end = start + self.maxSamplesPerPush

            # Send end-of-stream as false for all sub-packets except for the
            # last one (when the end of the sub-packet goes past the end of the
            # input data), which gets the input EOS.
            if end >= len(data):
                packetEOS = EOS
            else:
                packetEOS = False

            # Push the current slice of the input data
            self._pushPacket(data[start:end], T, packetEOS, streamID);

    def _pushPacket(self, data, T, EOS, streamID):
        
        portListed = False
        
        for connId, port in self.outConnections.items():
            for ftPtr in self.filterTable:

                if ftPtr.port_name == self.name : 
                    portListed = True

                if (ftPtr.port_name == self.name) and (ftPtr.connection_id == connId) and (ftPtr.stream_id == streamID):
                    try:
                        if port != None:
                            if connId not in self.sriDict[streamID].connections:
                                port.pushSRI(self.sriDict[streamID].sri)
                                self.sriDict[streamID].connections.add(connId)
                            port.pushPacket(data, T, EOS, streamID)
                            self.stats.update(len(data), 0, EOS, streamID, connId)
                    except Exception, e:
                        if self.logger:
                            self.logger.error("The call to pushPacket failed on port %s connection %s instance %s", self.name, connId, port)

        if not portListed:
            for connId, port in self.outConnections.items():
                try:
                    if port != None:
                        if connId not in self.sriDict[streamID].connections:
                            port.pushSRI(self.sriDict[streamID].sri)
                            self.sriDict[streamID].connections.add(connId)
                        port.pushPacket(data, T, EOS, streamID)
                        self.stats.update(len(data), 0, EOS, streamID, connId)
                except Exception, e:
                    if self.logger:
                        self.logger.error("The call to pushPacket failed on port %s connection %s instance %s", self.name, connId, port)
        if EOS==True:
            if self.sriDict.has_key(streamID):
                tmp = self.sriDict.pop(streamID)
 
    def pushPacket(self, data, T, EOS, streamID):

        if self.logger:
            self.logger.trace('bulkio::OutPort  pushPacket ENTER ')

        if not self.sriDict.has_key(streamID):
            sri = BULKIO.StreamSRI(1, 0.0, 1.0, 1, 0, 0.0, 0.0, 0, 0, streamID, False, [])
            self.pushSRI(sri)

        self.port_lock.acquire()
        try:
            self._pushOversizedPacket(data, T, EOS, streamID)
        finally:
            self.port_lock.release()

        if self.logger:
            self.logger.trace('bulkio::OutPort  pushPacket EXIT ')


class OutCharPort(OutPort):
    TRANSFER_TYPE = 'c'
    def __init__(self, name, logger=None ):
        OutPort.__init__(self, name, BULKIO.dataChar, OutCharPort.TRANSFER_TYPE , logger, noData='' )

class OutOctetPort(OutPort):
    TRANSFER_TYPE = 'B'
    def __init__(self, name, logger=None ):
        OutPort.__init__(self, name, BULKIO.dataOctet, OutOctetPort.TRANSFER_TYPE , logger, noData='')

class OutShortPort(OutPort):
    TRANSFER_TYPE = 'h'
    def __init__(self, name, logger=None ):
        OutPort.__init__(self, name, BULKIO.dataShort, OutShortPort.TRANSFER_TYPE , logger )

class OutUShortPort(OutPort):
    TRANSFER_TYPE = 'H'
    def __init__(self, name, logger=None ):
        OutPort.__init__(self, name, BULKIO.dataUshort, OutUShortPort.TRANSFER_TYPE , logger )

class OutLongPort(OutPort):
    TRANSFER_TYPE = 'i'
    def __init__(self, name, logger=None ):
        OutPort.__init__(self, name, BULKIO.dataLong, OutLongPort.TRANSFER_TYPE , logger )

class OutULongPort(OutPort):
    TRANSFER_TYPE = 'I'
    def __init__(self, name, logger=None ):
        OutPort.__init__(self, name, BULKIO.dataUlong, OutULongPort.TRANSFER_TYPE , logger )

class OutLongLongPort(OutPort):
    TRANSFER_TYPE = 'q'
    def __init__(self, name, logger=None ):
        OutPort.__init__(self, name, BULKIO.dataLongLong, OutLongLongPort.TRANSFER_TYPE , logger )

class OutULongLongPort(OutPort):
    TRANSFER_TYPE = 'Q'
    def __init__(self, name, logger=None ):
        OutPort.__init__(self, name, BULKIO.dataUlongLong, OutULongLongPort.TRANSFER_TYPE , logger )

class OutFloatPort(OutPort):
    TRANSFER_TYPE = 'f'
    def __init__(self, name, logger=None ):
        OutPort.__init__(self, name, BULKIO.dataFloat, OutFloatPort.TRANSFER_TYPE , logger )

class OutDoublePort(OutPort):
    TRANSFER_TYPE = 'd'
    def __init__(self, name, logger=None ):
        OutPort.__init__(self, name, BULKIO.dataDouble, OutDoublePort.TRANSFER_TYPE , logger )

class OutFilePort(OutPort):
    TRANSFER_TYPE = 'c'
    def __init__(self, name, logger=None ):
        OutPort.__init__(self, name, BULKIO.dataFile, OutFilePort.TRANSFER_TYPE , logger, noData='' )

    def pushPacket(self, URL, T, EOS, streamID):

        if self.logger:
            self.logger.trace('bulkio::OutFilePort  pushPacket ENTER ')

        if not self.sriDict.has_key(streamID):
            sri = BULKIO.StreamSRI(1, 0.0, 1.0, 1, 0, 0.0, 0.0, 0, 0, streamID, False, [])
            self.pushSRI(sri)

        self.port_lock.acquire()

        try:
            portListed = False
            for connId, port in self.outConnections.items():
                for ftPtr in self.filterTable:
                    if ftPtr.port_name == self.name : 
                        portListed = True
                    if (ftPtr.port_name == self.name) and (ftPtr.connection_id == connId) and (ftPtr.stream_id == streamID):
                        try:
                            if port != None:
                                port.pushPacket(URL, T, EOS, streamID)
                                self.stats.update(1, 0, EOS, streamID, connId)
                        except Exception:
                            if self.logger:
                                self.logger.error("The call to OutFilePort::pushPacket failed on port %s connection %s instance %s", self.name, connId, port)

            if not portListed:
                for connId, port in self.outConnections.items():
                    try:
                        if port != None:
                            port.pushPacket(URL, T, EOS, streamID)
                            self.stats.update(1, 0, EOS, streamID, connId)
                    except Exception:
                        if self.logger:
                            self.logger.error("The call to OutFilePort::pushPacket failed on port %s connection %s instance %s", self.name, connId, port)
            if EOS==True:
                if self.sriDict.has_key(streamID):
                    tmp = self.sriDict.pop(streamID)
        finally:
            self.port_lock.release()

        if self.logger:
            self.logger.trace('bulkio::OutFilePort  pushPacket EXIT ')
 
class OutXMLPort(OutPort):
    TRANSFER_TYPE = 'c'
    def __init__(self, name, logger=None ):
        OutPort.__init__(self, name, BULKIO.dataXML, OutXMLPort.TRANSFER_TYPE , logger, noData='' )

    def pushPacket(self, xml_string, EOS, streamID):

        if self.logger:
            self.logger.trace('bulkio::OutXMLPort  pushPacket ENTER ')

        if not self.sriDict.has_key(streamID):
            sri = BULKIO.StreamSRI(1, 0.0, 1.0, 1, 0, 0.0, 0.0, 0, 0, streamID, False, [])
            self.pushSRI(sri)

        self.port_lock.acquire()

        try:
            portListed = False
            for connId, port in self.outConnections.items():
                for ftPtr in self.filterTable:
                    if ftPtr.port_name == self.name : 
                        portList = True
                    if (ftPtr.port_name == self.name) and (ftPtr.connection_id == connId) and (ftPtr.stream_id == streamID):
                        try:
                            if port != None:
                                port.pushPacket(xml_string, EOS, streamID)
                                self.stats.update(len(xml_string), 0, EOS, streamID, connId)
                        except Exception:
                            if self.logger:
                                self.logger.error("The call to OutXMLPort::pushPacket failed on port %s connection %s instance %s", self.name, connId, port)
            if not portListed:
                for connId, port in self.outConnections.items():
                    try:
                        if port != None:
                            port.pushPacket(xml_string, EOS, streamID)
                            self.stats.update(len(xml_string), 0, EOS, streamID, connId)
                    except Exception:
                        if self.logger:
                            self.logger.error("The call to OutXMLPort::pushPacket failed on port %s connection %s instance %s", self.name, connId, port)
            if EOS==True:
                if self.sriDict.has_key(streamID):
                    tmp = self.sriDict.pop(streamID)
        finally:
            self.port_lock.release()

        if self.logger:
            self.logger.trace('bulkio::OutXMLPort  pushPacket EXIT ')

    def disconnectPort(self, connectionId):
        if self.logger:
            self.logger.trace('bulkio::OutXMLPort  disconnectPort ENTER ')
        self.port_lock.acquire()
        connId = str(connectionId)
        portListed = False
        for filt in self.filterTable:
            if filt.port_name == self.name:
                portList = True
                break
        for streamid in self.sriDict.keys():
            sid = str(streamid)
            if portListed:
                for filt in self.filterTable:
                    if self.name == filt.port_name and sid == filt.stream_id and connId == filt.connection_name:
                        self.outConnections[connId].pushPacket(self.noData, True, sid)
            else:
                self.outConnections[connId].pushPacket(self.noData, True, sid)
        try:
            self.outConnections.pop(connId, None)
            for key,value in self.sriDict.items():
                # if connID exist in set, remove it, otherwise do nothing (that is what discard does)
                self.sriDict[key].connections.discard(connId)
            if self.logger:
                self.logger.debug( "bulkio::OutXMLPort DISCONNECT PORT:" + str(self.name) + " CONNECTION:" + str(connId) )
                self.logger.trace( "bulkio::OutXMLPort DISCONNECT PORT:" + str(self.name) + " updated sriDict" + str(sriDict) )
        finally:
            self.port_lock.release()

        if self.logger:
            self.logger.trace('bulkio::OutXMLPort  disconnectPort EXIT ')

class OutAttachablePort:
    class StreamAttachment:
        def __init__(self, connectionId, attachId, inputPort):
            self.connectionId=connectionId
            self.attachId=attachId
            self.inputPort=inputPort

    class Stream:
        def __init__(self, streamDef, name, streamId=None, streamAttachments=[], sri=None, time=None):
            self.streamDef=streamDef
            self.name = name
            self.streamId=streamId
            self.streamAttachments=streamAttachments
            self.sri=sri
            self.time=time

        def detachByConnectionID(self, connectionId):
            for att in list(self.streamAttachments):
                if att.connectionId == connectionId and att.inputPort and att.attachId:
                    att.inputPort.detach(att.attachId)
                    self.streamAttachments.remove(att)

        def detachByAttachID(self, attachId):
            for att in list(self.streamAttachments):
                if att.attachId and att.inputPort and att.attachId == attachId:
                    att.inputPort.detach(att.attachId)
                    self.streamAttachments.remove(att)

        def detachByAttachIdConnectionID(self, connectionId):
            for att in list(self.streamAttachments):
                if att.attachId and att.inputPort and att.attachId == attachId and att.connectionId == connectionId:
                    att.inputPort.detach(att.attachId)
                    self.streamAttachments.remove(att)

        def createNewAttachment(self,connectionId, port):
            newAttachment = OutAttachablePort.StreamAttachment(connectionId=connectionId, attachId=None, inputPort=port)
            try:
                newAttachment.attachId = port.attach(self.streamDef, self.name)
                self.streamAttachments.append(newAttachment)
                if (self.sri and self.time):
                    port.pushSRI(self.sri, self.time)
            except Exception, e:
                print "Stream: createNewAttachment() Exception while calling attach for connectionId %s streamId %s: %s" % (connectionId, self.streamDef.id, str(e))
                raise
 
        def hasConnectionId(self, connectionId):
            for att in list(self.streamAttachments):
                if att.connectionId == connectionId:
                    return True 
            return False

        def updateAttachments(self, expectedAttachments):
            expectedConnectionIds = []
            # Add new attachments that do not already exist
            for att in expectedAttachments:
                if not self.hasConnectionId(att.connectionId):
                    self.createNewAttachment(att.connectionId, att.inputPort)
                expectedConnectionIds.append(att.connectionId)

            # Iterate through attachments and compare to expected connectionIds
            connectionsToRemove = []
            for att in stream.streamAttachments:
                existingConnectionId = att.connectionId
                detachConnection = True
                for connId in expectedConnectionIds:
                    if existingConnectionId == connId:
                        detachConnection = False
                        break
                if detachConnection == True:
                    # Store off and apply detach outside of this loop
                    # Removing now will mess up iterator
                    connectionsToRemove.append(existingConnectionId)
                    
            for connId in connectionsToRemove:
                self.detachByConnectionId(connId)
            
        def detachAll(self):
            for att in list(self.streamAttachments):
                att.inputPort.detach(att.attachId)
                self.streamAttachments.remove(att)

    class StreamContainer:
        def __init__(self, streams=[]):
            self.streams = streams
            self.logger = None

        def printState(self, title):
            for stream in self.streams:
                self.printBlock("Stream", stream.streamId,0)
                for att in stream.streamAttachments:
                    printBlock("Attachment",att.attachId,1)

        def printBlock(self, title, id, indents):
            indent = ""
            if indents > 0:
                indent = "    "
            totalIndent = "" 
            for ii in range(ii):
                totalIndent += indent
            line = "---------------"

            print totalIndent + " |" + line
            print totalIndent + " |" + title
            print totalIndent + " |   '" + id + "'" 
            print totalIndent + " |" + line

        def hasStreams(self):
            if len(self.streams) > 0:
                return True
            else:
                return False

        def hasStreamId(self, streamId):
            for stream in self.streams:
                if stream.streamId == streamId:
                    return True
            return False

        def getStreamIds(self):
            streamIds = []
            for stream in self.streams:
                streamIds.append(stream.streamId)
            return streamIds

        def addConnectionToAllStreams(self, connectionId, port):
            for stream in self.streams:
                if not stream.hasConnectionId(connectionId):
                    stream.createNewAttachment(connectionId, port)

        def addConnectionToStream(self, connectionId, port, streamId):
            for stream in self.streams:
                if stream.streamId == streamId:
                    if not stream.hasConnectionId(connectionId):
                        stream.createNewAttachment(connectionId, port)

        def updateSRIForAllStreams(self, currentSRIs):
            for stream in self.streams:
                if currentSRIs.has_key(stream.streamId):
                    stream.sri = currentSRIs[stream.streamId].sri
                    stream.time = currentSRIs[stream.streamId].time

        def updateStreamSRI(self, streamId, sri):
            for stream in self.streams:
                if stream.streamId == streamId:
                    stream.sri = sri

        def updateStreamTime(self, streamId, time):
            for stream in self.streams:
                if stream.streamId == streamId:
                    stream.time = time

        def addStream(self, stream):
            self.streams.append(stream)

        def removeStreamByStreamId(self, streamId):
            for s in list(self.streams):
                if s.id == streamId: 
                   self.streams.pop(s)

        def findByAttachId(self, attachId):
            streamList = []
            for s in self.streams:
                if s.attachId == attachId:
                    streamList.append(s) 
            return streamList            

        def findByStreamId(self, streamId):
            for s in self.streams:
                if s.streamId == streamId:
                    return s
            return None            

        def findByConnectionId(self, connectionId):
            streamList = []
            for s in self.streams:
                for a in s.streamAttachments:
                    if a.connectionId == connectionId:
                        streamList.append(s)
            return streamList

        def detachByAttachIdConnectionId(self, attachId=None, connectionId=None):
            for stream in self.streams:
                for atts in list(stream.streamAttachments):
                    if atts.connectionId == connectionId and atts.inputPort and atts.attachId and atts.attachId == attachId:
                        atts.inputPort.detach(atts.attachId)
                        stream.streamAttachments.pop(atts)

        def detachAllStreams(self):
            for stream in self.streams:
                for atts in list(stream.streamAttachments):
                    if atts.inputPort and atts.attachId:
                        atts.inputPort.detach(atts.attachId)
                        stream.streamAttachments.pop(atts)

        def detachByConnectionId(self, connectionId=None):
            for stream in self.streams:
                for atts in list(stream.streamAttachments):
                    if atts.connectionId == connectionId and atts.inputPort and atts.attachId:
                        atts.inputPort.detach(atts.attachId)
                        stream.streamAttachments.pop(atts)

        def detachByAttachId(self, attachId=None):
            for stream in self.streams:
                for atts in list(stream.streamAttachments):
                    if atts.attachId and atts.attachId == attachId and atts.inputPort:
                        atts.inputPort.detach(atts.attachId)
                        stream.streamAttachments.pop(atts)

        def findStreamAttachmentsByConnectionId(self, connectionId):
            attachList = []
            for stream in self.streams:
                for att in stream.streamAttachments:
                    if att.connectionId == connectionId:
                        attachList.append(att)    
            return attachList

        def findStreamAttachmentsByAttachId(self, attachId):
            attachList = []
            for stream in self.streams:
                for att in stream.streamAttachments:
                    if att.attachId == attachId:
                        attachList.append(att)    
            return attachList

        def setLogger(self, logger):
            self.logger = logger

class OutSDDSPort(OutPort):
    TRANSFER_TYPE = 'c'
    def __init__(self, name, max_attachments=None, logger=None ):
        OutPort.__init__(self, name, BULKIO.dataSDDS, OutSDDSPort.TRANSFER_TYPE , logger )
        self.max_attachments = max_attachments
        self.streamContainer = OutAttachablePort.StreamContainer() 
        self.defaultStreamSRI = sri.create()
        self.defaultTime = timestamp.now()
        self.sriDict = {} # key=streamID  value=SriMapStruct
        self.filterTable = []
        
    def _get_state(self):
        self.port_lock.acquire()
        if len(self._attachedStreams.values()) == 0:
            return BULKIO.IDLE
        else:
            return BULKIO.ACTIVE

    def _get_attachedSRIs(self):
        return self._get_activeSRIs()

    def attachedStreams(self):
        streams = []
        for stream in self.streamContainer.streams:
            streams.append(stream.streamDef)
        return streams 

    def connectPort(self, connection, connectionId):
        OutPort.connectPort( self, connection, connectionId )
        self.port_lock.acquire()
        try:
            try:
                portListed = False
                port = self.outConnections[str(connectionId)]

                if self.logger:
                    self.logger.trace("bulkio::OutSDDSPort, Filter Table %s" % self.filterTable)
                for ftPtr in self.filterTable:
                    # check if port was listed in connection filter table
                    if ftPtr.port_name == self.name:
                        portListed = True

                    if (ftPtr.port_name == self.name) and (ftPtr.connection_id == connectionId):
                        desiredStreamId = ftPtr.stream_id
                        if self.sriDict.has_key(desiredStreamId):
                            sriMap = self.sriDict[desiredStreamId]
                            stream.sri = sriMap.sri
                            stream.time = sriMap.time
                        self.streamContainer.addConnectionToStream(connectionId,port,desiredStreamId)

                if not portListed:
                    self.streamContainer.updateSRIForAllStreams(self.sriDict)
                    self.streamContainer.addConnectionToAllStreams(connectionId,port) 

            except Exception, e:
                if self.logger:
                    self.logger.error("Exception while calling connectPort for connectionId %s: %s" % (connectionId, str(e)))
                raise Port.InvalidPort(1, "Invalid Port for Connection ID:" + str(connectionId) )
        finally:
            self.port_lock.release()
    
    def disconnectPort(self, connectionId):
        try:
            self.port_lock.acquire()
            try:
                self.streamContainer.detachByConnectionId(connectionId)
            except:
                if self.logger:
                    self.logger.error("Unable to detach %s, should not have happened", str(connId))
        finally:
            self.port_lock.release()
        OutPort.disconnectPort( self, connectionId )

    def detach(self, attachId=None, connectionId=None):
        if self.logger:
            self.logger.trace("bulkio::OutSDDSPort, DETACH ENTER ")

        self.port_lock.acquire()
        try:
            if connectionId:
                for stream in self.streamContainer.streams:
                    stream.detachByConnectionId(connectionId)

            if attachId:
                for stream in self.streamContainer.streams:
                    for atts in list(stream.streamAttachments):
                        if atts.attachId == attachId:
                            atts.inputPort.detach(attachId)
                            stream.streamAttachments.pop(atts)

            if not attachId and not connectionId:
                for stream in self.streamContainer.streams:
                    for atts in list(stream.streamAttachments):
                        atts.inputPort.detach(attachId)
                self.streamContainer = OutAttachablePort.StreamContainer()

        finally:
            self.port_lock.release()

        if self.logger:
            self.logger.trace("bulkio::OutSDDSPort, DETACH EXIT ")
    
    def attach(self, streamData, name):

        if self.logger:
            self.logger.trace("bulkio::OutSDDSPort, ATTACH ENTER ")

        ids = []
        self.port_lock.acquire()
        try:
            stream = self.streamContainer.findByStreamId(streamData.id)
            if stream:
                stream.detachAll()
            else:
                stream = OutAttachablePort.Stream(streamDef=streamData, name=name, streamId=streamData.id)
                self.streamContainer.addStream(stream) 

            portListed = False
            for connId, port in self.outConnections.items():
                for ftPtr in self.filterTable:

                    # check if port was listed in connection filter table
                    if ftPtr.port_name == self.name:
                        portListed = True

                    if (ftPtr.port_name == self.name) and (ftPtr.connection_id == connId) and (ftPtr.stream_id == stream.streamId):
                        try:
                           if self.sriDict.has_key(stream.streamId):
                              sriMap = self.sriDict[stream.streamId]
                              stream.sri = sriMap.sri
                              stream.time = sriMap.time
                           stream.createNewAttachment(connectionId,port)
                        except Exception, e:
                            if self.logger:
                                self.logger.error("bulkio.OutSDDSPort attach() Unable to deliver update to %s: %s" % (str(connId), str(e)))

            if not portListed: 
                if self.sriDict.has_key(stream.streamId):
                    sriMap = self.sriDict[stream.streamId]
                    stream.sri = sriMap.sri
                    stream.time = sriMap.time
                for connId,port in self.outConnections.items():
                    try:
                       stream.createNewAttachment(connectionId,port)
                    except Exception, e:
                        if self.logger:
                            self.logger.error("bulkio.OutSDDSPort attach() Unable to deliver update to %s: %s" % (str(connId), str(e)))

        finally:
            self.port_lock.release()

        for atts in stream.streamAttachments:
            ids.append(atts.attachId)
            if self.logger:
                self.logger.debug("SDDS PORT, ATTACH COMPLETED ID " + str(atts.attachId) + " CONNECTION ID:" + str(atts.connectionId))
        if self.logger:
            self.logger.trace("bulkio::OutSDDSPort, ATTACH EXIT ")

        return ids

    def getStreamDefinition(self, attachId):
        streamDefList = []
        for stream in self.streamContainer.streams:
            for atts in stream.streamAttachments:
                if atts.attachId == attachId:
                    streamDefList.append(stream.streamDef)
        return streamDefList 

    def getUser(self, attachId):
        nameList = []
        for stream in self.streamContainer.streams:
            for atts in stream.streamAttachments:
                if atts.attachId == attachId:
                    nameList.append(stream.name)
        return nameList 
    
    def pushSRI(self, H, T):
        if self.logger:
            self.logger.trace("bulkio::OutSDDSPort, PUSH-SRI ENTER ")

        self.port_lock.acquire()
        self.sriDict[H.streamID] = OutPort.SriMapStruct(sri=copy.deepcopy(H), connections=set(), time=copy.deepcopy(T)) 
        self.defaultStreamSRI = H
        self.defaultTime = T
        try:
            portListed = False
            for connId, port in self.outConnections.items():
                for ftPtr in self.filterTable:

                    # check if port was listed in connection filter table
                    if ftPtr.port_name == self.name:
                        portListed = True

                    if (ftPtr.port_name == self.name) and (ftPtr.connection_id == connId) and (ftPtr.stream_id == H.streamID):
                        try:
                            if port != None:
                                port.pushSRI(H, T)
                                self.sriDict[H.streamID].connections.add(connId)
                                stream = self.streamContainer.findByStreamId(H.streamID)
                                if stream:
                                    stream.sri = H
                                    stream.time = T
                        except Exception:
                            if self.logger:
                                self.logger.error("The call to pushSRI failed on port %s connection %s instance %s", self.name, connId, port)

            if not portListed:
                for connId, port in self.outConnections.items():
                    try:
                        if port != None:
                            port.pushSRI(H, T)
                            self.sriDict[H.streamID].connections.add(connId)
                            stream = self.streamContainer.findByStreamId(H.streamID)
                            if stream:
                                stream.sri = H
                                stream.time = T
                    except Exception:
                        if self.logger:
                            self.logger.error("The call to pushSRI failed on port %s connection %s instance %s", self.name, connId, port)
        finally:
            self.port_lock.release() 

        if self.logger:
            self.logger.trace("bulkio::OutSDDSPort, PUSH-SRI EXIT ")

    def updateConnectionFilter(self, _filterTable):
        self.port_lock.acquire()
        if _filterTable == None :
            _filterTable = []
        self.filterTable = _filterTable

        #1. loop over filterTable
            #A. ignore other port_names
            #B. create mapping of streamid->connections(attachments) 

        hasPortEntry = False
        streamsFound = {}
        streamAttachments = {}
        # Populate streamsFound 
        knownStreamIds = self.streamContainer.getStreamIds()
        for id in knownStreamIds:
            streamsFound[id] = False

        # Iterate through each filterTable entry and capture state
        for entry in self.filterTable:
            if entry.port_name != self.name:
                continue

            hasPortEntry = True
            if entry.connection_id in self.outConnections.keys():
                connectedPort = self.outConnections.get(entry.connection_id)
            else:
                if self.logger:
                    self.logger.warn("bulkio::OutSDDSPort, updateConnectionFilter() Unable to find connected port with connectionId: " + entry.connection_id)
                continue

            if self.streamContainer.hasStreamId(entry.stream_id):
                streamsFound[entry.stream_id] = True
                expectedAttachment = OutAttachablePort.StreamAttachment(entry.connection_id, None, connectedPort)
                if not streamAttachments.has_key(entry.stream_id):
                    streamAttachments[entry.stream_id] = []
                streamAttachments[entry.stream_id].append(expectedAttachment)

        for entry in streamAttachments:
            streamId = entry.streamId
            foundStream = self.streamContainer.findByStreamId(streamId)
            if foundStream:
                foundStream.updateAttachments(entry)
            else:
                if self.logger:
                    self.logger.warn("bulkio::OutSDDSPort, updateConnectionFilter() Unable to locate stream definition for streamId: " +streamId)

        
        if hasPortEntry:
            # If there's a valid port entry, we need to detach unmentioned streams
            for streamId,found in streamsFound.items():
                if not found:
                    stream = self.streamContainer.findByStreamId(streamId)
                    if stream:
                        stream.detachAll()
        else:
            # No port entry == All connections on
            for connId, port in self.outConnections.items():
                self.streamContainer.addConnectionToAllStreams(connId_,port)

        self.port_lock.release()

class OutVITA49Port(OutPort):
    TRANSFER_TYPE = 'c'
    def __init__(self, name, max_attachments=None, logger=None ):
        OutPort.__init__(self, name, BULKIO.dataVITA49, OutVITA49Port.TRANSFER_TYPE , logger )
        self.max_attachments = max_attachments
        self.streamContainer = OutAttachablePort.StreamContainer() 
        self.defaultStreamSRI = sri.create()
        self.defaultTime = timestamp.now()
        self.sriDict = {} # key=streamID  value=SriMapStruct
        self.filterTable = []
        
    def _get_state(self):
        self.port_lock.acquire()
        if len(self._attachedStreams.values()) == 0:
            return BULKIO.IDLE
        else:
            return BULKIO.ACTIVE

    def _get_attachedSRIs(self):
        return self._get_activeSRIs()

    def attachedStreams(self):
        streams = []
        for stream in self.streamContainer.streams:
            streams.append(stream.streamDef)
        return streams 

    def connectPort(self, connection, connectionId):
        OutPort.connectPort( self, connection, connectionId )
        self.port_lock.acquire()
        try:
            try:
                portListed = False
                port = self.outConnections[str(connectionId)]

                if self.logger:
                    self.logger.trace("bulkio::OutVITA49Port, Filter Table %s" % self.filterTable)
                for ftPtr in self.filterTable:
                    # check if port was listed in connection filter table
                    if ftPtr.port_name == self.name:
                        portListed = True

                    if (ftPtr.port_name == self.name) and (ftPtr.connection_id == connectionId):
                        desiredStreamId = ftPtr.stream_id
                        if self.sriDict.has_key(desiredStreamId):
                            sriMap = self.sriDict[desiredStreamId]
                            stream.sri = sriMap.sri
                            stream.time = sriMap.time
                        self.streamContainer.addConnectionToStream(connectionId,port,desiredStreamId)

                if not portListed:
                    self.streamContainer.updateSRIForAllStreams(self.sriDict)
                    self.streamContainer.addConnectionToAllStreams(connectionId,port) 

            except Exception, e:
                if self.logger:
                    self.logger.error("Exception while calling connectPort for connectionId %s: %s" % (connectionId, str(e)))
                raise Port.InvalidPort(1, "Invalid Port for Connection ID:" + str(connectionId) )
        finally:
            self.port_lock.release()
    
    def disconnectPort(self, connectionId):
        try:
            self.port_lock.acquire()
            try:
                self.streamContainer.detachByConnectionId(connectionId)
            except:
                if self.logger:
                    self.logger.error("Unable to detach %s, should not have happened", str(connId))
        finally:
            self.port_lock.release()
        OutPort.disconnectPort( self, connectionId )

    def detach(self, attachId=None, connectionId=None):
        if self.logger:
            self.logger.trace("bulkio::OutVITA49Port, DETACH ENTER ")

        self.port_lock.acquire()
        try:
            if connectionId:
                for stream in self.streamContainer.streams:
                    stream.detachByConnectionId(connectionId)

            if attachId:
                for stream in self.streamContainer.streams:
                    for atts in list(stream.streamAttachments):
                        if atts.attachId == attachId:
                            atts.inputPort.detach(attachId)
                            stream.streamAttachments.pop(atts)

            if not attachId and not connectionId:
                for stream in self.streamContainer.streams:
                    for atts in list(stream.streamAttachments):
                        atts.inputPort.detach(attachId)
                self.streamContainer = OutAttachablePort.StreamContainer()

        finally:
            self.port_lock.release()

        if self.logger:
            self.logger.trace("bulkio::OutVITA49Port, DETACH EXIT ")
    
    def attach(self, streamData, name):

        if self.logger:
            self.logger.trace("bulkio::OutVITA49Port, ATTACH ENTER ")

        ids = []
        self.port_lock.acquire()
        try:
            stream = self.streamContainer.findByStreamId(streamData.id)
            if stream:
                stream.detachAll()
            else:
                stream = OutAttachablePort.Stream(streamDef=streamData, name=name, streamId=streamData.id)
                self.streamContainer.addStream(stream) 

            portListed = False
            for connId, port in self.outConnections.items():
                for ftPtr in self.filterTable:

                    # check if port was listed in connection filter table
                    if ftPtr.port_name == self.name:
                        portListed = True

                    if (ftPtr.port_name == self.name) and (ftPtr.connection_id == connId) and (ftPtr.stream_id == stream.streamId):
                        try:
                           if self.sriDict.has_key(stream.streamId):
                              sriMap = self.sriDict[stream.streamId]
                              stream.sri = sriMap.sri
                              stream.time = sriMap.time
                           stream.createNewAttachment(connectionId,port)
                        except Exception, e:
                            if self.logger:
                                self.logger.error("bulkio.OutVITA49Port attach() Unable to deliver update to %s: %s" % (str(connId), str(e)))

            if not portListed: 
                if self.sriDict.has_key(stream.streamId):
                    sriMap = self.sriDict[stream.streamId]
                    stream.sri = sriMap.sri
                    stream.time = sriMap.time
                for connId,port in self.outConnections.items():
                    try:
                       stream.createNewAttachment(connectionId,port)
                    except Exception, e:
                        if self.logger:
                            self.logger.error("bulkio.OutVITA49Port attach() Unable to deliver update to %s: %s" % (str(connId), str(e)))

        finally:
            self.port_lock.release()

        for atts in stream.streamAttachments:
            ids.append(atts.attachId)
            if self.logger:
                self.logger.debug("VITA49 PORT, ATTACH COMPLETED ID " + str(atts.attachId) + " CONNECTION ID:" + str(atts.connectionId))
        if self.logger:
            self.logger.trace("bulkio::OutVITA49Port, ATTACH EXIT ")

        return ids

    def getStreamDefinition(self, attachId):
        streamDefList = []
        for stream in self.streamContainer.streams:
            for atts in stream.streamAttachments:
                if atts.attachId == attachId:
                    streamDefList.append(stream.streamDef)
        return streamDefList 

    def getUser(self, attachId):
        nameList = []
        for stream in self.streamContainer.streams:
            for atts in stream.streamAttachments:
                if atts.attachId == attachId:
                    nameList.append(stream.name)
        return nameList 
    
    def pushSRI(self, H, T):
        if self.logger:
            self.logger.trace("bulkio::OutVITA49Port, PUSH-SRI ENTER ")

        self.port_lock.acquire()
        self.sriDict[H.streamID] = OutPort.SriMapStruct(sri=copy.deepcopy(H), connections=set(), time=copy.deepcopy(T)) 
        self.defaultStreamSRI = H
        self.defaultTime = T
        try:
            portListed = False
            for connId, port in self.outConnections.items():
                for ftPtr in self.filterTable:

                    # check if port was listed in connection filter table
                    if ftPtr.port_name == self.name:
                        portListed = True

                    if (ftPtr.port_name == self.name) and (ftPtr.connection_id == connId) and (ftPtr.stream_id == H.streamID):
                        try:
                            if port != None:
                                port.pushSRI(H, T)
                                self.sriDict[H.streamID].connections.add(connId)
                                stream = self.streamContainer.findByStreamId(H.streamID)
                                if stream:
                                    stream.sri = H
                                    stream.time = T
                        except Exception:
                            if self.logger:
                                self.logger.error("The call to pushSRI failed on port %s connection %s instance %s", self.name, connId, port)

            if not portListed:
                for connId, port in self.outConnections.items():
                    try:
                        if port != None:
                            port.pushSRI(H, T)
                            self.sriDict[H.streamID].connections.add(connId)
                            stream = self.streamContainer.findByStreamId(H.streamID)
                            if stream:
                                stream.sri = H
                                stream.time = T
                    except Exception:
                        if self.logger:
                            self.logger.error("The call to pushSRI failed on port %s connection %s instance %s", self.name, connId, port)
        finally:
            self.port_lock.release() 

        if self.logger:
            self.logger.trace("bulkio::OutVITA49Port, PUSH-SRI EXIT ")

    def updateConnectionFilter(self, _filterTable):
        self.port_lock.acquire()
        if _filterTable == None :
            _filterTable = []
        self.filterTable = _filterTable

        #1. loop over filterTable
            #A. ignore other port_names
            #B. create mapping of streamid->connections(attachments) 

        hasPortEntry = False
        streamsFound = {}
        streamAttachments = {}
        # Populate streamsFound 
        knownStreamIds = self.streamContainer.getStreamIds()
        for id in knownStreamIds:
            streamsFound[id] = False

        # Iterate through each filterTable entry and capture state
        for entry in self.filterTable:
            if entry.port_name != self.name:
                continue

            hasPortEntry = True
            if entry.connection_id in self.outConnections.keys():
                connectedPort = self.outConnections.get(entry.connection_id)
            else:
                if self.logger:
                    self.logger.warn("bulkio::OutVITA49Port, updateConnectionFilter() Unable to find connected port with connectionId: " + entry.connection_id)
                continue

            if self.streamContainer.hasStreamId(entry.stream_id):
                streamsFound[entry.stream_id] = True
                expectedAttachment = OutAttachablePort.StreamAttachment(entry.connection_id, None, connectedPort)
                if not streamAttachments.has_key(entry.stream_id):
                    streamAttachments[entry.stream_id] = []
                streamAttachments[entry.stream_id].append(expectedAttachment)

        for entry in streamAttachments:
            streamId = entry.streamId
            foundStream = self.streamContainer.findByStreamId(streamId)
            if foundStream:
                foundStream.updateAttachments(entry)
            else:
                if self.logger:
                    self.logger.warn("bulkio::OutVITA49Port, updateConnectionFilter() Unable to locate stream definition for streamId: " +streamId)

        
        if hasPortEntry:
            # If there's a valid port entry, we need to detach unmentioned streams
            for streamId,found in streamsFound.items():
                if not found:
                    stream = self.streamContainer.findByStreamId(streamId)
                    if stream:
                        stream.detachAll()
        else:
            # No port entry == All connections on
            for connId, port in self.outConnections.items():
                self.streamContainer.addConnectionToAllStreams(connId_,port)

        self.port_lock.release()
