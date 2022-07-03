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


cplusplus {{
#include "IPv4Address.h"

// base header: version, length, LSR ID, Label space
#define AF_BF_BASEHEADER_BYTES  10

// FIXME: the length below is just a guess. TBD find lengths for individual TLVs
// making up different AF_BF packet types, and determine length for each packet type
#define AF_BF_HEADER_BYTES  (AF_BF_BASEHEADER_BYTES+20)
}}


class noncobject IPv4Address;

//
// AF_BF message types
//
enum AF_BF_MESSAGE_TYPES
{

    NOTIFICATION = 10;
    HELLO = 11;
    INITIALIZATION = 12;
    KEEP_ALIVE = 13;
    ADDRESS = 14;
    ADDRESS_WITHDRAW = 15;
    LABEL_MAPPING = 16;
    LABEL_REQUEST = 17;
    LABEL_WITHDRAW = 18;
    LABEL_RELEASE = 19;
    UNKNOWN = 20;
}

enum AF_BF_STATUS_TYPES
{

    NO_ROUTE = 13;
}

struct FEC_TLV
{
    IPv4Address addr;
    int length;
}

//
// Base class for AF_BF packets
//
packet AF_BFPacket
{
    int type;
    IPv4Address senderAddress;
    IPv4Address receiverAddress;
}


//
// AF_BF Label Mapping Message
//
packet AF_BFLabelMapping extends AF_BFPacket
{
    FEC_TLV fec;
    int label;
}

//
// AF_BF Label Request Message
//
packet AF_BFLabelRequest extends AF_BFPacket
{
    FEC_TLV fec;
}


//
// AF_BF Hello Message
//
packet AF_BFHello extends AF_BFPacket
{
    double holdTime;
    bool tbit;
    bool rbit;
}

//
// AF_BF notification message
//
packet AF_BFNotify extends AF_BFPacket
{
    int status;
    FEC_TLV fec;
}

//
// AF_BF Ini Message
//
//# FIXME currently unused
packet AF_BFIni extends AF_BFPacket
{
    double keepAliveTime;
    bool abit;
    bool dbit;
    int pvLim;
    string receiverAF_BFIdentifier;
}

//
// AF_BF Address Message
//
//# FIXME currently unused
packet AF_BFAddress extends AF_BFPacket
{
    bool isWithdraw;
    string family;
    string addresses[];
}



