//
// Created by marcin on 06.06.16.
//

#ifndef MENTAL_RETARDATION_THIEF_H
#define MENTAL_RETARDATION_THIEF_H

#include <mpi.h>
#include <vector>
#include "LamportClock.h"
#include "Message.h"

class Thief {
private:
    int processId;
    int numberOfHouses;
    int commSize;
    int numberOfFences;
    std::vector<bool> houses;
    std::vector<int> busyThieves;
    LamportClock clock;
public:
    Thief(int processId, int numberOfHouses, int numberOfFences, int commSize);
    void sendRequestToAll(int requestType);
    void enterHouseQueue();
};


#endif //MENTAL_RETARDATION_THIEF_H
