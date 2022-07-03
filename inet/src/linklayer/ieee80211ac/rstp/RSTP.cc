//
// Copyright (C) 2011 Juan Luis Garrote Molinero
// Copyright (C) 2013 Zsolt Prontvai
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "RSTP.h"

#include "Ieee802Ctrl_m.h"
#include "InterfaceEntry.h"
#include "ModuleAccess.h"
#include "NodeOperations.h"
#include "NodeStatus.h"

Define_Module(RSTP);

RSTP::RSTP()
{
    helloTimer = NULL;
    upgradeTimer = NULL;
}

RSTP::~RSTP()
{
    cancelAndDelete(helloTimer);
    cancelAndDelete(upgradeTimer);
}

void RSTP::initialize(int stage)
{
    STPBase::initialize(stage);

    if (stage == 0)
    {
        autoEdge = par("autoEdge");
        tcWhileTime = par("tcWhileTime");
        migrateTime = par("migrateTime");
        helloTimer = new cMessage("itshellotime", SELF_HELLOTIME);
        upgradeTimer = new cMessage("upgrade", SELF_UPGRADE);
    }

    if (stage == 2) // "auto" MAC addresses assignment takes place in stage 0
    {
        initPorts();
        updateDisplay();
        // programming next auto-messages.
        scheduleAt(simTime(), helloTimer);
    }
}

void RSTP::scheduleNextUpgrde()
{
    cancelEvent(upgradeTimer);
    ieee80211acInterfaceData *nextInterfaceData = NULL;
    for (unsigned int i = 0; i < numPorts; i++)
    {
        if(getPortInterfaceEntry(i)->hasCarrier())
        {
            ieee80211acInterfaceData * iPort = getPortInterfaceData(i);
            if (iPort->getRole() == ieee80211acInterfaceData::NOTASSIGNED)
            {
                if(nextInterfaceData == NULL)
                    nextInterfaceData = iPort;
                else if(iPort->getNextUpgrade() < nextInterfaceData->getNextUpgrade())
                    nextInterfaceData = iPort;
            }
            else if (iPort->getRole() == ieee80211acInterfaceData::DESIGNATED)
            {
                if (iPort->getState() == ieee80211acInterfaceData::DISCARDING)
                {
                    if(nextInterfaceData == NULL)
                        nextInterfaceData = iPort;
                    else if(iPort->getNextUpgrade() < nextInterfaceData->getNextUpgrade())
                        nextInterfaceData = iPort;
                }
                else if (iPort->getState() == ieee80211acInterfaceData::LEARNING)
                {
                    if(nextInterfaceData == NULL)
                        nextInterfaceData = iPort;
                    else if(iPort->getNextUpgrade() < nextInterfaceData->getNextUpgrade())
                        nextInterfaceData = iPort;
                }
            }
        }
    }
    if(nextInterfaceData != NULL)
        scheduleAt(nextInterfaceData->getNextUpgrade(), upgradeTimer);
}

void RSTP::handleMessage(cMessage *msg)
{
    // it can receive BPDU or self messages
    if (!isOperational)
    {
        if (msg->isSelfMessage())
            throw cRuntimeError("Model error: self msg '%s' received when isOperational is false", msg->getName());
        EV << "Message '" << msg << "' arrived when module status is down, dropped\n";
        delete msg;
        return;
    }

    if (msg->isSelfMessage())
    {
        switch (msg->getKind())
        {
        case SELF_HELLOTIME:
            handleHelloTime(msg);
            break;

        case SELF_UPGRADE:
            // designated ports state upgrading (discarding-->learning, learning-->forwarding)
            handleUpgrade(msg);
            break;

        default:
            error("Unknown self message");
            break;
        }
    }
    else
    {
        handleIncomingFrame(check_and_cast<BPDU *> (msg)); // handling BPDU
    }
}

void RSTP::handleUpgrade(cMessage * msg)
{
    for (unsigned int i = 0; i < numPorts; i++)
    {
        ieee80211acInterfaceData * iPort = getPortInterfaceData(i);
        if(getPortInterfaceEntry(i)->hasCarrier() && iPort->getNextUpgrade() == simTime())
        {
            if (iPort->getRole() == ieee80211acInterfaceData::NOTASSIGNED)
            {
                EV_DETAIL << "MigrateTime. Setting port " << i << "to designated." << endl;
                iPort->setRole(ieee80211acInterfaceData::DESIGNATED);
                iPort->setState(ieee80211acInterfaceData::DISCARDING); // contest to become forwarding.
                iPort->setNextUpgrade(simTime() + forwardDelay);
            }
            else if (iPort->getRole() == ieee80211acInterfaceData::DESIGNATED)
            {
                if (iPort->getState() == ieee80211acInterfaceData::DISCARDING)
                {
                    EV_INFO << "UpgradeTime. Setting port " << i << " state to learning." << endl;
                    iPort->setState(ieee80211acInterfaceData::LEARNING);
                    iPort->setNextUpgrade(simTime() + forwardDelay);
                }
                else if (iPort->getState() == ieee80211acInterfaceData::LEARNING)
                {
                    EV_INFO << "UpgradeTime. Setting port " << i << " state to forwarding." << endl;
                    iPort->setState(ieee80211acInterfaceData::FORWARDING);
                    flushOtherPorts(i);
                }
            }
        }
    }
    updateDisplay();
    scheduleNextUpgrde();
}

void RSTP::handleHelloTime(cMessage * msg)
{
    EV_DETAIL << "Hello time." << endl;
    for (unsigned int i=0; i<numPorts; i++)
    {
        // sends hello through all active (learning, forwarding or not assigned) ports
        // increments LostBPDU just from ROOT, ALTERNATE and BACKUP
        ieee80211acInterfaceData * iPort = getPortInterfaceData(i);
        if (!iPort->isEdge()
                && (iPort->getRole() == ieee80211acInterfaceData::ROOT
                        || iPort->getRole() == ieee80211acInterfaceData::ALTERNATE
                        || iPort->getRole() == ieee80211acInterfaceData::BACKUP))
        {
            iPort->setLostBPDU(iPort->getLostBPDU()+1);
            if (iPort->getLostBPDU()>3) // 3 HelloTime without the best BPDU.
            {
                EV_DETAIL << "3 HelloTime without the best BPDU" << endl;
                // starts contest
                if (iPort->getRole() == ieee80211acInterfaceData::ROOT)
                {
                    // looking for the best ALTERNATE port
                    int candidate=getBestAlternate();
                    if (candidate!=-1)
                    {
                        // if an alternate gate has been found, switch to alternate
                        EV_DETAIL << "It was the root port. Alternate gate has been found. Setting port " << candidate << " to root. Setting current root port (port" << i << ") to designated." << endl;
                        // ALTERNATE->ROOT. DISCARDING->FORWARDING (immediately)
                        // old root gate goes to DESIGNATED and DISCARDING
                        // a new contest should be done to determine the new root path from this LAN
                        // updating root vector.
                        ieee80211acInterfaceData * candidatePort = getPortInterfaceData(candidate);
                        iPort->setRole(ieee80211acInterfaceData::DESIGNATED);
                        iPort->setState(ieee80211acInterfaceData::DISCARDING);// if there is not a better BPDU, that will become FORWARDING
                        iPort->setNextUpgrade(simTime() + forwardDelay);
                        scheduleNextUpgrde();
                        initInterfacedata(i);// reset, then a new BPDU will be allowed to upgrade the best received info for this port
                        candidatePort->setRole(ieee80211acInterfaceData::ROOT);
                        candidatePort->setState(ieee80211acInterfaceData::FORWARDING);
                        candidatePort->setLostBPDU(0);
                        flushOtherPorts(candidate);
                        macTable->copyTable(i,candidate); // copy cache from old to new root
                    }
                    else
                    {
                        // alternate not found, selects a new root
                        EV_DETAIL << "It was the root port. Alternate not found. Starts from beginning." << endl;
                        // initializing ports, start from the beginning
                        initPorts();
                    }
                }
                else if (iPort->getRole() == ieee80211acInterfaceData::ALTERNATE
                        ||iPort->getRole() == ieee80211acInterfaceData::BACKUP)
                {
                    EV_DETAIL << "Setting port " << i << " to designated." << endl;
                    // it should take care of this LAN, switching to designated
                    iPort->setRole(ieee80211acInterfaceData::DESIGNATED);
                    iPort->setState(ieee80211acInterfaceData::DISCARDING);// a new content will start in case of another switch were in alternate
                    iPort->setNextUpgrade(simTime() + forwardDelay);
                    scheduleNextUpgrde();
                    // if there is no problem, this will become forwarding in a few seconds
                    initInterfacedata(i);
                }
                iPort->setLostBPDU(0); // reseting lost bpdu counter after a change.
            }
        }
    }
    sendBPDUs(); // generating and sending new BPDUs
    sendTCNtoRoot();
    updateDisplay();
    scheduleAt(simTime()+helloTime, msg);// programming next hello time
}

void RSTP::checkTC(BPDU * frame, int arrival)
{
    ieee80211acInterfaceData * port = getPortInterfaceData(arrival);
    if ((frame->getTcFlag() == true) && (port->getState() == ieee80211acInterfaceData::FORWARDING))
    {
        EV_DETAIL << "TCN received" <<endl;
        findContainingNode(this)->bubble("TCN received");
        for (unsigned int i = 0; i < numPorts; i++)
        {
            if ((int) i != arrival)
            {
                ieee80211acInterfaceData * port2 = getPortInterfaceData(i);
                // flushing other ports
                // TCN over other ports
                macTable->flush(i);
                port2->setTCWhile(simTime()+tcWhileTime);
            }
        }
    }
}

void RSTP::handleBackup(BPDU * frame, unsigned int arrivalPortNum)
{
    EV_DETAIL << "More than one port in the same LAN"<<endl;
    ieee80211acInterfaceData * port = getPortInterfaceData(arrivalPortNum);
    if ((frame->getPortPriority() < port->getPortPriority())
            || ((frame->getPortPriority() == port->getPortPriority()) && (frame->getPortNum() < arrivalPortNum)))
    {
        // flushing arrival port
        macTable->flush(arrivalPortNum);
        port->setRole(ieee80211acInterfaceData::BACKUP);
        port->setState(ieee80211acInterfaceData::DISCARDING);
        port->setLostBPDU(0);
        EV_DETAIL << "Setting port " << arrivalPortNum << "to backup" << endl;
    }
    else if (frame->getPortPriority() > port->getPortPriority()
            || (frame->getPortPriority() == port->getPortPriority() && frame->getPortNum() > arrivalPortNum))
    {
        ieee80211acInterfaceData * port2 = getPortInterfaceData(frame->getPortNum());
        // flushing sender port
        macTable->flush(frame->getPortNum()); // portNum is sender port number, it is not arrival port
        port2->setRole(ieee80211acInterfaceData::BACKUP);
        port2->setState(ieee80211acInterfaceData::DISCARDING);
        port2->setLostBPDU(0);
        EV_DETAIL << "Setting port " << frame->getPortNum() << "to backup" << endl;
    }
    else
    {
        ieee80211acInterfaceData * port2 = getPortInterfaceData(frame->getPortNum());
        // unavoidable loop, received its own message at the same port
        // switch to disabled
        EV_DETAIL << "Unavoidable loop. Received its own message at the same port. Setting port "<< frame->getPortNum() << " to disabled." << endl;
        // flushing that port
        macTable->flush(frame->getPortNum()); // portNum is sender port number, it is not arrival port
        port2->setRole(ieee80211acInterfaceData::DISABLED);
        port2->setState(ieee80211acInterfaceData::DISCARDING);
    }
}

void RSTP::handleIncomingFrame(BPDU *frame)
{
    // incoming BPDU handling
    // checking message age
    Ieee802Ctrl * etherctrl = check_and_cast<Ieee802Ctrl *>(frame->removeControlInfo());
    int arrivalPortNum = etherctrl->getSwitchPort();
    MACAddress src = etherctrl->getSrc();
    delete etherctrl;
    EV_INFO << "BPDU received at port " << arrivalPortNum << "." << endl;
    if (frame->getMessageAge() < maxAge)
    {
        // checking TC
        checkTC(frame, arrivalPortNum); // sets TCWhile if arrival port was FORWARDING
        // checking possible backup
        if (src.compareTo(bridgeAddress) == 0)// more than one port in the same LAN
            handleBackup(frame, arrivalPortNum);
        else
            processBPDU(frame, arrivalPortNum);
    }
    else
        EV_DETAIL << "Expired BPDU" << endl;
    delete frame;

    updateDisplay();
}

void RSTP::processBPDU(BPDU *frame, unsigned int arrivalPortNum)
{
    //three challenges.
    //
    //first:  vs best received BPDU for that port --------->case
    //second: vs root BPDU--------------------------------->case1
    //third:  vs BPDU that would be sent from this Bridge.->case2
    ieee80211acInterfaceData * arrivalPort = getPortInterfaceData(arrivalPortNum);
    bool flood = false;
    if (compareInterfacedata(arrivalPortNum, frame, arrivalPort->getLinkCost()) > 0 //better root
            && frame->getRootAddress().compareTo(bridgeAddress) != 0) // root will not participate in a loop with its own address
        flood = processBetterSource(frame, arrivalPortNum);
    else if (frame->getBridgeAddress().compareTo(arrivalPort->getBridgeAddress()) == 0 // worse or similar, but the same source
            && frame->getRootAddress().compareTo(bridgeAddress) != 0) // root will not participate
        flood = processSameSource(frame, arrivalPortNum);
    if (flood)
    {
        sendBPDUs(); //expedited BPDU
        sendTCNtoRoot();
    }
}

bool RSTP::processBetterSource(BPDU *frame, unsigned int arrivalPortNum)
{
    EV_DETAIL << "Better BDPU received than the current best for this port." << endl;
    // update that port rstp info
    updateInterfacedata(frame, arrivalPortNum);
    ieee80211acInterfaceData * arrivalPort = getPortInterfaceData(arrivalPortNum);
    int r = getRootIndex();
    if (r == -1)
    {
        EV_DETAIL << "There was no root. Setting the arrival port to root." << endl;
        // there was no root
        arrivalPort->setRole(ieee80211acInterfaceData::ROOT);
        arrivalPort->setState(ieee80211acInterfaceData::FORWARDING);
        arrivalPort->setLostBPDU(0);
        flushOtherPorts(arrivalPortNum);
        return true;
    }
    else
    {
        ieee80211acInterfaceData * rootPort = getPortInterfaceData(r);
        // there was a Root -> challenge 2 (compare with the root)
        int case2 = compareInterfacedata(r, frame, arrivalPort->getLinkCost()); // comparing with root port's BPDU
        int case3 = 0;

        switch (case2)
        {
        case SIMILAR:// double link to the same port of the root source -> Tie breaking (better local port first)
            EV_DETAIL << "Double link to the same port of the root source." << endl;
            if (rootPort->getPortPriority() < arrivalPort->getPortPriority()
                    || (rootPort->getPortPriority() == arrivalPort->getPortPriority() && (unsigned int)r < arrivalPortNum))
            {
                // flushing that port
                EV_DETAIL << "The current root has better local port. Setting the arrival port to alternate." << endl;
                macTable->flush(arrivalPortNum);
                arrivalPort->setRole(ieee80211acInterfaceData::ALTERNATE);
                arrivalPort->setState(ieee80211acInterfaceData::DISCARDING);
                arrivalPort->setLostBPDU(0);
            }
            else
            {
                if (arrivalPort->getState() != ieee80211acInterfaceData::FORWARDING)
                    flushOtherPorts(arrivalPortNum);
                else
                    macTable->flush(r); // flushing r, needed in case arrival were previously FORWARDING
                EV_DETAIL << "This has better local port. Setting the arrival port to root. Setting current root port (port " << r << ") to alternate." << endl;
                rootPort->setRole(ieee80211acInterfaceData::ALTERNATE);
                rootPort->setState(ieee80211acInterfaceData::DISCARDING); // comes from root, preserve lostBPDU
                arrivalPort->setRole(ieee80211acInterfaceData::ROOT);
                arrivalPort->setState(ieee80211acInterfaceData::FORWARDING);
                arrivalPort->setLostBPDU(0);
                macTable->copyTable(r, arrivalPortNum); // copy cache from old to new root
                // the change does not deserve flooding
            }
            break;

        case BETTER_ROOT: // new port rstp info is better than the root in another gate -> root change
            EV_DETAIL << "Better root received than the current root. Setting the arrival port to root." << endl;
            for (unsigned int i = 0; i < numPorts; i++)
            {
                ieee80211acInterfaceData * iPort = getPortInterfaceData(i);
                if (!iPort->isEdge())   // avoiding clients reseting
                {
                    if (arrivalPort->getState() != ieee80211acInterfaceData::FORWARDING)
                        iPort->setTCWhile(simTime()+tcWhileTime);
                    macTable->flush(i);
                    if (i!=(unsigned)arrivalPortNum)
                    {
                        iPort->setRole(ieee80211acInterfaceData::NOTASSIGNED);
                        iPort->setState(ieee80211acInterfaceData::DISCARDING);
                        iPort->setNextUpgrade(simTime() + migrateTime);
                        scheduleNextUpgrde();
                        initInterfacedata(i);
                    }
                }
            }
            arrivalPort->setRole(ieee80211acInterfaceData::ROOT);
            arrivalPort->setState(ieee80211acInterfaceData::FORWARDING);
            arrivalPort->setLostBPDU(0);

            return true;

        case BETTER_RPC:// same that Root but better RPC
        case BETTER_SRC:// same that Root RPC but better source
        case BETTER_PORT:// same that root RPC and source but better port
            if (arrivalPort->getState()!=ieee80211acInterfaceData::FORWARDING)
            {
                EV_DETAIL << "Better route to the current root. Setting the arrival port to root." << endl;
                flushOtherPorts(arrivalPortNum);
            }
            arrivalPort->setRole(ieee80211acInterfaceData::ROOT);
            arrivalPort->setState(ieee80211acInterfaceData::FORWARDING);
            arrivalPort->setLostBPDU(0);
            rootPort->setRole(ieee80211acInterfaceData::ALTERNATE); // temporary, just one port can be root at contest time
            macTable->copyTable(r,arrivalPortNum);// copy cache from old to new root
            case3=contestInterfacedata(r);
            if (case3>=0)
            {
                EV_DETAIL << "Setting current root port (port " << r << ") to alternate." << endl;
                rootPort->setRole(ieee80211acInterfaceData::ALTERNATE);
                rootPort->setState(ieee80211acInterfaceData::DISCARDING);
                // not lostBPDU reset
                // flushing r
                macTable->flush(r);
            }
            else
            {
                EV_DETAIL << "Setting current root port (port " << r << ") to designated." << endl;
                rootPort->setRole(ieee80211acInterfaceData::DESIGNATED);
                rootPort->setState(ieee80211acInterfaceData::DISCARDING);
                rootPort->setNextUpgrade(simTime() + forwardDelay);
                scheduleNextUpgrde();
            }
            return true;

        case WORSE_ROOT:
            EV_DETAIL << "Worse BDPU received than the current root. Sending BPDU to show him a better root as soon as possible." << endl;
            sendBPDU(arrivalPortNum);// BPDU to show him a better root as soon as possible
            break;

        case WORSE_RPC:// same Root but worse RPC
        case WORSE_SRC:// same Root RPC but worse source
        case WORSE_PORT:// same Root RPC and source but worse port
            case3=contestInterfacedata(frame,arrivalPortNum);// case 0 not possible
            if (case3<0)
            {
                EV_DETAIL << "Worse route to the current root. Setting the arrival port to designated." << endl;
                arrivalPort->setRole(ieee80211acInterfaceData::DESIGNATED);
                arrivalPort->setState(ieee80211acInterfaceData::DISCARDING);
                arrivalPort->setNextUpgrade(simTime() + forwardDelay);
                scheduleNextUpgrde();
                sendBPDU(arrivalPortNum); // BPDU to show him a better root as soon as possible
            }
            else
            {
                EV_DETAIL << "Worse route to the current root. Setting the arrival port to alternate." << endl;
                // flush arrival
                macTable->flush(arrivalPortNum);
                arrivalPort->setRole(ieee80211acInterfaceData::ALTERNATE);
                arrivalPort->setState(ieee80211acInterfaceData::DISCARDING);
                arrivalPort->setLostBPDU(0);
            }
            break;
        }
    }
    return false;
}


bool RSTP::processSameSource(BPDU *frame, unsigned int arrivalPortNum)
{
    EV_DETAIL << "BDPU received from the same source than the current best for this port" << endl;
    ieee80211acInterfaceData * arrivalPort = getPortInterfaceData(arrivalPortNum);
    int case0 = compareInterfacedata(arrivalPortNum, frame, arrivalPort->getLinkCost());
    // source has updated BPDU information
    switch(case0)
    {
    case SIMILAR:
        arrivalPort->setLostBPDU(0);  // same BPDU, not updated
        break;

    case WORSE_ROOT:
        EV_DETAIL << "Worse root received than the current best for this port." << endl;
        if (arrivalPort->getRole() == ieee80211acInterfaceData::ROOT)
        {
            int alternative=getBestAlternate(); // searching for alternate
            if (alternative>=0)
            {
                EV_DETAIL << "This port was the root, but there is a better alternative. Setting the arrival port to designated and port " << alternative << "to root." << endl;
                ieee80211acInterfaceData * alternativePort = getPortInterfaceData(alternative);
                arrivalPort->setRole(ieee80211acInterfaceData::DESIGNATED);
                arrivalPort->setState(ieee80211acInterfaceData::DISCARDING);
                arrivalPort->setNextUpgrade(simTime() + forwardDelay);
                scheduleNextUpgrde();
                macTable->copyTable(arrivalPortNum,alternative); // copy cache from old to new root
                flushOtherPorts(alternative);
                alternativePort->setRole(ieee80211acInterfaceData::ROOT);
                alternativePort->setState(ieee80211acInterfaceData::FORWARDING); // comes from alternate, preserves lostBPDU
                updateInterfacedata(frame,arrivalPortNum);
                sendBPDU(arrivalPortNum);// show him a better Root as soon as possible
            }
            else
            {
                EV_DETAIL << "This port was the root and there no alternative. Initialize all ports" << endl;
                int case2=0;
                initPorts();// allowing other ports to contest again
                // flushing all ports
                for (unsigned int j=0; j<numPorts; j++)
                    macTable->flush(j);
                case2=compareInterfacedata(arrivalPortNum,frame,arrivalPort->getLinkCost());
                if (case2>0)
                {
                    EV_DETAIL << "This switch is not better, keep arrival port as a ROOT" << endl;
                    updateInterfacedata(frame,arrivalPortNum); // if this module is not better, keep it as a ROOT
                    arrivalPort->setRole(ieee80211acInterfaceData::ROOT);
                    arrivalPort->setState(ieee80211acInterfaceData::FORWARDING);
                }
                // propagating new information
                return true;
            }
        }
        else if (arrivalPort->getRole() == ieee80211acInterfaceData::ALTERNATE)
        {
            EV_DETAIL << "This port was an alternate, setting to designated" << endl;
            arrivalPort->setRole(ieee80211acInterfaceData::DESIGNATED);
            arrivalPort->setState(ieee80211acInterfaceData::DISCARDING);
            arrivalPort->setNextUpgrade(simTime() + forwardDelay);
            scheduleNextUpgrde();
            updateInterfacedata(frame,arrivalPortNum);
            sendBPDU(arrivalPortNum); //Show him a better Root as soon as possible
        }
        break;

    case WORSE_RPC:
    case WORSE_SRC:
    case WORSE_PORT:
        EV_DETAIL << "Worse route to the current root than the current best for this port." << endl;
        if (arrivalPort->getRole() == ieee80211acInterfaceData::ROOT)
        {
            arrivalPort->setLostBPDU(0);
            int alternative=getBestAlternate(); // searching for alternate
            if (alternative>=0)
            {
                ieee80211acInterfaceData * alternativePort = getPortInterfaceData(alternative);
                int case2=0;
                case2=compareInterfacedata(alternative,frame,arrivalPort->getLinkCost());
                if (case2<0) // if alternate is better, change
                {
                    alternativePort->setRole(ieee80211acInterfaceData::ROOT);
                    alternativePort->setState(ieee80211acInterfaceData::FORWARDING);
                    arrivalPort->setRole(ieee80211acInterfaceData::DESIGNATED); // temporary, just one port can be root at contest time
                    int case3=0;
                    case3=contestInterfacedata(frame,arrivalPortNum);
                    if (case3<0)
                    {
                        EV_DETAIL << "This port was the root, but there is a better alternative. Setting the arrival port to designated and port " << alternative << "to root." << endl;
                        arrivalPort->setRole(ieee80211acInterfaceData::DESIGNATED);
                        arrivalPort->setState(ieee80211acInterfaceData::DISCARDING);
                        arrivalPort->setNextUpgrade(simTime() + forwardDelay);
                        scheduleNextUpgrde();
                    }
                    else
                    {
                        EV_DETAIL << "This port was the root, but there is a better alternative. Setting the arrival port to alternate and port " << alternative << "to root." << endl;
                        arrivalPort->setRole(ieee80211acInterfaceData::ALTERNATE);
                        arrivalPort->setState(ieee80211acInterfaceData::DISCARDING);
                    }
                    flushOtherPorts(alternative);
                    macTable->copyTable(arrivalPortNum,alternative); // copy cache from old to new root
                }
            }
            updateInterfacedata(frame,arrivalPortNum);
            // propagating new information
            return true;
            // if alternate is worse than root, or there is not alternate, keep old root as root
        }
        else if (arrivalPort->getRole() == ieee80211acInterfaceData::ALTERNATE)
        {
            int case2=0;
            case2=contestInterfacedata(frame,arrivalPortNum);
            if (case2<0)
            {
                EV_DETAIL << "This port was an alternate, setting to designated" << endl;
                arrivalPort->setRole(ieee80211acInterfaceData::DESIGNATED); // if the frame is worse than this module generated frame, switch to Designated/Discarding
                arrivalPort->setState(ieee80211acInterfaceData::DISCARDING);
                arrivalPort->setNextUpgrade(simTime() + forwardDelay);
                scheduleNextUpgrde();
                sendBPDU(arrivalPortNum);// show him a better BPDU as soon as possible
            }
            else
            {
                arrivalPort->setLostBPDU(0); // if it is better than this module generated frame, keep it as alternate
                // this does not deserve expedited BPDU
            }
        }
        updateInterfacedata(frame,arrivalPortNum);
        break;
    }
    return false;
}

void RSTP::sendTCNtoRoot()
{
    // if TCWhile is not expired, sends BPDU with TC flag to the root
    this->bubble("SendTCNtoRoot");
    EV_DETAIL << "SendTCNtoRoot" << endl;
    int r = getRootIndex();
    if ((r >= 0) && ((unsigned int) r < numPorts))
    {
        ieee80211acInterfaceData * rootPort = getPortInterfaceData(r);
        if (rootPort->getRole() != ieee80211acInterfaceData::DISABLED)
        {
            if (simTime()<rootPort->getTCWhile())
            {
                BPDU * frame = new BPDU();
                Ieee802Ctrl * etherctrl= new Ieee802Ctrl();

                frame->setRootPriority(rootPort->getRootPriority());
                frame->setRootAddress(rootPort->getRootAddress());
                frame->setMessageAge(rootPort->getAge());
                frame->setRootPathCost(rootPort->getRootPathCost());
                frame->setBridgePriority(bridgePriority);
                frame->setTcaFlag(false);
                frame->setPortNum(r);
                frame->setBridgeAddress(bridgeAddress);
                frame->setTcFlag(true);
                frame->setName("BPDU");
                frame->setMaxAge(maxAge);
                frame->setHelloTime(helloTime);
                frame->setForwardDelay(forwardDelay);
                if (frame->getByteLength() < MIN_ETHERNET_FRAME_BYTES)
                    frame->setByteLength(MIN_ETHERNET_FRAME_BYTES);
                etherctrl->setSrc(bridgeAddress);
                etherctrl->setDest(MACAddress::STP_MULTICAST_ADDRESS);
                etherctrl->setSwitchPort(r);
                frame->setControlInfo(etherctrl);
                send(frame,"relayOut");
            }
        }
    }
}

void RSTP::sendBPDUs()
{
    // send BPDUs through all ports, if they are required
    for (unsigned int i = 0; i < numPorts; i++)
    {
        ieee80211acInterfaceData * iPort = getPortInterfaceData(i);
        if ((iPort->getRole() != ieee80211acInterfaceData::ROOT)
                && (iPort->getRole() != ieee80211acInterfaceData::ALTERNATE)
                && (iPort->getRole() != ieee80211acInterfaceData::DISABLED) && (!iPort->isEdge()))
        {
            sendBPDU(i);
        }
    }
}

void RSTP::sendBPDU(int port)
{
    // send a BPDU throuth port
    ieee80211acInterfaceData * iport = getPortInterfaceData(port);
    int r = getRootIndex();
    ieee80211acInterfaceData * rootPort;
    if (r != -1)
        rootPort = getPortInterfaceData(r);
    if (iport->getRole() != ieee80211acInterfaceData::DISABLED)
    {
        BPDU * frame = new BPDU();
        Ieee802Ctrl * etherctrl = new Ieee802Ctrl();
        if (r != -1)
        {
            frame->setRootPriority(rootPort->getRootPriority());
            frame->setRootAddress(rootPort->getRootAddress());
            frame->setMessageAge(rootPort->getAge());
            frame->setRootPathCost(rootPort->getRootPathCost());
        }
        else
        {
            frame->setRootPriority(bridgePriority);
            frame->setRootAddress(bridgeAddress);
            frame->setMessageAge(0);
            frame->setRootPathCost(0);
        }
        frame->setBridgePriority(bridgePriority);
        frame->setTcaFlag(false);
        frame->setPortNum(port);
        frame->setBridgeAddress(bridgeAddress);
        if (simTime() < iport->getTCWhile())
            frame->setTcFlag(true);
        else
            frame->setTcFlag(false);
        frame->setName("BPDU");
        frame->setMaxAge(maxAge);
        frame->setHelloTime(helloTime);
        frame->setForwardDelay(forwardDelay);
        if (frame->getByteLength() < MIN_ETHERNET_FRAME_BYTES)
            frame->setByteLength(MIN_ETHERNET_FRAME_BYTES);
        etherctrl->setSrc(bridgeAddress);
        etherctrl->setDest(MACAddress::STP_MULTICAST_ADDRESS);
        etherctrl->setSwitchPort(port);
        frame->setControlInfo(etherctrl);
        send(frame, "relayOut");
    }
}

void RSTP::printState()
{
    //  prints current database info
    EV_DETAIL << "Switch " << findContainingNode(this)->getFullName() << " state:" << endl;
    int rootIndex = getRootIndex();
    EV_DETAIL << "  Priority: " << bridgePriority << endl;
    EV_DETAIL << "  Local MAC: " << bridgeAddress << endl;
    if (rootIndex >= 0)
    {
        ieee80211acInterfaceData * rootPort = getPortInterfaceData(rootIndex);
        EV_DETAIL << "  Root Priority: " << rootPort->getRootPriority() << endl;
        EV_DETAIL << "  Root Address: " << rootPort->getRootAddress().str() << endl;
        EV_DETAIL << "  Cost: " << rootPort->getRootPathCost() << endl;
        EV_DETAIL << "  Age:  " << rootPort->getAge() << endl;
        EV_DETAIL << "  Bridge Priority: " << rootPort->getBridgePriority() << endl;
        EV_DETAIL << "  Bridge Address: " << rootPort->getBridgeAddress().str() << endl;
        EV_DETAIL << "  Src TxGate Priority: " << rootPort->getPortPriority() << endl;
        EV_DETAIL << "  Src TxGate: " << rootPort->getPortNum() << endl;
    }
    EV_DETAIL << "Port State/Role:" << endl;
    for (unsigned int i=0; i<numPorts; i++)
    {
        ieee80211acInterfaceData * iPort = getPortInterfaceData(i);
        EV_DETAIL << "  " << i << ": " << iPort->getStateName() << "/" << iPort->getRoleName() << (iPort->isEdge() ? " (Client)" : "") << endl;
    }
    EV_DETAIL << "Per-port best sources, Root/Src:" << endl;
    for (unsigned int i=0; i<numPorts; i++)
    {
        ieee80211acInterfaceData * iPort = getPortInterfaceData(i);
        EV_DETAIL << "  " << i << ": " << iPort->getRootAddress().str() << "/" << iPort->getBridgeAddress().str() << endl;
    }
    EV_DETAIL << endl;
}

void RSTP::initInterfacedata(unsigned int portNum)
{
    ieee80211acInterfaceData * ifd = getPortInterfaceData(portNum);
    ifd->setRootPriority(bridgePriority);
    ifd->setRootAddress(bridgeAddress);
    ifd->setRootPathCost(0);
    ifd->setAge(0);
    ifd->setBridgePriority(bridgePriority);
    ifd->setBridgeAddress(bridgeAddress);
    ifd->setPortPriority(-1);
    ifd->setPortNum(-1);
    ifd->setLostBPDU(0);
}

void RSTP::initPorts()
{
    for (unsigned int j = 0; j < numPorts; j++)
    {
        ieee80211acInterfaceData * jPort = getPortInterfaceData(j);
        if (!jPort->isEdge())
        {
            jPort->setRole(ieee80211acInterfaceData::NOTASSIGNED);
            jPort->setState(ieee80211acInterfaceData::DISCARDING);
            jPort->setNextUpgrade(simTime() + migrateTime);
        }
        else
        {
            jPort->setRole(ieee80211acInterfaceData::DESIGNATED);
            jPort->setState(ieee80211acInterfaceData::FORWARDING);
        }
        initInterfacedata(j);
        macTable->flush(j);
    }
    scheduleNextUpgrde();
}

void RSTP::updateInterfacedata(BPDU *frame, unsigned int portNum)
{
    ieee80211acInterfaceData * ifd = getPortInterfaceData(portNum);
    ifd->setRootPriority(frame->getRootPriority());
    ifd->setRootAddress(frame->getRootAddress());
    ifd->setRootPathCost(frame->getRootPathCost() + ifd->getLinkCost());
    ifd->setAge(frame->getMessageAge() + 1);
    ifd->setBridgePriority(frame->getBridgePriority());
    ifd->setBridgeAddress(frame->getBridgeAddress());
    ifd->setPortPriority(frame->getPortPriority());
    ifd->setPortNum(frame->getPortNum());
    ifd->setLostBPDU(0);
}

RSTP::CompareResult RSTP::contestInterfacedata(unsigned int portNum)
{
    int r = getRootIndex();
    ieee80211acInterfaceData * rootPort = getPortInterfaceData(r);
    ieee80211acInterfaceData * ifd = getPortInterfaceData(portNum);

    return compareRSTPData(rootPort->getRootPriority(),                          ifd->getRootPriority(),
                           rootPort->getRootAddress(),                           ifd->getRootAddress(),
                           rootPort->getRootPathCost() + ifd->getLinkCost(),     ifd->getRootPathCost(),
                           bridgePriority,                                       ifd->getBridgePriority(),
                           bridgeAddress,                                        ifd->getBridgeAddress(),
                           ifd->getPortPriority(),                               ifd->getPortPriority(),
                           portNum,                                              ifd->getPortNum());
}

RSTP::CompareResult RSTP::contestInterfacedata(BPDU* msg, unsigned int portNum)
{
    int r = getRootIndex();
    ieee80211acInterfaceData * rootPort = getPortInterfaceData(r);
    ieee80211acInterfaceData * ifd = getPortInterfaceData(portNum);

    return compareRSTPData(rootPort->getRootPriority(),     msg->getRootPriority(),
                           rootPort->getRootAddress(),      msg->getRootAddress(),
                           rootPort->getRootPathCost(),     msg->getRootPathCost(),
                           bridgePriority,                  msg->getBridgePriority(),
                           bridgeAddress,                   msg->getBridgeAddress(),
                           ifd->getPortPriority(),          msg->getPortPriority(),
                           portNum,                         msg->getPortNum());
}

RSTP::CompareResult RSTP::compareInterfacedata(unsigned int portNum, BPDU * msg, int linkCost)
{
    ieee80211acInterfaceData * ifd = getPortInterfaceData(portNum);

    return compareRSTPData(ifd->getRootPriority(),     msg->getRootPriority(),
                           ifd->getRootAddress(),      msg->getRootAddress(),
                           ifd->getRootPathCost(),     msg->getRootPathCost() + linkCost,
                           ifd->getBridgePriority(),   msg->getBridgePriority(),
                           ifd->getBridgeAddress(),    msg->getBridgeAddress(),
                           ifd->getPortPriority(),     msg->getPortPriority(),
                           ifd->getPortNum(),          msg->getPortNum());
}

RSTP::CompareResult RSTP::compareRSTPData(int rootPriority1, int rootPriority2,
        MACAddress rootAddress1, MACAddress rootAddress2,
        int rootPathCost1, int rootPathCost2,
        int bridgePriority1, int bridgePriority2,
        MACAddress bridgeAddress1, MACAddress bridgeAddress2,
        int portPriority1, int portPriority2,
        int portNum1, int portNum2)
{
    if (rootPriority1 != rootPriority2)
            return (rootPriority1 < rootPriority2) ? WORSE_ROOT : BETTER_ROOT;

        int c = rootAddress1.compareTo(rootAddress2);
        if (c != 0)
            return (c < 0) ? WORSE_ROOT : BETTER_ROOT;

        if (rootPathCost1 != rootPathCost2)
            return (rootPathCost1 < rootPathCost2) ? WORSE_RPC : BETTER_RPC;

        if (bridgePriority1 != bridgePriority2)
            return (bridgePriority1 < bridgePriority2) ? WORSE_SRC : BETTER_SRC;

        c = bridgeAddress1.compareTo(bridgeAddress2);
        if (c != 0)
            return (c < 0) ? WORSE_SRC : BETTER_SRC;

        if (portPriority1 != portPriority2)
            return (portPriority1 < portPriority2) ? WORSE_PORT : BETTER_PORT;

        if (portNum1 != portNum2)
            return (portNum1 < portNum2) ? WORSE_PORT : BETTER_PORT;

        return SIMILAR;
}

int RSTP::getBestAlternate()
{
    int candidate = -1;  // index of the best alternate found
    for (unsigned int j = 0; j < numPorts; j++)
    {
        ieee80211acInterfaceData * jPort = getPortInterfaceData(j);
        if (jPort->getRole() == ieee80211acInterfaceData::ALTERNATE) // just from alternates, others are not updated
        {
            if (candidate < 0)
                candidate = j;
            else
            {
                ieee80211acInterfaceData * candidatePort = getPortInterfaceData(candidate);
                if (compareRSTPData(jPort->getRootPriority(),     candidatePort->getRootPriority(),
                                    jPort->getRootAddress(),      candidatePort->getRootAddress(),
                                    jPort->getRootPathCost(),     candidatePort->getRootPathCost(),
                                    jPort->getBridgePriority(),   candidatePort->getBridgePriority(),
                                    jPort->getBridgeAddress(),    candidatePort->getBridgeAddress(),
                                    jPort->getPortPriority(),     candidatePort->getPortPriority(),
                                    jPort->getPortNum(),          candidatePort->getPortNum()) <  0)
                {
                    // alternate better than the found one
                    candidate = j; // new candidate
                }
            }
        }
    }
    return candidate;
}

void RSTP::flushOtherPorts(unsigned int portNum)
{
    for (unsigned int i=0; i<numPorts; i++)
    {
        ieee80211acInterfaceData * iPort = getPortInterfaceData(i);
        iPort->setTCWhile(simulation.getSimTime()+tcWhileTime);
        if (i!=portNum)
            macTable->flush(i);
    }
}

void RSTP::receiveChangeNotification(int category, const cObject *details)
{
    if (category == NF_INTERFACE_STATE_CHANGED)
    {
        for (unsigned int i = 0; i < numPorts; i++)
        {
            InterfaceEntry * gateIfEntry = getPortInterfaceEntry(i);
            if (gateIfEntry == check_and_cast<const InterfaceEntry *>(details))
            {
                if(gateIfEntry->hasCarrier())
                {
                    ieee80211acInterfaceData * iPort = getPortInterfaceData(i);
                    if (iPort->getRole() == ieee80211acInterfaceData::NOTASSIGNED)
                        iPort->setNextUpgrade(simTime() + migrateTime);
                    else if (iPort->getRole() == ieee80211acInterfaceData::DESIGNATED
                             && (iPort->getState() == ieee80211acInterfaceData::DISCARDING || iPort->getState() == ieee80211acInterfaceData::LEARNING))
                            iPort->setNextUpgrade(simTime() + forwardDelay);
                    scheduleNextUpgrde();
                }
            }
        }
    }
}

void RSTP::start()
{
    STPBase::start();
    initPorts();
    scheduleAt(simTime(), helloTimer);
}

void RSTP::stop()
{
    STPBase::stop();
    cancelEvent(helloTimer);
    cancelEvent(upgradeTimer);
}
