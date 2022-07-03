#ifndef TRANSACTIONDATA_H_
#define TRANSACTIONDATA_H_

#include<string.h>
#include<omnetpp.h>
class TransactionData{
public:
    std::string skey;
    time_t timestamp;

    TransactionData(){};

    TransactionData(std::string key, time_t time){
        skey = key;
        timestamp = time;
    };
};



#endif
