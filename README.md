# mental-retardation - distributed computing project
Thieves rob houses in distributed environment

# how to launch

mpic++ -std=c++0x main_mpi.cpp -o retards.out Thief.cpp
mpirun -np 2 ./retards.out
