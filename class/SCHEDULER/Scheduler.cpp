#define TIME_TICK 10000; //10ms
// IPC 키 설정
#define IPC_KEY_TO_USER 1234
#define IPC_KEY_TO_SCHEDULER 5678
#define CPU_REPORT 1
#define IO_REPORT 2
#define CPU_DECREASE 3
#define IO_DECREASE 4
#define TERMINATE 5

#include "./Scheduler.h"
#include <sys/ipc.h>
#include <sys/msg.h>
#include <iostream>
#include <signal.h>
#include <sys/time.h>
#include <unistd.h>
using namespace std;
Scheduler* Scheduler::global_scheduler = nullptr; 
// 생성자
Scheduler::Scheduler(vector<FeedbackQueue*> feedbackQueues, WaitQueue* waitQueue, CPU* cpu, IOdevice* ioDevice)
    : feedbackQueues(feedbackQueues), waitQueue(waitQueue), cpu(cpu), ioDevice(ioDevice) {
    global_scheduler = this; // 현재 Scheduler 객체를 전역으로 설정
    start_timer();
}

// 프로세스 추가
void Scheduler::addProcess(PCB* process) {
    feedbackQueues[0]->enqueue(process);
    allPCBs.push_back(process);          // 모든 PCB 벡터에 추가
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

    // 메시지를 통해 User 프로세스 종료 명령 전송
    int msgid_to_user = msgget(IPC_KEY_TO_USER, 0666 | IPC_CREAT);
    IPCMessageToUser msg_to_user;

    msg_to_user.type = TERMINATE;
    msg_to_user.sender_pid = getpid();
    msg_to_user.mtype = process->pid;

    if (msgsnd(msgid_to_user, &msg_to_user, sizeof(msg_to_user) - sizeof(long), 0) == -1) {
        cerr << "Scheduler: Failed to send TERMINATE command to PID " << process->pid << "\n";
    }

    // allPCBs에서 제거
    auto it = std::remove(allPCBs.begin(), allPCBs.end(), process);
    allPCBs.erase(it, allPCBs.end());

    delete process; // PCB 객체 삭제
}


// 타이머 핸들러
void Scheduler::timer_handler(int signum) {
    if (global_scheduler) {
        global_scheduler->tick(); // 전역 Scheduler 객체의 tick 호출
    } else {
        cerr << "Error: global_scheduler is null!" << endl;
    }
}

// 타이머 시작
void Scheduler::start_timer() {
    struct sigaction sa;
    struct itimerval timer;

    // 타이머 핸들러 설정
    sa.sa_handler = &Scheduler::timer_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGALRM, &sa, nullptr);

    // 타이머 값 설정 (10ms)
    timer.it_value.tv_sec = 0;
    timer.it_value.tv_usec = 10000; // 10ms
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = 10000; // 10ms
    setitimer(ITIMER_REAL, &timer, nullptr);
}

void Scheduler::tick() {
    cpu->tick();
    ioDevice->tick();
    
    int msgid_to_scheduler = msgget(IPC_KEY_TO_SCHEDULER, 0666 | IPC_CREAT); // 메시지 큐 ID
    IPCMessageToScheduler msg_from_user;

    // 메시지 수신 (비차단 모드)
    if (msgrcv(msgid_to_scheduler, &msg_from_user, sizeof(msg_from_user) - sizeof(long), 0, IPC_NOWAIT) > 0) {
        // 메시지를 성공적으로 수신한 경우
        if (msg_from_user.mtype == CPU_REPORT) {
            cout << "Scheduler: Received CPU completion message from PID " << msg_from_user.pid << "\n";
            // PID 기반으로 PCB 검색
            PCB* process = findPCB(msg_from_user.pid);
            if (process) {
                process->cpu_burst = msg_from_user.cpu_burst;
                process->io_burst = msg_from_user.io_burst;
                // 남은 CPU와 IO Burst 출력
                cout << "Scheduler: PID " << process->pid 
                    << " | Remaining CPU Burst: " << process->cpu_burst 
                    << " | Remaining IO Burst: " << process->io_burst << "\n";
                // 스케줄링 로직
                if (process->cpu_burst > 0) {
                    runQueueEnqueue(process); // 실행 대기열로 복귀
                    cout << "Scheduler: Process " << process->pid << " moved to RunQueue.\n";

                } else if (process->cpu_burst == 0 && process->io_burst > 0) {
                    waitQueueEnqueue(process); // 대기열로 이동
                    cout << "Scheduler: Process " << process->pid << " moved to WaitQueue.\n";
                } else if (process->cpu_burst == 0 && process->io_burst == 0) {
                    terminateProcess(process); // 프로세스 종료
                }
            } else {
                cerr << "Scheduler: Failed to find PCB for PID " << msg_from_user.pid << "\n";
            }
        } else if (msg_from_user.mtype == IO_REPORT) {
            cout << "Scheduler: Received IO completion message from PID " << msg_from_user.pid << "\n";

            // PID 기반으로 PCB 검색
            PCB* process = findPCB(msg_from_user.pid);
            if (process) {
                process->cpu_burst = msg_from_user.cpu_burst;
                process->io_burst = msg_from_user.io_burst;
                // 남은 CPU와 IO Burst 출력
                cout << "Scheduler: PID " << process->pid 
                    << " | Remaining CPU Burst: " << process->cpu_burst 
                    << " | Remaining IO Burst: " << process->io_burst << "\n";

                process->state = READY; // 상태를 READY로 변경
                runQueueEnqueue(process); // 실행 큐로 이동
                cout << "Scheduler: Process " << process->pid << " moved to RunQueue.\n";
            } else {
                cerr << "Scheduler: Failed to find PCB for PID " << msg_from_user.pid << "\n";
            }
        }
    }

    // CPU가 비어있으면 새로운 프로세스를 스케줄링
    if (cpu->is_idle()) {
        // cout << "Tick: CPU is idle, attempting to schedule a new process.\n";
        CPUSchedule();
    }

    // IO 장치가 비어있으면 새로운 IO 작업 스케줄링
    if (ioDevice->is_idle()) {
        // cout << "Tick: IO device is idle, attempting to schedule a new IO process.\n";
        IOSchedule();
    }

    // Aging 호출
    aging();
}

PCB* Scheduler::findPCB(pid_t pid) {
    for (PCB* process : allPCBs) {
        if (process->pid == pid) {
            return process; // PID에 해당하는 PCB를 찾으면 반환
        }
    }
    return nullptr; // PID에 해당하는 PCB를 찾지 못한 경우
}

// 모든 프로세스 완료 여부 확인
bool Scheduler::allProcessesCompleted() {
    return allPCBs.empty(); // 모든 PCB가 제거되었는지 확인
}

// 실행 루프
void Scheduler::run() {
    while (!allProcessesCompleted()) {
        pause(); // 타이머 신호(SIGALRM)를 대기
    }
    cout << "Scheduler: All processes completed. Exiting...\n";
}

void Scheduler::handleCPUReport(const IPCMessageToScheduler& msg) {
    PCB* process = findPCB(msg.pid);
    if (!process) {
        cerr << "Scheduler: Failed to find PCB for PID " << msg.pid << "\n";
        return;
    }

    // PCB 업데이트
    process->cpu_burst = msg.cpu_burst;
    process->io_burst = msg.io_burst;

    // 디버깅 정보 출력
    cout << "Scheduler: Handling CPU Report for PID " << process->pid
        << " | Remaining CPU Burst: " << process->cpu_burst
        << " | Remaining IO Burst: " << process->io_burst << "\n";

    // 스케줄링 로직
    if (process->cpu_burst > 0) {
        runQueueEnqueue(process);
        cout << "Scheduler: Process " << process->pid << " moved to RunQueue.\n";
    } else if (process->io_burst > 0) {
        waitQueueEnqueue(process);
        cout << "Scheduler: Process " << process->pid << " moved to WaitQueue.\n";
    } else {
        terminateProcess(process);
        cout << "Scheduler: Process " << process->pid << " terminated.\n";
    }
}

void Scheduler::handleIOReport(const IPCMessageToScheduler& msg) {
    PCB* process = findPCB(msg.pid);
    if (!process) {
        cerr << "Scheduler: Failed to find PCB for PID " << msg.pid << "\n";
        return;
    }

    // PCB 업데이트
    process->cpu_burst = msg.cpu_burst;
    process->io_burst = msg.io_burst;

    // 디버깅 정보 출력
    cout << "Scheduler: Handling IO Report for PID " << process->pid
        << " | Remaining CPU Burst: " << process->cpu_burst
        << " | Remaining IO Burst: " << process->io_burst << "\n";

    // IO 작업 완료 후 READY 상태로 변경
    process->state = READY;
    runQueueEnqueue(process);
    cout << "Scheduler: Process " << process->pid << " moved to RunQueue.\n";
}

void Scheduler::aging() {
    for (int i = 1; i < feedbackQueues.size(); ++i) { // 최상위 큐(0번)는 Aging 처리하지 않음
        FeedbackQueue* currentQueue = feedbackQueues[i];
        queue<PCB*> tempQueue; // 임시 큐로 대기열을 재구성

        while (!currentQueue->isEmpty()) {
            PCB* process = currentQueue->dequeue();

            // 대기 시간 증가
            process->waitingTime += currentQueue->getTimeQuantum();

            // 10,000ms 이상 대기하면 한 단계 높은 우선순위 큐로 이동
            if (process->waitingTime >= 10000) {
                process->waitingTime = 0; // 대기 시간 초기화
                int next_level = i - 1;  // 한 단계 높은 우선순위 큐
                feedbackQueues[next_level]->enqueue(process);
                cout << "Scheduler: Process " << process->pid 
                    << " aged to FeedbackQueue " << next_level << "\n";
            } else {
                tempQueue.push(process); // 대기 시간이 부족하면 그대로 유지
            }
        }
        // 임시 큐의 내용을 다시 원래 큐로 복원
        while (!tempQueue.empty()) {
            currentQueue->enqueue(tempQueue.front());
            tempQueue.pop();
        }
    }
}
