#include "./FeedbackQueue.h"
using namespace std;
// 생성자
FeedbackQueue::FeedbackQueue(int tq) : time_quantum(tq) {}

// 프로세스 추가
void FeedbackQueue::enqueue(PCB* process) {
    processQueue.push(process);
}

// 프로세스 제거 및 반환
PCB* FeedbackQueue::dequeue() {
    if (processQueue.empty()) return nullptr;
    PCB* process = processQueue.front();
    processQueue.pop();
    return process;
}

// 큐가 비어 있는지 확인
bool FeedbackQueue::isEmpty() const {
    return processQueue.empty();
}

// 타임 퀀텀 반환
int FeedbackQueue::getTimeQuantum() const {
    return time_quantum;
}

// PID로 PCB를 찾는 메서드
PCB* FeedbackQueue::find_process_by_pid(int pid) {
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
