#include <iostream>
#include <csignal>  // kill 함수 사용을 위한 헤더 파일
#include "PCB.cpp"
using namespace std;


class FeedbackQueue {
private:
    int time_quantum;  // 각 큐의 타임 퀀텀
    queue<PCB> queue;

public:
    FeedbackQueue(int tq) : time_quantum(tq) {}

    void enqueue(PCB process) {
        queue.push(process);
    }

    PCB dequeue() {
        PCB process = queue.front();
        queue.pop();
        return process;
    }

    bool isEmpty() const {
        return queue.empty();
    }

    int getTimeQuantum() const {
        return time_quantum;
    }

    // 현재 큐의 스케줄링 로직
    void schedule() {
        if (!isEmpty()) {
            PCB process = dequeue();
            process.remaining_time -= time_quantum;
            cout << "Dispatching process " << process.pid << " with remaining CPU burst: " 
                      << process.remaining_time << "ms in queue with TQ=" << time_quantum << "ms\n";

            if (process.remaining_time <= 0) {
                cout << "Process " << process.pid << " completed\n";
                kill(process.pid, SIGKILL);  // 실제 프로세스 종료
            } else {
                process.state = "WAITING";
                queue.push(process);
                cout << "Process " << process.pid << " re-enqueued in queue with TQ=" << time_quantum << "\n";
            }
        }
    }
};