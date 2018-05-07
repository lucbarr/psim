#include <queue>
#include <vector>
#include <iostream>

using namespace std;

struct Process {
  Process(int c, int i) : cpub(c), iob(i) { id = count++; }
  int cpub; // CPU burst;
  int iob; // IO burst;
  int id;

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
    void print ( int id, int burst ){
      cout << "|" ;
      for (int i = 0 ; i < burst/20 ; ++i) cout << "-" ;
      cout << "P" << id ;
      for (int i = 0 ; i < (burst+10)/20 ; ++i) cout << "-" ;
      cout << "|" ;
    }
    int size() { return processes.size(); }
    PQueue* next;
    queue<Process*> processes;
};

class RoundRobin : public PQueue{
  public:
    RoundRobin(int q) : quantum(q) {}
    virtual Process* pop(){
      if (processes.empty()) return nullptr;

      auto front = processes.front();
      processes.pop();
      if (front == nullptr) return nullptr; // not sure about this

      if (front->cpub > quantum){
        front->cpub -= quantum;
        print ( front->id , quantum );
        return front;
      }
      else {
        print ( front->id ,front->cpub );
        return nullptr;
      }
    }

  private:
    int quantum;
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


  void run () { while(step()); }

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

  bool step(){
    for (auto q : queues){
      if (q->size() > 0 ){
        auto done = q->pop();
        if(done!=nullptr) q->next->push( done );
        return true;
      }
    }
    return false;
  }
};

int main (){
  Process processes[] = {
    { 20,10 },
    { 30,10 },
    { 40,10 },
    { 60,10 },
    { 10,10 }
  };
  Monitor m;
  PQueue* q = new RoundRobin(20);
  for ( auto& p : processes ){
    q->push(&p);
  }
  m.linkQueue(q);
  m.run();
  cout << endl;
  return 0;

}

