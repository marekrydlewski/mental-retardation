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


int Thief::sendRequestToAvailable(int requestType, int info = -1) {
    Message msg;

    msg.processId = this->processId;
    msg.info = info;
    msg.timestamp = timestamp;

    auto count = 0;
    for (auto i = 0; i < commSize; i++) {
        if (i != processId && !busyThieves[i]) { // nie wysyłamy do zajętych

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

    while(processedRequests < count)
    {
        this->clock.incrementClock();
        MPI_Recv(&msg, 1, mpi_message_type, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        if (msg.clock > this->clock.getClock())
            this->clock.setClock(msg.clock + 1);

        if(status.MPI_TAG == requestType)
        {
            if(msg.info == this->status)
            {
                auto p = Process(msg.timestamp, msg.processId);
                queueVec.push_back(p); //vector queue
            }
            processedRequests++;
            //printf("Process %d received clock %d from process %d\n", this->processId, msg.clock, msg.processId);
        }
        else
        {
            for(auto &el : queueVec)
            {
                if(el.processId == msg.processId)
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
    /*
    int count = sendRequestToAvailable((int) RequestEnum::HOUSE_REQUEST);
    //
    queueHouses = getResponseFromAll((int) RequestEnum::HOUSE_REQUEST_ACK,
                                     count); // I'm using cast, because it's recommended

    if (queueHouses.back().clock == this->clock.getClock() && queueHouses.back().processId == this->processId) {
        if (getLowestFreeHouseId() != -1) {
            houses[getLowestFreeHouseId()] = false;
            this->sendRequestToAll((int) RequestEnum::ENTER_HOME, getLowestFreeHouseId());
            this->robbingHome();
            houses[getLowestFreeHouseId()] = true;
            this->sendRequestToAll((int) RequestEnum::HOME_FREE, getLowestFreeHouseId());
        }
    }
    else {

        //waiting
        // TU JEBNIJMY WUCHTE RECEIVÓW
    }

    printf("It works!!!! - process %d of %d\n", processId, commSize);
     */
    this->status = StatusEnum::HOUSE_QUEUED;
    ++timestamp;
    int count = sendRequestToAll((int) RequestEnum::HOUSE_REQUEST);
    queueHouses = getResponseFromAll((int) RequestEnum::HOUSE_REQUEST_ACK, count);
    printf("Process %d clock %d : first in queue pid %d clock %d \n", this->processId, timestamp, queueHouses.back().processId, queueHouses.back().clock);
    if(firstInQueue())
    {
        robbingHomeWithInfo();
    }
    else {
        bool flag = false;
        Message msg;
        MPI_Status status;
        while(!flag)
        {
            this->clock.incrementClock();
            MPI_Recv(&msg, 1, mpi_message_type, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

            if (msg.clock > this->clock.getClock())
                this->clock.setClock(msg.clock + 1);

            respondToRequest(msg, status.MPI_TAG);

            if((firstInQueue())) // if not first, continue loop
                flag = true; // in that place we should update busy houses etc.
        }
        robbingHomeWithInfo();
    }
}

void Thief::robbingHome() {
    auto s = rand() % 2000 + 1000;
    printf("Robbing in progress - process %d of %d sleep %d\n", processId, commSize,s );
    std::this_thread::sleep_for(std::chrono::milliseconds(s));
    printf("Robbing ended - process %d of %d\n", processId, commSize);
}


int Thief::getLowestFreeHouseId() {
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

void Thief::respondToRequest(Message msg, int requestType)
{
    Message response;
    if(requestType == (int) RequestEnum::HOUSE_REQUEST)
    {
        bool update = false;
        for(auto &el : queueHouses)
        {
            if(el.processId == msg.processId)
            {
                el.clock = msg.timestamp;
                update = true;
                break;
            }
        }
        if(!update)
        {
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
    else if(requestType == RequestEnum::ENTER_HOME)
    {
        printf("Enter home got it! - process %d of %d\n", processId, commSize);
        this->houses[msg.info] = false;
    }
    else if(requestType == RequestEnum::HOME_FREE) {
        printf("Home free got it! - process %d of %d\n", processId, commSize);
        this->houses[msg.info] = true;
        queueHouses.pop_back();
    }
    else if(requestType == RequestEnum::FENCE_REQUEST)
    {
        bool update = false;
        for(auto &el : queueFences)
        {
            if(el.processId == msg.processId)
            {
                el.clock = msg.timestamp;
                update = true;
                break;
            }
        }
        if(!update)
        {
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
    else if(requestType == RequestEnum::ENTER_FENCE)
    {
        this->numberOfFences--;
    }
    else if(requestType == RequestEnum::FENCE_FREE)
    {
        this->numberOfFences++;
        queueFences.pop_back();
    }
}

void Thief::robbingHomeWithInfo() {
    int homeId = getLowestFreeHouseId();
    if (homeId != -1) {
        houses[homeId] = false;
        ++timestamp;
        this->sendRequestToAll((int) RequestEnum::ENTER_HOME, homeId);
        this->status = StatusEnum::BUSY;
        this->robbingHome();
        houses[homeId] = true;
        ++timestamp;
        this->sendRequestToAll((int) RequestEnum::HOME_FREE, homeId);
    }
}

void Thief::enterFenceQueue() {

}