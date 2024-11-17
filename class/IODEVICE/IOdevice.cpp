#define TIME_TICK 10000; //10ms

#define IPC_KEY_TO_USER 1234
#define CPU_REPORT 0
#define IO_REPORT 1
#define CPU_DECREASE 2
#define IO_DECREASE 3
#define TERMINATE 4

#include "./IOdevice.h"
#include <sys/ipc.h>
#include <sys/msg.h>
#include <iostream>
#include <signal.h>
#include <sys/time.h>
#include <unistd.h>
using namespace std;

// 기본 생성자
IOdevice::IOdevice() : process(nullptr), time_slice(0) {}

// User 객체를 통해 프로세스를 할당
void IOdevice::assign_process(PCB* cur_process, int slice) {
    this->process = cur_process;
    time_slice = slice;
    cout << "IOdevice: Assigning process " << process->pid << " with time slice " << slice << " units.\n";
}

// IO 작업 처리 (tick 메시지 수신)
void IOdevice::tick() {
    if (process == nullptr) return; // 현재 실행 중인 프로세스가 없으면 무시
    // IPC 메시지 작성
    int msgid_to_user = msgget(IPC_KEY_TO_USER, 0666 | IPC_CREAT);

    IPCMessageToUser msg_to_user;
    msg_to_user.sender_pid = getpid();
    msg_to_user.receiver_pid = process->pid;

    if (time_slice == 0) {
        // 타임 슬라이스 만료 시: RUNQUEUE로 이동
        msg_to_user.mtype = IO_REPORT;
        cout << "IOdevice: Sending REPORT command to PID " << process->pid << "\n";

    } else {
        // 타임 슬라이스 진행 중: DECREASE 명령
        msg_to_user.mtype = IO_DECREASE;
        cout << "IOdevice: Sending IO_DECREASE command to PID " << process->pid << "\n";
    }

    // 메시지 전송
    if (msgsnd(msgid_to_user, &msg_to_user, sizeof(msg_to_user) - sizeof(long), 0) == -1) {
        cerr << "IOdevice: Failed to send message to User process " << process->pid << "\n";
    }
    // 타임 슬라이스 감소
    time_slice -= TIME_TICK;
    if (time_slice < 0) time_slice = 0;
}

// 현재 프로세스를 반환하고 CPU를 비웁니다.
PCB* IOdevice::release_process() {
    PCB* temp_process = process;
    process = nullptr;
    return temp_process;
}

// CPU가 비어 있는지 확인
bool IOdevice::is_idle() const {
    return process == nullptr;
}
