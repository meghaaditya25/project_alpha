
#ifndef __EAR_P2P_H_
#define __EAR_P2P_H_

#include <omnetpp.h>


class EAR_P2P : public cSimpleModule
{
  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
	int Stitch_long_distance_EAR_P2P(int Source_node,int Destination_node,int Landmark_node);
	int Agent2_EAR_P2P(int Channel_capacity,int SNR,int link_lifetime);
    int Link_Oriented_Metrics(int Deadline ,int Packet_Size,int Mobility);
    int Agent3_EAR_P2P(int val1,int val2,int val3);
};

#endif
