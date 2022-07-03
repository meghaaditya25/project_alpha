//
// This program is property of its copyright holder. All rights reserved.
//

#include "SKHData.h"

GTLQR_NAMESPACE_BEGIN

SKHData::SKHData() {
    isBroken = false;
    sequenceNumber = 0;
    lastUsed = 0;
    expirationTime = 0;
    metricType = HOP_COUNT;
}

GTLQR_NAMESPACE_END
