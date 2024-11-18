#ifndef CPU_H
#define CPU_H

#include "../IPCMESSAGE/IPCMessage.h"
#include "../PCB/PCB.h"
#include "../CPU/CPU.h"
#include "../USER/User.h"  // User 클래스 포함
class CPU {
  PCB* process;
  int time_slice;      // 현재 프로세스에 할당된 타임 슬라이스

public:
  // 기본 생성자
  CPU();

  // User 객체를 통해 프로세스를 할당
  void assign_process(PCB* cur_process, int slice);

  // CPU 사이클을 처리하고 타임 슬라이스 확인
  void tick();

  // 현재 프로세스를 반환하고 CPU를 비웁니다.
  PCB* release_process();

  // CPU가 비어 있는지 확인
  bool is_idle() const;

  int getTimeSlice();
};

#endif  // CPU_H
