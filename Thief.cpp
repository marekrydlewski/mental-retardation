//
// Created by marcin on 06.06.16.
//

#include "Thief.h"

Thief::Thief(int processId, int numberOfHouses, int numberOfFences, int commSize)
{
    this->processId = processId;
    this->numberOfHouses = numberOfHouses;
    this->numberOfFences = numberOfFences;
    this->commSize = commSize;
    houses = new vector<bool>(numberOfHouses, true);
    clock = LamportClock();
}

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
    sendRequestToAll(this->processId, this->clock.getClock(), RequestEnum::ENTER_HOME, this->commSize);

}