
#ifndef _CAOD_H_
#define _CAOD_H_

#include <omnetpp.h>


class CAOD : public cSimpleModule
{
  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    int Node_Oriented_Metrics(int Node_energy,int ETX,int distance);
    int Channel_Oriented_Metrics (int Channel_capacity,int Channel_Bandwidth,int RSRQ);
    int Link_Oriented_Metrics(int Deadline ,int Packet_Size,int Mobility);
    int Outputfuzzy(int val1,int val2,int val3);
};

#endif
