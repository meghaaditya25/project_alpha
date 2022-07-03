package inet.networklayer.routing.aodv;
simple EAR_P2P
{
    
    parameters:
        @display("i=block/table2");
        double agingTime @unit = default(30);
        bool askGratuitousRREP = default(false); // see RFC 3561: 6.6.3
        bool useHelloMessages = default(false); // see RFC 3561: 6.9
        bool useLocalRepair = default(false); // see RFC 3561: 6.12 *not implemented yet*
        int udpPort = default(354);
        int Residualenergy = default(14);
        double maxJitter @unit("s") = default(1s);
}
