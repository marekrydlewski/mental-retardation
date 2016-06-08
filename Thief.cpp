//
// Created by marcin on 06.06.16.
//

#include "Thief.h"
#include "Message.h"
#include "RequestEnum.h"


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


int Thief::sendRequestToAll(int requestType, int info = -1) {
    Message msg;

    this->clock.incrementClock();
    msg.processId = this->processId;
    msg.clock = this->clock.getClock();
    msg.info = info;

    auto count = 0;
    for (auto i = 0; i < commSize; i++) {
        if (i != processId) {
            MPI_Send(&msg, 1, mpi_message_type, i, requestType, MPI_COMM_WORLD); //requestType is a tag
            ++count;
        }
    }
    return count;
}


int Thief::sendRequestToAvailable(int requestType, int info = -1) {
    Message msg;

    this->clock.incrementClock();
    msg.processId = this->processId;
    msg.clock = this->clock.getClock();
    msg.info = info;

    auto count = 0;
    for (auto i = 0; i < commSize; i++) {
        if (i != processId && !busyThieves[i]) { // nie wysyłamy do zajętych
            MPI_Send(&msg, 1, mpi_message_type, i, requestType, MPI_COMM_WORLD); //requestType is a tag
            ++count;
        }
    }
    return count;
}

std::vector<Process> Thief::getResponseFromAll(int requestType, int count) {
    MPI_Status status;
    Message msg;
    std::vector<Process> queueVec;
    for (auto i = 0; i < count; ++i) {
        MPI_Recv(&msg, 1, mpi_message_type, MPI_ANY_SOURCE, requestType, MPI_COMM_WORLD, &status);
        if (msg.clock > this->clock.getClock())
            this->clock.setClock(msg.clock);
        auto p = Process(msg.clock, msg.processId);
        queueVec.push_back(p); //vector queue
    }
    queueVec.push_back(Process(clock.getClock(), processId)); // add itself to queue
    std::sort(queueVec.begin(), queueVec.end()); //last item = biggest clock && rank
    return queueVec;
}

void Thief::enterHouseQueue() {
    int count = sendRequestToAvailable((int) RequestEnum::HOUSE_REQUEST);
    //
    queueHouses = getResponseFromAll((int) RequestEnum::HOUSE_REQUEST_ACK,
                                     count); // I'm using cast, because it's recommended

    if (queueHouses.back().clock == this->clock.getClock() && queueHouses.back().processId == this->processId) {
        if (isHouseFree() != -1) {
            houses[isHouseFree()] = false;
            this->sendRequestToAll((int) RequestEnum::ENTER_HOME, isHouseFree());
            this->robbingHome();
            houses[isHouseFree()] = true;
            this->sendRequestToAll((int) RequestEnum::HOME_FREE, isHouseFree());
        }
    }
    else {

        //waiting
        // TU JEBNIJMY WUCHTE RECEIVÓW
    }

    printf("It works!!!! - process %d of %d\n", processId, commSize);
}

void Thief::robbingHome() {
    std::mt19937 rnumber; // Mersenne Twister
    std::uniform_int_distribution<> u(700, 2000); //700-2000ms
    printf("Robbing in progress - process %d of %d\n", processId, commSize);
    sleep(u(rnumber));
    printf("Robbing ended - process %d of %d\n", processId, commSize);
}


int Thief::isHouseFree() {
    for (auto i = 0; i < houses.size(); ++i) {
        if (houses[i])
            return i;
    }
    return -1; //return the lowest free id
}

int Thief::getMyPosition() {
    for (auto i = 0; i < queueHouses.size(); ++i) {
        if (queueHouses[i].processId == processId && queueHouses[i].clock == clock.getClock())
            return queueHouses.size() - i;//returns 1 when i'm first, 2 when second etc.
    }
    return -1;
}

std::vector<int> Thief::getFreeHouses() {
    auto out = std::vector<int>();
    for (auto i = 0; i < houses.size(); ++i) {
        if (houses[i]) out.push_back(i);
    }
    return out;
}