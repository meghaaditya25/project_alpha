


package inet.base;
import inet.base.CommunicationUnit;
import inet.linklayer.IMACRelayUnit;
import inet.linklayer.IWiredNic;
import inet.linklayer.IWirelessNic;
import inet.linklayer.IMACAddressTable;
import inet.linklayer.channels.Statplot;
import inet.mobility.contract.IMobility;
import inet.networklayer.common.InterfaceTable;
import inet.status.NodeStatus;

module Stationnode
{
    parameters:
        @node();
        @labels(node,ethernet-node,wireless-node);
        string mobilityType = default("StationaryMobility"); // how the node moves around. see: inet.mobility
        string relayUnitType = default(firstAvailable("Ieee8021dRelay","MACRelayUnit","MACRelayUnit_None")); // type of the IMACRelayUnit (MACRelayUnit, Ieee8021dRelay, etc)
        string macTableType = default("MACAddressTable"); // type of the ~IMACAddressTable
        int numradios = default(10);
        bool hasStatus = default(false);
        wlan[*].mgmtType = default("Ieee80211MgmtAP");
        eth[*].encapType = "EtherEncapDummy";
    gates:
        input radioIn[numradios] @directIn;
        inout ethg[] @labels(EtherFrame-conn);
    submodules:
        status: NodeStatus if hasStatus { // IDS
            @display("p=51,43");
        }
        interfaceTable: InterfaceTable { // fingerprint unit
            @display("p=51,121;is=s");
        }
        communicationUnit: CommunicationUnit {  //// communication unit
            parameters:
                @display("p=128,27;is=s");
        }
        mobility: <mobilityType> like IMobility {
            parameters:
                @display("p=208,43");
        }
        macTable: <macTableType> like IMACAddressTable {
            @display("p=73,19;is=s");
        }
        relayUnit: <relayUnitType> like IMACRelayUnit if sizeof(ethg)+numradios>1 {
            parameters:
                @display("p=151,121");
        }
        wlan[numradios]: <default("Ieee80211Nic")> like IWirelessNic {
            parameters:
                @display("p=89,225,row;q=queue");
        }
        eth[sizeof(ethg)]: <default("EthernetInterface")> like IWiredNic {
            parameters:
                mac.promiscuous = true;
                @display("p=208,225,row;q=txQueue");
        }

        statplot: Statplot {
            @display("p=1,1;is=vs");
        }
    connections allowunconnected:
        // connections to network outside
        // wireless interfaces MUST be connected first (i.e. ports 0..numradios-1)
        // because broadcasts must be handled differently for wireless IFs by the relayUnit
        for i=0..numradios-1 {
            radioIn[i] --> wlan[i].radioIn;
            wlan[i].upperLayerOut --> relayUnit.ifIn++ if sizeof(ethg)+numradios>1;
            wlan[i].upperLayerIn <-- relayUnit.ifOut++ if sizeof(ethg)+numradios>1;
        }
        // ethernet must be connected only AFTER wireless ports
        for i=0..sizeof(ethg)-1 {
            eth[i].phys <--> ethg[i];
            eth[i].upperLayerIn <-- relayUnit.ifOut++ if sizeof(ethg)+numradios>1;
            eth[i].upperLayerOut --> relayUnit.ifIn++ if sizeof(ethg)+numradios>1;
        }
}
