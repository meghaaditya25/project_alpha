//
// This program is property of its copyright holder. All rights reserved.
//

#include "QLearnData.h"

Fuzzy_NAMESPACE_BEGIN

QLearnData::QLearnData() {
    isBroken = false;
    sequenceNumber = 0;
    lastUsed = 0;
    expirationTime = 0;
    metricType = HOP_COUNT;
}

Fuzzy_NAMESPACE_END
