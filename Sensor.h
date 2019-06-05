/*
 * Sensor.h
 *
 *  Created on: Jan 11, 2018
 *      Author: laboratorio
 */

#ifndef SENSOR_H_
#define SENSOR_H_





#include <omnetpp.h>

using namespace omnetpp;

namespace randomaccess {


class Sensor : public cSimpleModule
{
  private:
    // parameters

    double txRate;
    simtime_t maxBackOff;
    int lengthRTS;
    cModule *satelite;
    cMessage *backOff;
    //cMessage *endTx;
    bool isSlotted;
    bool discard;
    bool sent;
    int numSensors;
    int minData;
    int maxData;
    double mediaExp;
    double PER;
    int distF;
    bool reverseExponential;

    //improved protocol
    bool improvedProtocol;
    double improvedBackOffFactor;

    ///Statistics
    double sensorTries;
    double sensorChecks;
    bool firstRts;
    simtime_t sensorDelay;
    simtime_t firstRtsTime;
    simtime_t backOffTime;


  public:
    Sensor();
    virtual ~Sensor();
    //virtual simtime_t getRadioDelay() const;
    simtime_t radioDelay;
  protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    simtime_t getNextTransmissionTime();
    virtual void finish() override;

};

}; //namespace

#endif

