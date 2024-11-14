// User.h
#ifndef USER_H
#define USER_H

#include <iostream>
#include "PCB.h"
#include <sys/types.h>  // pid_t 타입을 위해 필요

class User {
public:
    pid_t pid;           // 프로세스 ID
    int cpu_burst;       // CPU 버스트 시간
    int io_burst;        // I/O 버스트 시간
    PCB pcb;             // PCB 객체로 상태와 우선순위를 관리

    // 생성자에서 PID, CPU 버스트, IO 버스트를 받아 초기화
    User(pid_t p, int cpu, int io)
        : pid(p), cpu_burst(cpu), io_burst(io), pcb(p, cpu, io, this) {}

    // 프로세스 상태 확인
    ProcessState getProcessState() const {
        return pcb.state;
    }

    // 프로세스 상태 설정
    void setProcessState(ProcessState newState) {
        pcb.state = newState;
    }

    // 프로세스에 우선순위 할당
    void setPriority(int priority) {
        pcb.priority = priority;
    }

    // 프로세스 정보 출력 (디버깅용)
    void printProcessInfo() const {
        std::cout << "PID: " << pid 
                  << ", CPU Burst: " << cpu_burst 
                  << ", IO Burst: " << io_burst 
                  << ", Priority: " << pcb.priority 
                  << ", State: " << pcb.state << std::endl;
    }
};

#endif
