/*
 * Satelite.cc
 *
 *  Created on: Jan 11, 2018
 *      Author: laboratorio
 */



#include "Satelite.h"

#include "mensaje_m.h"
#include <numeric>


namespace randomaccess {

Define_Module(Satelite);

Satelite::Satelite()
{
    endRx = nullptr;
    endTimeout=nullptr;
    waitData =nullptr;
    dataCheck=nullptr;
}

Satelite::~Satelite()
{
    cancelAndDelete(endRx);
    cancelAndDelete(endTimeout);
    cancelAndDelete(waitData);
    cancelAndDelete(dataCheck);
}

void Satelite::initialize()
{

    endRx = new cMessage("end-reception");
    dataCheck= new cMessage("dataCheck");
    waitData= new cMessage("waitData");
    endTimeout = new cMessage("end-timeout");
    maxBackOff=par("maxBackOff");//par() is to import parameter from NED file
    txRate = par("txRate");
    numSensors = par("numSensors");
    lengthCTS=par("lengthCTS");
    lengthBcast= par("lengthBcast");
    PER=par("PER");
    numberOfCycles=par("numberOfCycles");
    maxRadioDelay=0.003;
    processingTime=0.003;

    channelBusy = false;
    ctsTries=2;
    j =0;

    //improved protocol
    improvedProtocol = par("improvedProtocol");
    improvedBackOffFactor = par("improvedBackOffFactor");

    ////statistics////
    channelOccupation=0;
    dataRcvd=0;
    recSen.setName("Recognized Sensors");
    recDisputa.setName("DisputasFallidas");
    recExito.setName("PrimerExito");
    cycles=0;
    successCycles=0;
    failedCycles=0;
    for(int i = 0; i < numSensors; i++){
        fairnessVec.push_back(0);

    }

    ///SATELLITE SEND BROADCAST TO SENSORS
        broadcast();

    cyclesUntil100=0; // recollected statistics at the time that all sensors sent data
    dataRcvdUntil100=0;
    channelOccupationUntil100=0;
    successCyclesUntil100=0;
    alreadyPass=false;

    ////////
    gate("in")->setDeliverOnReceptionStart(true);//execute actions on start of reception
}

void Satelite::handleMessage(cMessage *msg)
{

if (msg== dataCheck){
    broadcast();
    //delete msg;
}

else if (msg== waitData){
    if (j<ctsTries){
        MENSAJE *pkt = new MENSAJE("CTS");
        pkt->setWhatIs(1);//is CTS
        pkt->setLengthData(lengthCTS);
        simtime_t duration = pkt->getLengthData() / txRate;
        sendDirect(pkt,sensor->par("radioDelay") ,duration, sensor->gate("sensorIn"));
        simtime_t  tRx=2*(maxRadioDelay+processingTime);/////////////////////////////////////////////////////////////
        scheduleAt(simTime()+tRx,waitData);
        j++;
        }
    else{broadcast();
    }
}
else if (msg == endTimeout){//endTimeout is a self-message that indicate the end of the channel dispute

    //recSen.record(lista.size());//Record number of sensors registered in the last dispute

    //gate("in")->setDeliverOnReceptionStart(false);//execute actions on start of reception

    if (lista.size()==0){//channel dispute is over and no sensor could be registered
        failedCycles++;
        broadcast();//Start the channel dispute
        EV<<"Disputa canal";

    }else{//channel dispute is over and at least one sensor could be registered
        //recDisputa.record(failedCycles);
        failedCycles=0;
        int n = rand() % lista.size();//
        int nodo=lista[n]-2;// select a random sensor
        std::string str="sensor["+std::to_string(nodo)+"]";
        const char *cstr = str.c_str();
        sensor= getModuleByPath(cstr); // get the path to the module

        EV<<"ganó la disputa el sensor"<<nodo;
        successCycles++;
        fairnessVec[nodo]=1;
        //enviar CTS


        MENSAJE *pkt = new MENSAJE("CTS");
        pkt->setWhatIs(1);//is CTS
        pkt->setLengthData(lengthCTS);
        simtime_t duration = pkt->getLengthData() / txRate;
        sendDirect(pkt,sensor->par("radioDelay") ,duration, sensor->gate("sensorIn"));
        lista.clear();
        j++;
        ///Wait for data for tRx=2*(maxRadioDelay+processingTime),
        //if no data is received send another CTS (m tries), if there still no data received, start channel dispute

        if (PER>0){
        simtime_t  tRx=2*(maxRadioDelay+processingTime);/////////////////////////////////////////////////////////////
       // waitData= new cMessage("waitData");
        scheduleAt(simTime()+tRx,waitData);
        }
        }
}
else if (msg==endRx){ //self-message that indicate the end of the reception of RTS
            EV << "reception finished\n";
            if (lista.size() == 1){
            recExito.record(firstPck);// arrival time for the first RTS since broadcast
            }
            channelBusy = false;


}

else if (!(msg->isSelfMessage())) {//if isn't a sel-message, it can be Data or RTS
    MENSAJE *pkt = check_and_cast<MENSAJE *>(msg);
    if (uniform(0,1)>PER){//recognize message with probability of 1-PER
        if (pkt->getWhatIs()==0){//is RTS

            firstPck = pkt->getBackOffTime(); //arrival time for this RTS
            simtime_t endReceptionTime = simTime() + pkt->getDuration();

            if (!channelBusy) {//if the satellite isn't receiving another packet
                EV << "started receiving\n";
                recvStartTime = simTime();
                channelBusy = true;
                currentl=lista.size();
                lista.push_back(pkt->getSrcAddr());
                EV<<"añadido";
                EV<<lista.back()-2;


                scheduleAt(endReceptionTime, endRx); //signalizes the end of reception
            }
            else {//if the channel is busy there is a collision
                EV << "another frame arrived while receiving -- collision!\n";

                if (endReceptionTime >= endRx->getArrivalTime()) {
                    cancelEvent(endRx);
                    scheduleAt(endReceptionTime, endRx);

                    if (lista.size()>currentl){//delete from list the collided sensor
                       EV<<"eliminado";
                       EV<<lista.back()-2;
                       lista.pop_back();
                    }
                }

            }
            channelBusy = true;
            }



    else if (pkt->getWhatIs()==2){//is Data
        //send broadcast or end simulation if all sensors send their data
            j=0;
            cancelEvent(waitData);////////////////////////////////////////

            dataRcvd+=pkt->getLengthData();
            channelOccupation += pkt->getDuration();
            //broadcast();
                /*if (std::accumulate(fairnessVec.begin(),fairnessVec.end(), 0)==numSensors & !alreadyPass){

                    channelOccupationUntil100=channelOccupation;
                    cyclesUntil100=cycles;
                    dataRcvdUntil100=dataRcvd;
                    successCyclesUntil100=successCycles;
                    recordScalar("ChannelOccupation100fairness",channelOccupationUntil100);
                    recordScalar("Cycles100fairness",cyclesUntil100);
                    recordScalar("ReceivedData100fairness",dataRcvdUntil100);
                    recordScalar("SuccessCycles100fairness",successCyclesUntil100);
                    recordScalar("Throughput100fairness",dataRcvdUntil100/simTime());
                    recordScalar("Duration100fairness",simTime());
                    bool alreadyPass=true;
                    //endSimulation();
                    }*/
                //dataCheck= new cMessage("dataCheck");
                scheduleAt(simTime()+pkt->getDuration(), dataCheck);
               }
    }
    //delete pkt;
    delete msg;
}

}

void Satelite::finish()
{
    //simtime_t timeOut= maxBackOff+((lengthBcast/txRate)+2*maxRadioDelay);
    //int sum = std::accumulate(fairnessVec.begin(),fairnessVec.end(), 0);
    EV << "duration: " << simTime() << endl;
    recordScalar("duration", simTime());
    recordScalar("TiempoDatos", dataRcvd/txRate);
    recordScalar("TiempoDisputa",simTime()- dataRcvd/txRate);
    recordScalar("TiempoAsignacion",(simTime()- dataRcvd/txRate)/successCycles);
    recordScalar("Throughput",(dataRcvd/simTime())/txRate);
    recordScalar("ChannelOccupation", channelOccupation/simTime());
    recordScalar("PorcentajeExitos",float(successCycles)/float(cycles));
    recordScalar("SuccessCycles",successCycles);
    recordScalar("Cycles",cycles);
}




void Satelite::broadcast()
{
    EV<<"Comienza un nuevo ciclo de disputa\n";
    if (cycles == numberOfCycles){
        endSimulation();
    }
    MENSAJE *bcast=new MENSAJE("Start");
    bcast->setWhatIs(3);//is Broadcast
    bcast->setLengthData(lengthBcast);
    simtime_t duration = lengthBcast / txRate;
    for (int i = 0; i < numSensors; i++)
    {
    std::string str="sensor["+std::to_string(i)+"]";
    const char *cstr = str.c_str();
    EV<<*cstr;
    sensor= getModuleByPath(cstr);
    MENSAJE *copy = bcast->dup();
    sendDirect(copy,sensor->par("radioDelay"), duration, sensor->gate("sensorIn"));


     }
     delete bcast;
     //date timeout
     //endTimeout = new cMessage("end-timeout");
     simtime_t timeOut;
     if (improvedProtocol){
         timeOut= simTime()+improvedBackOffFactor*maxBackOff+((lengthBcast/txRate)+2*maxRadioDelay);
     }
     else {
         timeOut= simTime()+maxBackOff+((lengthBcast/txRate)+2*maxRadioDelay);
     }
     //simtime_t timeOut= simTime()+maxBackOff+((lengthBcast/txRate)+2*maxRadioDelay);//factor 2 is considering the arrive time of bcast and arrive time of RTS (its asume tBcast=tRTS)
     EV<<timeOut;
     scheduleAt(timeOut, endTimeout);
     //gate("in")->setDeliverOnReceptionStart(true);//execute actions on start of reception
     cycles++;
}


}; //namespace
