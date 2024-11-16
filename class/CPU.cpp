#define TIME_TICK 10000; //10ms

#include <iostream>
#include "PCB.h"
#include "CPU.h"
#include "User.h"  // User 클래스 포함
using namespace std;

// 기본 생성자
CPU::CPU() : current_user(nullptr), time_slice(0) {}

void CPU::assign_process(User* user,int slice){
    current_user = user;
    time_slice = slice;
    cout << "CPU: Executing process " << user->pid << " with time slice " << slice << " units.\n";
}

void CPU::tick() {
    if (current_user != nullptr) {
        // time_slice에서 TIME_TICK을 차감
        time_slice -= TIME_TICK;

        // time_slice가 0보다 작으면 0으로 설정
        if (time_slice < 0) {
            time_slice = 0;
        }

        // time_slice가 0이면 프로세스 해제
        if (time_slice == 0) {
            cout << "CPU: Time slice expired for process " << current_user->pid << "\n";
            current_user = nullptr;  // 프로세스 해제
        }
    }
}

// 현재 프로세스를 반환하고 CPU를 비웁니다.
User* CPU::release_process() {
    User* temp_user = current_user;
    current_user = nullptr;
    return temp_user;
}

bool CPU::is_idle() const {
    return current_user == nullptr;
}