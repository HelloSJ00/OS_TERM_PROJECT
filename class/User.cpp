#include "User.h"
#include <iostream>
using namespace std;

// 생성자
User::User(pid_t p, int cpu, int io)
    : pid(p), cpu_burst(cpu), io_burst(io), pcb(p, cpu, io, this) {}

// 프로세스 상태 확인
ProcessState User::getProcessState() const {
    return pcb.state;
}

// 프로세스 상태 설정
void User::setProcessState(ProcessState newState) {
    pcb.state = newState;
}

// 프로세스에 우선순위 할당
void User::setPriority(int priority) {
    pcb.priority = priority;
}

// 프로세스 정보 출력 (디버깅용)
void User::printProcessInfo() const {
    cout << "PID: " << pid
    << ", CPU Burst: " << cpu_burst
    << ", IO Burst: " << io_burst
    << ", Priority: " << pcb.priority
    << ", State: " << pcb.state << endl;
}
