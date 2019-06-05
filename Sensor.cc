/*
 * Sensor.cc
 *
 *  Created on: Jan 11, 2018
 *      Author: laboratorio
 */


#include "Sensor.h"

#include "mensaje_m.h"

namespace randomaccess {

Define_Module(Sensor);

Sensor::Sensor()
{
    backOff = nullptr;
}

Sensor::~Sensor()
{
    cancelAndDelete(backOff);
}

void Sensor::initialize()
{

    backOff = new cMessage("backoff");
    txRate = par("txRate");
    radioDelay = par("radioDelay");
    maxBackOff = par("maxBackOff");
    lengthRTS= par("lengthRTS");
    discard=par("discard");
    isSlotted=par("isSlotted");
    sent=false;
    numSensors=par("numSensors");
    mediaExp= par("mediaExp");
    minData=par("minData");
    maxData=par("maxData");
    PER=par("PER");
    distF=par("distF");
    reverseExponential=par("reverseExponential");

    //improved protocol

    improvedProtocol = par("improvedProtocol");
    improvedBackOffFactor = par("improvedBackOffFactor");

    //Statistics
    sensorTries=0;
    sensorChecks=0;
    firstRts=false;
    backOffTime = 0;

}

void Sensor::handleMessage(cMessage *msg)
{

    satelite = getModuleByPath("satelite");
    if ((msg->isSelfMessage())){//self message to indicate the end of the back off time->send RTS


        MENSAJE *pkt = new MENSAJE("RTS");
        pkt->setLengthData(lengthRTS);
        pkt->setSrcAddr(getId());
        pkt->setWhatIs(0); //is RTS
        pkt->setBackOffTime(backOffTime);
        simtime_t duration = pkt->getLengthData() / txRate;
        sendDirect(pkt,radioDelay,duration, satelite->gate("in"));
        sensorTries++;

        if (!firstRts){//save time of first sended rts
            firstRtsTime=simTime();
            firstRts=true;
        }

    }
    else if (!(msg->isSelfMessage())){//received CTS or Broadcast
        MENSAJE *pkt = check_and_cast<MENSAJE *>(msg);
        if (uniform(0,1)>PER){//recognize message with a probability of 1-PER

        if (pkt->getWhatIs()==1){//is CTS->send data

           EV<<"enviando data";
           MENSAJE *datos=new MENSAJE("datos");
           datos->setWhatIs(2);//is data
           ///set random length data between 1Kb-2kb
           //int n = rand() % 10;
           //int lp=minData+(maxData-minData)*n/10;
           int lp=(minData+maxData)/2;
           datos->setLengthData(lp);
           sent=true;//if the sensor send his data already
           sensorChecks++;
           simtime_t duration = datos->getLengthData() / txRate;
           sendDirect(datos,radioDelay,duration, satelite->gate("in"));


           sensorDelay=simTime()-firstRtsTime;//register delay since first RTS until data
           //recordScalar("SensorDelay",sensorDelay);
           firstRts=false;

           }
        else if (pkt->getWhatIs()==3){//arrive broadcast->send RTS after random time
            //if the sensor already sent data, decline his envy
            //if (!(discard & sent)){
            backOffTime = getNextTransmissionTime();
            if (improvedProtocol) {
                if (backOffTime <= improvedBackOffFactor*maxBackOff) {
                    scheduleAt(simTime()+backOffTime, backOff);

                }
            }
            else {
                scheduleAt(simTime()+backOffTime, backOff);
            }
            //  }
            EV<<"tiempo de espera: \n";
            EV<<backOffTime;
            }
        }
        delete msg;
        }
        //delete msg;
        //delete pkt;
    }

simtime_t Sensor::getNextTransmissionTime()
{
    //simtime_t ti=simTime();
    simtime_t iaTime;


    if (distF==0){
        iaTime=exponential(maxBackOff*mediaExp);
    }
    if (distF==1){
        iaTime=uniform(0, maxBackOff);
    }

    if (iaTime>maxBackOff){
        iaTime=maxBackOff;
        //iaTime=0;
    }

    if (reverseExponential){
        iaTime = maxBackOff - iaTime;
    }

    simtime_t t = iaTime;

    simtime_t slotTime= lengthRTS/txRate;
    if (!isSlotted){
        return t;
    }
    else{
        // align time of next transmission to a slot boundary
        return  slotTime * round(iaTime/slotTime);
    }
}

void Sensor::finish()
{
    //recordScalar("SuccessSensorTries%", 100*sensorChecks/sensorTries);

}


}; //namespace


