//
// Created by marcin on 06.06.16.
//

#include "Thief.h"
#include "Message.h"
#include "RequestEnum.h"
#include "StatusEnum.h"


Thief::Thief(int processId, int numberOfHouses, int numberOfFences, int commSize,
             MPI_Datatype mpi_message_type) {
    this->processId = processId;
    this->numberOfHouses = numberOfHouses;
    this->numberOfFences = numberOfFences;
    this->commSize = commSize;
    this->mpi_message_type = mpi_message_type;
    this->status = StatusEnum::BUSY;

    std::vector<bool> tempVec(numberOfHouses, true);
    this->houses = tempVec;
    std::vector<bool> tempVec2(numberOfHouses, false);
    this->busyThieves = tempVec2;

    this->timestamp = 0;
    clock = LamportClock();
}


int Thief::sendRequestToAll(int requestType, int info = -1) {
    Message msg;

    msg.processId = this->processId;
    msg.info = info;
    msg.timestamp = timestamp;

    auto count = 0;
    for (auto i = 0; i < commSize; i++) {
        if (i != processId) {
            this->clock.incrementClock();
            msg.clock = this->clock.getClock();
            MPI_Send(&msg, 1, mpi_message_type, i, requestType, MPI_COMM_WORLD); //requestType is a tag
            ++count;
        }
    }
    return count;
}


int Thief::sendRequestToModulo(int requestType, int info = -1) {
    Message msg;

    msg.processId = this->processId;
    msg.info = info;
    msg.timestamp = timestamp;

    auto count = 0;
    auto mod = processId % (numberOfHouses - 1);
    for (auto i = mod; i < commSize; i+=numberOfHouses) {
        if (i != processId) {
            this->clock.incrementClock();
            msg.clock = this->clock.getClock();
            MPI_Send(&msg, 1, mpi_message_type, i, requestType, MPI_COMM_WORLD); //requestType is a tag
            ++count;
        }
    }
    return count;
}

std::vector<Process> Thief::getResponseFromAll(int requestType, int count) {
    Message msg;
    MPI_Status status;
    int processedRequests = 0;
    std::vector<Process> queueVec;

    while (processedRequests < count) {
        this->clock.incrementClock();
        MPI_Recv(&msg, 1, mpi_message_type, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        if (msg.clock > this->clock.getClock())
            this->clock.setClock(msg.clock + 1);

        if (status.MPI_TAG == requestType) {
            if (msg.info == this->status) {
                auto p = Process(msg.timestamp, msg.processId);
                queueVec.push_back(p); //vector queue
            }
            processedRequests++;
        }
        else {
            for (auto &el : queueVec) {
                if (el.processId == msg.processId)
                    el.clock = msg.timestamp;
            }
            respondToRequest(msg, status.MPI_TAG);
        }
    }

    queueVec.push_back(Process(timestamp, processId)); // add itself to queue
    std::sort(queueVec.begin(), queueVec.end(), std::greater<Process>()); //last item = lowest clock && rank
    return queueVec;
}

void Thief::enterHouseQueue() {
    this->status = StatusEnum::HOUSE_QUEUED;
    ++timestamp;
    printf("Lamport %d - %d: Process %d, (%d) requesting house\n", clock.getClock(), timestamp, processId,
           RequestEnum::HOUSE_REQUEST);
    int count = sendRequestToModulo((int) RequestEnum::HOUSE_REQUEST);
    queueHouses = getResponseFromAll((int) RequestEnum::HOUSE_REQUEST_ACK, count);
    auto pos = getMyPosition(queueHouses);
    if (pos != -1 && pos <= numberOfHouses && houses[pos - 1]) {
        printf("Lamport % d - %d: Process %d, (%d) in position to enter house\n", clock.getClock(), timestamp,
               processId, RequestEnum::HOUSE_REQUEST_ACK);
        robbingHomeWithInfo();
    }
    else {
        printf("Lamport %d - %d: Process %d, (%d) waiting for position allowing to get into house\n", clock.getClock(),
               timestamp, processId, RequestEnum::HOUSE_REQUEST_ACK);
        bool flag = false;
        Message msg;
        MPI_Status status;
        while (!flag) {
            this->clock.incrementClock();
            MPI_Recv(&msg, 1, mpi_message_type, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

            if (msg.clock > this->clock.getClock())
                this->clock.setClock(msg.clock + 1);

            respondToRequest(msg, status.MPI_TAG);
            auto pos = getMyPosition(queueHouses);
            if (pos != -1 && pos <= numberOfHouses && houses[pos - 1]) // if not first, continue loop
                flag = true; // in that place we should update busy houses etc.
        }
        printf("Lamport %d - %d: Process %d, (%d) in position to enter house\n", clock.getClock(), timestamp, processId,
               RequestEnum::HOUSE_REQUEST_ACK);
        robbingHomeWithInfo();
    }
}

void Thief::pause() {
    auto s = rand() % 2000 + 1000;
    std::this_thread::sleep_for(std::chrono::milliseconds(s));
}

int Thief::getMyPosition(std::vector<Process> queue) {
    for (auto i = 0; i < queue.size(); ++i) {
        if (queue[i].processId == processId && queue[i].clock == timestamp)
            return queue.size() - i;//returns 1 when i'm first, 2 when second etc.
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

int Thief::getLowestFreeHouseId() {
    auto h = houses[processId % (numberOfHouses )];
    if (h) return processId % (numberOfHouses );
    return -1; //return the lowest free id
}

void Thief::respondToRequest(Message msg, int requestType) {
    Message response;
    if (requestType == (int) RequestEnum::HOUSE_REQUEST) {
        bool update = false;
        for (auto &el : queueHouses) {
            if (el.processId == msg.processId) {
                el.clock = msg.timestamp;
                update = true;
                break;
            }
        }
        if (!update) {
            auto p = Process(msg.timestamp, msg.processId);
            queueHouses.push_back(p); //vector queue
        }
        std::sort(queueHouses.begin(), queueHouses.end(), std::greater<Process>()); //last item = lowest clock && rank

        this->clock.incrementClock();
        response.processId = this->processId;
        response.clock = this->clock.getClock();
        response.info = this->status;
        response.timestamp = timestamp;
        MPI_Send(&response, 1, mpi_message_type, msg.processId, (int) RequestEnum::HOUSE_REQUEST_ACK, MPI_COMM_WORLD);
    }
    else if (requestType == RequestEnum::ENTER_HOME) {
        this->houses.at(msg.info) = false;
    }
    else if (requestType == RequestEnum::HOME_FREE) {
        this->houses.at(msg.info) = true;
        // intelligent pop back xD
        queueHouses.erase(std::remove_if(queueHouses.begin(), queueHouses.end(),
                                         [&msg](const Process &p) { return p.processId == msg.processId; }),
                          queueHouses.end());
        //queueHouses.pop_back();
    }
    else if (requestType == RequestEnum::FENCE_REQUEST) {
        bool update = false;
        for (auto &el : queueFences) {
            if (el.processId == msg.processId) {
                el.clock = msg.timestamp;
                update = true;
                break;
            }
        }
        if (!update) {
            auto p = Process(msg.timestamp, msg.processId);
            queueFences.push_back(p); //vector queue
        }
        std::sort(queueFences.begin(), queueFences.end(), std::greater<Process>()); //last item = lowest clock && rank

        this->clock.incrementClock();
        response.processId = this->processId;
        response.clock = this->clock.getClock();
        response.info = this->status;
        response.timestamp = timestamp;
        MPI_Send(&response, 1, mpi_message_type, msg.processId, (int) RequestEnum::FENCE_REQUEST_ACK, MPI_COMM_WORLD);
    }
    else if (requestType == RequestEnum::ENTER_FENCE) {
        this->numberOfFences--;
    }
    else if (requestType == RequestEnum::FENCE_FREE) {
        this->numberOfFences++;
        queueFences.erase(std::remove_if(queueFences.begin(), queueFences.end(),
                                         [&msg](const Process &p) { return p.processId == msg.processId; }),
                          queueFences.end());
        //queueFences.pop_back();
    }
}

void Thief::robbingHomeWithInfo() {
    int homeId = getMyPosition(queueHouses);
    printf("position: %d\n", homeId);
    houses.at(homeId) = false;
    ++timestamp;
    printf("Lamport %d - %d: Process %d, (%d) entering free house %d\n", clock.getClock(), timestamp, processId,
           RequestEnum::ENTER_HOME, homeId);
    this->sendRequestToAll((int) RequestEnum::ENTER_HOME, homeId);
    this->status = StatusEnum::BUSY;
    this->pause();
    enterFenceQueue();
    houses.at(homeId) = true;
    ++timestamp;
    printf("Lamport %d - %d: Process %d, (%d) leaving house %d\n", clock.getClock(), timestamp, processId,
           RequestEnum::HOME_FREE, homeId);
    this->sendRequestToAll((int) RequestEnum::HOME_FREE, homeId);

}

void Thief::enterFenceQueue() {
    this->status = StatusEnum::FENCE_QUEUED;
    ++timestamp;
    printf("Lamport %d - %d: Process %d, (%d) awaiting free fence\n", clock.getClock(), timestamp, processId,
           RequestEnum::FENCE_REQUEST);
    int count = sendRequestToAll((int) RequestEnum::FENCE_REQUEST);
    queueFences = getResponseFromAll((int) RequestEnum::FENCE_REQUEST_ACK, count);
    if (getMyPosition(queueFences) <= numberOfFences) {
        doingBusiness();
    }
    else {
        bool flag = false;
        Message msg;
        MPI_Status status;
        printf("Lamport %d - %d: Process %d, (%d) fence not yet available, waiting\n", clock.getClock(), timestamp,
               processId, RequestEnum::FENCE_REQUEST_ACK);
        while (!flag) {
            this->clock.incrementClock();
            MPI_Recv(&msg, 1, mpi_message_type, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

            if (msg.clock > this->clock.getClock())
                this->clock.setClock(msg.clock + 1);

            respondToRequest(msg, status.MPI_TAG);

            if (getMyPosition(queueFences) <= numberOfFences) // if not first, continue loop
                flag = true; // in that place we should update busy houses etc.
        }
        doingBusiness();
    }
}

void Thief::doingBusiness() {

    numberOfFences--;
    ++timestamp;
    printf("Lamport %d - %d: Process %d, (%d) entering free fence\n", clock.getClock(), timestamp, processId,
           RequestEnum::ENTER_FENCE);
    this->sendRequestToAll((int) RequestEnum::ENTER_FENCE);
    this->status = StatusEnum::BUSY;
    this->pause();
    numberOfFences++;
    ++timestamp;
    printf("Lamport %d - %d: Process %d, (%d) in position to get fence\n", clock.getClock(), timestamp, processId,
           RequestEnum::FENCE_FREE);
    this->sendRequestToAll((int) RequestEnum::FENCE_FREE);
}