//
// (C) 2005 Vojta Janota
// (C) 2004 Andras Varga
//
// This library is free software, you can redistribute it
// and/or modify
// it under  the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation;
// either version 2 of the License, or any later version.
// The library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//


package inet.networklayer.AF_BF;

//
// Handles and processes AF_BF messages.
//
// The AF_BF protocol is used by one LSR (Label Switched Router) to inform
// another LSR of the label bindings it has made. The LSR uses this protocol
// to establish label switched paths through a network by mapping network layer
// routing information directly to data-link layer switched paths.
//
// Operations
//
// All AF_BF message types are subclassed from ~AF_BFPacket, and include
// ~AF_BFHello, ~AF_BFNotify, ~AF_BFLabelRequest, ~AF_BFIni and ~AF_BFAddress.
//
// Message processing in the ~AF_BF module is explained in
// <a href="AF_BF-processing.html">~AF_BF Message Processing</a>
//
// @author Andras Varga, Vojta Janota
//
simple AF_BF
{
    parameters:
        double holdTime @unit("s") = default(15s);
        double helloInterval @unit("s") = default(5s);
        string dataTransferMode @enum("bytecount", "object", "bytestream") = default("bytecount"); // TCP data transfer mode
        @display("i=block/control");
    gates:
        input tcpIn @labels(TCPCommand/up);
        input udpIn @labels(UDPControlInfo/up);
        output tcpOut @labels(TCPCommand/down);
        output udpOut @labels(UDPControlInfo/down);
}
