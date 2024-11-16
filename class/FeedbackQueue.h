#ifndef FEEDBACKQUEUE_H
#define FEEDBACKQUEUE_H

#include <queue>
#include "PCB.h"  // PCB 클래스 정의 포함

class FeedbackQueue {
private:
    std::queue<PCB*> processQueue;  // PCB 포인터를 저장하는 큐
    int time_quantum;               // 각 큐의 타임 퀀텀

public:
    // 생성자
    FeedbackQueue(int tq);

    // 프로세스 추가
    void enqueue(PCB* process);

    // 프로세스 제거 및 반환
    PCB* dequeue();

    // 큐가 비어 있는지 확인
    bool isEmpty() const;

    // 타임 퀀텀 반환
    int getTimeQuantum() const;

    // PID로 PCB를 찾는 메서드
    PCB* find_process_by_pid(int pid);
};

#endif // FEEDBACKQUEUE_H
