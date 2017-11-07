// C++ standard library includes
#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <unordered_map>

// CAF includes
#include "caf/all.hpp"
#include "caf/io/all.hpp"

// Boost includes
CAF_PUSH_WARNINGS
#include <boost/multiprecision/cpp_int.hpp>
CAF_POP_WARNINGS

// Own includes
#include "is_probable_prime.hpp"
#include "int512_serialization.hpp"

using std::cout;
using std::cerr;
using std::endl;
using std::vector;
using std::string;
using std::pair;
using std::unordered_map;

using boost::multiprecision::int512_t;

using namespace caf;
//////////////////////////////////////////////////////
/**
 * @author Nils Eggebrecht
 * @author Lennart Hartmann
 * @version 5.11.2017
 */
#include <string>
#include <iostream>
#include <vector>
#include <algorithm>
#include "caf/all.hpp"
#include "caf/io/all.hpp"
#include <bits/stdc++.h>
#include <boost/multiprecision/cpp_int.hpp>
#include <time.h>
#include <sys/time.h>
#include "int512_serialization.hpp"

#define NR_OF_WORKERS 4
#define IP "localhost"
#define PORT 9900
#define IS_SERVER true
#define IS_CLIENT true

using namespace caf;
using namespace std;
using namespace boost::multiprecision;
using namespace boost::multiprecision::literals;


using std::vector;
using std::cerr;
using std::cerr;
using std::endl;
using namespace caf;

#include <string>
#include <iostream>
#include <vector>

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

using add_atom = atom_constant<atom("add")>;
using rmv_atom = atom_constant<atom("rmv")>;

//persistent memory for manangers
struct state {
    actor worker;
    actor UI;
    std::vector<actor> workers;
    int value=-1;
};

std::ostream& operator<<(std::ostream& out, const expected<message>& x) {
    return out << to_string(x);
}

class Worker{
public:

private:
    int count=0;
};

//behavior of workers
behavior workerBeh(event_based_actor* self) {
    self->set_exit_handler([=](const exit_msg& xm) {
        cout << "< Worker recognized client died >" << endl;
        self->quit();
    });

    return {
            [=](int N) {
                aout(self) << "worker erreicht" << endl;
                if (!self->mailbox().empty()) {
                    cerr << "worker got a new job";
                    self->quit(sec::request_receiver_down);
                }
                return N / 2;
            }
    };
}

behavior UIBeh(event_based_actor* self){
    //todo: implementation
    aout(self) << "UI spawned" << endl;

    return {
        [&](int input){
            aout(self) << "UI erreicht" <<endl;
            aout(self) << "add or remove manger? Type + or - \n >> ";
            cin >> input;
            return input;
        }
    };
}

//behaviour of managers
behavior managerBeh(stateful_actor<state>* self, int noOfWorkers) {
    //initialisation
    //self->state.UI=self->spawn(UIBeh);

    self->set_exit_handler([=](const exit_msg& xm) {
        cout << "< Manager recognized client died >" << endl;
#if IS_CLIENT
        //Kills process if it doubles as client and server
        self->quit();
#endif
        for(auto worker: self->state.workers){
            self->send(worker,  xm);
        }
    });

    self->set_down_handler([=](const down_msg& dm) {
        cout << "<<restarting terminated worker>>\n";
        self->state.workers.erase(std::remove(self->state.workers.begin(),
                                              self->state.workers.end(), dm.source), self->state.workers.end());
        // <monitored> is a shortcut for calling monitor() afterwards
        self->state.workers.emplace_back( self->spawn<monitored>(workerBeh));
    });

    for (auto i = 0; i < noOfWorkers; ++i){
        auto worker=self->spawn(workerBeh);
        self->monitor(worker);
        self->state.workers.emplace_back(worker);
    }

    //self->send(self->state.UI,1);


    //actual bahaviour
    return {
            [=](int N) {
                aout(self) << "manager erreicht" <<endl;
                int i=0;
                for(auto& worker: self->state.workers) {
                    self->delegate(worker, N);
                    aout(self) << "sende an worker " << i << endl;
                    i++;
                }
            //////////// "threadsafe", aber wie aufrufen??? ///////////////
            },[=](add_atom,actor_system& sys) {
                aout(self) << "adding worker" << endl;
                self->state.workers.emplace_back(sys.spawn(workerBeh));
            },
            [=](rmv_atom) {
                aout(self) << "removing worker" << endl;
                self->state.workers;
                if(!self->state.workers.empty())self->state.workers.pop_back();

            }
    };
}


//Add a new server ////////////// stürzt zur Laufzeit ab ////////////////////
void addManager(const string &ip_addr, const uint16_t &port, scoped_actor &coordinator, vector<actor>& managers, actor_system& sys ){
    cout << "start adding manager"<< endl;
    auto &mm = sys.middleman();
    cout << ". adding manager"<< endl;
    auto man_ptr = mm.remote_actor(ip_addr, port); // stürzt hier ab!
    cout << ".. adding manager"<< endl;
    //if (!man_ptr){
    if (true){
        //cerr << "unable to connect to server: " << sys.render(man_ptr.error()) << "\n";
        cerr << "unable to connect to server: \n";// << sys.render(man_ptr.error()) << "\n";
    }else {
        managers.emplace_back(*man_ptr);
        //establish link
        coordinator->link_to(*man_ptr);
    }
    cout << "... adding manager"<< endl;
}

//remove a server ////////////// stürzt zur Laufzeit ab ////////////////////
void rmvManager(vector<actor>& managers){
    cout << "removing manager"<< endl;
    managers.pop_back();
}


void caf_main(actor_system& sys) {
    auto& mm = sys.middleman();

#if IS_SERVER
    //server-actor
    actor manager = sys.spawn(managerBeh,NR_OF_WORKERS);
    //auto ba= sys.spawn(blocking_calculator_fun);

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

    //available servers
    vector<pair<string,uint16_t>> serverList;
    pair<string,uint16_t> server1(IP,PORT);
    serverList.emplace_back(server1);

    //lookup managers
    for(pair<string,uint16_t> server : serverList){
        //auto remotePtr = mm.remote_actor(IP, PORT);
        auto remotePtr = mm.remote_actor(server.first, server.second);
        if (!remotePtr){
            cerr << "unable to connect to server: " << sys.render(remotePtr.error()) << "\n";
        }else {
            managers.emplace_back(*remotePtr);
            //establish link
            self->link_to(*remotePtr);
        }
    }
////////////////// coordinator behaviour ///////////////////////
    cout << "Primfaktorberechnung\nZu faktorisierende Zahl: ";
    int n;
    cin >> n;
    //static int n=16;
    cout << "launching coordinator" << endl;


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




/**
namespace {

struct config : actor_system_config {
  string host = "localhost";
  uint16_t port = 0;
  size_t num_workers = 0;
  string mode = "server";
  config() {
    add_message_type<int512_t>("int512_t");
    add_message_type<vector<int512_t>>("vector<int512_t>");
    opt_group{custom_options_, "global"}
    .add(host, "host,H", "server host (ignored in server mode)")
    .add(port, "port,p", "port")
    .add(num_workers, "num-workers,w", "number of workers (in manager mode)")
    .add(mode, "mode,m", "one of 'server', 'manager' or 'client'");
  }
};

void run_server(actor_system& sys, const config& cfg) {
  cout << "run_server: implement me" << endl;
}

void run_manager(actor_system& sys, const config& cfg) {
  cout << "run_manager: implement me" << endl;
}

void run_client(actor_system& sys, const config& cfg) {
  cout << "run_client: implement me" << endl;
}

// dispatches to run_* function depending on selected mode
void caf_main(actor_system &sys, const config &cfg) {
  using map_t = unordered_map<string, void (*)(actor_system &, const config &)>;
  map_t modes{
      {"server", run_server},
      {"manager", run_manager},
      {"client", run_client}
  };
  auto i = modes.find(cfg.mode);
  if (i != modes.end())
    (i->second)(sys, cfg);
  else
    cerr << "*** invalid mode specified" << endl;
}

} // namespace <anonymous>

CAF_MAIN(io::middleman)
*/
