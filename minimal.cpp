#include <iostream>
#include <vector>

#define QUANTUM  10
#define IO_TIME  20
#define TIME_THR 30
#define RESOLUTION 1

#define DEBUG 1

using namespace std;

struct Interval{
  Interval( int d, int id ) : delta(d), id(id) {}
  int delta;
  int id;
  void print() {
    const int burst = delta;
    cout << "|" ;
    for (int i = 0 ; i < burst/(2*RESOLUTION) ; ++i) cout << "-" ;
    if (id!=-1) cout << "P" << id ;
    else        cout << "IO" ;
    for (int i = 0 ; i < (burst+RESOLUTION)/(2*RESOLUTION) ; ++i) cout << "-" ;
    cout << "|" ;
  }
};

// Too many variables, maybe we can make this simpler...
struct Process {
  Process( int c, int i ) : cpub(c), tick(0), cpuc(0), iob(0), ioc(i), wait(0) { id = count++; }
  Process( int c, int i, int id ) : cpub(0), cpuc(c), iob(0), ioc(i), wait(0), id(id) {}
  // TODO: these names are not that intuitive...
  int cpub; // CPU burst;
  int tick; // CPU burst now (to save);
  int cpuc; // Current CPU counter
  int iob;  // current IO burst counter
  int ioc;  // IO count;
  int wait; // FCFS waiting count;
  int id;
  static int count;
  void debug() const {
    cout << "{" <<  cpub << ", " << cpuc << ", " << iob << ", " << ioc << ", " << wait << ", " << id << "}" << endl;
  }
};

int Process::count = 0;

//NOTE: this pop process could be done faster by implementing a linked list instead,
//      since we will always need to iterate through the list whenever it is in the
//      second queue, due to the 30ms rule
struct PQueue{
  PQueue() : next( nullptr ) {}
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
  void iopush( Process* p ){
    if ( pop()!= nullptr and p->ioc > 0 ) {
      ioq->push(p);
      p->ioc--;
      p->cpuc=0;
      p->tick=0;
    }
  }
  bool empty() const { return processes.empty(); }
};

bool stepRR     ( PQueue& q , vector<Interval>& v );
bool stepFCFS   ( PQueue& q , vector<Interval>& v );
bool stepIO     ( PQueue& q , vector<Interval>& v );
void updateWait ( PQueue& q );

int main (){
  PQueue q1;
  PQueue q2;
  PQueue ioq;
  vector<Interval> ans;
  q1.next = &q2; // q1.next = &q2;
  q2.next = &q1;
  q1.ioq = &ioq;
  q2.ioq = &ioq;
  ioq.next = &q1;

  Process procs[] = {
    { 8, 3 },
    { 40, 1 },
    { 10, 2 },
    { 30, 1 }
  };
  for ( auto& p : procs ) q1.push(&p);
  cout << "========================" << endl;
  cout << "Processes:" << endl;
  for ( auto  p : procs ) cout << "{ burst: " <<  p.cpub << "\t n of io: " << p.ioc << "}" << endl;
  cout << "========================" << endl;

  while (1) {

#if DEBUG
  cout << endl << "Processes in q1" << endl << "========================" << endl;
  for ( auto p : q1.processes ) p->debug();
  cout << "========================" << endl;

  cout << "Processes in q2" << endl << "========================" << endl;
  for ( auto p : q2.processes ) p->debug();
  cout << "========================" << endl;

  cout << "Processes in ioq" << endl << "========================" << endl;
  for ( auto p : ioq.processes ) p->debug();
  cout << "========================" << endl;
#endif

    bool donerr = stepRR(q1,ans);  // can add to ioq
    bool donefcfs = (donerr)?false:stepFCFS(q2,ans);
    updateWait(q2);

    stepIO(ioq,ans);
    if (!donerr and !donefcfs and !ioq.empty()){
      int count = 1;
      while (q1.processes.empty() and stepIO(ioq,ans)){
        count++;
      }
      ans.push_back( { count, -1 } );
    }
    if ( q1.empty() and q2.empty() and ioq.empty() ) break;
  }


  cout << "Gantt (each - represents a " << RESOLUTION << "ms in time):" << endl << endl;
  for ( auto i : ans ) i.print();
  cout << endl << endl << "========================" << endl;
  cout << "Intervals (pid -1 holds for io waiting cpu):" << endl;
  for ( auto i : ans ) cout << "{ dt:" << i.delta  << ", pid:" << i.id << "}" << endl;
  cout << "========================" << endl;
}

// increases cpuc burst counter until it reaches the cpu burst
// maybe decreasing cpuc is better ?? ... it changes the whole thing up though :X
bool stepRR ( PQueue& q , vector<Interval>& v ){
  if ( q.processes.empty() ) return false;
  auto p = q.processes[0];
  p->cpuc++;
  if ( p->cpuc == p->cpub ) { // TODO: duplicated code1
    // concat interval to v
    v.push_back( { p->cpuc-p->tick, p->id } );
    // try to push into IO queue
    q.iopush( p );
    return true; // eww
  }
  if ( p->cpuc-p->tick == QUANTUM ) {
    // concat interval to v
    v.push_back( { QUANTUM , p->id } );
    // update tick counter ( we have to save how many quantums has done )
    p->tick+=QUANTUM;
    // pushes the process to the next queue
    if ( q.pop()!= nullptr ) q.next->push(p); //should never be false by logic
  }
  return true;
}

void updateWait ( PQueue& q ){

  // update wait
  const size_t size = q.processes.size();
  for (size_t i = 0 ; i < size ; ++i){ // skip first process??
    auto p = q.processes[i];
    p->wait++;
    // check for waiting time has passed
    if ( p->wait >= TIME_THR ) {
      p->wait = 0 ;
      if ( q.pop()!= nullptr ) q.next->push(p);
    }
  }
}

bool stepFCFS ( PQueue& q , vector<Interval>& v){
  if ( q.processes.empty() ) return false;


  auto p = q.processes[0];
  p->cpuc++;
  if ( p->cpuc == p->cpub ) { // TODO: duplicated code1
    // concat interval to v
    v.push_back( { p->cpuc-p->tick, p->id } );
    // if ioc>0, insert into io queue , reset cpuc
    q.iopush( p );
  }

  return true;
}

bool stepIO   ( PQueue& q , vector<Interval>& v ){
  if ( q.processes.empty() ) return false;
  auto p = q.processes[0];
  p->iob++;
  if ( p->iob == IO_TIME ) {
    if ( q.pop()!= nullptr ) q.next->push( p );
    p->iob = 0;
  }

  return true;
}

