#include <queue>
#include "PCB.h"  // PCB 클래스 정의가 포함된 헤더 파일을 사용
using namespace std;

class FeedbackQueue {
private:
    queue<PCB*> processQueue;  // PCB 포인터를 저장하는 큐

public:
    int time_quantum;  // 각 큐의 타임 퀀텀

    FeedbackQueue(int tq) : time_quantum(tq) {}

    void enqueue(PCB* process) {
        processQueue.push(process);
    }

    PCB* dequeue() {
        if (processQueue.empty()) return nullptr;
        PCB* process = processQueue.front();
        processQueue.pop();
        return process;
    }

    bool isEmpty() const {
        return processQueue.empty();
    }

    int getTimeQuantum() const {
        return time_quantum;
    }

    // PID로 PCB를 찾는 메서드
    PCB* find_process_by_pid(int pid) {
        queue<PCB*> tempQueue;  // PCB 포인터를 임시 저장할 큐
        PCB* foundProcess = nullptr;

        // 큐에서 프로세스를 찾아 임시 큐에 재배열
        while (!processQueue.empty()) {
            PCB* process = processQueue.front();
            processQueue.pop();

            if (process->pid == pid) {
                foundProcess = process;
            }
            tempQueue.push(process);  // 임시 큐에 PCB 복원
        }

        // 임시 큐의 PCB 포인터들을 다시 원래 큐로 복원
        while (!tempQueue.empty()) {
            processQueue.push(tempQueue.front());
            tempQueue.pop();
        }

        return foundProcess;
    }
};
