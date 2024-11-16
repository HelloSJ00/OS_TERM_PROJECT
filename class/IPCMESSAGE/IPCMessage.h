#ifndef IPC_MESSAGES_H
#define IPC_MESSAGES_H

#include <sys/types.h>

// Scheduler -> CPU 메시지 구조체
struct IPCMessageToCPU {
    long mtype;  // 메시지 타입 (1로 설정)
    pid_t pid;   // 프로세스 ID
};

// Scheduler -> IOdevice 메시지 구조체
struct IPCMessageToIODevice {
    long mtype;  // 메시지 타입
    pid_t pid;   // 프로세스 ID
};

// CPU -> Scheduler 메시지 구조체
// IOdevice -> Scheduler 메시지 구조체
struct IPCMessageToScheduler {
    long mtype;       // 메시지 타입 (1로 설정)
    pid_t pid;        // 프로세스 ID
    int cpu_burst;    // 남은 CPU 버스트
    int io_burst;     // 남은 IO 버스트
    bool time_slice_expired;  // 타임 슬라이스 만료 여부
    bool io_complete;  // IO 완료 여부
};

#endif  // IPC_MESSAGES_H
