#ifndef IPC_MESSAGE_H
#define IPC_MESSAGE_H

#include <sys/types.h>

// IPC 메시지 구조체
struct IPCMessage {
    long mtype;       // 메시지 타입
    pid_t pid;        // 프로세스 ID
    int cpu_burst;    // 남은 CPU 버스트
    int io_burst;     // 남은 IO 버스트
};

#endif  // IPC_MESSAGE_H
