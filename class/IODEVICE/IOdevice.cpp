#define TIME_TICK 10000; //10ms

#define IPC_KEY_TO_IODEVICE 3456
#define IPC_KEY_TO_SCHEDULER 5678

#include "IOdevice.h"
#include "IPCMESSAGE/IPCMessage.h"
#include <sys/ipc.h>
#include <sys/msg.h>
#include <iostream>
using namespace std;

// 기본 생성자
IOdevice::IOdevice() : current_user(nullptr), time_slice(0) {}

// User 객체를 통해 프로세스를 할당
void IOdevice::assign_process(User* user, int slice) {
    current_user = user;
    time_slice = slice;
    cout << "IOdevice: Assigning process " << user->pid << " with time slice " << slice << " units.\n";
}

// IO 작업 처리 (tick 메시지 수신)
void IOdevice::tick() {
    int msgid_to_iodevice = msgget(IPC_KEY_TO_IODEVICE, 0666 | IPC_CREAT);
    int msgid_to_scheduler = msgget(IPC_KEY_TO_SCHEDULER, 0666 | IPC_CREAT);

    if (msgid_to_iodevice == -1 || msgid_to_scheduler == -1) {
        perror("msgget");
        return;
    }

    IPCMessageToIODevice msg;
    if (msgrcv(msgid_to_iodevice, &msg, sizeof(msg) - sizeof(long), 1, IPC_NOWAIT) != -1) {
        if (current_user == nullptr || current_user->pid != msg.pid) {
            cout << "IOdevice: No matching process for message.\n";
            return;
        }

        // IO 버스트 감소
        current_user->io_burst -= TIME_TICK;
        if (current_user->io_burst < 0) {
            current_user->io_burst = 0;
        }

        cout << "IOdevice: Process " << current_user->pid 
            << " tick received. io_burst=" << current_user->io_burst << "ms remaining.\n";

        // IO 완료 시 Scheduler로 메시지 전송
        if (current_user->io_burst == 0) {
            IPCMessageToScheduler response;
            response.mtype = 1;
            response.pid = current_user->pid;
            response.io_burst = current_user->io_burst;
            response.io_complete = true;

            if (msgsnd(msgid_to_scheduler, &response, sizeof(response) - sizeof(long), 0) == -1) {
                perror("msgsnd to scheduler");
                return;
            }

            cout << "IOdevice: Sent IO complete message to Scheduler for process " << current_user->pid << ".\n";
            current_user = nullptr;  // 프로세스 해제
        }
    }
}

// 현재 프로세스를 반환하고 CPU를 비웁니다.
User* IOdevice::release_process() {
    User* temp_user = current_user;
    current_user = nullptr;
    return temp_user;
}

// CPU가 비어 있는지 확인
bool IOdevice::is_idle() const {
    return current_user == nullptr;
}
