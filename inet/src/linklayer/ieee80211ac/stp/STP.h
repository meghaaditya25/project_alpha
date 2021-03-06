//
// Copyright (C) 2013 OpenSim Ltd.
// Copyright (C) ANSA Team
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//
// Authors: ANSA Team, Benjamin Martin Seregi
//

#ifndef INET_SPANNINGTREE_H_
#define INET_SPANNINGTREE_H_

#include "INETDefs.h"
#include "MACAddress.h"
#include "ieee80211acBPDU_m.h"
#include "InterfaceTable.h"
#include "ieee80211acInterfaceData.h"
#include "NodeOperations.h"
#include "NodeStatus.h"
#include "STPBase.h"

/**
 * Implements the Spanning Tree Protocol. See the NED file for details.
 */
class INET_API STP : public STPBase
{
    public:
        typedef ieee80211acInterfaceData::PortInfo PortInfo;
        enum BDPUType {CONFIG_BDPU = 0, TCN_BPDU = 1};
    protected:
        static const double tickInterval; // interval between two ticks
        bool isRoot;
        unsigned int rootPort;
        std::vector<unsigned int> desPorts; // set of designated ports

        // Discovered values
        unsigned int rootPathCost;
        unsigned int rootPriority;
        MACAddress rootAddress;

        simtime_t currentMaxAge;
        simtime_t currentFwdDelay;
        simtime_t currentHelloTime;
        simtime_t helloTime;

        // Parameter change detection
        unsigned int currentBridgePriority;
        // Topology change commencing
        bool topologyChangeNotification;
        bool topologyChangeRecvd;

        PortInfo defaultPort;
        cMessage * tick;

    public:
        STP();
        virtual ~STP();

        /*
         * Bridge Protocol Data Unit handling
         */
        void handleBPDU(BPDU * bpdu);
        virtual void initInterfacedata(unsigned int portNum);

        /**
         * Topology change handling
         */
        void handleTCN(BPDU * tcn);
        virtual void handleMessage(cMessage * msg);
        virtual void initialize(int stage);
        virtual int numInitStages() const { return 2; }

        /*
         * Send BPDU with specified parameters (portNum, TCA flag, etc.)
         */
        void generateBPDU(int portNum, const MACAddress& address = MACAddress::STP_MULTICAST_ADDRESS, bool tcFlag = false, bool tcaFlag = false);

        /*
         * Send hello BDPUs on all ports (only for root switches)
         * Invokes generateBPDU(i) where i goes through all ports
         */
        void generateHelloBDPUs();

        /*
         * Generate and send Topology Change Notification
         */
        void generateTCN();

        /*
         * Comparison of all IDs in ieee80211acInterfaceData::PortInfo structure
         * Invokes: superiorID(), superiorPort()
         */
        int comparePorts(ieee80211acInterfaceData * portA, ieee80211acInterfaceData * portB);
        int compareBridgeIDs(unsigned int, MACAddress, unsigned int, MACAddress);
        int comparePortIDs(unsigned int, unsigned int, unsigned int, unsigned int);

        /*
         * Check of the received BPDU is superior to port information from InterfaceTable
         */
        bool isSuperiorBPDU(int portNum, BPDU * bpdu);
        void setSuperiorBPDU(int portNum, BPDU * bpdu);

        void handleTick();

        /*
         * Check timers to make state changes
         */
        void checkTimers();
        void checkParametersChange();

        /*
         * Set the default port information for the InterfaceTable
         */
        void initPortTable();
        void selectRootPort();
        void selectDesignatedPorts();

        /*
         * Set all ports to designated (for root switch)
         */
        void setAllDesignated();

        /*
         * Helper functions to handle state changes
         */
        void lostRoot();
        void lostAlternate(int port);
        void reset();

        /*
         * Determine who is eligible to become the root switch
         */
        bool checkRootEligibility();
        void tryRoot();

    public:
        friend inline std::ostream& operator<<(std::ostream& os, const ieee80211acInterfaceData::PortRole r);
        friend inline std::ostream& operator<<(std::ostream& os, const ieee80211acInterfaceData::PortState s);
        friend inline std::ostream& operator<<(std::ostream& os, ieee80211acInterfaceData * p);
        friend inline std::ostream& operator<<(std::ostream& os, STP i);

        // for lifecycle:
    protected:
        virtual void start();
        virtual void stop();
};

inline std::ostream& operator<<(std::ostream& os, const ieee80211acInterfaceData::PortRole r)
{

    switch (r)
    {
        case ieee80211acInterfaceData::NOTASSIGNED:
            os << "Unkn";
            break;
        case ieee80211acInterfaceData::ALTERNATE:
            os << "Altr";
            break;
        case ieee80211acInterfaceData::DESIGNATED:
            os << "Desg";
            break;
        case ieee80211acInterfaceData::ROOT:
            os << "Root";
            break;
        default:
            os << "<?>";
            break;
    }

    return os;
}

inline std::ostream& operator<<(std::ostream& os, const ieee80211acInterfaceData::PortState s)
{

    switch (s)
    {
        case ieee80211acInterfaceData::DISCARDING:
            os << "DIS";
            break;
        case ieee80211acInterfaceData::LEARNING:
            os << "LRN";
            break;
        case ieee80211acInterfaceData::FORWARDING:
            os << "FWD";
            break;
        default:
            os << "<?>";
            break;
    }

    return os;
}

inline std::ostream& operator<<(std::ostream& os, ieee80211acInterfaceData * p)
{
    os << "[";
    if (p->isLearning())
        os << "L";
    else
        os << "_";
    if (p->isForwarding())
        os << "F";
    else
        os << "_";
    os << "]";

    os << " " << p->getRole() << " " << p->getState() << " ";
    os << p->getLinkCost() << " ";
    os << p->getPriority() << " ";

    return os;
}

inline std::ostream& operator<<(std::ostream& os, STP i)
{
    os << "RootID Priority: " << i.rootPriority << " \n";
    os << "  Address: " << i.rootAddress << " \n";
    if (i.isRoot)
        os << "  This bridge is the Root. \n";
    else
    {
        os << "  Cost: " << i.rootPathCost << " \n";
        os << "  Port: " << i.rootPort << " \n";
    }
    os << "  Hello Time: " << i.currentHelloTime << " \n";
    os << "  Max Age: " << i.currentMaxAge << " \n";
    os << "  Forward Delay: " << i.currentFwdDelay << " \n";
    os << "BridgeID Priority: " << i.bridgePriority << "\n";
    os << "  Address: " << i.bridgeAddress << " \n";
    os << "  Hello Time: " << i.helloTime << " \n";
    os << "  Max Age: " << i.maxAge << " \n";
    os << "  Forward Delay: " << i.forwardDelay << " \n";
    os << "Port Flag Role State Cost Priority \n";
    os << "-----------------------------------------\n";

    for (unsigned int x = 0; x < i.numPorts; x++)
        os << x << "  " << i.getPortInterfaceData(x) << " \n";

    return os;
}

#endif
