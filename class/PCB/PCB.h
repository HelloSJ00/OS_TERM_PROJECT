#ifndef PCB_H
#define PCB_H

#include <sys/types.h>
#include "User.h" // User 클래스 선언 포함

enum ProcessState { READY, RUNNING, WAITING, TERMINATED };

class PCB {
public:
    pid_t pid;
    int cpu_burst;
    int io_burst;
    int priority;
    ProcessState state;
    User* user;  // User 객체에 대한 포인터

    PCB(int p, int cpu, int io, User* u)
        : pid(p), cpu_burst(cpu), io_burst(io), priority(0), state(READY), user(u) {}
};

#endif
