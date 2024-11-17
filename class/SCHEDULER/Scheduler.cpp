#define TIME_TICK 10000; //10ms
// IPC 키 설정
#define IPC_KEY_TO_USER 1234
#define IPC_KEY_TO_SCHEDULER 5678
#define CPU_REPORT 0
#define IO_REPORT 1
#define CPU_DECREASE 2
#define IO_DECREASE 3
#define TERMINATE 4

#include "./Scheduler.h"
#include <sys/ipc.h>
#include <sys/msg.h>
#include <iostream>
#include <signal.h>
#include <sys/time.h>
#include <unistd.h>
using namespace std;

// 생성자
Scheduler::Scheduler(vector<FeedbackQueue*> feedbackQueues, WaitQueue* waitQueue, CPU* cpu, IOdevice* ioDevice)
    : feedbackQueues(feedbackQueues), waitQueue(waitQueue), cpu(cpu), ioDevice(ioDevice) {
    start_timer();
}

// 프로세스 추가
void Scheduler::addProcess(PCB* process) {
    feedbackQueues[0]->enqueue(process);
    cout << "Process " << process->pid << " added to FeedbackQueue 0\n";
}

// 스케줄링 메서드
void Scheduler::CPUSchedule() {
    for (auto queue : feedbackQueues) {
        if (!queue->isEmpty()) {
            PCB* pcb = queue->dequeue();
            pcb->state = RUNNING;
            cpu->assign_process(pcb, queue->getTimeQuantum());
            return;
        }
    }
    cout << "No processes in FeedbackQueues to schedule.\n";
}

void Scheduler::IOSchedule(){
    if(!waitQueue->isEmpty()){
        PCB* pcb = waitQueue->dequeue();
        pcb->state = IO;
        ioDevice->assign_process(pcb,50);
        return;
    }
}

// 프로세스 강등
void Scheduler::runQueueEnqueue(PCB* process) {
    int current_level = process->priority;
    int next_level = min(current_level + 1, (int)feedbackQueues.size() - 1);

    process->priority = next_level;
    feedbackQueues[next_level]->enqueue(process);
    process->state = READY;
    cout << "Process " << process->pid << " demoted to FeedbackQueue " << next_level << "\n";
}

void Scheduler::waitQueueEnqueue(PCB* process) {
    process->state = WAITING;  // 상태를 WAITING으로 설정
    waitQueue->enqueue(process);  // WAITING 큐에 삽입
    cout << "Process " << process->pid << " moved to WAITING queue.\n";
}

// 프로세스 종료 처리
void Scheduler::terminateProcess(PCB* process) {
    cout << "Scheduler: Terminating process " << process->pid << "\n";

    // IPC 메시지를 통해 User 프로세스 종료 명령 전송
    int msgid_to_user = msgget(IPC_KEY_TO_USER, 0666 | IPC_CREAT);

    IPCMessageToUser msg_to_user;
    msg_to_user.mtype = TERMINATE;
    msg_to_user.sender_pid = getpid();
    msg_to_user.receiver_pid = process->pid;
    if (msgsnd(msgid_to_user, &msg_to_user, sizeof(msg_to_user) - sizeof(long), 0) == -1) {
        cerr << "Scheduler: Failed to send TERMINATE command to PID " << process->pid << "\n";
    }

    // PCB 삭제
    delete process;
}

// 타이머 핸들러
void Scheduler::timer_handler(int signum) {
    // 타이머 이벤트 처리
    Scheduler* scheduler = reinterpret_cast<Scheduler*>(signum);
    scheduler->tick();
}

// 타이머 시작
void Scheduler::start_timer() {
    struct itimerval timer;
    timer.it_value.tv_sec = 0;
    timer.it_value.tv_usec = TIME_TICK;  // 10ms
    timer.it_interval = timer.it_value;
    setitimer(ITIMER_REAL, &timer, NULL);
    signal(SIGALRM, timer_handler);
}

void Scheduler::tick() {
    int msgid_to_scheduler = msgget(IPC_KEY_TO_SCHEDULER, 0666 | IPC_CREAT); // 메시지 큐 ID
    IPCMessageToScheduler msg_from_user;

    // 메시지 수신 (비차단 모드)
    if (msgrcv(msgid_to_scheduler, &msg_from_user, sizeof(msg_from_user) - sizeof(long), 0, IPC_NOWAIT) > 0) {
        // 메시지를 성공적으로 수신한 경우
        if (msg_from_user.mtype == CPU_REPORT) {
            cout << "Scheduler: Received CPU completion message from PID " << msg_from_user.pid << "\n";

            // CPU에서 프로세스 해제
            PCB* process = cpu->release_process();
            if (process) {
                process->cpu_burst = msg_from_user.cpu_burst;
                process->io_burst = msg_from_user.io_burst;

                // 스케줄링 로직
                if (process->cpu_burst > 0) {
                    runQueueEnqueue(process);
                } else if (process->cpu_burst == 0 && process->io_burst > 0) {
                    waitQueueEnqueue(process);
                    cout << "Scheduler: Process " << process->pid << " moved to WaitQueue.\n";
                } else if (process->cpu_burst == 0 && process->io_burst == 0) {
                    terminateProcess(process);
                }
            }
        } else if (msg_from_user.mtype == IO_REPORT) {
            cout << "Scheduler: Received IO completion message from PID " << msg_from_user.pid << "\n";

            // I/O 장치에서 프로세스 해제
            PCB* process = ioDevice->release_process();
            if (process) {
                process->cpu_burst = msg_from_user.cpu_burst;
                process->io_burst = msg_from_user.io_burst;
                process->state = READY; // 상태를 READY로 변경
                runQueueEnqueue(process); // 실행 큐로 이동
                cout << "Scheduler: Process " << process->pid << " moved to RunQueue.\n";
            }
        }
    } else {
        // 메시지가 없으면 CPU와 IO tick 실행
        cpu->tick();
        ioDevice->tick();
    }


    // CPU가 비어있으면 스케줄링
    if (cpu->is_idle()) {
        CPUSchedule();
    }

    if (ioDevice->is_idle()){
        IOSchedule();
    }
}