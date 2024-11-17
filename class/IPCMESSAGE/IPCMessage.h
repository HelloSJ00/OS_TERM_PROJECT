#ifndef IPC_MESSAGES_H
#define IPC_MESSAGES_H

#include <sys/types.h>
// CPU -> User 메시지 구조체
struct IPCMessageToUser {
    long mtype;  // 메시지 타입
    pid_t sender_pid;   // 메시지를 보낸 프로세스의 PID
    pid_t receiver_pid; // 메시지를 받을 프로세스의 PID
};

// User -> Scheduler 메시지 구조체
struct IPCMessageToScheduler {
    long mtype;       // 메시지 타입 (MSG_CPU_COMPLETION 또는 MSG_IO_COMPLETION)
    pid_t pid;   // 메시지를 보낸 프로세스의 PID
    int cpu_burst;    // 남은 CPU 버스트
    int io_burst;     // 남은 IO 버스트
};

#endif  // IPC_MESSAGES_H
