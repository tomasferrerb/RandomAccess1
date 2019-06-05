simple Satelite
{
    parameters:
        @display("i=device/satellite");

       double maxBackOff @unit(s);///
       double txRate @unit(bps);         // transmission rate
 	   int lengthCTS @unit(b)= default(56b);
 	   int lengthBcast @unit(b)=default(56b);
       int numSensors;
       double PER = default(-1);
       int numberOfCycles = default(5000);
       double improvedBackOffFactor;
        bool improvedProtocol= default(false) ;
      
    gates:
        input in @directIn;
}
