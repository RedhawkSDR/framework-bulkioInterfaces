#!/usr/bin/env python
import unittest
import ossie.utils.testing
import os
import time
from omniORB import any
from ossie.utils import sb

from ossie.utils.sandbox import debugger
#from ossie.utils.log4py import logging
#logging.basicConfig()
#logging.getLogger().setLevel(logging.DEBUG)

class ResourceTests(ossie.utils.testing.ScaComponentTestCase):
    """Test for all resource implementations in multiout_attachable"""

    def setUp(self):
        ossie.utils.testing.ScaComponentTestCase.setUp(self)
        #self.source = sb.launch("../multiout_attachable.spd.xml", impl=self.impl, execparams={"DEBUG_LEVEL":4})
        #self.sink1 = sb.launch("../multiout_attachable.spd.xml", impl=self.impl)#, execparams={"DEBUG_LEVEL":4})
        self.source = sb.launch("../multiout_attachable.spd.xml", impl=self.impl)
        self.sink1 = sb.launch("../multiout_attachable.spd.xml", impl=self.impl)
        self.sink2 = sb.launch("../multiout_attachable.spd.xml", impl=self.impl)
        self.sink3 = sb.launch("../multiout_attachable.spd.xml", impl=self.impl)
        self.sink4 = sb.launch("../multiout_attachable.spd.xml", impl=self.impl)

    def tearDown(self):
        #ossie.utils.testing.ScaComponentTestCase.tearDown(self)
        self.source.releaseObject()
        self.sink1.releaseObject()
        self.sink2.releaseObject()
        self.sink3.releaseObject()
        self.sink4.releaseObject()
        del self.source
        del self.sink1
        del self.sink2
        del self.sink3
        del self.sink4

    def connectAllSdds(self):
        self.source.connect(self.sink1, 'dataSDDS_in', connectionId="conn1")
        self.source.connect(self.sink2, 'dataSDDS_in', connectionId="conn2")
        self.source.connect(self.sink3, 'dataSDDS_in', connectionId="conn3")
        self.source.connect(self.sink4, 'dataSDDS_in', connectionId="conn4")

    def connectAllVita(self):
        self.source.connect(self.sink1, 'dataVITA49_in', connectionId="vita1")
        self.source.connect(self.sink2, 'dataVITA49_in', connectionId="vita2")
        self.source.connect(self.sink3, 'dataVITA49_in', connectionId="vita3")
        self.source.connect(self.sink4, 'dataVITA49_in', connectionId="vita4")
    
    def disconnectAll(self):
        self.source.disconnect(self.sink1)
        self.source.disconnect(self.sink2)
        self.source.disconnect(self.sink3)
        self.source.disconnect(self.sink4)
    
    def addSddsStream(self, streamId):
        newStream = {
            'sdds::privateInfo': "whoops", 
            'sdds::timeTagValid': False, 
            'sdds::sampleRate': 0L, 
            'sdds::id': streamId, 
            'sdds::multicastAddress': '0.0.0.0', 
            'sdds::port': 0L, 
            'sdds::vlan': 0L
        }
        self.source.SDDSStreamDefinitions.append(newStream)

    def addVitaStream(self, streamId):
        newStream = {
            'vita49::data_item_size': 0, 
            'vita49::vlan': 0L, 
            'vita49::vector_size': 0, 
            'vita49::valid_data_format': False, 
            'vita49::event_tag_size': 0, 
            'vita49::channel_tag_size': 0, 
            'vita49::port': 0L, 
            'vita49::repeat_count': 0, 
            'vita49::item_packing_field_size': 0, 
            'vita49::ip_address': '0.0.0.0', 
            'vita49::packing_method_processing_efficient': False, 
            'vita49::id': streamId, 
            'vita49::repeating': False
        }
        self.source.VITA49StreamDefinitions.append(newStream)


    def addConnectionTableEntry(self, connId, streamId, portName):
        entry = {
            'connectionTable::stream_id': streamId, 
            'connectionTable::connection_id': connId, 
            'connectionTable::port_name': portName
        }
        self.source.connectionTable.append(entry)

    def removeAllSddsStreams(self):
        self.source.SDDSStreamDefinitions = []
    
    def removeAllVitaStreams(self):
        self.source.VITA49StreamDefinitions = []

    def removeAllConnectionTableEntries(self):
        self.source.connectionTable = []

    def testSddsConnectionsWithNoStreams(self):
        self.connectAllSdds()       
 
        # No attaches should have been made
        self.assertEquals(self.sink1.callback_stats.num_sdds_attaches, 0)
        self.assertEquals(self.sink2.callback_stats.num_sdds_attaches, 0)
        self.assertEquals(self.sink3.callback_stats.num_sdds_attaches, 0)
        self.assertEquals(self.sink4.callback_stats.num_sdds_attaches, 0)
        
        # No detaches should have been made
        self.assertEquals(self.sink1.callback_stats.num_sdds_attaches, 0)
        self.assertEquals(self.sink2.callback_stats.num_sdds_attaches, 0)
        self.assertEquals(self.sink3.callback_stats.num_sdds_attaches, 0)
        self.assertEquals(self.sink4.callback_stats.num_sdds_attaches, 0)
    
    def testVitaConnectionsWithNoStreams(self):
        self.connectAllVita()       
 
        # No attaches should have been made
        self.assertEquals(self.sink1.callback_stats.num_vita49_attaches, 0)
        self.assertEquals(self.sink2.callback_stats.num_vita49_attaches, 0)
        self.assertEquals(self.sink3.callback_stats.num_vita49_attaches, 0)
        self.assertEquals(self.sink4.callback_stats.num_vita49_attaches, 0)
        
        # No detaches should have been made
        self.assertEquals(self.sink1.callback_stats.num_vita49_attaches, 0)
        self.assertEquals(self.sink2.callback_stats.num_vita49_attaches, 0)
        self.assertEquals(self.sink3.callback_stats.num_vita49_attaches, 0)
        self.assertEquals(self.sink4.callback_stats.num_vita49_attaches, 0)

    # 
    # Test adding streams while connections exist
    #
    def testAddingSDDSStreamToActiveConnections(self):
        self.connectAllSdds()
        self.addSddsStream("Stream1")

        self.assertEquals(self.sink1.callback_stats.num_sdds_attaches, 1)
        self.assertEquals(self.sink2.callback_stats.num_sdds_attaches, 1)
        self.assertEquals(self.sink3.callback_stats.num_sdds_attaches, 1)
        self.assertEquals(self.sink4.callback_stats.num_sdds_attaches, 1)
    
    def testAddingSDDSStreamsToActiveConnections(self):
        self.connectAllSdds()
        self.addSddsStream("Stream1")
        self.addSddsStream("Stream2")
        self.addSddsStream("Stream3")
        
        self.assertEquals(self.sink1.callback_stats.num_sdds_attaches, 3)
        self.assertEquals(self.sink2.callback_stats.num_sdds_attaches, 3)
        self.assertEquals(self.sink3.callback_stats.num_sdds_attaches, 3)
        self.assertEquals(self.sink4.callback_stats.num_sdds_attaches, 3)
   
    # 
    # Test adding connections after streams
    #
    def testAddingSDDSConnectionsToActiveStream(self):
        self.addSddsStream("Stream1")
        self.connectAllSdds()

        self.assertEquals(self.sink1.callback_stats.num_sdds_attaches, 1)
        self.assertEquals(self.sink2.callback_stats.num_sdds_attaches, 1)
        self.assertEquals(self.sink3.callback_stats.num_sdds_attaches, 1)
        self.assertEquals(self.sink4.callback_stats.num_sdds_attaches, 1)

    def testAddingSDDSConnectionsToActiveStreams(self):
        self.addSddsStream("Stream1")
        self.addSddsStream("Stream2")
        self.addSddsStream("Stream3")
        self.connectAllSdds()
        
        self.assertEquals(self.sink1.callback_stats.num_sdds_attaches, 3)
        self.assertEquals(self.sink2.callback_stats.num_sdds_attaches, 3)
        self.assertEquals(self.sink3.callback_stats.num_sdds_attaches, 3)
        self.assertEquals(self.sink4.callback_stats.num_sdds_attaches, 3)
    
    #
    # Test removing streams
    #
    def testDisconnectingActiveSDDSStreamsSF(self):
        self.addSddsStream("Stream1")
        self.addSddsStream("Stream2")
        self.addSddsStream("Stream3")
        self.connectAllSdds()
        self.disconnectAll()
        
        self.assertEquals(self.sink1.callback_stats.num_sdds_detaches, 3)
        self.assertEquals(self.sink2.callback_stats.num_sdds_detaches, 3)
        self.assertEquals(self.sink3.callback_stats.num_sdds_detaches, 3)
        self.assertEquals(self.sink4.callback_stats.num_sdds_detaches, 3)
    
    def testDisconnectingActiveSDDSStreamsCF(self):
        self.connectAllSdds()
        self.addSddsStream("Stream1")
        self.addSddsStream("Stream2")
        self.addSddsStream("Stream3")
        self.disconnectAll()

        self.assertEquals(self.sink1.callback_stats.num_sdds_detaches, 3)
        self.assertEquals(self.sink2.callback_stats.num_sdds_detaches, 3)
        self.assertEquals(self.sink3.callback_stats.num_sdds_detaches, 3)
        self.assertEquals(self.sink4.callback_stats.num_sdds_detaches, 3)
    
    #
    # Test reconnecting streams
    #
    def testReconnectingActiveSDDSStreams(self):
        self.addSddsStream("Stream1")
        self.addSddsStream("Stream2")
        self.addSddsStream("Stream3")
        self.connectAllSdds()
        self.removeAllSddsStreams()
        self.addSddsStream("Stream1")
        self.addSddsStream("Stream2")
        self.addSddsStream("Stream3")

        self.assertEquals(self.sink1.callback_stats.num_sdds_attaches, 6)
        self.assertEquals(self.sink2.callback_stats.num_sdds_attaches, 6)
        self.assertEquals(self.sink3.callback_stats.num_sdds_attaches, 6)
        self.assertEquals(self.sink4.callback_stats.num_sdds_attaches, 6)
        
        self.assertEquals(self.sink1.callback_stats.num_sdds_detaches, 3)
        self.assertEquals(self.sink2.callback_stats.num_sdds_detaches, 3)
        self.assertEquals(self.sink3.callback_stats.num_sdds_detaches, 3)
        self.assertEquals(self.sink4.callback_stats.num_sdds_detaches, 3)

        self.assertEquals(len(self.sink1.received_sdds_attachments), 3)
        self.assertEquals(len(self.sink2.received_sdds_attachments), 3)
        self.assertEquals(len(self.sink3.received_sdds_attachments), 3)
        self.assertEquals(len(self.sink4.received_sdds_attachments), 3)

    #
    # Test connection table
    #
    def testFilterTablePostSDDSConnection(self):
        self.addSddsStream("Stream1")
        self.addSddsStream("Stream2")
        self.addSddsStream("Stream3")
        self.connectAllSdds()
        self.addConnectionTableEntry("conn1","Stream1","dataSDDS_out")
        
        self.assertEquals(len(self.sink1.received_sdds_attachments), 1)
        self.assertEquals(len(self.sink2.received_sdds_attachments), 0)
        self.assertEquals(len(self.sink3.received_sdds_attachments), 0)
        self.assertEquals(len(self.sink4.received_sdds_attachments), 0)
    
    def testFilterTablePreSDDSConnection(self):
        self.addSddsStream("Stream1")
        self.addSddsStream("Stream2")
        self.addSddsStream("Stream3")
        self.addConnectionTableEntry("conn1","Stream1","dataSDDS_out")
        self.connectAllSdds()
        
        self.assertEquals(len(self.sink1.received_sdds_attachments), 1)
        self.assertEquals(len(self.sink2.received_sdds_attachments), 0)
        self.assertEquals(len(self.sink3.received_sdds_attachments), 0)
        self.assertEquals(len(self.sink4.received_sdds_attachments), 0)
    
    def testFilterTablePreSDDSStreams(self):
        self.addConnectionTableEntry("conn1","Stream1","dataSDDS_out")
        self.addSddsStream("Stream1")
        self.addSddsStream("Stream2")
        self.addSddsStream("Stream3")
        self.connectAllSdds()
        
        self.assertEquals(len(self.sink1.received_sdds_attachments), 1)
        self.assertEquals(len(self.sink2.received_sdds_attachments), 0)
        self.assertEquals(len(self.sink3.received_sdds_attachments), 0)
        self.assertEquals(len(self.sink4.received_sdds_attachments), 0)
    
    def testFilterTableInvalidSDDSPort(self):
        self.addSddsStream("Stream1")
        self.addSddsStream("Stream2")
        self.addSddsStream("Stream3")
        self.connectAllSdds()
        self.addConnectionTableEntry("conn1","Stream1","whoopsie")
        
        self.assertEquals(len(self.sink1.received_sdds_attachments), 3)
        self.assertEquals(len(self.sink2.received_sdds_attachments), 3)
        self.assertEquals(len(self.sink3.received_sdds_attachments), 3)
        self.assertEquals(len(self.sink4.received_sdds_attachments), 3)

    def testFilterTableInvalidSDDSStream(self):
        self.addSddsStream("Stream1")
        self.addSddsStream("Stream2")
        self.addSddsStream("Stream3")
        self.connectAllSdds()
        self.addConnectionTableEntry("conn1","whoopsie","dataSDDS_out")
        
        self.assertEquals(len(self.sink1.received_sdds_attachments), 0)
        self.assertEquals(len(self.sink2.received_sdds_attachments), 0)
        self.assertEquals(len(self.sink3.received_sdds_attachments), 0)
        self.assertEquals(len(self.sink4.received_sdds_attachments), 0)
    
    def testFilterTableInvalidSDDSConnectionId(self):
        self.addSddsStream("Stream1")
        self.addSddsStream("Stream2")
        self.addSddsStream("Stream3")
        self.connectAllSdds()
        self.addConnectionTableEntry("whoopsie","Stream1","dataSDDS_out")
        
        self.assertEquals(len(self.sink1.received_sdds_attachments), 0)
        self.assertEquals(len(self.sink2.received_sdds_attachments), 0)
        self.assertEquals(len(self.sink3.received_sdds_attachments), 0)
        self.assertEquals(len(self.sink4.received_sdds_attachments), 0)
    
    def testFilterTableMultiSDDS(self):
        self.addConnectionTableEntry("conn1","Stream1","dataSDDS_out")
        self.addSddsStream("Stream1")
        self.addSddsStream("Stream2")
        self.addSddsStream("Stream3")
        self.addConnectionTableEntry("conn2","Stream2","dataSDDS_out")
        self.connectAllSdds()
        self.addConnectionTableEntry("conn3","Stream3","dataSDDS_out")
        self.addConnectionTableEntry("conn4","Stream3","dataSDDS_out")
        
        self.assertEquals(len(self.sink1.received_sdds_attachments), 1)
        self.assertEquals(len(self.sink2.received_sdds_attachments), 1)
        self.assertEquals(len(self.sink3.received_sdds_attachments), 1)
        self.assertEquals(len(self.sink4.received_sdds_attachments), 1)

    # 
    # Test adding streams while connections exist
    #
    def testAddingVITAStreamToActiveConnections(self):
        self.connectAllVita()
        self.addVitaStream("Stream1")

        self.assertEquals(self.sink1.callback_stats.num_vita49_attaches, 1)
        self.assertEquals(self.sink2.callback_stats.num_vita49_attaches, 1)
        self.assertEquals(self.sink3.callback_stats.num_vita49_attaches, 1)
        self.assertEquals(self.sink4.callback_stats.num_vita49_attaches, 1)
    
    def testAddingVITAStreamsToActiveConnections(self):
        self.connectAllVita()
        self.addVitaStream("Stream1")
        self.addVitaStream("Stream2")
        self.addVitaStream("Stream3")
        
        self.assertEquals(self.sink1.callback_stats.num_vita49_attaches, 3)
        self.assertEquals(self.sink2.callback_stats.num_vita49_attaches, 3)
        self.assertEquals(self.sink3.callback_stats.num_vita49_attaches, 3)
        self.assertEquals(self.sink4.callback_stats.num_vita49_attaches, 3)
   
    # 
    # Test adding connections after streams
    #
    def testAddingVITAConnectionsToActiveStream(self):
        self.addVitaStream("Stream1")
        self.connectAllVita()

        self.assertEquals(self.sink1.callback_stats.num_vita49_attaches, 1)
        self.assertEquals(self.sink2.callback_stats.num_vita49_attaches, 1)
        self.assertEquals(self.sink3.callback_stats.num_vita49_attaches, 1)
        self.assertEquals(self.sink4.callback_stats.num_vita49_attaches, 1)

    def testAddingVITAConnectionsToActiveStreams(self):
        self.addVitaStream("Stream1")
        self.addVitaStream("Stream2")
        self.addVitaStream("Stream3")
        self.connectAllVita()
        
        self.assertEquals(self.sink1.callback_stats.num_vita49_attaches, 3)
        self.assertEquals(self.sink2.callback_stats.num_vita49_attaches, 3)
        self.assertEquals(self.sink3.callback_stats.num_vita49_attaches, 3)
        self.assertEquals(self.sink4.callback_stats.num_vita49_attaches, 3)
    
    #
    # Test removing streams
    #
    def testDisconnectingActiveVITAStreamsSF(self):
        self.addVitaStream("Stream1")
        self.addVitaStream("Stream2")
        self.addVitaStream("Stream3")
        self.connectAllVita()
        self.disconnectAll()

        self.assertEquals(self.sink1.callback_stats.num_vita49_detaches, 3)
        self.assertEquals(self.sink2.callback_stats.num_vita49_detaches, 3)
        self.assertEquals(self.sink3.callback_stats.num_vita49_detaches, 3)
        self.assertEquals(self.sink4.callback_stats.num_vita49_detaches, 3)
    
    def testDisconnectingActiveVITAStreamsCF(self):
        self.connectAllVita()
        self.addVitaStream("Stream1")
        self.addVitaStream("Stream2")
        self.addVitaStream("Stream3")
        self.disconnectAll()

        self.assertEquals(self.sink1.callback_stats.num_vita49_detaches, 3)
        self.assertEquals(self.sink2.callback_stats.num_vita49_detaches, 3)
        self.assertEquals(self.sink3.callback_stats.num_vita49_detaches, 3)
        self.assertEquals(self.sink4.callback_stats.num_vita49_detaches, 3)
    
    #
    # Test reconnecting streams
    #
    def testReconnectingActiveVITAStreams(self):
        self.addVitaStream("Stream1")
        self.addVitaStream("Stream2")
        self.addVitaStream("Stream3")
        self.connectAllVita()
        self.removeAllVitaStreams()
        self.addVitaStream("Stream1")
        self.addVitaStream("Stream2")
        self.addVitaStream("Stream3")

        self.assertEquals(self.sink1.callback_stats.num_vita49_attaches, 6)
        self.assertEquals(self.sink2.callback_stats.num_vita49_attaches, 6)
        self.assertEquals(self.sink3.callback_stats.num_vita49_attaches, 6)
        self.assertEquals(self.sink4.callback_stats.num_vita49_attaches, 6)
        
        self.assertEquals(self.sink1.callback_stats.num_vita49_detaches, 3)
        self.assertEquals(self.sink2.callback_stats.num_vita49_detaches, 3)
        self.assertEquals(self.sink3.callback_stats.num_vita49_detaches, 3)
        self.assertEquals(self.sink4.callback_stats.num_vita49_detaches, 3)

        self.assertEquals(len(self.sink1.received_vita49_attachments), 3)
        self.assertEquals(len(self.sink2.received_vita49_attachments), 3)
        self.assertEquals(len(self.sink3.received_vita49_attachments), 3)
        self.assertEquals(len(self.sink4.received_vita49_attachments), 3)

    #
    # Test connection table
    #
    def testFilterTablePostVITAConnection(self):
        self.addVitaStream("Stream1")
        self.addVitaStream("Stream2")
        self.addVitaStream("Stream3")
        self.connectAllVita()
        self.addConnectionTableEntry("vita1","Stream1","dataVITA49_out")
        
        self.assertEquals(len(self.sink1.received_vita49_attachments), 1)
        self.assertEquals(len(self.sink2.received_vita49_attachments), 0)
        self.assertEquals(len(self.sink3.received_vita49_attachments), 0)
        self.assertEquals(len(self.sink4.received_vita49_attachments), 0)
    
    def testFilterTablePreVITAConnection(self):
        self.addVitaStream("Stream1")
        self.addVitaStream("Stream2")
        self.addVitaStream("Stream3")
        self.addConnectionTableEntry("vita1","Stream1","dataVITA49_out")
        self.connectAllVita()
        
        self.assertEquals(len(self.sink1.received_vita49_attachments), 1)
        self.assertEquals(len(self.sink2.received_vita49_attachments), 0)
        self.assertEquals(len(self.sink3.received_vita49_attachments), 0)
        self.assertEquals(len(self.sink4.received_vita49_attachments), 0)
    
    def testFilterTablePreVITAStreams(self):
        self.addConnectionTableEntry("vita1","Stream1","dataVITA49_out")
        self.addVitaStream("Stream1")
        self.addVitaStream("Stream2")
        self.addVitaStream("Stream3")
        self.connectAllVita()
        
        self.assertEquals(len(self.sink1.received_vita49_attachments), 1)
        self.assertEquals(len(self.sink2.received_vita49_attachments), 0)
        self.assertEquals(len(self.sink3.received_vita49_attachments), 0)
        self.assertEquals(len(self.sink4.received_vita49_attachments), 0)
    
    def testFilterTableInvalidVITAPort(self):
        self.addVitaStream("Stream1")
        self.addVitaStream("Stream2")
        self.addVitaStream("Stream3")
        self.connectAllVita()
        self.addConnectionTableEntry("vita1","Stream1","whoopsie")
        
        self.assertEquals(len(self.sink1.received_vita49_attachments), 3)
        self.assertEquals(len(self.sink2.received_vita49_attachments), 3)
        self.assertEquals(len(self.sink3.received_vita49_attachments), 3)
        self.assertEquals(len(self.sink4.received_vita49_attachments), 3)

    def testFilterTableInvalidVITAStream(self):
        self.addVitaStream("Stream1")
        self.addVitaStream("Stream2")
        self.addVitaStream("Stream3")
        self.connectAllVita()
        self.addConnectionTableEntry("vita1","whoopsie","dataVITA49_out")
        
        self.assertEquals(len(self.sink1.received_vita49_attachments), 0)
        self.assertEquals(len(self.sink2.received_vita49_attachments), 0)
        self.assertEquals(len(self.sink3.received_vita49_attachments), 0)
        self.assertEquals(len(self.sink4.received_vita49_attachments), 0)
    
    def testFilterTableInvalidVITAConnectionId(self):
        self.addVitaStream("Stream1")
        self.addVitaStream("Stream2")
        self.addVitaStream("Stream3")
        self.connectAllVita()
        self.addConnectionTableEntry("whoopsie","Stream1","dataVITA49_out")
        
        self.assertEquals(len(self.sink1.received_vita49_attachments), 0)
        self.assertEquals(len(self.sink2.received_vita49_attachments), 0)
        self.assertEquals(len(self.sink3.received_vita49_attachments), 0)
        self.assertEquals(len(self.sink4.received_vita49_attachments), 0)
    
    def testFilterTableMultiVITA(self):#
        self.addConnectionTableEntry("vita1","Stream1","dataVITA49_out")
        self.addVitaStream("Stream1")
        self.addVitaStream("Stream2")
        self.addVitaStream("Stream3")
        self.addConnectionTableEntry("vita2","Stream2","dataVITA49_out")
        self.connectAllVita()
        self.addConnectionTableEntry("vita3","Stream3","dataVITA49_out")
        self.addConnectionTableEntry("vita4","Stream3","dataVITA49_out")
       
        self.assertEquals(len(self.sink1.received_vita49_attachments), 1)
        self.assertEquals(len(self.sink2.received_vita49_attachments), 1)
        self.assertEquals(len(self.sink3.received_vita49_attachments), 1)
        self.assertEquals(len(self.sink4.received_vita49_attachments), 1)

    def testSDDSAttachmentUpdate(self):
        self.addSddsStream("Stream1")
        self.connectAllSdds()
        newPortValue = 12345
        self.source.SDDSStreamDefinitions[0].port = newPortValue

        self.assertEquals(len(self.sink1.received_sdds_attachments), 1)
        self.assertEquals(len(self.sink2.received_sdds_attachments), 1)
        self.assertEquals(len(self.sink3.received_sdds_attachments), 1)
        self.assertEquals(len(self.sink4.received_sdds_attachments), 1)

        self.assertEquals(self.sink1.received_sdds_attachments[0].port, newPortValue)
        self.assertEquals(self.sink2.received_sdds_attachments[0].port, newPortValue)
        self.assertEquals(self.sink3.received_sdds_attachments[0].port, newPortValue)
        self.assertEquals(self.sink4.received_sdds_attachments[0].port, newPortValue)
    
    def testVITAAttachmentUpdate(self):
        self.addVitaStream("Stream1")
        self.connectAllVita()
        newPortValue = 12345
        self.source.VITA49StreamDefinitions[0].port = newPortValue

        self.assertEquals(len(self.sink1.received_vita49_attachments), 1)
        self.assertEquals(len(self.sink2.received_vita49_attachments), 1)
        self.assertEquals(len(self.sink3.received_vita49_attachments), 1)
        self.assertEquals(len(self.sink4.received_vita49_attachments), 1)

        self.assertEquals(self.sink1.received_vita49_attachments[0].port, newPortValue)
        self.assertEquals(self.sink2.received_vita49_attachments[0].port, newPortValue)
        self.assertEquals(self.sink3.received_vita49_attachments[0].port, newPortValue)
        self.assertEquals(self.sink4.received_vita49_attachments[0].port, newPortValue)

if __name__ == "__main__":
    ossie.utils.testing.main("../multiout_attachable.spd.xml") # By default tests all implementations
