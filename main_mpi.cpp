#include <mpi.h>
#include <stdio.h>
#include <iostream>
#include "Message.h"
#include "Thief.h"
#include <cstddef>

int numberOfHouses = 7;
int numberOfFences = 8;

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

int main (int argc, char* argv[])
{
    // starting
    int rank, size;
    MPI_Init(&argc, &argv);      /* starts MPI */
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);        /* get current process id */
    MPI_Comm_size(MPI_COMM_WORLD, &size);        /* get number of processes */
    if (argc != 1)
    {
        for (auto i = 1; i < argc; ++i) {
            std::string arg = argv[i];
            if (arg == "--help") {
                if (rank == 0) std::cout << "\t" << "-h/--houses: number of houses, -f/--fences: number of fences" << std::endl;
                exit(0);
            } else if ((arg == "-h") || (arg == "--houses")) {
                if (i + 1 < argc) {
                    numberOfHouses = atoi(argv[i+1]);
                }
            } else if ((arg == "-f") || (arg == "--fences")) {
                if (i + 1 < argc) {
                    numberOfFences = atoi(argv[i+1]);
                }
            }
        }
    }
    else if (rank == 0)
    {
        std::cout<<"Run with default params: houses - " << numberOfHouses << " " << "fences - " << numberOfFences << std::endl;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    srand( time( NULL ) * rank);
    mpi_message_type = initMpiStruct();
    auto thief = Thief(rank, numberOfHouses, numberOfFences, size, mpi_message_type);
    while(1)
    thief.enterHouseQueue();

    MPI_Finalize();
    return 0;
}