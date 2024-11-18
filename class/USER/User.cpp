#define TIME_TICK 10 // 10ms
#define IPC_KEY_TO_USER 1234
#define IPC_KEY_TO_SCHEDULER 5678
#define CPU_REPORT 1
#define IO_REPORT 2
#define CPU_DECREASE 3
#define IO_DECREASE 4
#define TERMINATE 5

#include "./User.h"
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
    
    if (msgid_to_user == -1 || msgid_to_scheduler == -1) {
        perror("msgget failed");
        return; // 비정상 종료 방지
    }

    while (true) {
        // 메시지 수신 및 오류 확인
        ssize_t result = msgrcv(msgid_to_user, &msg_to_user, sizeof(msg_to_user) - sizeof(long), getpid(), 0);
        if (result == -1) {  // 메시지 수신 실패
            perror("msgrcv failed");
            continue;  // 루프 계속
        }

        // 수신한 메시지 타입에 따른 처리
        if (msg_to_user.type == CPU_REPORT) {
            msg_to_scheduler.mtype = CPU_REPORT; // Scheduler 수신용 타입
            msg_to_scheduler.pid = getpid();
            msg_to_scheduler.cpu_burst = cpu_burst;
            msg_to_scheduler.io_burst = io_burst;
            if (msgsnd(msgid_to_scheduler, &msg_to_scheduler, sizeof(msg_to_scheduler) - sizeof(long), 0) == -1) {
                perror("msgsnd failed");
                cout << "Calculated message size: " << sizeof(msg_to_scheduler) - sizeof(long) << endl;
                cout << "Error code: " << errno << endl;
            } else{
                cout << "User " << getpid() << ": Reported CPU and IO burst to Scheduler.\n";
            }


        } else if (msg_to_user.type == IO_REPORT) {
            msg_to_scheduler.mtype = IO_REPORT; // Scheduler 수신용 타입
            msg_to_scheduler.pid = getpid();
            msg_to_scheduler.cpu_burst = cpu_burst;
            msg_to_scheduler.io_burst = io_burst;
            if (msgsnd(msgid_to_scheduler, &msg_to_scheduler, sizeof(msg_to_scheduler) - sizeof(long), 0) == -1){
                perror("msgsnd failed");
            }else {
                cout << "User " << getpid() << ": Reported CPU and IO burst to Scheduler.\n";
            }

        } else if (msg_to_user.type == CPU_DECREASE) {
            cpu_burst -= TIME_TICK;
            if (cpu_burst < 0) cpu_burst = 0;
            cout << "User " << getpid() << ": CPU burst decreased to " << cpu_burst << "\n";

        } else if (msg_to_user.type == IO_DECREASE) {
            io_burst -= TIME_TICK;
            if (io_burst < 0) io_burst = 0;
            cout << "User " << getpid() << ": IO burst decreased to " << io_burst << "\n";

        } else if (msg_to_user.type == TERMINATE) {
            cout << "User " << getpid() << ": Received TERMINATE command. Exiting.\n";
            break;  // 루프 탈출 후 종료
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
