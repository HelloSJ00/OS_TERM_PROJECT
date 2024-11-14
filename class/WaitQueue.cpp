#include "PCB.h"
class WaitQueue{
private:
  queue<PCB*> queue;
public:
  void enqueue(PCB* process) {
    queue.push(process);
  }

  PCB* dequeue() {
    if(queue.empty()) return nullptr;
    PCB* process = queue.front();
    queue.pop();
    return process;
  }

  bool isEmpty() const {
    return queue.empty();
  }
};