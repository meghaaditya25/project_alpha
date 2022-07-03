/*
 * GPCR.cc
 *
 *  Created on: Aug 26, 2015
 *      Author: cloudsim
 */



#include "GPCR.h"
#include "InterfaceTableAccess.h"
#include "IPProtocolId_m.h"
#include "IPSocket.h"
#include "IPv4ControlInfo.h"
#include "NodeOperations.h"

Define_Module(GPCR);

#define GPCR_EV EV << "GPCR at " << getHostName() << " "

// TODO: use some header?
static double const NaN = 0.0 / 0.0;

static inline double determinant(double a1, double a2, double b1, double b2) {
    return a1 * b2 - a2 * b1;
}

static inline bool isNaN(double d) { return d != d;}

// KLUDGE: implement position registry protocol
PositionTable GPCR::globalPositionTable;

GPCR::GPCR()
{
    host = NULL;
    nodeStatus = NULL;
    mobility = NULL;
    interfaceTable = NULL;
    routingTable = NULL;
    networkProtocol = NULL;
    dataTimer = NULL;
    purgeNeighborsTimer = NULL;
}

GPCR::~GPCR()
{
    cancelAndDelete(dataTimer);
    cancelAndDelete(purgeNeighborsTimer);
    nb = CommunicationUnitAccess().getIfExists(this);
    if (nb)
        nb->unsubscribe(this, NF_LINK_BREAK);
}

//
// module interface
//

void GPCR::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == 0)
    {
        // GPSR parameters
        planarizationMode = (GPCRPlanarizationMode)(int)par("planarizationMode");
        interfaces = par("interfaces");
        dataInterval = par("dataInterval");
        maxJitter = par("maxJitter");
        neighborValidityInterval = par("neighborValidityInterval");
        // context
        host = getContainingNode(this);
        nodeStatus = dynamic_cast<NodeStatus *>(host->getSubmodule("status"));
        interfaceTable = InterfaceTableAccess().get(this);
        mobility = check_and_cast<IMobility *>(host->getSubmodule("mobility"));
        routingTable = check_and_cast<IRoutingTable *>(getModuleByPath(par("routingTableModule")));
        networkProtocol = check_and_cast<INetfilter *>(getModuleByPath(par("networkProtocolModule")));
        // internal
        dataTimer = new cMessage("BeaconTimer");
        purgeNeighborsTimer = new cMessage("PurgeNeighborsTimer");
        scheduleBeaconTimer();
        schedulePurgeNeighborsTimer();
    }
    else if (stage == 5)
    {
        IPSocket socket(gate("ipOut"));
        socket.registerProtocol(IP_PROT_MANET);

        globalPositionTable.clear();
        nb = CommunicationUnitAccess().get();
        nb->subscribe(this, NF_LINK_BREAK);
        networkProtocol->registerHook(0, this);
        if (isNodeUp())
            configureInterfaces();
    }
}

void GPCR::handleMessage(cMessage * message)
{
    if (message->isSelfMessage())
        processSelfMessage(message);
    else
        processMessage(message);
}

//
// handling messages
//

void GPCR::processSelfMessage(cMessage * message)
{
    if (message == dataTimer)
        processBeaconTimer();
    else if (message == purgeNeighborsTimer)
        processPurgeNeighborsTimer();
    else
        throw cRuntimeError("Unknown self message");
}

void GPCR::processMessage(cMessage * message)
{
    if (dynamic_cast<UDPPacket *>(message))
        processUDPPacket((UDPPacket *)message);
    else
        throw cRuntimeError("Unknown message");
}

//
// data timers
//

void GPCR::scheduleBeaconTimer()
{
    GPCR_EV << "Scheduling data timer" << endl;
    scheduleAt(simTime() + dataInterval, dataTimer);
}

void GPCR::processBeaconTimer()
{
    GPCR_EV << "Processing data timer" << endl;
    IPvXAddress selfAddress = getSelfAddress();
    if (!selfAddress.isUnspecified()) {
        sendBeacon(createBeacon(), uniform(0, maxJitter).dbl());
        // KLUDGE: implement position registry protocol
        globalPositionTable.setPosition(selfAddress, mobility->getCurrentPosition());
    }
    scheduleBeaconTimer();
    schedulePurgeNeighborsTimer();
}

//
// handling purge neighbors timers
//

void GPCR::schedulePurgeNeighborsTimer()
{
    GPCR_EV << "Scheduling purge neighbors timer" << endl;
    simtime_t nextExpiration = getNextNeighborExpiration();
    if (nextExpiration == SimTime::getMaxTime()) {
        if (purgeNeighborsTimer->isScheduled())
            cancelEvent(purgeNeighborsTimer);
    }
    else {
        if (!purgeNeighborsTimer->isScheduled())
            scheduleAt(nextExpiration, purgeNeighborsTimer);
        else {
            if (purgeNeighborsTimer->getArrivalTime() != nextExpiration) {
                cancelEvent(purgeNeighborsTimer);
                scheduleAt(nextExpiration, purgeNeighborsTimer);
            }
        }
    }
}

void GPCR::processPurgeNeighborsTimer()
{
    GPCR_EV << "Processing purge neighbors timer" << endl;
    purgeNeighbors();
    schedulePurgeNeighborsTimer();
}

//
// handling UDP packets
//

void GPCR::sendUDPPacket(UDPPacket * packet, double delay)
{
    if (delay == 0)
        send(packet, "ipOut");
    else
        sendDelayed(packet, delay, "ipOut");
}

void GPCR::processUDPPacket(UDPPacket * packet)
{
    cPacket * encapsulatedPacket = packet->decapsulate();
    if (dynamic_cast<GPCRBeacon *>(encapsulatedPacket))
        processBeacon((GPCRBeacon *)encapsulatedPacket);
    else
        throw cRuntimeError("Unknown UDP packet");
    delete packet;
}

//
// handling beacons
//

GPCRBeacon * GPCR::createBeacon()
{
    GPCRBeacon * data = new GPCRBeacon();
    data->setAddress(getSelfAddress());
    data->setPosition(mobility->getCurrentPosition());
    return data;
}

void GPCR::sendBeacon(GPCRBeacon * data, double delay)
{
    GPCR_EV << "Sending data: address = " << data->getAddress() << ", position = " << data->getPosition() << endl;
    IPv4ControlInfo * networkProtocolControlInfo = new IPv4ControlInfo();
    networkProtocolControlInfo->setProtocol(IP_PROT_MANET);
    networkProtocolControlInfo->setTimeToLive(255);
    networkProtocolControlInfo->setDestAddr(IPv4Address::LL_MANET_ROUTERS);
    networkProtocolControlInfo->setSrcAddr(getSelfAddress().get4());
    UDPPacket * udpPacket = new UDPPacket(data->getName());
    udpPacket->encapsulate(data);
    udpPacket->setSourcePort(GPSR_UDP_PORT);
    udpPacket->setDestinationPort(GPSR_UDP_PORT);
    udpPacket->setControlInfo(networkProtocolControlInfo);
    sendUDPPacket(udpPacket, delay);
}

void GPCR::processBeacon(GPCRBeacon * data)
{
    GPCR_EV << "Processing data: address = " << data->getAddress() << ", position = " << data->getPosition() << endl;
    neighborPositionTable.setPosition(data->getAddress(), data->getPosition());
    delete data;
}

//
// handling packets
//

GPCRPacket * GPCR::createPacket(IPvXAddress destination, cPacket * content)
{
    GPCRPacket * gpcrPacket = new GPCRPacket(content->getName());
    gpcrPacket->setRoutingMode(GPCR_GREEDY_ROUTING);
    // KLUDGE: implement position registry protocol
    gpcrPacket->setDestinationPosition(getDestinationPosition(destination));
    gpcrPacket->setBitLength(computePacketBitLength(gpcrPacket));
    gpcrPacket->encapsulate(content);
    return gpcrPacket;
}

int GPCR::computePacketBitLength(GPCRPacket * packet)
{
    // routingMode
    int routingMode = 1;
    // destinationPosition, perimeterRoutingStartPosition, perimeterRoutingForwardPosition
    int positions = 8 * 3 * 2 * 4;
    // currentFaceFirstSenderAddress, currentFaceFirstReceiverAddress, senderAddress
    int addresses = 8 * 3 * 4;
    // TODO: address size
    return routingMode + positions + addresses;
}

//
// configuration
//

bool GPCR::isNodeUp() const
{
    return !nodeStatus || nodeStatus->getState() == NodeStatus::UP;
}

void GPCR::configureInterfaces()
{
    // join multicast groups
    cPatternMatcher interfaceMatcher(interfaces, false, true, false);
    for (int i = 0; i < interfaceTable->getNumInterfaces(); i++) {
        InterfaceEntry * interfaceEntry = interfaceTable->getInterface(i);
        if (interfaceEntry->isMulticast() && interfaceMatcher.matches(interfaceEntry->getName()))
            interfaceEntry->joinMulticastGroup(IPv4Address::LL_MANET_ROUTERS);
    }
}

//
// position
//

Coord GPCR::intersectSections(Coord & begin1, Coord & end1, Coord & begin2, Coord & end2)
{
    double x1 = begin1.x;
    double y1 = begin1.y;
    double x2 = end1.x;
    double y2 = end1.y;
    double x3 = begin2.x;
    double y3 = begin2.y;
    double x4 = end2.x;
    double y4 = end2.y;
    double a = determinant(x1, y1, x2, y2);
    double b = determinant(x3, y3, x4, y4);
    double c = determinant(x1 - x2, y1 - y2, x3 - x4, y3 - y4);
    double x = determinant(a, x1 - x2, b, x3 - x4) / c;
    double y = determinant(a, y1 - y2, b, y3 - y4) / c;
    if (x1 < x && x < x2 && x3 < x && x < x4 && y1 < y && y < y2 && y3 < y && y < y4)
        return Coord(x, y, 0);
    else
        return Coord(NaN, NaN, NaN);
}

Coord GPCR::getDestinationPosition(const IPvXAddress & address) const
{
    // KLUDGE: implement position registry protocol
    return globalPositionTable.getPosition(address);
}

Coord GPCR::getNeighborPosition(const IPvXAddress & address) const
{
    return neighborPositionTable.getPosition(address);
}

//
// angle
//

double GPCR::getVectorAngle(Coord vector)
{
    double angle = atan2(-vector.y, vector.x);
    if (angle < 0)
        angle += 2 * PI;
    return angle;
}

double GPCR::getDestinationAngle(const IPvXAddress & address)
{
    return getVectorAngle(getDestinationPosition(address) - mobility->getCurrentPosition());
}

double GPCR::getNeighborAngle(const IPvXAddress & address)
{
    return getVectorAngle(getNeighborPosition(address) - mobility->getCurrentPosition());
}

//
// address
//

std::string GPCR::getHostName() const
{
    return host->getFullName();
}

IPvXAddress GPCR::getSelfAddress() const
{
    return routingTable->getRouterId();
}

IPvXAddress GPCR::getSenderNeighborAddress(IPv4Datagram * datagram) const
{
    GPCRPacket * packet = check_and_cast<GPCRPacket *>(dynamic_cast<cPacket *>(datagram)->getEncapsulatedPacket());
    return packet->getSenderAddress().get4();
}

//
// neighbor
//

simtime_t GPCR::getNextNeighborExpiration()
{
    simtime_t oldestPosition = neighborPositionTable.getOldestPosition();
    if (oldestPosition == SimTime::getMaxTime())
        return oldestPosition;
    else
        return oldestPosition + neighborValidityInterval;
}

void GPCR::purgeNeighbors()
{
    neighborPositionTable.removeOldPositions(simTime() - neighborValidityInterval);
}

std::vector<IPvXAddress> GPCR::getPlanarNeighbors()
{
    std::vector<IPvXAddress> planarNeighbors;
    std::vector<IPvXAddress> neighborAddresses = neighborPositionTable.getAddresses();
    Coord selfPosition = mobility->getCurrentPosition();
    for (std::vector<IPvXAddress>::iterator it = neighborAddresses.begin(); it != neighborAddresses.end(); it++) {
        const IPvXAddress & neighborAddress = *it;
        Coord neighborPosition = neighborPositionTable.getPosition(neighborAddress);
        if (planarizationMode == GPCR_RNG_PLANARIZATION) {
            double neighborDistance = (neighborPosition - selfPosition).length();
            for (std::vector<IPvXAddress>::iterator jt = neighborAddresses.begin(); jt != neighborAddresses.end(); jt++) {
                const IPvXAddress & witnessAddress = *jt;
                Coord witnessPosition = neighborPositionTable.getPosition(witnessAddress);
                double witnessDistance = (witnessPosition - selfPosition).length();;
                double neighborWitnessDistance = (witnessPosition - neighborPosition).length();
                if (*it == *jt)
                    continue;
                else if (neighborDistance > std::max(witnessDistance, neighborWitnessDistance))
                    goto eliminate;
            }
        }
        else if (planarizationMode == GPCR_GG_PLANARIZATION) {
            Coord middlePosition = (selfPosition + neighborPosition) / 2;
            double neighborDistance = (neighborPosition - middlePosition).length();
            for (std::vector<IPvXAddress>::iterator jt = neighborAddresses.begin(); jt != neighborAddresses.end(); jt++) {
                const IPvXAddress & witnessAddress = *jt;
                Coord witnessPosition = neighborPositionTable.getPosition(witnessAddress);
                double witnessDistance = (witnessPosition - middlePosition).length();;
                if (*it == *jt)
                    continue;
                else if (witnessDistance < neighborDistance)
                    goto eliminate;
            }
        }
        else
            throw cRuntimeError("Unknown planarization mode");
        planarNeighbors.push_back(*it);
        eliminate: ;
    }
    return planarNeighbors;
}

IPvXAddress GPCR::getNextPlanarNeighborCounterClockwise(const IPvXAddress& startNeighborAddress, double startNeighborAngle)
{
    GPCR_EV << "Finding next planar neighbor (counter clockwise): startAddress = " << startNeighborAddress << ", startAngle = " << startNeighborAngle << endl;
    IPvXAddress bestNeighborAddress = startNeighborAddress;
    double bestNeighborAngleDifference = 2 * PI;
    std::vector<IPvXAddress> neighborAddresses = getPlanarNeighbors();
    for (std::vector<IPvXAddress>::iterator it = neighborAddresses.begin(); it != neighborAddresses.end(); it++) {
        const IPvXAddress & neighborAddress = *it;
        double neighborAngle = getNeighborAngle(neighborAddress);
        double neighborAngleDifference = neighborAngle - startNeighborAngle;
        if (neighborAngleDifference < 0)
            neighborAngleDifference += 2 * PI;
        GPCR_EV << "Trying next planar neighbor (counter clockwise): address = " << neighborAddress << ", angle = " << neighborAngle << endl;
        if (neighborAngleDifference != 0 && neighborAngleDifference < bestNeighborAngleDifference) {
            bestNeighborAngleDifference = neighborAngleDifference;
            bestNeighborAddress = neighborAddress;
        }
    }
    return bestNeighborAddress;
}

//
// next hop
//

IPvXAddress GPCR::findNextHop(IPv4Datagram * datagram, const IPvXAddress & destination)
{
    GPCRPacket * packet = check_and_cast<GPCRPacket *>(dynamic_cast<cPacket *>(datagram)->getEncapsulatedPacket());
    if (packet->getRoutingMode() == GPCR_GREEDY_ROUTING)
        return findGreedyRoutingNextHop(datagram, destination);
    else if (packet->getRoutingMode() == GPCR_PERIMETER_ROUTING)
        return findPerimeterRoutingNextHop(datagram, destination);
    else
        throw cRuntimeError("Unknown routing mode");
}

IPvXAddress GPCR::findGreedyRoutingNextHop(IPv4Datagram * datagram, const IPvXAddress & destination)
{
    GPCR_EV << "Finding next hop using greedy routing: destination = " << destination << endl;
    GPCRPacket * packet = check_and_cast<GPCRPacket *>(dynamic_cast<cPacket *>(datagram)->getEncapsulatedPacket());
    IPvXAddress selfAddress = getSelfAddress();
    Coord selfPosition = mobility->getCurrentPosition();
    Coord destinationPosition = packet->getDestinationPosition();
    double bestDistance = (destinationPosition - selfPosition).length();
    IPvXAddress bestNeighbor;
    std::vector<IPvXAddress> neighborAddresses = neighborPositionTable.getAddresses();
    for (std::vector<IPvXAddress>::iterator it = neighborAddresses.begin(); it != neighborAddresses.end(); it++) {
        const IPvXAddress & neighborAddress = *it;
        Coord neighborPosition = neighborPositionTable.getPosition(neighborAddress);
        double neighborDistance = (destinationPosition - neighborPosition).length();
        if (neighborDistance < bestDistance) {
            bestDistance = neighborDistance;
            bestNeighbor = neighborAddress.get4();
        }
    }
    if (bestNeighbor.isUnspecified()) {
        GPCR_EV << "Switching to perimeter routing: destination = " << destination << endl;
        packet->setRoutingMode(GPCR_PERIMETER_ROUTING);
        packet->setPerimeterRoutingStartPosition(selfPosition);
        packet->setCurrentFaceFirstSenderAddress(selfAddress);
        packet->setCurrentFaceFirstReceiverAddress(IPvXAddress());
        return findPerimeterRoutingNextHop(datagram, destination);
    }
    else
        return bestNeighbor;
}

IPvXAddress GPCR::findPerimeterRoutingNextHop(IPv4Datagram * datagram, const IPvXAddress & destination)
{
    GPCR_EV << "Finding next hop using perimeter routing: destination = " << destination << endl;
    GPCRPacket * packet = check_and_cast<GPCRPacket *>(dynamic_cast<cPacket *>(datagram)->getEncapsulatedPacket());
    IPvXAddress selfAddress = getSelfAddress();
    Coord selfPosition = mobility->getCurrentPosition();
    Coord perimeterRoutingStartPosition = packet->getPerimeterRoutingStartPosition();
    Coord destinationPosition = packet->getDestinationPosition();
    double selfDistance = (destinationPosition - selfPosition).length();
    double perimeterRoutingStartDistance = (destinationPosition - perimeterRoutingStartPosition).length();
    if (selfDistance < perimeterRoutingStartDistance) {
        GPCR_EV << "Switching to greedy routing: destination = " << destination << endl;
        packet->setRoutingMode(GPCR_GREEDY_ROUTING);
        packet->setPerimeterRoutingStartPosition(Coord());
        packet->setPerimeterRoutingForwardPosition(Coord());
        return findGreedyRoutingNextHop(datagram, destination);
    }
    else {
        IPvXAddress & firstSenderAddress = packet->getCurrentFaceFirstSenderAddress();
        IPvXAddress & firstReceiverAddress = packet->getCurrentFaceFirstReceiverAddress();
        IPvXAddress nextNeighborAddress = getSenderNeighborAddress(datagram);
        bool hasIntersection;
        do {
            if (nextNeighborAddress.isUnspecified())
                nextNeighborAddress = getNextPlanarNeighborCounterClockwise(nextNeighborAddress, getDestinationAngle(destination));
            else
                nextNeighborAddress = getNextPlanarNeighborCounterClockwise(nextNeighborAddress, getNeighborAngle(nextNeighborAddress));
            if (nextNeighborAddress.isUnspecified())
                break;
            GPCR_EV << "Intersecting towards next hop: nextNeighbor = " << nextNeighborAddress << ", firstSender = " << firstSenderAddress << ", firstReceiver = " << firstReceiverAddress << ", destination = " << destination << endl;
            Coord nextNeighborPosition = getNeighborPosition(nextNeighborAddress);
            Coord intersection = intersectSections(perimeterRoutingStartPosition, destinationPosition, selfPosition, nextNeighborPosition);
            hasIntersection = !isNaN(intersection.x);
            if (hasIntersection) {
                GPCR_EV << "Edge to next hop intersects: intersection = " << intersection << ", nextNeighbor = " << nextNeighborAddress << ", firstSender = " << firstSenderAddress << ", firstReceiver = " << firstReceiverAddress << ", destination = " << destination << endl;
                packet->setCurrentFaceFirstSenderAddress(selfAddress);
                packet->setCurrentFaceFirstReceiverAddress(IPvXAddress());
            }
        }
        while (hasIntersection);
        if (firstSenderAddress == selfAddress && firstReceiverAddress == nextNeighborAddress) {
            GPCR_EV << "End of perimeter reached: firstSender = " << firstSenderAddress << ", firstReceiver = " << firstReceiverAddress << ", destination = " << destination << endl;
            return IPvXAddress();
        }
        else {
            if (packet->getCurrentFaceFirstReceiverAddress().isUnspecified())
                packet->setCurrentFaceFirstReceiverAddress(nextNeighborAddress);
            return nextNeighborAddress;
        }
    }
}

//
// routing
//

INetfilter::IHook::Result GPCR::routeDatagram(IPv4Datagram * datagram, const InterfaceEntry *& outputInterfaceEntry, IPv4Address & nextHop)
{
    const IPvXAddress source = datagram->getSrcAddress();
    const IPvXAddress destination = datagram->getDestAddress();
    GPCR_EV << "Finding next hop: source = " << source << ", destination = " << destination << endl;
    nextHop = findNextHop(datagram, destination).get4();
    if (nextHop.isUnspecified()) {
        GPCR_EV << "No next hop found, dropping packet: source = " << source << ", destination = " << destination << endl;
        return DROP;
    }
    else {
        GPCR_EV << "Next hop found: source = " << source << ", destination = " << destination << ", nextHop: " << nextHop << endl;
        GPCRPacket * packet = check_and_cast<GPCRPacket *>(dynamic_cast<cPacket *>(datagram)->getEncapsulatedPacket());
        packet->setSenderAddress(getSelfAddress());
        // KLUDGE: find output interface
        outputInterfaceEntry = interfaceTable->getInterface(1);
        return ACCEPT;
    }
}

//
// netfilter
//

INetfilter::IHook::Result GPCR::datagramPreRoutingHook(IPv4Datagram * datagram, const InterfaceEntry * inputInterfaceEntry, const InterfaceEntry *& outputInterfaceEntry, IPv4Address & nextHop)
{
    const IPv4Address & destination = datagram->getDestAddress();
    if (destination.isMulticast() || destination.isLimitedBroadcastAddress() || routingTable->isLocalAddress(destination))
        return ACCEPT;
    else
        return routeDatagram(datagram, outputInterfaceEntry, nextHop);
}

INetfilter::IHook::Result GPCR::datagramLocalInHook(IPv4Datagram * datagram, const InterfaceEntry * inputInterfaceEntry)
{
    cPacket * networkPacket = dynamic_cast<cPacket *>(datagram);
    GPCRPacket * gpcrPacket = dynamic_cast<GPCRPacket *>(networkPacket->getEncapsulatedPacket());
    if (gpcrPacket) {
        networkPacket->decapsulate();
        networkPacket->encapsulate(gpcrPacket->decapsulate());
        delete gpcrPacket;
    }
    return ACCEPT;
}

INetfilter::IHook::Result GPCR::datagramLocalOutHook(IPv4Datagram * datagram, const InterfaceEntry *& outputInterfaceEntry, IPv4Address & nextHop)
{
    const IPv4Address & destination = datagram->getDestAddress();
    if (destination.isMulticast() || destination.isLimitedBroadcastAddress() || routingTable->isLocalAddress(destination))
        return ACCEPT;
    else {
        cPacket * networkPacket = dynamic_cast<cPacket *>(datagram);
        GPCRPacket * gpcrPacket = createPacket(datagram->getDestAddress(), networkPacket->decapsulate());
        networkPacket->encapsulate(gpcrPacket);
        return routeDatagram(datagram, outputInterfaceEntry, nextHop);
    }
}


//
// lifecycle
//

bool GPCR::handleOperationStage(LifecycleOperation * operation, int stage, IDoneCallback * doneCallback)
{
    Enter_Method_Silent();
    if (dynamic_cast<NodeStartOperation *>(operation))  {
        if (stage == NodeStartOperation::STAGE_APPLICATION_LAYER)
            configureInterfaces();
    }
    else if (dynamic_cast<NodeShutdownOperation *>(operation)) {
        if (stage == NodeShutdownOperation::STAGE_APPLICATION_LAYER)
            // TODO: send a data to remove ourself from peers neighbor position table
            neighborPositionTable.clear();
    }
    else if (dynamic_cast<NodeCrashOperation *>(operation)) {
        if (stage == NodeCrashOperation::STAGE_CRASH)
            neighborPositionTable.clear();
    }
    else throw cRuntimeError("Unsupported lifecycle operation '%s'", operation->getClassName());
    return true;
}

//
// notification
//

void GPCR::receiveChangeNotification(int signalID, const cObject *obj)
{
    Enter_Method("receiveChangeNotification");
    if (signalID == NF_LINK_BREAK) {
        GPCR_EV << "Received link break" << endl;
        // TODO: shall we remove the neighbor?
    }
}

