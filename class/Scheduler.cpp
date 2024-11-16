#define TIME_TICK 10000; //10ms
#define IPC_KEY 1234 // IPC 메시지 큐 키

#include "Scheduler.h"
#include "IPCMessage.h"
#include <sys/ipc.h>
#include <sys/msg.h>
using namespace std;
// 생성자
Scheduler::Scheduler() : feedbackQueues({
    FeedbackQueue(50),
    FeedbackQueue(100),
    FeedbackQueue(200)
}) {
    start_timer();
}

// 프로세스 추가
void Scheduler::addProcess(PCB* process) {
    feedbackQueues[0].enqueue(process);
    cout << "Process " << process->pid << " added to FeedbackQueue 0\n";
}

// 스케줄링 메서드
void Scheduler::schedule() {
    if (cpu.is_idle()) {
        for (int i = 0; i < feedbackQueues.size(); ++i) {
            if (!feedbackQueues[i].isEmpty()) {
                PCB* pcb = feedbackQueues[i].dequeue();
                pcb->state = RUNNING;
                cpu.assign_process(pcb->user, feedbackQueues[i].getTimeQuantum());
                return;
            }
        }
        cout << "No processes in FeedbackQueues to schedule.\n";
    }
}

// 프로세스 강등
void Scheduler::demoteProcess(PCB* process) {
    int current_level = process->priority;
    int next_level = std::min(current_level + 1, (int)feedbackQueues.size() - 1);

    process->priority = next_level;
    feedbackQueues[next_level].enqueue(process);
    process->state = READY;
    cout << "Process " << process->pid << " demoted to FeedbackQueue " << next_level << "\n";
}

// PID로 PCB 찾기
PCB* Scheduler::get_pcb_by_pid(int pid) {
    for (auto& queue : feedbackQueues) {
        PCB* process = queue.find_process_by_pid(pid);
        if (process != nullptr) {
            return process;
        }
    }
    return nullptr;
}

// Tick 처리
void Scheduler::tick() {
    cpu.tick();  // CPU의 현재 프로세스 타임 슬라이스 감소

    // 메시지 큐에서 메시지 수신
    int msgid = msgget(IPC_KEY, 0666 | IPC_CREAT);
    if (msgid == -1) {
        perror("msgget");
        return;
    }

    IPCMessage msg;
    while (msgrcv(msgid, &msg, sizeof(msg) - sizeof(long), 0, IPC_NOWAIT) != -1) {
        // 메시지 처리
        PCB* pcb = get_pcb_by_pid(msg.pid);
        if (!pcb) {
            cerr << "Scheduler: PCB not found for PID " << msg.pid << endl;
            continue;
        }

        pcb->cpu_burst = msg.cpu_burst;
        pcb->io_burst = msg.io_burst;

        if (pcb->cpu_burst == 0 && pcb->io_burst > 0) {
            // IO 작업 대기로 WaitQueue로 이동
            waitQueue.enqueue(pcb);
            pcb->state = WAITING;
            cout << "Scheduler: Process " << msg.pid << " moved to WaitQueue.\n";
        } else if (pcb->cpu_burst == 0 && pcb->io_burst == 0) {
            // 프로세스 종료
            pcb->state = TERMINATED;
            cout << "Scheduler: Process " << msg.pid << " terminated.\n";
        } else if (pcb->cpu_burst > 0) {
            // FeedbackQueue로 재삽입
            demoteProcess(pcb);
        }
    }

    // 다음 프로세스 스케줄링
    if (cpu.is_idle()) {
        schedule();
    }
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
