#define TIME_TICK 10; //10ms

// IPC 키 설정
#define IPC_KEY_TO_USER 1234
#define CPU_REPORT 1
#define IO_REPORT 2
#define CPU_DECREASE 3
#define IO_DECREASE 4
#define TERMINATE 5

#include "./CPU.h"
#include <sys/ipc.h>
#include <sys/msg.h>
#include <iostream>
#include <signal.h>
#include <sys/time.h>
#include <unistd.h>
#include <cstring>
#include <fstream>  // 파일 출력을 위한 헤더 추가


using namespace std;

// 기본 생성자
CPU::CPU() : process(nullptr), time_slice(0) ,time(0) {}

void CPU::assign_process(PCB* cur_process, int slice) {
    this->process = cur_process;
    this->time_slice = slice;

    // 시간 t와 관련된 로그 출력
    cout << "(At time " << time << "ms, process " 
        << process->pid << " obtained the CPU, remaining CPU burst is " 
        << process->cpu_burst << ")\n";

    // 덤프 파일이 저장될 폴더 경로
    string folder_path = "./dump";

    // 덤프 파일 경로 설정
    string file_path = folder_path + "/run_queue_dump.txt";

    // 폴더가 존재하지 않으면 생성
    if (!filesystem::exists(folder_path)) {
        filesystem::create_directories(folder_path);
    }
    // 덤프 파일에 기록
    ofstream out(file_path, std::ios::app); // 덮어쓰기 방지, 추가 모드
    if (out.is_open()) {
        out << "At time " << time << "ms, process " << process->pid 
            << " obtained the CPU for " << this->time_slice 
            << ", remaining CPU burst is " << process->cpu_burst << ".\n";
        out.close();
    } else {
        cerr << "Error: Unable to open run_queue_dump.txt for writing.\n";
    }
}

void CPU::tick() {
    // cout << "CPU: tick" << endl;
    time += TIME_TICK;
    if (process == nullptr) return; // 현재 실행 중인 프로세스가 없으면 무시
    // IPC 메시지 작성
    int msgid_to_user = msgget(IPC_KEY_TO_USER, 0666 | IPC_CREAT);

    IPCMessageToUser msg_to_user;        
    msg_to_user.sender_pid = getpid();
    msg_to_user.mtype = process->pid;

    if (time_slice == 0) {
        // 타임 슬라이스 만료 시: REPORT 명령
        msg_to_user.type = CPU_REPORT; // REPORT는 #define으로 정의된 정수
        cout << "CPU: Sending REPORT command to PID " << process->pid << "\n";
        // 프로세스를 CPU에서 해제
        PCB* temp_process = release_process();
        if (temp_process) {
            cout << "CPU: Process " << temp_process->pid << " released from CPU.\n";
        }

    } else {
        // 타임 슬라이스 진행 중: DECREASE 명령
        msg_to_user.type = CPU_DECREASE; // REPORT는 #define으로 정의된 정수
        cout << "CPU: Sending CPU_DECREASE command to PID " << process->pid << "\n";
    }

    // 메시지 전송
    if (msgsnd(msgid_to_user, &msg_to_user, sizeof(msg_to_user) - sizeof(long), 0) == -1) {
        cerr << "CPU: Failed to send message to User process " << process->pid << "\n";
    }
    // 타임 슬라이스 감소
    time_slice -= TIME_TICK;
    if (time_slice < 0) time_slice = 0;
}

// 현재 프로세스를 반환하고 CPU를 비웁니다.
PCB* CPU::release_process() {
    PCB* temp_process = process;
    process = nullptr;
    return temp_process;
}

bool CPU::is_idle() const {
    return process== nullptr;
}

int CPU::getTimeSlice(){
    return time_slice;
}