//
// Copyright (C) 2004 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

cplusplus {{
#include "IPv4Address.h"
#include "Fuzzydefs.h"
}}

class noncobject IPv4Address;

class noncobject FuzzySequenceNumber;

class noncobject FuzzyMetricType;

namespace Fuzzy;

//
// classes
//

class AddressBlock {
    IPv4Address address;
    int Kconnect;
    bool hasValidityTime;
    simtime_t validityTime;
    bool lreliable;
    double rveloci;
    bool lreliableType;
    FuzzyMetricType metricType;
    bool hasSequenceNumber;
    FuzzySequenceNumber sequenceNumber;
}

//
// Fuzzy timer messages
//

message RREQTimer {
    IPv4Address target;
}

message RREQWaitRREPTimer extends RREQTimer {
    int retryCount;
}

message RREQBackoffTimer extends RREQTimer {
    int retryCount;
}

message RREQHolddownTimer extends RREQTimer {
}

//
// Fuzzy packets
//

packet FuzzyPacket {
    int hopLimit;
}

packet RteMsg extends FuzzyPacket {
    int hopCount;
    AddressBlock originatorNode;
    AddressBlock targetNode;
    AddressBlock addedNode[];
}

//
// Fuzzy RREQ packet
//
packet RREQ extends RteMsg {
}

//
// Fuzzy RREP packet
//
packet RREP extends RteMsg {
}

//
// Fuzzy RREP ACK packet
//
packet RREP_ACK extends FuzzyPacket {
}

//
// Fuzzy RERR packet
//
packet RERR extends FuzzyPacket {
    AddressBlock unreachableNode[];
}
