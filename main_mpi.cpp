#include <mpi.h>
#include <stdio.h>
#include <iostream>
#include "Message.h"
#include "Thief.h"
#include <cstddef>
#include "LamportClock.h"
#include "Thief.h"
#include "RequestEnum.h"

const int numberOfHouses = 10;
const int numberOfFences = 5;

MPI_Datatype mpi_message_type;

MPI_Datatype initMpiStruct() {
    const int numberOfItems = 4;
    int blockLengths[4] = {1, 1, 1, 1};
    MPI_Datatype mpi_message_type;
    MPI_Datatype types[4] = {MPI_INT, MPI_INT, MPI_INT, MPI_INT};
    MPI_Aint offsets[4];

    offsets[0] = offsetof(Message, clock);
    offsets[1] = offsetof(Message, processId);
    offsets[2] = offsetof(Message, info);
    offsets[3] = offsetof(Message, timestamp);


    MPI_Type_create_struct(numberOfItems, blockLengths, offsets, types, &mpi_message_type);
    MPI_Type_commit(&mpi_message_type);
    return mpi_message_type;
}

void test(int rank) {
    MPI_Status status;
    Message msg;
    if (rank == 0) {
        msg.clock = 20;
        msg.processId = rank;
        msg.info = RequestEnum::HOME_FREE;

        MPI_Send(&msg, 1, mpi_message_type, 1, 0, MPI_COMM_WORLD);
    }
    else if (rank == 1) {
        MPI_Recv(&msg, 1, mpi_message_type, 0, 0, MPI_COMM_WORLD, &status);
        std::cout << "From " << msg.processId << ", info " << msg.info << ", clock " << msg.clock << std::endl;
    }
}

int main (int argc, char* argv[])
{
    // starting
    int rank, size;
    MPI_Init(&argc, &argv);      /* starts MPI */
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);        /* get current process id */
    MPI_Comm_size(MPI_COMM_WORLD, &size);        /* get number of processes */
    srand( time( NULL ) * rank);
    mpi_message_type = initMpiStruct();
    auto thief = Thief(rank, numberOfHouses, numberOfFences, size, mpi_message_type);
    while(1)
    thief.enterHouseQueue();

    MPI_Finalize();
    return 0;
}