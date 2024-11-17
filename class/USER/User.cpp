#define TIME_TICK 10000 // 10ms
#define IPC_KEY_TO_USER 1234
#define IPC_KEY_TO_SCHEDULER 5678
#define CPU_REPORT 0
#define IO_REPORT 1
#define CPU_DECREASE 2
#define IO_DECREASE 3
#define TERMINATE 4

#include "User.h"
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h> // sleep
using namespace std;

// 생성자
User::User(pid_t p, int cpu, int io)
    : pid(p), cpu_burst(cpu), io_burst(io) {}


// 부모로부터 명령 수신
void User::receiveCommand() {
    int msgid_to_user = msgget(IPC_KEY_TO_USER, 0666 | IPC_CREAT);
    int msgid_to_scheduler = msgget(IPC_KEY_TO_SCHEDULER, 0666 | IPC_CREAT);

    IPCMessageToUser msg_to_user;
    IPCMessageToScheduler msg_to_scheduler;

    while (true) {
        // 메시지 수신
        if (msgrcv(msgid_to_user, &msg_to_user, sizeof(msg_to_user) - sizeof(long), 0, 0) > 0) {
            // 자신의 PID와 receiver_pid가 동일한지 확인
            if (msg_to_user.receiver_pid == getpid()) {
                if (msg_to_user.mtype == CPU_REPORT) {
                    // REPORT 명령 처리: Scheduler로 상태 보고
                    msg_to_scheduler.mtype = CPU_REPORT; // Scheduler 수신용 타입
                    msg_to_scheduler.pid = getpid();
                    msg_to_scheduler.cpu_burst = cpu_burst;
                    msg_to_scheduler.io_burst = io_burst;
                    msgsnd(msgid_to_scheduler, &msg_to_scheduler, sizeof(msg_to_scheduler) - sizeof(long), 0);

                    cout << "User " << getpid() << ": Reported CPU and IO burst to Scheduler.\n";
                } else if (msg_to_user.mtype == IO_REPORT) {
                    // REPORT 명령 처리: Scheduler로 상태 보고
                    msg_to_scheduler.mtype = IO_REPORT; // Scheduler 수신용 타입
                    msg_to_scheduler.pid = getpid();
                    msg_to_scheduler.cpu_burst = cpu_burst;
                    msg_to_scheduler.io_burst = io_burst;
                    msgsnd(msgid_to_scheduler, &msg_to_scheduler, sizeof(msg_to_scheduler) - sizeof(long), 0);
                } else if (msg_to_user.mtype == CPU_DECREASE) {
                    // DECREASE 명령 처리: CPU burst 감소
                    cpu_burst -= TIME_TICK;
                    if (cpu_burst < 0) cpu_burst = 0;
                    cout << "User " << getpid() << ": CPU burst decreased to " << cpu_burst << "\n";
                } else if (msg_to_user.mtype == IO_DECREASE) {
                    io_burst -= TIME_TICK;
                    if (io_burst < 0) io_burst = 0;
                    cout << "User " << getpid() << ": IO burst decreased to " << io_burst << "\n";
                } else if (msg_to_user.mtype == TERMINATE) {
                    // TERMINATE 명령 처리
                    cout << "User " << getpid() << ": Received TERMINATE command. Exiting.\n";
                    break;
                }
            }
            // receiver_pid가 다르면 무시
            else {
                cout << "User " << getpid() << ": Ignored message for PID " << msg_to_user.receiver_pid << "\n";
            }
        }
        usleep(1000); // 1ms 대기
    }
}

// 디버깅용 정보 출력
void User::printProcessInfo() const {
    cout << "PID: " << pid 
        << ", CPU Burst: " << cpu_burst 
        << ", IO Burst: " << io_burst << endl;
}
