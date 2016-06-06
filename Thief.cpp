//
// Created by marcin on 06.06.16.
//

#include "Thief.h"
#include "Message.h"
#include "RequestEnum.h"

Thief::Thief(int processId, int numberOfHouses, int numberOfFences, int commSize,
             MPI_Datatype mpi_message_type)
{
    this->processId = processId;
    this->numberOfHouses = numberOfHouses;
    this->numberOfFences = numberOfFences;
    this->commSize = commSize;
    this->mpi_message_type = mpi_message_type;
    std::vector<bool> tempVec (numberOfHouses, true);
    this->houses = tempVec;
    clock = LamportClock();
};


void Thief::sendRequestToAll(int requestType)
{
    Message msg;

    msg.processId = this->processId;
    msg.clock = this->clock.getClock();
    msg.requestType = requestType;

    for(int i = 0; i < commSize; i++)
    {
        if (i != processId) MPI_Send(&msg, 1, mpi_message_type, i, 0, MPI_COMM_WORLD);
    }
}

void Thief::enterHouseQueue()
{
    //sendRequestToAll(this->processId, this->clock.getClock(), RequestEnum::ENTER_HOME, this->commSize);
    //wut? bad signature
}