#define TIME_TICK 10000; //10ms
// IPC 키 설정
#define IPC_KEY_TO_CPU 1234
#define IPC_KEY_TO_IODEVICE 3456
#define IPC_KEY_TO_SCHEDULER 5678

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
    int msgid_to_cpu = msgget(IPC_KEY_TO_CPU, 0666 | IPC_CREAT);
    int msgid_to_iodevice = msgget(IPC_KEY_TO_IODEVICE, 0666 | IPC_CREAT);
    int msgid_to_scheduler = msgget(IPC_KEY_TO_SCHEDULER, 0666 | IPC_CREAT);

    if (msgid_to_cpu == -1 || msgid_to_iodevice == -1 || msgid_to_scheduler == -1) {
        perror("msgget");
        return;
    }

    // CPU로 메시지 전송
    if (!cpu.is_idle()) {
        IPCMessageToCPU msg;
        msg.mtype = 1;
        msg.pid = cpu.release_process()->pid;

        if (msgsnd(msgid_to_cpu, &msg, sizeof(msg) - sizeof(long), 0) == -1) {
            perror("msgsnd to CPU");
            return;
        }

        cout << "Scheduler: Sent tick message to CPU for process " << msg.pid << ".\n";
    }

    // IODevice로 메시지 전송
    if (!ioDevice.is_idle()) {
        IPCMessageToIODevice msg;
        msg.mtype = 1;
        msg.pid = ioDevice.release_process()->pid;

        if (msgsnd(msgid_to_iodevice, &msg, sizeof(msg) - sizeof(long), 0) == -1) {
            perror("msgsnd to IODevice");
            return;
        }

        cout << "Scheduler: Sent tick message to IODevice for process " << msg.pid << ".\n";
    }

    // CPU의 응답 메시지 처리
    IPCMessageToScheduler response;
    while (msgrcv(msgid_to_scheduler, &response, sizeof(response) - sizeof(long), 1, IPC_NOWAIT) != -1) {
        PCB* pcb = get_pcb_by_pid(response.pid);

        if (!pcb) {
            cerr << "Scheduler: PCB not found for PID " << response.pid << ".\n";
            continue;
        }

        pcb->cpu_burst = response.cpu_burst;
        pcb->io_burst = response.io_burst;

        if (pcb->cpu_burst == 0 && pcb->io_burst > 0) {
            waitQueue.enqueue(pcb);  // IO 대기
            pcb->state = WAITING;
            cout << "Scheduler: Process " << response.pid << " moved to WaitQueue.\n";
        } else if (pcb->cpu_burst == 0 && pcb->io_burst == 0) {
            pcb->state = TERMINATED;  // 프로세스 종료
            cout << "Scheduler: Process " << response.pid << " terminated.\n";
        } else if (response.io_complete) {
            feedbackQueues[0].enqueue(pcb);  // IO 완료 후 FeedbackQueue로 이동
            pcb->state = READY;
            cout << "Scheduler: Process " << response.pid << " moved to FeedbackQueue 0 (IO complete).\n";
        } else {
            demoteProcess(pcb);  // FeedbackQueue로 재삽입
        }
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
