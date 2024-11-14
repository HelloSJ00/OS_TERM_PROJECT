#include <iostream>
#include <queue>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <csignal>    // signal, SIGALRM
#include <sys/time.h> // setitimer
#include <vector>
#include <cstdlib>
#include <ctime>
#include <csignal>  // kill 함수 사용을 위한 헤더 파일

#include "class/FeedbackQueue.cpp"
#include "class/PCB.h"
#include "class/User.h"
#include "class/Scheduler.cpp"

#define NUM_OF_PROCESSES 10
using namespace std;

int main(){
  Scheduler Round_Robin_Scheduler;

  srand(time(0)); // 랜덤 시드 설정

  // 자식 프로세스 10개 생성
  for (int i = 0; i < NUM_OF_PROCESSES; i++) {
    pid_t pid = fork();
    if (pid == 0) {
        pause();  // 자식 프로세스는 대기
    } else if (pid > 0) {
        int cpu_burst = rand() % 500 + 100;
        int io_burst = rand() % 300 + 100;
        User* user = new User(pid, cpu_burst, io_burst);
        // 스케줄러에 User의 PCB 포인터 추가
        Round_Robin_Scheduler.addProcess(&(user->pcb));
        cout << "Created process " << pid << " with CPU burst " << cpu_burst << "ms and IO burst " << io_burst << "ms\n";
    } else {
        cerr << "Fork failed\n";
        exit(1);
    }
  }

  // 모든 자식 프로세스가 종료될 때까지 대기
  for (int i = 0; i < NUM_OF_PROCESSES; i++) {
    wait(NULL);
  }

  cout << "All child processes have completed.\n";
  return 0; 
}