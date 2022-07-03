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
#include "GTLQRdefs.h"
}}

class noncobject IPv4Address;

class noncobject GTLQRSequenceNumber;

class noncobject GTLQRMetricType;

namespace GTLQR;

//
// classes
//

class AddressBlock {
    IPv4Address address;
    int prefixLength;
    bool hasValidityTime;
    simtime_t validityTime;
    bool hasMetric;
    double metric;
    bool hasMetricType;
    GTLQRMetricType metricType;
    bool hasSequenceNumber;
    GTLQRSequenceNumber sequenceNumber;
}

//
// GTLQR timer messages
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
// GTLQR packets
//

packet GTLQRPacket {
    int hopLimit;
}

packet RteMsg extends GTLQRPacket {
    int hopCount;
    AddressBlock originatorNode;
    AddressBlock targetNode;
    AddressBlock addedNode[];
}

//
// GTLQR RREQ packet
//
packet RREQ extends RteMsg {
}

//
// GTLQR RREP packet
//
packet RREP extends RteMsg {
}

//
// GTLQR RREP ACK packet
//
packet RREP_ACK extends GTLQRPacket {
}

//
// GTLQR RERR packet
//
packet RERR extends GTLQRPacket {
    AddressBlock unreachableNode[];
}
