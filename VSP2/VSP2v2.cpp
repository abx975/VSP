#include <string>
#include <iostream>
#include <stdexcept>
#include <vector>

#include "caf/all.hpp"
#include "caf/io/all.hpp"

using std::cout;
using std::endl;
using std::cerr;
using std::endl;
using namespace caf;

using add_atom = atom_constant<atom("add")>;

struct config : actor_system_config {
  std::string host = "localhost";
  uint16_t port = 9900;
  size_t num_workers = 2;
  bool server = false;
  config() {
    opt_group{custom_options_, "global"}
    .add(host, "host,H", "hostname of server")
    .add(port, "port,p", "IP port for publish/remote_actor")
    .add(num_workers, "num-workers,w", "number of workers (in manager mode)")
    .add(server, "server,s", "run as server");
  }
};

struct state {
  actor worker;
  std::vector<actor> workers;
};

//Verhalten der Worker
behavior workerBeh(event_based_actor* self) {
  return {
      [=](int x, int y) -> int {
      aout(self) << "worker erreicht" <<endl;
      if(x==1 && y==1){return -1;}
      int i = 1;
      int res=x*y;
      while(i<3){
        res=res*res;
        i++;
      }
      return res;
    }
  };
}

//Verhalten der Manager
behavior managerBeh(stateful_actor<state>* self,int n) {
  //self->state.worker = self->spawn(workerBeh);
  for (auto i = 0; i < n; ++i){
    self->state.workers.emplace_back(self->spawn(workerBeh));
  }
  return {
    //[=](add_atom a, int x, int y) {
    [=](int x, int y) {
      
      aout(self) << "manager erreicht" <<endl;
      int i=0;
      for(auto& worker: self->state.workers){
        self->delegate(worker, x, y);
        aout(self) << "sende an worker " << i << endl;
        i++;
      }
    }
  };
}


void coordBeh(event_based_actor* self, const actor& server, std::vector<actor> managers) {  
  for(auto& manager : managers){
    self->send(manager,257,648);
  }
  self->become (
    [](int p){
      cout << "Koordinator erhält: " << p << std::endl;
    }
  );
}

///////////////////////////////////////////////////////////////////////////////////
//todo: if/else einkommentieren für Prozesse auf separaten Rechnern
void caf_main(actor_system& sys, const config& cfg) {

  auto& mm = sys.middleman();
 // if (cfg.server) {
    auto p = mm.publish(sys.spawn(managerBeh,cfg.num_workers), cfg.port);
    if (!p){
      cerr << "unable to publish actor: " << sys.render(p.error()) << "\n";
    }else{
      cout << "math actor published at port " << *p << "\n";
    }
  //}else{
    auto x = mm.remote_actor(cfg.host, cfg.port);
    if (!x){
      cerr << "unable to connect to server: " << sys.render(x.error()) << "\n";
    }else {
      std::vector<actor> managers;
      managers.emplace_back(*x);
      auto coordinator = sys.spawn(coordBeh,*x,managers);
      
    }
 }


CAF_MAIN(io::middleman)


