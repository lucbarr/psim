#include <iostream>
#include <vector>
#include <queue>

#define QUANTUM  20
#define TIME_THR 30

using namespace std;

struct Interval{
  Interval( int d, int id ) : delta(d), id(id) {}
  int delta;
  int id;
};

struct Process {
  Process(int c, int i) : cpub(c), cpuc(0), iob(0), ioc(i), wait(0) { id = count++; }
  Process(int c, int i, int id) : cpub(0), cpuc(c), iob(0), ioc(i), wait(0), id(id) {}
  // TODO: these names are not that intuitive...
  int cpub; // CPU burst;
  int cpuc; // Current CPU counter
  int iob;  // current IO burst counter
  int ioc;  // IO count;
  int wait; // FCFS waiting count;
  int id;
  static int count;
};

int Process::count = 0;

struct PQueue{
  PQueue() : next(nullptr) {}
  vector<Process*> processes;
  void push( Process* p ) { processes.emplace_back( p ); }
  PQueue* next;
  PQueue* ioq;
};

bool stepRR   ( PQueue& q , vector<Interval>& v);
bool stepFCFS ( PQueue& q , vector<Interval>& v);
bool stepIO   ( PQueue& q , vector<Interval>& v);

void printInterval( Interval i );

int main (){
  PQueue q1;
  PQueue q2;
  PQueue ioq;
  vector<Interval> ans;
  q1.next = &q2;
  q2.next = &q1;
  q1.ioq = &ioq;
  q2.ioq = &ioq;
  ioq.next = &q1;
  while (1){
    // run q1, if empty:
    // run q2,
    // run ioq regardless
  }
  for ( auto i : ans ) {
    printInterval(i);
  }
}

bool stepRR ( PQueue& q , vector<Interval>& v){
  if (q.processes.empty()) return false;
  auto p = q.processes[0];
  p->cpuc++;
  if ( p->cpuc == p->cpub ) {
    // if ioc>0, insert into io queue , reset cpuc
    // p->ioc--;
    // concat interval to v
  }
  else if ( p->cpuc ==  QUANTUM ) {
    // insert into q2, don't reset cpuc
    // concat interval to v
  }
  return true;
}

bool stepFCFS ( PQueue& q , vector<Interval>& v){
  if (q.processes.empty()) return false;

  // update wait
  for ( auto& p : q.processes ) {
    p->wait++;
    if ( p->wait == TIME_THR ){
      // insert into q1 logic;
    }
  }

  auto p = q.processes[0];
  p->cpuc++;
  if ( p->cpuc == p->cpub ) {
    // insert into io queue, reset cpuc
    // p->ioc--;
    // concat interval to v
  }

  return true;
}
bool stepIO   ( PQueue& q , vector<Interval>& v ){
  if (q.processes.empty()) return false;
  auto p = q.processes[0];
  p->iob++;
  if ( p->iob == p->ioc ) {
    // insert into q1,
    // reset iob
  }

  return true;
}
