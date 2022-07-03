//
// This program is property of its copyright holder. All rights reserved.
//

#ifndef __INET_GTLQRROUTE_H_
#define __INET_GTLQRROUTE_H_

#include <omnetpp.h>
#include "IPv4Route.h"
#include "GTLQRdefs.h"

GTLQR_NAMESPACE_BEGIN

/**
 * GTLQR specific extra route data attached to routes in the routing table.
 */
class INET_API SKHData : public cObject {
  private:
    bool isBroken;
    GTLQRSequenceNumber sequenceNumber;
    simtime_t lastUsed;
    simtime_t expirationTime;
    GTLQRMetricType metricType;

  public:
    SKHData();
    virtual ~SKHData() { }

    bool getBroken() const { return isBroken; }
    void setBroken(bool isBroken) { this->isBroken = isBroken; }

    GTLQRSequenceNumber getSequenceNumber() const { return sequenceNumber; }
    void setSequenceNumber(GTLQRSequenceNumber sequenceNumber) { this->sequenceNumber = sequenceNumber; }

    simtime_t getLastUsed() const { return lastUsed; }
    void setLastUsed(simtime_t lastUsed) { this->lastUsed = lastUsed; }

    simtime_t getExpirationTime() const { return expirationTime; }
    void setExpirationTime(simtime_t expirationTime) { this->expirationTime = expirationTime; }

    GTLQRMetricType getMetricType() const { return metricType; }
    void setMetricType(GTLQRMetricType metricType) { this->metricType = metricType; }
};

GTLQR_NAMESPACE_END

#endif
