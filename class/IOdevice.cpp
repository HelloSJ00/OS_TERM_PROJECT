#define TIME_TICK 10000; //10ms

#include "IOdevice.h"
#include <iostream>
using namespace std;

// 기본 생성자
IOdevice::IOdevice() : current_user(nullptr), time_slice(0) {}

// User 객체를 통해 프로세스를 할당
void IOdevice::assign_process(User* user, int slice) {
    current_user = user;
    time_slice = slice;
    cout << "IOdevice: Assigning process " << user->pid << " with time slice " << slice << " units.\n";
}

// CPU 사이클을 처리하고 타임 슬라이스 확인
void IOdevice::tick() {
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
User* IOdevice::release_process() {
    User* temp_user = current_user;
    current_user = nullptr;
    return temp_user;
}

// CPU가 비어 있는지 확인
bool IOdevice::is_idle() const {
    return current_user == nullptr;
}
