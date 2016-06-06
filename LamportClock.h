#ifndef MENTAL_RETARDATION_LAMPORTCLOCK_H
#define MENTAL_RETARDATION_LAMPORTCLOCK_H


class LamportClock {
private:
    int clock;
public:
    LamportClock(){clock = 0;};
    int getClock(){return clock;};
    void incrementClock(){++clock;};
};


#endif //MENTAL_RETARDATION_LAMPORTCLOCK_H
