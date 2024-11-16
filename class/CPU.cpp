#define TIME_TICK 10000; //10ms
#define IPC_KEY 1234 // IPC 메시지 큐 키


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

#include "CPU.h"
#include "IPCMessage.h"
#include <sys/ipc.h>
#include <sys/msg.h>
#include <iostream>

using namespace std;

// IPC 메시지 큐 키
#define IPC_KEY 1234

void CPU::tick() {
    if (current_user != nullptr) {
        // time_slice에서 TIME_TICK을 차감
        time_slice -= TIME_TICK;
        current_user->cpu_burst -= TIME_TICK;

        // cpu_burst가 0보다 작으면 0으로 설정
        if (current_user->cpu_burst < 0) {
            current_user->cpu_burst = 0;
        }

        // 메시지 전송 조건
        if (current_user->cpu_burst == 0 || time_slice == 0) {
            // 메시지 큐에 메시지 전송
            IPCMessage msg;
            msg.mtype = 1;  // 메시지 타입
            msg.pid = current_user->pid;
            msg.cpu_burst = current_user->cpu_burst;
            msg.io_burst = current_user->io_burst;

            int msgid = msgget(IPC_KEY, 0666 | IPC_CREAT);
            if (msgid == -1) {
                perror("msgget");
                return;
            }

            if (msgsnd(msgid, &msg, sizeof(msg) - sizeof(long), 0) == -1) {
                perror("msgsnd");
                return;
            }

            cout << "CPU: Sent message for process " << current_user->pid
                << " to scheduler (cpu_burst=" << current_user->cpu_burst
                << ", io_burst=" << current_user->io_burst << ")\n";

            // 프로세스 해제
            current_user = nullptr;
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