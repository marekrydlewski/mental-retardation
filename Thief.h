//
// Created by marcin on 06.06.16.
//

#ifndef MENTAL_RETARDATION_THIEF_H
#define MENTAL_RETARDATION_THIEF_H

#include <mpi.h>
#include <vector>
#include "LamportClock.h"


class Thief {
private:
    int processId;
    int numberOfHouses;
    int commSize;
    int numberOfFences;
    std::vector<bool> houses;
    std::vector<bool> busyThieves;
    LamportClock clock;
    MPI_Datatype mpi_message_type;
public:
    Thief(int processId, int numberOfHouses, int numberOfFences, int commSize, MPI_Datatype mpi_message_type);
    int sendRequestToAll(int requestType);
    void getResponseFromAll(int requestType,int count);
    void enterHouseQueue();
};


#endif //MENTAL_RETARDATION_THIEF_H
