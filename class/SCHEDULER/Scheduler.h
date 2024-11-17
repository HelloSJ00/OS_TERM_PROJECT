#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "../RUNQUEUE/FeedbackQueue.h"
#include "../WAITQUEUE/WaitQueue.h"
#include "../CPU/CPU.h"
#include "../PCB/PCB.h"
#include "../IODEVICE/IOdevice.h"
#include "../SCHEDULER/Scheduler.h"
#include "../IPCMESSAGE/IPCMessage.h"
using namespace std;

class Scheduler {
private:
    vector<FeedbackQueue*> feedbackQueues; // 피드백 큐 리스트 (포인터)
    WaitQueue* waitQueue;                 // 대기 큐 (포인터)
    CPU* cpu;                             // CPU 객체 (포인터)
    IOdevice* ioDevice;                   // IO 장치 (포인터)

    void terminateProcess(PCB* process);
    void runQueueEnqueue(PCB* process);
    void waitQueueEnqueue(PCB* process); 
    void CPUSchedule();  // 다음 프로세스 스케줄링
    void IOSchedule();
    static void timer_handler(int signum);
    void start_timer();

public:
    Scheduler(vector<FeedbackQueue*> feedbackQueues, WaitQueue* waitQueue, CPU* cpu, IOdevice* ioDevice);
    void addProcess(PCB* process);
    void tick();
};

#endif  // SCHEDULER_H
