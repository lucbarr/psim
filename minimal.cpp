#include <iostream>
#include <vector>
#include <queue>

#define QUANTUM  10
#define IO_TIME  20
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
  int tick; // CPU burst now (to save);
  int cpuc; // Current CPU counter
  int iob;  // current IO burst counter
  int ioc;  // IO count;
  int wait; // FCFS waiting count;
  int id;
  static int count;
  void debug() {
    cout << "{" <<  cpub << ", " << cpuc << ", " << iob << ", " << ioc << ", " << wait << ", " << id << "}" << endl;
  }
};

void printInterval( Interval i ){
  const int burst = i.delta;
  const int id = i.id;
  cout << "|" ;
  for (int i = 0 ; i < burst/20 ; ++i) cout << "-" ;
  cout << "P" << id ;
  for (int i = 0 ; i < (burst+10)/20 ; ++i) cout << "-" ;
  cout << "|" ;
}

int Process::count = 0;

//NOTE: this pop process could be done faster by implementing a linked list instead,
//      since we will always need to iterate through the list whenever it is in the
//      second queue, due to the 30ms rule
struct PQueue{
  PQueue() : next(nullptr) {}
  vector<Process*> processes;
  PQueue* next;
  PQueue* ioq;

  void push( Process* p ) { processes.emplace_back( p ); }

  Process* pop (){
    if ( processes.empty() ) return nullptr;
    auto res = processes[0];
    processes.erase( processes.begin() );
    return res; // poping does not deallocate the process;
    // it keeps existing but can be placed in other queue
  }
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
  q1.next = &q1; // q1.next = &q2;
  q2.next = &q1;
  q1.ioq = &ioq;
  q2.ioq = &ioq;
  ioq.next = &q1;

  Process procs[] = {
    { 20, 0 }
  };
  for ( auto& p : procs ) q1.push(&p);

  for (int i = 0 ; i < 19 ; ++i ) stepRR(q1,ans);

#if 1
  cout << "Processes in q1" << endl << "========================" << endl;
  for (auto p : q1.processes ) p->debug();
  cout << "========================" << endl;

  cout << "Processes in q2" << endl << "========================" << endl;
  for (auto p : q2.processes ) p->debug();
  cout << "========================" << endl;

  cout << "Processes in ioq" << endl << "========================" << endl;
  for (auto p : ioq.processes ) p->debug();
  cout << "========================" << endl;
#endif

#if 0
  while (1){
    // if not empty, run q1:
    // else run q2,
    // run ioq regardless
    // if q1 and q2 empty : io generates ocious time
    // until q1 is not empty
  }
#endif
  for ( auto i : ans ) {
    printInterval(i);
  }
}

// increases cpuc burst counter until it reaches the cpu burst
// wait a second, just found a bug...
// maybe decreasing cpuc is better ?? ... it changes the whole thing up though :X
bool stepRR ( PQueue& q , vector<Interval>& v){
  if ( q.processes.empty() ) return false;
  auto p = q.processes[0];
  p->cpuc++;
  if ( p->cpuc == p->cpub ) {
    if (p->ioc>=0 && q.pop()!= nullptr) {
      q.ioq->push(p);
      p->ioc--;
    }
    // if ioc>0, insert into io queue , reset cpuc
    // p->ioc--;
    // concat interval to v
    v.push_back( { p->cpuc-p->tick, p->id } );
  }
  if ( p->cpuc ==  QUANTUM && p->cpub!=QUANTUM ) { // TODO: this is ugly
    // pushes the process to the next queue
    p->tick+=QUANTUM;
    if (q.pop()!= nullptr) q.next->push(p);
    // insert into q2, don't reset cpuc
    // concat interval to v
    v.push_back( { QUANTUM, p->id } );
  }
  return true;
}

bool stepFCFS ( PQueue& q , vector<Interval>& v){
  if (q.processes.empty()) return false;

  // update wait
  for (size_t i = 1 ; i < q.processes.size() ; ++i){ // skip first process
    q.processes[i]->wait++;
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
  if ( p->iob == IO_TIME ) {
    // insert into q1,
    // reset iob
  }

  return true;
}

