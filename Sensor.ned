simple Sensor
{
    parameters:
        bool isSlotted=default(false);
        double txRate @unit(bps);         // transmission rate
        double radioDelay @unit(s);       // propagation delay of radio link
        @display("i=device/pc_s");
        double maxBackOff @unit(s);///
        int lengthRTS @unit(b);
        bool discard=default(false); 
        bool reverseExponential=default(false); 
        int minData @unit(b);
        int maxData @unit(b); 
        double mediaExp; 
        double PER = default(-1);
        int numSensors;
        int distF=default(0);
        double improvedBackOffFactor;
        bool improvedProtocol= default(false) ;
    gates://
       input sensorIn @directIn;//
}
