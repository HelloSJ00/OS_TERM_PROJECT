#define TIME_TICK 10000; //10ms

// IPC 키 설정
#define IPC_KEY_TO_USER 1234
#define CPU_REPORT 0
#define IO_REPORT 1
#define CPU_DECREASE 2
#define IO_DECREASE 3
#define TERMINATE 4

#include "./CPU.h"
#include <sys/ipc.h>
#include <sys/msg.h>
#include <iostream>
#include <signal.h>
#include <sys/time.h>
#include <unistd.h>
#include <cstring>

using namespace std;

// 기본 생성자
CPU::CPU() : process(nullptr), time_slice(0) {}

void CPU::assign_process(PCB* cur_process,int slice){
    this->process = cur_process;
    time_slice = slice;
    cout << "CPU: Executing process " << process->pid << " with time slice " << slice << " units.\n";
}

void CPU::tick() {
    if (process == nullptr) return; // 현재 실행 중인 프로세스가 없으면 무시
    // IPC 메시지 작성
    int msgid_to_user = msgget(IPC_KEY_TO_USER, 0666 | IPC_CREAT);

    IPCMessageToUser msg_to_user;        
    msg_to_user.sender_pid = getpid();
    msg_to_user.receiver_pid = process->pid;

    if (time_slice == 0) {
        // 타임 슬라이스 만료 시: REPORT 명령
        msg_to_user.mtype = CPU_REPORT; // REPORT는 #define으로 정의된 정수
        cout << "CPU: Sending REPORT command to PID " << process->pid << "\n";
    } else {
        // 타임 슬라이스 진행 중: DECREASE 명령
        msg_to_user.mtype = CPU_DECREASE; // REPORT는 #define으로 정의된 정수
        cout << "CPU: Sending CPU_DECREASE command to PID " << process->pid << "\n";
    }

    // 메시지 전송
    if (msgsnd(msgid_to_user, &msg_to_user, sizeof(msg_to_user) - sizeof(long), 0) == -1) {
        cerr << "CPU: Failed to send message to User process " << process->pid << "\n";
    }
    // 타임 슬라이스 감소
    time_slice -= TIME_TICK;
    if (time_slice < 0) time_slice = 0;
}

// 현재 프로세스를 반환하고 CPU를 비웁니다.
PCB* CPU::release_process() {
    PCB* temp_process = process;
    process = nullptr;
    return temp_process;
}

bool CPU::is_idle() const {
    return process== nullptr;
}

int CPU::getTimeSlice(){
    return time_slice;
}