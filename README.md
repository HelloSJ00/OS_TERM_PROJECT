# OS Term Project #1

**Prof. Eun-Seok Ryu**

# 객체지향적으로 만들어본 Multiple Feedback Queue Scheduler

---

## 실행

| **Action**           | **Details**                                                                                                                                                                                                              |
| -------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| **Clone Repository** | `git clone <repository-url>`                                                                                                                                                                                             |
| **환경**             | - **Operating System:** macOS<br>- **Editor:** Visual Studio Code                                                                                                                                                        |
| **컴파일 명령어**    | `g++ -std=c++17 -arch arm64 -o main main.cpp ./class/RUNQUEUE/FeedbackQueue.cpp ./class/CPU/CPU.cpp ./class/IODEVICE/IOdevice.cpp ./class/SCHEDULER/Scheduler.cpp ./class/USER/User.cpp ./class/WAITQUEUE/WaitQueue.cpp` |
| **실행**             | `./main`                                                                                                                                                                                                                 |

---

## Project 1: Simple Scheduling

### Programming Assignment #1

**Due Date:** Nov. 20, 2024 (11:59pm KST)

---

## 1. Background: Round-Robin Scheduling

Round Robin (RR) scheduling algorithm is specifically designed for **time-sharing systems**. It is a preemptive version of first-come, first-served scheduling. Processes are dispatched in a **first-in-first-out** sequence, but each process can run for only a limited amount of time. This time interval is known as a **time-slice** or **quantum**.

It is similar to FIFO scheduling but includes **preemption**, enabling the switch between processes.

---

## 2. Basic Requirements for CPU Scheduling

### Parent Process

1. **Child Process Creation**
   - Create **10 child processes** from the parent process.
2. **Scheduling Policy**

   - Schedule child processes according to the **Round-Robin scheduling policy**.
   - Define scheduling parameters such as **time quantum** and **timer tick interval**.

3. **Timer Signal**

   - Periodically receive the **ALARM signal** by registering a timer event (refer to the `setitimer` system call).
   - The ALARM signal acts as a **periodic timer interrupt** (time tick).

4. **Queues**

   - Maintain the following:
     - **Run-queue:** Holds child processes in the ready state.
     - **Wait-queue:** Holds child processes in the waiting state.

5. **Scheduling Execution**

   - Account for the **remaining time quantum** of all child processes.
   - Allocate a **time slice** to the child process using **IPC messages** (refer to `msgget`, `msgsnd`, and `msgrcv` system calls).
   - Use the `IPC_NOWAIT` flag for IPC.

6. **Waiting Time**
   - Account for the **waiting time** of all child processes.

---

### Child Process

1. **Simulating User Processes**

   - Simulate a user process with an infinite loop of **dynamic CPU-burst** and **I/O-burst**.
   - Begin execution with two parameters: `cpu_burst` and `io_burst`. These values are **randomly generated**.

2. **Receiving Time Slice**

   - When receiving a time slice from the OS via IPC, make progress during the **CPU-burst phase**.

3. **Decreasing CPU-Burst**
   - On receiving the IPC message, decrease the **CPU-burst** value.

---

## 3. Optional Requirements for I/O Involvement

### Child Process

1. **I/O Requests**
   - Make **I/O requests** after completing a CPU-burst.
   - If `CPU-burst` reaches zero:
     - Send an IPC message to the parent process with the next **I/O-burst** value.

---

### Parent Process

1. **Handling I/O Requests**

   - On receiving an IPC message, move the child process from the **run-queue** to the **wait-queue**.

2. **Managing I/O-Burst**

   - Remember the child’s **I/O-burst** value.
   - On every time tick:
     - Decrease the I/O-burst value of all processes in the **wait-queue**.

3. **I/O Completion**

   - When a child process completes I/O:
     - Move it from the **wait-queue** back to the **run-queue** for scheduling.

4. **Scheduling Events**
   - Trigger scheduling upon the following events:
     - Expiry of **time quantum** for a process.
     - A process completes its **CPU-burst** and makes an **I/O request**.

---

## 4. Program Output: Hard-Copy Report

1. **Output Format**

   - Print scheduling operations in the following format:
     ```
     At time t, process pid gets CPU time, remaining CPU-burst.
     Run-queue dump: ...
     Wait-queue dump: ...
     ```

2. **Output File**

   - Write all operations to `schedule_dump.txt`.

3. **Execution Duration**

   - All processes must run for **at least 1 minute**.

4. **Output Period**

   - Print scheduling operations during **0 ~ 10,000 time ticks**.

5. **System Calls and Functions**
   - Recommended references:
     - `sprintf`, `open`, `write`, `close`.

---

## Notes

- **Language Recommendation:** C or C++ (but not mandatory).
- For questions, contact **TA Jaeyeol Choi** (jaychoi@skku.edu).
