
#ifndef __MACARON_H_
#define __MACARON_H_

#include <omnetpp.h>


class MACARON : public cSimpleModule
{
  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
	int Stitch_long_distance_MACARON(int Source_node,int Destination_node,int Landmark_node);
	int Agent2_MACARON(int Channel_capacity,int SNR,int link_lifetime);
    int Link_Oriented_Metrics(int Routing ,int OLSR ,int Mobility);
    int Agent3_MACARON(int val1,int val2,int val3);
};

#endif
