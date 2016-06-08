//
// Created by marcin on 06.06.16.
//

#ifndef MENTAL_RETARDATION_THIEF_H
#define MENTAL_RETARDATION_THIEF_H

#include <mpi.h>
#include <vector>
#include <algorithm>
#include <random>
#include <unistd.h>
#include <iostream>

#include "LamportClock.h"
#include "Process.h"


class Thief {
private:
    int processId;
    int numberOfHouses;
    int commSize;
    int numberOfFences;
    std::vector<bool> houses;
    std::vector<bool> busyThieves;
    std::vector<Process> queueHouses;
    LamportClock clock;
    MPI_Datatype mpi_message_type;
public:
    Thief(int processId, int numberOfHouses, int numberOfFences, int commSize, MPI_Datatype mpi_message_type);
    int sendRequestToAll(int requestType, int info);
    int sendRequestToAvailable(int requestType, int info);
    int isHouseFree(); //returns id of first free house
    int getMyPosition();
    std::vector<Process> getResponseFromAll(int requestType,int count);
    std::vector<int> getFreeHouses();
    void enterHouseQueue();
    void robbingHome();
};


#endif //MENTAL_RETARDATION_THIEF_H
