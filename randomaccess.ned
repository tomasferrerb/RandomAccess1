network randomaccess
{
    parameters:
        int numSensors;  // number of hosts
        double txRate @unit(bps)=default (10kbps);  // transmission rate
        double maxBackOff @unit(s);
        int lengthRTS @unit(b)=default(56b);
        int lengthCTS @unit(b)=default(56b);
        @display("bgi=background/terrain,s");
        int maxData @unit(b)= default(4000b);
        int minData @unit(b)= default(0b);
        double mediaExp; //.
        double PER=default(-1);//packet error rate
        int numberOfCycles; //numbre of cycles to finish simulation
        bool reverseExponential=default(false);
        
        double improvedBackOffFactor;
        bool improvedProtocol= default(false) ;
        
        
    submodules:

        sensor[numSensors]: Sensor {
            txRate = txRate;
            maxBackOff = maxBackOff;
            lengthRTS=lengthRTS;
            minData=minData;
            maxData=maxData;
            mediaExp=mediaExp;
            PER=PER;
            numSensors=numSensors;
            reverseExponential = reverseExponential;
            improvedProtocol = improvedProtocol ;
            improvedBackOffFactor = improvedBackOffFactor;
        }
        
        satelite: Satelite{
            txRate=txRate;
            maxBackOff = maxBackOff;
            numSensors=numSensors;
            lengthCTS=lengthCTS;
            PER=PER;
            numberOfCycles=numberOfCycles;
            improvedProtocol = improvedProtocol ;
            improvedBackOffFactor = improvedBackOffFactor;
            
        }
}
