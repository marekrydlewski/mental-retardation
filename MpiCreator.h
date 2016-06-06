//
// Created by marek on 06.06.16.
//

#ifndef MENTAL_RETARDATION_MPICREATOR_H
#define MENTAL_RETARDATION_MPICREATOR_H

#include <mpi.h>
#include <stdio.h>
#include <cstddef>

class MpiCreator {
public:
    static MpiCreator& getInstance() {
        static MpiCreator instance;
        return instance;
    }
private:
    MpiCreator() { };
public:
    MpiCreator(MpiCreator const &) = delete;
    void operator=(MpiCreator const &) = delete;
};

#endif //MENTAL_RETARDATION_MPICREATOR_H
