#include "./WaitQueue.h"

// 프로세스를 큐에 추가
void WaitQueue::enqueue(PCB* process) {
    queue.push(process);
}

// 큐에서 프로세스를 제거하고 반환
PCB* WaitQueue::dequeue() {
    if (queue.empty()) return nullptr;
    PCB* process = queue.front();
    queue.pop();
    return process;
}

// 큐가 비어 있는지 확인
bool WaitQueue::isEmpty() const {
    return queue.empty();
}
