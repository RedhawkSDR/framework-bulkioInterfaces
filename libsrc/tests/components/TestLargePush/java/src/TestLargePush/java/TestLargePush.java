package TestLargePush.java;

import java.util.Properties;

public class TestLargePush extends TestLargePush_base {

    public TestLargePush() {
        super();
    }

    protected int serviceFunction() {
        String stream_id = "testStream";
        BULKIO.StreamSRI sri = new BULKIO.StreamSRI();
        sri.mode = 0;
        sri.xdelta = 0.0;
        sri.ydelta = 1.0;
        sri.subsize = 0;
        sri.xunits = 1; // TIME_S
        sri.streamID = (stream_id != null) ? stream_id : "";

        BULKIO.PrecisionUTCTime tstamp = bulkio.time.utils.now();

        Boolean  EOS = true;

        float[] outDataFloat = new float[this.numSamples.getValue().intValue()];
        this.port_dataFloat.pushPacket(outDataFloat, tstamp, EOS, sri.streamID);

        double[] outDataDouble = new double[this.numSamples.getValue().intValue()];
        this.port_dataDouble.pushPacket(outDataDouble, tstamp, EOS, sri.streamID);

        short[] outDataShort = new short[this.numSamples.getValue().intValue()];
        this.port_dataShort.pushPacket(outDataShort, tstamp, EOS, sri.streamID);

        short[] outDataUshort = new short[this.numSamples.getValue().intValue()];
        this.port_dataUshort.pushPacket(outDataUshort, tstamp, EOS, sri.streamID);

        int[] outDataLong = new int[this.numSamples.getValue().intValue()];
        this.port_dataLong.pushPacket(outDataLong, tstamp, EOS, sri.streamID);

        int[] outDataULong = new int[this.numSamples.getValue().intValue()];
        this.port_dataUlong.pushPacket(outDataULong, tstamp, EOS, sri.streamID);

        long[] outDataLongLong = new long[this.numSamples.getValue().intValue()];
        this.port_dataLongLong.pushPacket(outDataLongLong, tstamp, EOS, sri.streamID);

        long[] outDataUlongLong = new long[this.numSamples.getValue().intValue()];
        this.port_dataUlongLong.pushPacket(outDataUlongLong, tstamp, EOS, sri.streamID);

        byte[] outDataOctet = new byte[this.numSamples.getValue().intValue()];
        this.port_dataOctet.pushPacket(outDataOctet, tstamp, EOS, sri.streamID);

        return FINISH;
    }

    public static void configureOrb(final Properties orbProps) {
    }
}
