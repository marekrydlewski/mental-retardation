
#include <iostream>
#include <mpi.h>

int main(int argc, char *argv[])
{
    int size, rank;

    MPI::Init(argc, argv);

    size = MPI::COMM_WORLD.Get_size();
    rank = MPI::COMM_WORLD.Get_rank();

    printf( "Hello world from process %d of %d\n", rank, size );
    MPI::Finalize();
    return 0;
}