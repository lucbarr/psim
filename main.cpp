#include <queue>
#include <vector>
#include <iostream>

using namespace std;

struct Process {
  Process(int c, int i) : cpub(c), cpuc(c), ioc(i) { id = count++; }
  Process(int c, int i, int id) : cpub(c), cpuc(c), ioc(i), id(id) {}
  int cpub; // CPU burst;
  int cpuc; // Current CPU counter
  int ioc;  // IO count;
  int id;

  void print ( int burst ){
    cout << "|" ;
    for (int i = 0 ; i < burst/20 ; ++i) cout << "-" ;
    cout << "P" << id ;
    for (int i = 0 ; i < (burst+10)/20 ; ++i) cout << "-" ;
    cout << "|" ;
  }
  static int count;
};

int Process::count = 0;


class PQueue {
  public:
    PQueue() {}
    virtual Process* pop() = 0;

    Process* front(){
      return processes.front();
    }
    void push (Process* p){
      processes.push(p);
    }
    int size() { return processes.size(); }
    PQueue* next;
  protected:
    queue<Process*> processes;
};

class RoundRobin : public PQueue{
  public:
    RoundRobin(int q, bool silent = false) : quantum(q), silent(silent) {}
    virtual Process* pop(){
      if ( processes.empty() ) return nullptr;

      auto front = processes.front();
      processes.pop();
      if ( front == nullptr ) return nullptr; // not sure about this

      if (front->cpuc > quantum){
        // For counter greater than quantum, decrement and poped process is
        // process with updated counter
        front->cpuc -= quantum;
        if(!silent) front->print ( quantum );
      }
      else {
        // Otherwise, checks if there is an io operation remaining and returns
        // the process with refreshed counter and decremented io counter if so
        if(!silent) front->print ( front->cpuc );
        if( front->ioc == 0 ) return nullptr;
        else {
          front->ioc --;
          front->cpuc = front->cpub;
        }
      }
      return front;
    }

  private:
    int quantum;
    bool silent;
};


class FCFS : public PQueue{
  public:
    FCFS(int l) : limit(l) {}
    virtual Process* pop();
  private:
    int limit;
};

class Monitor {
  public:

    Monitor() {
      IOqueue = new RoundRobin(20, true);
    }
    void run() { while(step()); }

    void linkQueue( PQueue* queue ) {
      int size = queues.size();
      if (size>0) {
        queues[size-1]->next = queue;
      }
      queues.emplace_back(queue);
      queue->next = queues[0];
    }

  private:
    vector<PQueue*> queues;
    PQueue* IOqueue;

    bool step(){
      // TODO : sync IO with CPU
      bool io = stepIO();
      for (auto q : queues){
        if ( q->size() > 0 ){
          auto done = q->pop();
          if( done!=nullptr ) {
            q->next->push( done );
            IOqueue->push ( new Process(20,0, done->id) ); // TODO: Change IOqueue so
                                                           // doesn't allocate every time
          }
          return true;
        }
      }
      return io;
    }
    bool stepIO(){
      return IOqueue->pop() != nullptr;
    }
};

int main (){
  Process processes[] = {
    { 20,0 },
    { 30,0 },
    { 40,0 },
    { 60,0 },
    { 10,0 }
  };
  Monitor m;
  PQueue* q = new RoundRobin(10);
  for ( auto& p : processes ){
    q->push(&p);
  }
  m.linkQueue(q);
  m.run();
  cout << endl;
  return 0;

}

