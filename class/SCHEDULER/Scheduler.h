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
    vector<PCB*> allPCBs;                 // 모든 PCB를 보관하는 벡터
    static Scheduler* global_scheduler;           // Singleton-like 인스턴스 포인터

    void terminateProcess(PCB* process);
    void runQueueEnqueue(PCB* process);
    void waitQueueEnqueue(PCB* process); 
    void CPUSchedule();  // 다음 프로세스 스케줄링
    void IOSchedule();
    static void timer_handler(int signum);
    void start_timer();
    PCB* findPCB(pid_t pid);
    bool allProcessesCompleted(); // 모든 프로세스 완료 여부 확인
    void handleCPUReport(const IPCMessageToScheduler& msg);
    void handleIOReport(const IPCMessageToScheduler& msg);


public:
    Scheduler(vector<FeedbackQueue*> feedbackQueues, WaitQueue* waitQueue, CPU* cpu, IOdevice* ioDevice);
    void addProcess(PCB* process);
    void tick();
    void aging();
    void run(); // 실행 루프 추가

};

#endif  // SCHEDULER_H
