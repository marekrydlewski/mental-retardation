//
// Created by marcin on 06.06.16.
//

#include "Thief.h"
#include "Message.h"
#include "RequestEnum.h"
#include <random>
#include <unistd.h>

Thief::Thief(int processId, int numberOfHouses, int numberOfFences, int commSize,
             MPI_Datatype mpi_message_type) {
    this->processId = processId;
    this->numberOfHouses = numberOfHouses;
    this->numberOfFences = numberOfFences;
    this->commSize = commSize;
    this->mpi_message_type = mpi_message_type;

    std::vector<bool> tempVec(numberOfHouses, true);
    this->houses = tempVec;
    std::vector<bool> tempVec2(numberOfHouses, false);
    this->busyThieves = tempVec2;

    clock = LamportClock();
}


int Thief::sendRequestToAll(int requestType) {
    Message msg;

    msg.processId = this->processId;
    msg.clock = this->clock.getClock();
    msg.requestType = requestType;

    auto count = 0;
    for (auto i = 0; i < commSize; i++) {
        if (i != processId && !busyThieves[i]) { // nie wysyłamy do zajętych
            MPI_Send(&msg, 1, mpi_message_type, i, requestType, MPI_COMM_WORLD); //requestType is a tag
            ++count;
        }
    }
    return count;
}

void Thief::getResponseFromAll(int requestType, int count) {
    MPI_Status status;
    Message msg;
    for (auto i = 0; i < count; ++i) {
        MPI_Recv(&msg, 1, mpi_message_type, MPI_ANY_SOURCE, requestType, MPI_COMM_WORLD, &status);
        //accepts from any source messages with tag requestType
        //taki jest plan xD
        //jakiś Lamport tutaj i kolejka???? tudzież wektor
    }
}

void Thief::enterHouseQueue() {
    int count = sendRequestToAll((int) RequestEnum::ENTER_HOME);
    getResponseFromAll((int) RequestEnum::ENTER_HOME, count);
}

void Thief::robbingHome() {
    std::mt19937 rnumber; // Mersenne Twister
    std::uniform_int_distribution<unsigned> u (700, 2000); //700-2000ms
    sleep(u(rnumber));
}