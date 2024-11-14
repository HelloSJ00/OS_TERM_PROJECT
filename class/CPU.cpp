#include <iostream>
#include "PCB.h"
#include "User.h"  // User 클래스 포함

using namespace std;

class CPU {
public:
    User* current_user;  // 현재 실행 중인 User 객체
    int time_slice;      // 현재 프로세스에 할당된 타임 슬라이스

    CPU() : current_user(nullptr), time_slice(0) {}

    // User 객체를 통해 프로세스를 할당
    void assign_process(User* user, int slice) {
        current_user = user;
        time_slice = slice;
        cout << "CPU: Executing process " << user->pid << " with time slice " << slice << " units.\n";
    }

    // CPU 사이클을 처리하고 타임 슬라이스 확인
    void tick() {
        if (current_user != nullptr) {
            time_slice--;

            if (time_slice <= 0) {
                cout << "CPU: Time slice expired for process " << current_user->pid << "\n";
                current_user = nullptr;  // 프로세스 해제
            }
        }
    }

    // 현재 프로세스를 반환하고 CPU를 비웁니다.
    User* release_process() {
        User* temp_user = current_user;
        current_user = nullptr;
        return temp_user;
    }

    // CPU가 비어 있는지 확인
    bool is_idle() const {
        return current_user == nullptr;
    }
};
