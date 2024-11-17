#include <iostream>
#include <queue>
#include <unistd.h>
#include <sys/wait.h>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <csignal>  // kill 함수 사용을 위한 헤더 파일
#include "./class/RUNQUEUE/FeedbackQueue.h"
#include "./class/PCB/PCB.h"
#include "./class/USER/User.h"
#include "./class/SCHEDULER/Scheduler.h"
#include "./class/CPU/CPU.h"
#include "./class/IODEVICE/IOdevice.h"
#include "./class/WAITQUEUE/WaitQueue.h"

#define NUM_OF_PROCESSES 10 
#define MIN_EXECUTION_TIME 60000  // 최소 실행 시간 (60초)
#define TIME_TICK 10 // 10ms
using namespace std;

int main(){
  FeedbackQueue* fq1 = new FeedbackQueue(50);
  FeedbackQueue* fq2 = new FeedbackQueue(100);
  FeedbackQueue* fq3 = new FeedbackQueue(200);
  vector<FeedbackQueue*> feedbackQueues = {fq1, fq2, fq3};
  WaitQueue* waitQueue = new WaitQueue();
  CPU* cpu = new CPU();
  IOdevice* ioDevice = new IOdevice();
  Scheduler scheduler(feedbackQueues, waitQueue, cpu, ioDevice);
  vector<pid_t> child_pids;
  srand(time(0)); // 랜덤 시드 설정

  // 자식 프로세스 10개 생성
  for (int i = 0; i < NUM_OF_PROCESSES; ++i) {
    pid_t pid = fork(); // 자식 프로세스 생성
    int cpu_burst = (rand() % 1501 + 3000) * TIME_TICK; // 30초 ~ 45초
    int io_burst = (rand() % 1501 + 3000) * TIME_TICK;  // 30초 ~ 45초
    if (pid == 0) { // 자식 프로세스
      cout << "Child Process Created: PID = " << getpid()
            << ", CPU Burst = " << cpu_burst
            << ", IO Burst = " << io_burst << std::endl;
      User user(getpid(), cpu_burst, io_burst); // User 객체 생성
      user.receiveCommand();                  // 명령 수신 대기
      exit(0);                                // 자식 프로세스 종료
    } else if (pid > 0) { // 부모 프로세스
        PCB* pcb = new PCB(pid, cpu_burst, io_burst, nullptr); // PCB 생성
        scheduler.addProcess(pcb); // FeedbackQueue에 PCB 추가
    } else {
      cerr << "Fork failed.\n";
      return 1;
    }
  }

  // 모든 자식 프로세스가 종료될 때까지 대기
  for (int i = 0; i < NUM_OF_PROCESSES; i++) {
    wait(NULL);
  }

  cout << "All child processes have completed.\n";

  // 작업 수행 후 메모리 해제
  delete fq1;
  delete fq2;
  delete fq3;
  delete waitQueue;
  delete cpu;
  delete ioDevice;
  return 0;
}