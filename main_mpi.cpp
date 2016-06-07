
#include <mpi.h>
#include <stdio.h>
#include <iostream>
#include "Message.h"
#include "Thief.h"

const int numberOfHouses = 10;
const int numberOfFences = 5;

MPI_Datatype mpi_message_type;

MPI_Datatype initMpiStruct()
{
    const int numberOfItems = 3;
    int blockLengths[3] = {1,1,1};
    MPI_Datatype mpi_message_type;
    MPI_Datatype types[3] = {MPI_INT, MPI_INT, MPI_INT};
    MPI_Aint offsets[3];

    offsets[0] = offsetof(Message, clock);
    offsets[1] = offsetof(Message, processId);
    offsets[2] = offsetof(Message, requestType);

    MPI_Type_create_struct(numberOfItems, blockLengths, offsets, types, &mpi_message_type);
    MPI_Type_commit(&mpi_message_type);
    return mpi_message_type;
}

int main (int argc, char* argv[])
{
    // starting
    int rank, size;
    MPI_Init (&argc, &argv);      /* starts MPI */
    MPI_Comm_rank (MPI_COMM_WORLD, &rank);        /* get current process id */
    MPI_Comm_size (MPI_COMM_WORLD, &size);        /* get number of processes */
    printf( "Hello world from process %d of %d\n", rank, size );
    mpi_message_type = initMpiStruct();

    auto thief = Thief(rank, numberOfHouses, numberOfFences, size, mpi_message_type);


    MPI_Finalize();
    return 0;
}