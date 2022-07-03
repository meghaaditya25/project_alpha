//
// This program is property of its copyright holder. All rights reserved.
//

#ifndef __INET_FuzzyDEFS_H_
#define __INET_FuzzyDEFS_H_

#define Fuzzy_NAMESPACE_BEGIN namespace Fuzzy {
#define Fuzzy_NAMESPACE_END }

Fuzzy_NAMESPACE_BEGIN

// TODO: use generic MANET port
#define Fuzzy_UDP_PORT 269

typedef uint32_t FuzzySequenceNumber;

// TODO: metric types are defined in a separate [RFC 6551]
enum FuzzyMetricType {
    HOP_COUNT = 3, // Hop Count has Metric Type assignment 3
};

enum FuzzyRouteState {
    ACTIVE,
    IDLE,
    EXPIRED,
    BROKEN,
    TIMED
};

Fuzzy_NAMESPACE_END

#endif
