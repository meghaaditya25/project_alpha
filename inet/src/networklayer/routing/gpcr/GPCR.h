/*
 * GPCR.h
 *
 *  Created on: Aug 26, 2015
 *      Author: cloudsim
 */

#ifndef GPCR_H_
#define GPCR_H_



#include "INETDefs.h"
#include "Coord.h"
#include "ILifecycle.h"
#include "IMobility.h"
#include "INetfilter.h"
#include "IRoutingTable.h"
#include "NodeStatus.h"
#include "CommunicationUnit.h"
#include "PositionTable.h"
#include "UDPPacket.h"
#include "GPCR_m.h"

/**
 * This class implements the Greedy Perimeter Stateless Routing for Wireless Networks.
 * The implementation supports both GG and RNG planarization algorithms.
 *
 * For more information on the routing algorithm, see the GPSR paper
 * http://www.eecs.harvard.edu/~htk/publication/2000-mobi-karp-kung.pdf
 */
// TODO: optimize internal data structures for performance to use less lookups and be more prepared for routing a packet
// KLUDGE: implement position registry protocol instead of using a global variable
// KLUDGE: the GPSR packet is now used to wrap the content of network datagrams
// KLUDGE: we should rather add these fields as header extensions
class INET_API GPCR : public cSimpleModule, public ILifecycle, public INotifiable, public INetfilter::IHook
{
    private:
        // GPCR parameters
        GPCRPlanarizationMode planarizationMode;
        const char * interfaces;
        simtime_t dataInterval;
        simtime_t maxJitter;
        simtime_t neighborValidityInterval;

        // context
        cModule * host;
        NodeStatus * nodeStatus;
        IMobility * mobility;
        IInterfaceTable * interfaceTable;
        IRoutingTable * routingTable; // TODO: delete when necessary functions are moved to interface table
        INetfilter * networkProtocol;
        CommunicationUnit *nb;
        static PositionTable globalPositionTable; // KLUDGE: implement position registry protocol

        // internal
        cMessage * dataTimer;
        cMessage * purgeNeighborsTimer;
        PositionTable neighborPositionTable;

    public:
        GPCR();
        virtual ~GPCR();

    protected:
        // module interface
        virtual int numInitStages() const { return 6; }
        void initialize(int stage);
        void handleMessage(cMessage * message);

    private:
        // handling messages
        void processSelfMessage(cMessage * message);
        void processMessage(cMessage * message);

        // handling data timers
        void scheduleBeaconTimer();
        void processBeaconTimer();

        // handling purge neighbors timers
        void schedulePurgeNeighborsTimer();
        void processPurgeNeighborsTimer();

        // handling UDP packets
        void sendUDPPacket(UDPPacket * packet, double delay);
        void processUDPPacket(UDPPacket * packet);

        // handling beacons
        GPCRBeacon * createBeacon();
        void sendBeacon(GPCRBeacon * data, double delay);
        void processBeacon(GPCRBeacon * data);

        // handling packets
        GPCRPacket * createPacket(IPvXAddress destination, cPacket * content);
        int computePacketBitLength(GPCRPacket * packet);

        // configuration
        bool isNodeUp() const;
        void configureInterfaces();

        // position
        Coord intersectSections(Coord & begin1, Coord & end1, Coord & begin2, Coord & end2);
        Coord getDestinationPosition(const IPvXAddress & address) const;
        Coord getNeighborPosition(const IPvXAddress & address) const;

        // angle
        double getVectorAngle(Coord vector);
        double getDestinationAngle(const IPvXAddress & address);
        double getNeighborAngle(const IPvXAddress & address);

        // address
        std::string getHostName() const;
        IPvXAddress getSelfAddress() const;
        IPvXAddress getSenderNeighborAddress(IPv4Datagram * datagram) const;

        // neighbor
        simtime_t getNextNeighborExpiration();
        void purgeNeighbors();
        std::vector<IPvXAddress> getPlanarNeighbors();
        IPvXAddress getNextPlanarNeighborCounterClockwise(const IPvXAddress & startNeighborAddress, double startNeighborAngle);

        // next hop
        IPvXAddress findNextHop(IPv4Datagram * datagram, const IPvXAddress & destination);
        IPvXAddress findGreedyRoutingNextHop(IPv4Datagram * datagram, const IPvXAddress & destination);
        IPvXAddress findPerimeterRoutingNextHop(IPv4Datagram * datagram, const IPvXAddress & destination);

        // routing
        Result routeDatagram(IPv4Datagram * datagram, const InterfaceEntry *& outputInterfaceEntry, IPv4Address & nextHop);

        // netfilter
        virtual Result datagramPreRoutingHook(IPv4Datagram * datagram, const InterfaceEntry * inputInterfaceEntry, const InterfaceEntry *& outputInterfaceEntry, IPv4Address & nextHop);
        virtual Result datagramForwardHook(IPv4Datagram * datagram, const InterfaceEntry * inputInterfaceEntry, const InterfaceEntry *& outputInterfaceEntry, IPv4Address & nextHop) { return ACCEPT; }
        virtual Result datagramPostRoutingHook(IPv4Datagram * datagram, const InterfaceEntry * inputInterfaceEntry, const InterfaceEntry *& outputInterfaceEntry, IPv4Address & nextHop) { return ACCEPT; }
        virtual Result datagramLocalInHook(IPv4Datagram * datagram, const InterfaceEntry * inputInterfaceEntry);
        virtual Result datagramLocalOutHook(IPv4Datagram * datagram, const InterfaceEntry *& outputInterfaceEntry, IPv4Address & nextHop);

        // lifecycle
        virtual bool handleOperationStage(LifecycleOperation * operation, int stage, IDoneCallback * doneCallback);

        // notification
        virtual void receiveChangeNotification(int signalID, const cObject *obj);
};

#endif /* GPCR_H_ */
