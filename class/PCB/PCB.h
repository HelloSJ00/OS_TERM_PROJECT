#ifndef PCB_H
#define PCB_H

#include <sys/types.h>
#include "../USER/User.h" // User 클래스 선언 포함

enum ProcessState { READY, RUNNING,IO, WAITING, TERMINATED };

class PCB {
public:
    pid_t pid;
    int cpu_burst;
    int io_burst;
    int priority;
    int waitingTime;
    ProcessState state;

    PCB(int p, int cpu, int io)
        : pid(p), cpu_burst(cpu), io_burst(io), priority(0),waitingTime(0), state(READY) {}
};

#endif
