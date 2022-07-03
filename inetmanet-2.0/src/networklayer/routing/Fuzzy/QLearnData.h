//
// This program is property of its copyright holder. All rights reserved.
//

#ifndef __INET_FuzzyROUTE_H_
#define __INET_FuzzyROUTE_H_

#include <omnetpp.h>
#include "IPv4Route.h"
#include "Fuzzydefs.h"

Fuzzy_NAMESPACE_BEGIN

/**
 * Fuzzy specific extra route data attached to routes in the routing table.
 */
class INET_API QLearnData : public cObject {
  private:
    bool isBroken;
    FuzzySequenceNumber sequenceNumber;
    simtime_t lastUsed;
    simtime_t expirationTime;
    FuzzyMetricType metricType;


  public:
    QLearnData();
    virtual ~QLearnData() { }

    bool getBroken() const { return isBroken; }
    void setBroken(bool isBroken) { this->isBroken = isBroken; }

    FuzzySequenceNumber getSequenceNumber() const { return sequenceNumber; }
    void setSequenceNumber(FuzzySequenceNumber sequenceNumber) { this->sequenceNumber = sequenceNumber; }

    simtime_t getLastUsed() const { return lastUsed; }
    void setLastUsed(simtime_t lastUsed) { this->lastUsed = lastUsed; }

    simtime_t getExpirationTime() const { return expirationTime; }
    void setExpirationTime(simtime_t expirationTime) { this->expirationTime = expirationTime; }

    FuzzyMetricType getMetricType() const { return metricType; }
    void setMetricType(FuzzyMetricType metricType) { this->metricType = metricType; }


};

Fuzzy_NAMESPACE_END

#endif
