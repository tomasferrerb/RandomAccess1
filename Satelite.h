/*
 * Satelite.h
 *
 *  Created on: Jan 11, 2018
 *      Author: laboratorio
 */

#ifndef SATELITE_H_
#define SATELITE_H_

#include <omnetpp.h>
#include <vector>

using namespace omnetpp;

namespace randomaccess {

class Satelite : public cSimpleModule
{
  private:

    cMessage *endTimeout;
    cMessage *endRx;
    cMessage *waitData;
    cMessage *dataCheck;
    simtime_t recvStartTime;

    simtime_t maxBackOff;
    cModule *sensor;
    double txRate;
    int lengthBcast;
    int numSensors;
    int lengthCTS;
    simtime_t maxRadioDelay;
    simtime_t processingTime;
    std::vector<double> lista;
    int ctsTries;
    bool channelBusy;
    unsigned int currentl;
    double PER;
    int j;

    //improved protocol
    bool improvedProtocol;
    double improvedBackOffFactor;


////statistics////
    simtime_t channelOccupation;
    simtime_t currentTime;
    simtime_t firstPck;
    double dataRcvd;
    cOutVector recSen;
    cOutVector recDisputa;
    cOutVector recExito;
    int successCycles;
    int cycles;
    std::vector<int> fairnessVec;
    int cyclesUntil100;
    double dataRcvdUntil100;
    int successCyclesUntil100;
    simtime_t channelOccupationUntil100;
    bool alreadyPass;
    int failedCycles;
    int numberOfCycles;
    //////

  public:
    Satelite();
    virtual ~Satelite();

  protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void finish() override;
    void broadcast();
};

}; //namespace



#endif /* SATELITE_H_ */
