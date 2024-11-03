#include <iostream>
#include <queue>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <csignal>  // kill 함수 사용을 위한 헤더 파일

#include "FeedbackQueue.cpp"
#include "PCB.cpp"

#define NUM_OF_PROCESSES 10
#define TIME_QUANTUM 100  // 타임 퀀텀 시간 (ms)
using namespace std;

void dispatch();
int main(){

  /*
  다단계 피드백 큐 3개 생성
  */
    vector<FeedbackQueue> feedbackQueues = {
      FeedbackQueue(50),   // 첫 번째 큐: 타임 퀀텀 50ms
      FeedbackQueue(100),  // 두 번째 큐: 타임 퀀텀 100ms
      FeedbackQueue(200)   // 세 번째 큐: 타임 퀀텀 200ms
    };

  /*  
  자식 프로세스 10개 생성 
  자식 프로세스 fork() 및 PCB 생성 후 첫 번째 큐에 추가
  */
  // 
  for (int i = 0; i < NUM_OF_PROCESSES; i++) {
      pid_t pid = fork();
      if (pid == 0) {
          pause();  // 자식 프로세스는 대기
      } else if (pid > 0) {
          int cpu_burst = rand() % 500 + 100;
          int io_burst = rand() % 300 + 100;
          PCB process(pid, cpu_burst, io_burst);
          feedbackQueues[0].enqueue(process);  // 첫 번째 큐에 추가
          cout << "Created process " << pid << " with CPU burst " << cpu_burst 
                    << "ms and IO burst " << io_burst << "ms\n";
      } else {
          cerr << "Fork failed\n";
          exit(1);
      }
    }

    // 부모 프로세스에서 스케줄러 실행
    while (true) {
        bool allQueuesEmpty = true;
        for (auto& queue : feedbackQueues) {
            if (!queue.isEmpty()) {
                queue.schedule();
                allQueuesEmpty = false;
                break;  // 상위 큐에 있는 프로세스가 우선
            }
        }
        if (allQueuesEmpty) break;  // 모든 큐가 비었으면 종료
        usleep(50000);  // 50ms 대기 (타이머 틱 역할)
    }

    // 모든 자식 프로세스가 종료될 때까지 대기
    for (int i = 0; i < NUM_OF_PROCESSES; i++) {
        wait(NULL);
    }

    cout << "All child processes have completed.\n";
  return 0; 
}