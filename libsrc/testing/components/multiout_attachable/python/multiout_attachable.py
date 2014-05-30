#!/usr/bin/env python
#
#
# AUTO-GENERATED
#
# Source: multiout_attachable.spd.xml
from ossie.resource import start_component
import logging
import uuid

from multiout_attachable_base import *

class multiout_attachable_i(multiout_attachable_base):
        
    class SDDSCallback():
        def __init__(self, parent):
            self.parent = parent
        
        def attach(self, streamDef, user):
            self.parent.callback_stats.num_sdds_attaches += 1
            aid = str(uuid.uuid4())
            newAttachment = self.parent.SddsAttachment(streamDef.id, aid, streamDef.port)
            self.parent.received_sdds_attachments.append(newAttachment)
            return aid
                    
        def detach(self, attachId):
            for curr in list(self.parent.received_sdds_attachments):
                if curr.attachId == attachId:
                    self.parent.received_sdds_attachments.remove(curr)
                    self.parent.callback_stats.num_sdds_detaches += 1
                    
    class VITA49Callback():
        def __init__(self, parent):
            self.parent = parent
        
        def attach(self, streamDef, user):
            self.parent.callback_stats.num_vita49_attaches += 1
            aid = str(uuid.uuid4())
            newAttachment = self.parent.Vita49Attachment(streamDef.id, aid, streamDef.port)
            self.parent.received_vita49_attachments.append(newAttachment)
            return aid
                    
        def detach(self, attachId):
            for curr in list(self.parent.received_vita49_attachments):
                if curr.attachId == attachId:
                    self.parent.received_vita49_attachments.remove(curr)
                    self.parent.callback_stats.num_vita49_detaches += 1
    
    def initialize(self):
        multiout_attachable_base.initialize(self)
        self.reattaches = 0
    
        self.port_dataSDDS_in.logger = self._log
        self.port_dataVITA49_in.logger = self._log
        self.port_dataSDDS_out.setLogger(self._log)
        self.port_dataVITA49_out.setLogger(self._log)
        
        self.sddsCallback = self.SDDSCallback(self)
        self.vitaCallback = self.VITA49Callback(self)
    
        self.port_dataSDDS_in.setNewAttachDetachListener(self.sddsCallback)
        self.port_dataVITA49_in.setNewAttachDetachListener(self.vitaCallback)

        self.port_dataSDDS_in.setNewSriListener(self.newSriCallback)
        self.port_dataVITA49_in.setNewSriListener(self.newSriCallback)

        self.port_dataSDDS_in.setSriChangeListener(self.sriChangeCallback)
        self.port_dataVITA49_in.setSriChangeListener(self.sriChangeCallback)

    def onconfigure_prop_SDDSStreamDefinitions(self, oldVal, newVal):
        oldAttachIds = []
        newAttachIds = []
   
        for oldDef in oldVal: oldAttachIds.append(oldDef.id)
        for newDef in newVal: newAttachIds.append(newDef.id)

        # Find which streams need to be detaches
        for oldDef in oldVal:
            if newAttachIds.count(oldDef.id) == 0:
                self.port_dataSDDS_out.removeStream(oldDef.id)
        
        # Find which new streams need to be attached
        for newDef in newVal:
            isNew = (oldAttachIds.count(newDef.id) == 0)
            isUpdated = False
            
            for oldDef in oldVal:
                if oldDef.id == newDef.id:
                    # Test only allows for updated port value
                    isUpdated = (oldDef.port != newDef.port)
                    if isUpdated: break
        
            
            newAttachment = bulkio.BULKIO.SDDSStreamDefinition(newDef.id,
                                                               bulkio.BULKIO.SDDS_CF,
                                                               newDef.multicastAddress,
                                                               newDef.vlan,
                                                               newDef.port,
                                                               newDef.sampleRate,
                                                               newDef.timeTagValid,
                                                               newDef.privateInfo)
            
            if isNew: self.port_dataSDDS_out.addStream(newAttachment)
            if isUpdated: self.port_dataSDDS_out.updateStream(newAttachment)
        
        self.SDDSStreamDefinitions = newVal;
        

    def onconfigure_prop_VITA49StreamDefinitions(self, oldVal, newVal):       
        oldAttachIds = []
        newAttachIds = []
        
        for oldDef in oldVal: oldAttachIds.append(oldDef.id)
        for newDef in newVal: newAttachIds.append(newDef.id)

        # Find which streams need to be detaches
        for oldDef in oldVal:
            if newAttachIds.count(oldDef.id) == 0:
                self.port_dataVITA49_out.removeStream(oldDef.id)
        
        # Find which new streams need to be attached
        for newDef in newVal:
            isNew = (oldAttachIds.count(newDef.id) == 0)
            isUpdated = False
            
            for oldDef in oldVal:
                if oldDef.id == newDef.id:
                    # Test only allows for updated port value
                    isUpdated = (oldDef.port != newDef.port)
                    if isUpdated: break
        
            
            dataFormat = bulkio.BULKIO.VITA49DataPacketPayloadFormat(newDef.packing_method_processing_efficient,
                                                                     bulkio.BULKIO.VITA49_REAL,
                                                                     bulkio.BULKIO.VITA49_32F,
                                                                     newDef.repeating,
                                                                     newDef.event_tag_size,
                                                                     newDef.channel_tag_size,
                                                                     newDef.item_packing_field_size,
                                                                     newDef.data_item_size,
                                                                     newDef.repeat_count,
                                                                     newDef.vector_size)
            
            newAttachment = bulkio.BULKIO.VITA49StreamDefinition(newDef.ip_address,
                                                                 newDef.vlan,
                                                                 newDef.port,
                                                                 bulkio.BULKIO.VITA49_UDP_TRANSPORT,
                                                                 newDef.id,
                                                                 newDef.valid_data_format,
                                                                 dataFormat)
            
            if isNew: self.port_dataVITA49_out.addStream(newAttachment)
            if isUpdated: self.port_dataVITA49_out.updateStream(newAttachment)
            
        self.VITA49StreamDefinitions = newVal;

    def process(self):
        data, T, EOS, streamID, sri, sriChanged, inputQueueFlushed = self.port_dataFloat_in.getPacket()
        if sriChanged:
            logging.debug("process() sri changed : " + str(sri) + " T: " + str(T))
            self.port_dataSDDS_out.pushSRI(sri,T)
            self.port_dataVITA49_out.pushSRI(sri,T)

        return NOOP

    def newSriCallback(self,sri):
        # Query SRIs to ensure deadlock doesn't occur
        sddsSriList = self.port_dataSDDS_in._get_activeSRIs()
        vita49SriList = self.port_dataVITA49_in._get_activeSRIs()

        self.callback_stats.num_new_sri_callbacks += 1

    def sriChangeCallback(self,sri):
        # Query SRIs to ensure deadlock doesn't occur
        sddsSriList = self.port_dataSDDS_in._get_activeSRIs()
        vita49SriList = self.port_dataVITA49_in._get_activeSRIs()

        self.callback_stats.num_sri_change_callbacks += 1
  
if __name__ == '__main__':
    logging.getLogger().setLevel(logging.INFO)
    logging.debug("Starting Component")
    start_component(multiout_attachable_i)

