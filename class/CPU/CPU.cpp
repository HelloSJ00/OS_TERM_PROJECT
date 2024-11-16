#define TIME_TICK 10000; //10ms

// IPC 키 설정
#define IPC_KEY_TO_CPU 1234
#define IPC_KEY_TO_SCHEDULER 5678

#include <iostream>
#include "IPCMessage.h"
#include <sys/ipc.h>
#include <sys/msg.h>
#include "PCB.h"
#include "CPU.h"
#include "User.h"  // User 클래스 포함
using namespace std;

// 기본 생성자
CPU::CPU() : current_user(nullptr), time_slice(0) {}

void CPU::assign_process(User* user,int slice){
    current_user = user;
    time_slice = slice;
    cout << "CPU: Executing process " << user->pid << " with time slice " << slice << " units.\n";
}

void CPU::tick() {
    int msgid_to_cpu = msgget(IPC_KEY_TO_CPU, 0666 | IPC_CREAT);
    int msgid_to_scheduler = msgget(IPC_KEY_TO_SCHEDULER, 0666 | IPC_CREAT);

    if (msgid_to_cpu == -1 || msgid_to_scheduler == -1) {
        perror("msgget");
        return;
    }

    IPCMessageToCPU msg;
    if (msgrcv(msgid_to_cpu, &msg, sizeof(msg) - sizeof(long), 1, IPC_NOWAIT) != -1) {
        // 메시지 수신 성공
        if (current_user == nullptr || current_user->pid != msg.pid) {
            cout << "CPU: No matching process for message.\n";
            return;
        }

        // CPU burst 감소
        current_user->cpu_burst -= TIME_TICK;
        if (current_user->cpu_burst < 0) current_user->cpu_burst = 0;

        // 타임 슬라이스 감소
        time_slice -= TIME_TICK;
        if (time_slice < 0) time_slice = 0;

        cout << "CPU: Process " << current_user->pid
            << " tick received. cpu_burst=" << current_user->cpu_burst
            << ", time_slice=" << time_slice << "ms remaining.\n";

        // 조건에 따라 Scheduler로 메시지 전송
        if (current_user->cpu_burst == 0 || time_slice == 0) {
            IPCMessageToScheduler response;
            response.mtype = 1;
            response.pid = current_user->pid;
            response.cpu_burst = current_user->cpu_burst;
            response.io_burst = current_user->io_burst;
            response.time_slice_expired = (time_slice == 0);

            if (msgsnd(msgid_to_scheduler, &response, sizeof(response) - sizeof(long), 0) == -1) {
                perror("msgsnd to scheduler");
                return;
            }

            cout << "CPU: Sent message to Scheduler for process " << current_user->pid << ".\n";

            if (current_user->cpu_burst == 0 || time_slice == 0) {
                current_user = nullptr;  // 프로세스 해제
            }
        }
    }
}
// 현재 프로세스를 반환하고 CPU를 비웁니다.
User* CPU::release_process() {
    User* temp_user = current_user;
    current_user = nullptr;
    return temp_user;
}

bool CPU::is_idle() const {
    return current_user == nullptr;
}

int CPU::getTimeSlice(){
    return time_slice;
}