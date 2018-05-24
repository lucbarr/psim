#include <iostream>
#include <vector>

#define QUANTUM  10
#define IO_TIME  20
#define TIME_THR 30
#define RESOLUTION 1

#define DEBUG 0

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
  Process( int c, int i ) : cpub(c),  cpuc(0), iob(0), ioc(i), wait(0),
  rr(0),fcfs(0)
  { id = count++; }
  // TODO: these names are not that intuitive...
  int cpub; // CPU burst;
  int cpuc; // Current CPU counter
  int iob;  // current IO burst counter
  int ioc;  // IO count;
  int wait; // FCFS waiting count;
  int rr;
  int fcfs;
  int id;
  static int count;
  void debug() const {
    cout << "{" <<  cpub << ", " << cpuc << ", " << iob << ", " << ioc << ", " << wait << ", " << id << ", " << rr << ", " << fcfs <<  "}" << endl;
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
    }
  }
  bool empty() const { return processes.empty(); }
};

bool stepRR     ( PQueue& q , vector<Interval>& v );
bool stepFCFS   ( PQueue& q , vector<Interval>& v );
bool updateRR     ( PQueue& q , vector<Interval>& v );
bool updateFCFS   ( PQueue& q , vector<Interval>& v );
bool updateIO   ( PQueue& q );
bool updateWait   ( PQueue& q );
bool stepIO     ( PQueue& q );
void stepWait ( PQueue& q, bool skip_first );
bool step       ( PQueue& q , bool isrr = true);

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

#if 0
    bool donerr = stepRR(q1,ans);  // can add to ioq
    bool donefcfs = (donerr)?false:stepFCFS(q2,ans);
    stepWait(q2);
#endif
    bool donerr = step(q1);
    bool donefcfs = (donerr)?false:step(q2,false);
    stepIO(ioq);
    stepWait(q2,donefcfs);
    if (donerr) updateRR(q1,ans);
    else updateFCFS(q2,ans);
    updateWait(q2);
    updateIO(ioq);

    if (!donerr and !donefcfs and !ioq.empty()){
      int count = 1;
      while (q1.processes.empty() and stepIO(ioq)){
        updateIO(ioq);
        count++;
      }
      ans.push_back( { count, -1 } );
    }
    if ( q1.empty() and q2.empty() and ioq.empty() ) break;
  }
  vector<Interval> merged;
  // merge intervals
  for (int i = 1 ; i < ans.size() ; ++i){
    int aux = ans[i-1].delta;
    while ( ans[i].id == ans[i-1].id ) {
      aux++;
      i++;
    }
    merged.push_back( { aux, ans[i-1].id } );
  }

  cout << "Gantt (each - represents a " << RESOLUTION << "ms in time):" << endl << endl;
  for ( auto i : merged ) i.print();
  cout << endl << endl << "========================" << endl;
  cout << "Intervals (pid -1 holds for io waiting cpu):" << endl;
  for ( auto i : merged ) cout << "{ dt:" << i.delta  << ", pid:" << i.id << " }" << endl;
  cout << "========================" << endl;
}

bool step ( PQueue& q,  bool isrr ){
  if ( q.processes.empty() ) return false;
  auto p = q.processes[0];
  p->cpuc++;
  if (isrr) p->rr++;
  else      p->fcfs++;
  return true;
}

bool updateRR( PQueue& q , vector<Interval>& v ){

  if ( q.processes.empty() ) return false;
  auto p = q.processes[0];
  v.push_back( { 1, p->id } );
  p->wait = 0;
  if ( p->cpuc == p->cpub ) { // TODO: duplicated code1
    q.iopush( p );
    return true;
  }
  if ( p->rr == QUANTUM ){
    if ( q.pop()!= nullptr ) q.next->push(p); //should never be false by logic
    p->rr = 0;
  }
  return true;

}

bool updateFCFS( PQueue& q , vector<Interval>& v ){
  if ( q.processes.empty() ) return false;
  auto p = q.processes[0];
  v.push_back( { 1, p->id } );
  p->wait = 0;
  if ( p->cpuc == p->cpub ) { // TODO: duplicated code1
    // concat interval to v
    // try to push into IO queue
    q.iopush( p );
    return true; // eww
  }
  return true;

}

bool updateIO ( PQueue& q ){
  if ( q.processes.empty() ) return false;
  auto p = q.processes[0];

   if ( p->iob == IO_TIME ) {
     if ( q.pop()!= nullptr ) q.next->push( p );
     p->iob = 0;
     p->rr = 0;
     p->fcfs = 0;
   }
   return true;
}

void stepWait ( PQueue& q, bool skip_first ){

  // update wait
  const size_t size = q.processes.size();
  size_t i = 0;
  if (skip_first) ++i;
  for (; i < size ; ++i){ // skip first process??
    auto p = q.processes[i];
    p->wait++;
    // check for waiting time has passed
  }
}

bool updateWait ( PQueue& q ){
  if ( q.processes.empty() ) return false;

    //processes.erase( processes.begin() );
  for ( int i = 0 ; i < q.processes.size() ; ++i ){
    auto p = q.processes[i];
    if ( p->wait == TIME_THR ){
      q.processes.erase( q.processes.begin()+i );
      q.next->push ( p );
    }
  }
  return true;
}

bool stepIO   ( PQueue& q ){
  if ( q.processes.empty() ) return false;
  auto p = q.processes[0];
  p->iob++;
  /*
     if ( p->iob == IO_TIME ) {
     if ( q.pop()!= nullptr ) q.next->push( p );
     p->iob = 0;
     }
     */

  return true;
}

