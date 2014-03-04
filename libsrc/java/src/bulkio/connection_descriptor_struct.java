package bulkio;

import org.ossie.component.*;
import org.ossie.properties.*;

public class connection_descriptor_struct extends StructDef {
    public final StringProperty connection_name =
        new StringProperty(
            "connectionTable::connection_name", //id
            "connection_name", //name
            null, //default value
            Mode.READWRITE, //mode
            Action.EXTERNAL, //action
            new Kind[] {Kind.CONFIGURE} //kind
            );
    public final StringProperty stream_id =
        new StringProperty(
            "connectionTable::stream_id", //id
            "stream_id", //name
            null, //default value
            Mode.READWRITE, //mode
            Action.EXTERNAL, //action
            new Kind[] {Kind.CONFIGURE} //kind
            );
    public final StringProperty port_name =
        new StringProperty(
            "connectionTable::port_name", //id
            "port_name", //name
            null, //default value
            Mode.READWRITE, //mode
            Action.EXTERNAL, //action
            new Kind[] {Kind.CONFIGURE} //kind
            );
        public connection_descriptor_struct(String connection_name, String stream_id, String port_name) {
            this();
            this.connection_name.setValue(connection_name);
            this.stream_id.setValue(stream_id);
            this.port_name.setValue(port_name);
        }
    
        /**
         * @generated
         */
        public connection_descriptor_struct() {
            addElement(this.connection_name);
            addElement(this.stream_id);
            addElement(this.port_name);
        }
    
        public String getId() {
            return "connection_descriptor";
        }
};
