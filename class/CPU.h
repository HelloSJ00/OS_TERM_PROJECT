#ifndef CPU_H
#define CPU_H

#include "User.h"  // User 클래스 포함

class CPU {
  User* current_user;  // 현재 실행 중인 User 객체
  int time_slice;      // 현재 프로세스에 할당된 타임 슬라이스

public:
  // 기본 생성자
  CPU();

  // User 객체를 통해 프로세스를 할당
  void assign_process(User* user, int slice);

  // CPU 사이클을 처리하고 타임 슬라이스 확인
  void tick();

  // 현재 프로세스를 반환하고 CPU를 비웁니다.
  User* release_process();

  // CPU가 비어 있는지 확인
  bool is_idle() const;
};

#endif  // CPU_H
