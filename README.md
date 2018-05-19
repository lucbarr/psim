psim
=====

A simple program to simulate process scheduling according to OS class IME 2018.

build
=====
You'll need g++ or any other c++ compiler (might change the Makefile CXX flag if so)
Haven't tested on Windows, only Linux with Fedora 26, but there might be no problem as long as you
have C++11.

to build, simply:
        
        $ make

to run:

        $ make run

scheduling policy
=================

The program should simulate a scheduling algorithm with multiple feedbacking queues, following these rules:

  - There are two queues Q0 and Q1;
  - All processes start at Q0;
  - Q0 uses round-robin with 10ms quantum;
  - Q1 uses FCFS;
  - Scheduling is done by priority with preemption. Q0 rules over Q1;
  - A process goes from Q0 to Q1 when preempted by time;
  - A process goes from Q1 back to Q0 if at least 30ms in it;
  - All CPU bursts last the same long;
  - There is only an I/O device on the system and can only attend to a process at a time;
  - Any I/O burst takes 20ms to be done;
  - The program must receive a cpu burst and number of io ops for each process from a set of processes;
  - The output shall be a resulting Gantt diagram from the scheduling algorithm;

TODO
====

  - [x] Implement Round-Robin logic
  - [x] Implement FCFS logic
  - [x] Discover how IO is gonna work
  - [x] Implement IO preemption stuff 
  - [x] Implement IO sync with CPU 
  - [ ] Improve Gantt printing
  - [ ] Test a lot
  - [ ] Make it pretty (refactor)

REFACTOR & BUGFIX
=================

  - [ ] Encapsulate messy stuff ( variable updates under step functions )
  - [ ] Problem when process in FCFS queue gets preempted by waiting process going to RR queue



