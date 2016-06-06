
#include <mpi.h>
#include <stdio.h>
#include <vector>
#include <cstddef>
#include "LamportClock.h"
#include "Message.h"

const int numberOfHouses = 10;
const int numberOfFences = 5;

void enterHouseQueue()
{

}

int main (int argc, char* argv[])
{
    int rank, size;

    MPI_Init (&argc, &argv);      /* starts MPI */
    MPI_Comm_rank (MPI_COMM_WORLD, &rank);        /* get current process id */
    MPI_Comm_size (MPI_COMM_WORLD, &size);        /* get number of processes */

    const int numberOfItems = 3;
    int blockLengths[3] = {1,1,1};
    MPI_Datatype types[3] = {MPI_INT, MPI_INT, MPI_INT};
    MPI_Datatype mpi_message_type;
    MPI_Aint offsets[3];

    offsets[0] = offsetof(Message, clock);
    offsets[1] = offsetof(Message, processId);
    offsets[2] = offsetof(Message, requestType);

    MPI_Type_create_struct(numberOfItems, blockLengths, offsets, types, &mpi_message_type);
    MPI_Type_commit(&mpi_message_type);

    printf( "Hello world from process %d of %d\n", rank, size );
    
    std::vector<bool> houses(numberOfHouses, true);
    int fences = numberOfFences;
    auto clock = LamportClock();


    MPI_Finalize();
    return 0;
}