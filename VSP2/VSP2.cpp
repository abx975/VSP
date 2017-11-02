#include <string>
#include <iostream>
#include <stdexcept>
#include <vector>

#include "caf/all.hpp"
#include "caf/io/all.hpp"

using std::cout;
using std::endl;
using std::cerr;
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
    //[=](add_atom, int x, int y) -> result<int> {
      [=](int x, int y) -> int {
      aout(self) << "worker erreicht" <<endl;
    
    /**
      if (x % 2 == 1 || y % 2 == 1) {
        self->quit(sec::invalid_argument);
        return make_error(sec::invalid_argument,
                          "I don't do odd numbers!");
                 
      }*/
      return x + y;
    }
  };
}

//Verhalten der Manager
behavior managerBeh(stateful_actor<state>* self) {
  self->state.worker = self->spawn(workerBeh);
  for (auto i = 0; i < 5; ++i){
    self->state.workers.emplace_back(self->spawn(workerBeh));
  }
  
  /**self->link_to(self->state.worker);
  self->set_exit_handler([=](const exit_msg& dm) {
    if (dm.source == self->state.worker) {
      cout << "<<<restart failed worker>>>\n";
      // <linked> is a shortcut for calling link_to() afterwards
      self->state.worker = self->spawn<linked>(workerBeh);
    }
  });
  */
  return {
    //[=](add_atom a, int x, int y) {
    [=](int x, int y) {
      //self->mailbox().empty(); 
      aout(self) << "manager erreicht" <<endl;
      //return self->delegate(self->state.worker, a, x, y);
      return self->delegate(self->state.worker, x, y);
    }
  };
}


void coordBeh(event_based_actor* self, const actor& server) {  
  self->send(server,5,4);
  self->send(server, 1,3);
  self->send(server,257,648);
  self->become (
    [](int p){
      cout << p << std::endl;
    }
  );
}

/**
std::ostream& operator<<(std::ostream& out, expected<message>* x) {
  return out << to_string(x);
}
*/
///////////////////////////////////////////////////////////////////////////////////
//todo: if/else einkommentieren für Prozesse auf separaten Rechnern
void caf_main(actor_system& sys, const config& cfg) {
  auto& mm = sys.middleman();
 // if (cfg.server) {
    auto p = mm.publish(sys.spawn(managerBeh), cfg.port);
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
      auto coordinator = sys.spawn(coordBeh,*x);
    }
 // }
//////////////////////// zur Nutzung im gleichen Prozess //////////////////////   
  //auto manager = sys.spawn(managerBeh);
  //auto coordinator = sys.spawn(coordBeh,manager);
}

CAF_MAIN(io::middleman)

