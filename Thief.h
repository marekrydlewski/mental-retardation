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
#include <thread>
#include <chrono>

#include "LamportClock.h"
#include "Process.h"
#include "Message.h"


class Thief {
private:
    int processId;
    int numberOfHouses;
    int commSize;
    int numberOfFences;
    int status;
    int timestamp;
    std::vector<bool> houses;
    std::vector<bool> busyThieves;
    std::vector<Process> queueHouses;
    LamportClock clock;
    MPI_Datatype mpi_message_type;
public:
    Thief(int processId, int numberOfHouses, int numberOfFences, int commSize, MPI_Datatype mpi_message_type);
    int sendRequestToAll(int requestType, int info);
    int sendRequestToAvailable(int requestType, int info);
    int getLowestFreeHouseId(); //returns id of first free house
    int getMyPosition();
    std::vector<Process> getResponseFromAll(int requestType,int count);
    std::vector<int> getFreeHouses();
    void enterHouseQueue();
    void robbingHome();
    void respondToRequest(Message msg, int requestType);
    bool firstInQueue(){
        return (queueHouses.back().clock == timestamp && queueHouses.back().processId == this->processId);
    };
    void robbingHomeWithInfo();
};


#endif //MENTAL_RETARDATION_THIEF_H
