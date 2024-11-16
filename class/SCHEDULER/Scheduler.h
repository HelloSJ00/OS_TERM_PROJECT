#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "FeedbackQueue.h"
#include "WaitQueue.h"
#include "CPU.h"
#include "PCB.h"
#include "IOdevice.h"
#include <vector>
#include <iostream>
#include <signal.h>
#include <sys/time.h>
#include <unistd.h>
using namespace std;

class Scheduler {
private:
    vector<FeedbackQueue> feedbackQueues; // 피드백 큐 리스트
    WaitQueue waitQueue;                       // 대기 큐
    CPU cpu;                                   // CPU 객체
    IOdevice ioDevice;                         // IO 장치

    void demoteProcess(PCB* process);
    void schedule();  // 다음 프로세스 스케줄링
    PCB* get_pcb_by_pid(int pid);

    static void timer_handler(int signum);
    void start_timer();

public:
    Scheduler();
    void addProcess(PCB* process);
    void tick();
};

#endif  // SCHEDULER_H
