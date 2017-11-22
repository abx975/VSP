#include <string>
#include <iostream>
#include <vector>
#include <algorithm>
#include "caf/all.hpp"
#include "caf/io/all.hpp"

#define NR_OF_WORKERS 4
#define IP "localhost"
#define PORT 9900
#define IS_SERVER true
#define IS_CLIENT true

using std::vector;
using std::cout;
using std::cerr;
using std::endl;
using std::cin;
using namespace caf;

//persistent memory for manangers
struct state {
  actor worker;
  std::vector<actor> workers;
};

//behavior of workers
behavior workerBeh(event_based_actor* self) {
 
  return {
    [=](int N) {
      aout(self) << "worker erreicht" <<endl;
      self->quit(sec::invalid_argument);
      if( !self->mailbox().empty() ){ 
        cerr << "worker got a new job";  
        self->quit(sec::request_receiver_down);  
      }
      return N/2;
    }
  };
}

//behaviour of managers
behavior managerBeh(stateful_actor<state>* self, int noOfWorkers) {
  //initialisation  
  for (auto i = 0; i < noOfWorkers; ++i){
    auto worker=self->spawn(workerBeh);
    self->monitor(worker);
    self->state.workers.emplace_back(worker);
  }
  
  self->set_down_handler([=](const down_msg& dm) {
    cout << "<<<restart terminated worker>>>\n";
    self->state.workers.erase(std::remove(self->state.workers.begin(), 
      self->state.workers.end(), dm.source), self->state.workers.end());
    // <monitored> is a shortcut for calling monitor() afterwards
    self->state.workers.emplace_back( self->spawn<monitored>(workerBeh));  
  });

  //actual bahaviour
  return {
    [=](int N) {      
      aout(self) << "manager erreicht" <<endl;
      int i=0;
      for(auto& worker: self->state.workers){
        self->delegate(worker, N);
        aout(self) << "sende an worker " << i << endl;
        i++;
      }
    }   
  };
}

void caf_main(actor_system& sys) {
  auto& mm = sys.middleman();

#if IS_SERVER
  //server-actor                
  actor manager = sys.spawn(managerBeh,NR_OF_WORKERS);

  //publish manager
  auto p = mm.publish(manager, PORT);  
  if (!p){
    cerr << "unable to publish actor: " << sys.render(p.error()) << "\n";
  }else{
    cout << "math actor published at port " << *p << "\n";
  }
#endif

#if IS_CLIENT
  // scoped actor for ad hoc communication
  scoped_actor self{sys};
  
  //managers for coordinator to address
  std::vector<actor> managers;

  //lookup managers
  auto remotePtr = mm.remote_actor(IP, PORT);
  if (!remotePtr){
    cerr << "unable to connect to server: " << sys.render(remotePtr.error()) << "\n";
  }else {
    managers.emplace_back(*remotePtr);
  }

////////////////// coordinator behaviour ///////////////////////
  cout << "Primfaktorberechnung\nZu faktorisierende Zahl: " << endl;
  int n;
  cin >> n;
  //static int n=16;
  cout << "launching coordinator" << endl;
  //superloop  
  while(n>0){ 
    for(auto& manager : managers){
      self->request(manager, infinite, n).receive(
        [&](int p) {
          cout << "got p=" << p << "\n";
          n=p;
        },
        [&](error& err) {
          cout << "Error: " << sys.render(err) << "\n";
        }
      );
    }
  }
#endif
}

CAF_MAIN(io::middleman)

