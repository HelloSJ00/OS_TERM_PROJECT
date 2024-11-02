#include <iostream>
using namespace std;
class PCB {
public:
    int pid;
    int cpu_burst;
    int io_burst;
    int remaining_time;
    string state;  // "READY" | "WAITING"

    PCB(int p, int cpu, int io)
        : pid(p), cpu_burst(cpu), io_burst(io), remaining_time(cpu), state("READY") {}
};