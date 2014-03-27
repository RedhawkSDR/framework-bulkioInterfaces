
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


class OutPort (BULKIO__POA.UsesPortStatisticsProvider ):
    
    class connection_descriptor_struct:
        def __init__( self, pname=None, conn_name=None, stream_id=None ): 
            self.port_name=pname
            self.connection_id=conn_name
            self.stream_id = stream_id

    class SriMapStruct:
        def __init__( self, sri=None, connections=None): 
            self.sri=sri
            self.connections = connections #set of connection ID strings that have received this SRI

    class SDDSSriMapStruct(SriMapStruct):
        def __init__( self, sri=None, connections=None, time=None): 
            super(SriMapStruct,self).__init__(sri,connections)
            self.time = time
    
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

class OutSDDSPort(OutPort):
    TRANSFER_TYPE = 'c'
    def __init__(self, name, max_attachments=1, logger=None ):
        OutPort.__init__(self, name, BULKIO.dataSDDS, OutSDDSPort.TRANSFER_TYPE , logger )
        self.max_attachments = max_attachments
        self.attachedGroup = {} # key=connection_id,  value=attach_id
        self.lastStreamData = None
        self.lastName = None
        self.defaultStreamSRI = sri.create()
        self.defaultTime = timestamp.now()
        
    def _get_state(self):
        self.port_lock.acquire()
        if len(self._attachedStreams.values()) == 0:
            return BULKIO.IDLE
        # default behavior is to limit to one connection
        elif len(self._attachedStreams.values()) == 1:
            return BULKIO.BUSY
        else:
            return BULKIO.ACTIVE

    def _get_attachedSRIs(self):
        return self._get_activeSRIs()

    def connectPort(self, connection, connectionId):
        OutPort.connectPort( self, connection, connectionId )
        self.port_lock.acquire()
        try:
           try:
               port = self.outConnections[str(connectionId)]
               if self.lastStreamData:
                   self.attachedGroup[str(connectionId)] = port.attach(self.lastStreamData, self.lastName)
           except:
               raise Port.InvalidPort(1, "Invalid Port for Connection ID:" + str(connectionId) )
        finally:
            self.port_lock.release()
    
    def disconnectPort(self, connectionId):
        try:
            self.port_lock.acquire()
            connId = str(connectionId)
            entry = self.outConnections[connId]
            if connId in self.attachedGroup:
                try:
                    entry.detach(self.attachedGroup.pop(connId))
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
        if attachId == None:
            for entry in self.outConnections:
                try:
                    if entry in self.attachedGroup:
                        if connectionId == None or entry == connectionId:
                            self.outConnections[entry].detach(self.attachedGroup[entry])
                            self.attachedGroup.pop(entry)
                except:
                    if self.logger:
                        self.logger.error("Unable to detach %s", str(entry))
            self.lastStreamData = None
            self.lastName = None
        else:
            for entry in self.attachedGroup:
                try:
                    if self.attachedGroup[entry] == attachId:
                        if entry in self.outConnections:
                            if connectionId == None or entry == connectionId:
                                self.outConnections[entry].detach(self.attachedGroup[entry])
                        self.attachedGroup.pop(entry)
                        if len(self.attachedGroup) == 0:
                            self.lastStreamData = None
                            self.lastName = None
                        break
                except:
                    if self.logger:
                        self.logger.error("Unable to detach %s", str(entry))

        self.port_lock.release()
        if self.logger:
            self.logger.trace("bulkio::OutSDDSPort, DETACH EXIT ")
    
    def attach(self, streamData, name):

        if self.logger:
            self.logger.trace("bulkio::OutSDDSPort, ATTACH ENTER ")

        ids = []
        self.port_lock.acquire()
        for entry in self.outConnections:
            try:
                if entry in self.attachedGroup:
                    self.outConnections[entry].detach(self.attachedGroup[entry])
                self.attachedGroup[entry] = self.outConnections[entry].attach(streamData, name)
                ids.append(self.attachedGroup[entry])
            except:
                if self.logger:
                    self.logger.error("Unable to deliver update to %s", str(entry))
        self.lastStreamData = streamData
        self.lastName = name
        self.port_lock.release()

        if self.logger:
            if len(ids) > 0 :
                self.logger.debug("SDDS PORT, ATTACH COMPLETED ID " + str(ids[0]) + " NAME(user-id):" + str(name))
            self.logger.trace("bulkio::OutSDDSPort, ATTACH EXIT ")

        return ids

    def getStreamDefinition(self, attachId):
        return self.lastStreamData

    def getUser(self, attachId):
        return self.lastName
    
    def pushSRI(self, H, T):
        if self.logger:
            self.logger.trace("bulkio::OutSDDSPort, PUSH-SRI ENTER ")

        self.port_lock.acquire()
        self.sriDict[H.streamID] = OutPort.SDDSSriMapStruct(sri=copy.deepcopy(H), connections=set(), time=copy.deepcopy(T)) 
        self.defaultStreamSRI = H
        self.defaultTime = T
        try:
            for connId, port in self.outConnections.items():
               try:
                    if port != None:
                        port.pushSRI(H, T)
               except Exception:
                   if self.logger:
                       self.logger.error("The call to pushSRI failed on port %s connection %s instance %s", self.name, connId, port)
        finally:
            self.port_lock.release() 

        if self.logger:
            self.logger.trace("bulkio::OutSDDSPort, PUSH-SRI EXIT ")
