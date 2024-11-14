#include "FeedbackQueue.cpp"
#include "WaitQueue.cpp"
#include "CPU.cpp"
#include "PCB.h"
#include <vector>
#include <iostream>
#include <signal.h>
#include <sys/time.h>
#include <unistd.h>

using namespace std;

class Scheduler {
    vector<FeedbackQueue> feedbackQueues = {
        FeedbackQueue(50),
        FeedbackQueue(100),
        FeedbackQueue(200)
    };
    WaitQueue waitQueue;
    CPU cpu;

public:
    Scheduler() {
        start_timer();
    }

    void addProcess(PCB* process) {
        feedbackQueues[0].enqueue(process);
        cout << "Process " << process->pid << " added to FeedbackQueue 0\n";
    }

    void schedule() {
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

    void demoteProcess(PCB* process) {
        int current_level = process->priority;
        int next_level = min(current_level + 1, (int)feedbackQueues.size() - 1);

        process->priority = next_level;
        feedbackQueues[next_level].enqueue(process);
        process->state = READY;
        cout << "Process " << process->pid << " demoted to FeedbackQueue " << next_level << "\n";
    }

    PCB* get_pcb_by_pid(int pid) {
        for (auto& queue : feedbackQueues) {
            PCB* process = queue.find_process_by_pid(pid);
            if (process != nullptr) {
                return process;
            }
        }
        return nullptr;
    }
    
    void tick() {
        cpu.tick();  // CPU의 현재 프로세스 타임 슬라이스 감소

        if (cpu.time_slice <= 0 && !cpu.is_idle()) {
            User* finished_user = cpu.release_process();  // User*로 받아야 함
            PCB* finished_pcb = &(finished_user->pcb);    // User의 PCB에 접근

            if (finished_pcb) {
                if (finished_pcb->cpu_burst <= 0) {
                    cout << "Process " << finished_pcb->pid << " completed its execution.\n";
                    finished_pcb->state = TERMINATED;
                } else {
                    finished_pcb->state = READY;
                    demoteProcess(finished_pcb);
                }
            }

            schedule();  // 다음 프로세스 스케줄링
        }
    }

    static void timer_handler(int signum) {
        Scheduler* scheduler = reinterpret_cast<Scheduler*>(signum);
        scheduler->tick();
    }

    void start_timer() {
        struct itimerval timer;
        timer.it_value.tv_sec = 0;
        timer.it_value.tv_usec = 50000;
        timer.it_interval = timer.it_value;
        setitimer(ITIMER_REAL, &timer, NULL);

        signal(SIGALRM, timer_handler);
    }
};
