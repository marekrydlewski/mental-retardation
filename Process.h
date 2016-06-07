//
// Created by marek on 08.06.16.
//

#ifndef MENTAL_RETARDATION_PROCESS_H
#define MENTAL_RETARDATION_PROCESS_H


struct Process {
    Process(int c, int p): clock(c), processId(p) {};
    int processId;
    int clock;
    bool operator<(Process other) const
    {
        if (clock == other.clock)
            return processId < other.processId;
        else
            return clock < other.clock;
    } //fancy comparator
};


#endif //MENTAL_RETARDATION_PROCESS_H
