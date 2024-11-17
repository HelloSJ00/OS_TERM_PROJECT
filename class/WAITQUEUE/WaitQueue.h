#ifndef WAITQUEUE_H
#define WAITQUEUE_H

#include <queue>
#include "../PCB/PCB.h"
using namespace std;
class WaitQueue {
private:
    queue<PCB*> queue;  // PCB 포인터를 저장하는 큐

public:
    // 프로세스를 큐에 추가
    void enqueue(PCB* process);

    // 큐에서 프로세스를 제거하고 반환
    PCB* dequeue();

    // 큐가 비어 있는지 확인
    bool isEmpty() const;
};

#endif  // WAITQUEUE_H
