//
// This program is property of its copyright holder. All rights reserved.
//

#ifndef __INET_GTLQRROUTEDEFS_H_
#define __INET_GTLQRROUTEDEFS_H_

#define GTLQR_NAMESPACE_BEGIN namespace GTLQR {
#define GTLQR_NAMESPACE_END }

GTLQR_NAMESPACE_BEGIN

// TODO: use generic MANET port
#define GTLQR_UDP_PORT 269

typedef uint32_t GTLQRSequenceNumber;

// TODO: metric types are defined in a separate [RFC 6551]
enum GTLQRMetricType {
    HOP_COUNT = 3, // Hop Count has Metric Type assignment 3
};

enum GTLQRRouteState {
    ACTIVE,
    IDLE,
    EXPIRED,
    BROKEN,
    TIMED
};

GTLQR_NAMESPACE_END

#endif
