#ifndef USER_H
#define USER_H

#include <iostream>
#include <sys/types.h>  // pid_t 타입을 위해 필요
#include "../IPCMESSAGE/IPCMessage.h"


class User {
public:
    pid_t pid;           // 프로세스 ID
    int cpu_burst;       // CPU 버스트 시간
    int io_burst;        // I/O 버스트 시간

    // 생성자
    User(pid_t p, int cpu, int io);

    // 부모로부터 명령 수신
    void receiveCommand();

    // 프로세스 정보 출력 (디버깅용)
    void printProcessInfo() const;

};

#endif  // USER_H
