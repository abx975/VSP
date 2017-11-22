/**
 * @author Nils Eggebrecht
 * @author Lennart Hartmann
 * @version 8.11.2017
 */

// C++ standard library includes
#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <unordered_map>
#include <bits/stdc++.h>
#include <time.h>
#include <sys/time.h>

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
using std::find;
using std::vector;
using std::string;
using std::pair;
using std::unordered_map;
using boost::multiprecision::int512_t;

using namespace caf;
using namespace std;
using namespace boost::multiprecision;
using namespace boost::multiprecision::literals;

#define SERVER_IP "localhost"
#define SERVER_PORT 9900
#define COORD_PORT 9900
#define MANAGER_PORT 9900
#define NO_WORKERS 4

using add_mngr_atom = atom_constant<atom("add_mngr")>;
using add_client_atom = atom_constant<atom("add_client")>;
using add_worker_atom = atom_constant<atom("add_worker")>;
using ack_atom = atom_constant<atom("ack")>;
using request_atom = atom_constant<atom("request")>;
using reply_atom = atom_constant<atom("reply")>;
using cancel_atom = atom_constant<atom("cancel")>;
using new_job_atom = atom_constant<atom("new_job")>;
using deliver_atom = atom_constant<atom("deliver")>;


namespace {

struct config : actor_system_config {
    string host = "localhost";
    uint16_t port = 0;
    size_t num_workers = 0;
    string mode = "server";
//    string mode = "manager";
//    string mode = "client";

    config() {
        //add_message_type<int512_t>("add_client");
        add_message_type<int512_t>("int512_t");
        add_message_type<vector<int512_t>>("vector<int512_t>");
        opt_group{custom_options_, "global"}
        .add(host, "host,H", "server host (ignored in server mode)")
        .add(port, "port,p", "port")
        .add(num_workers, "num-workers,w", "number of workers (in manager mode)")
        .add(mode, "mode,m", "one of 'server', 'manager' or 'client'");
     }
};

int512_t evaluate(int512_t N){
    return N;
}

//worker behaviour
behavior worker(event_based_actor* self) {
        return {
                //stub
                [&](request_atom a,int512_t N) {
                    return N / 2;
                },
                [&](cancel_atom){
                    self->quit(sec::request_receiver_down);
                }
        };
}


//persistant memory for managers
struct managerState {
    vector<actor> workers;
    actor client;
    int noWorkers = 5;
    int512_t N;
};

//manager behaviour
behavior manager(stateful_actor<managerState>* self, int number){       //, actor_system* sys_ptr){
//    auto& mm = sys_ptr->middleman();
//    mm.publish(self,COORD_PORT);
    for(int i=0; i< number;i++){
        self->state.workers.emplace_back(self->spawn<monitored>(worker));
    }
    self->state.noWorkers=self->state.workers.size();

    self->set_exit_handler([=](const exit_msg& dm){
        aout(self) << "restart worker" << endl;
        self->state.workers.erase(std::remove(self->state.workers.begin(),
                                                  self->state.workers.end(), dm.source), self->state.workers.end());
        self->state.workers.emplace_back(self->spawn<monitored>(worker));
    });


    return{
            [&](add_worker_atom, string ip, uint16_t port){
                //todo: publish: wegen weak_ptr und C++11
                return ack_atom::value;
            },
            [&](new_job_atom atm,int512_t N){
                self->state.N=N;
                for(actor worker:self->state.workers){
                    self->send(worker,cancel_atom::value);
                    self->delegate(worker,atm ,N);
                }
            }
    };
}

//persistant memory for coordinators
struct coordinatorState {
    vector<actor> managers;
    actor client;
    int512_t N;
    bool done=false;
};

//coordinator behaviour
}behavior coordinator(stateful_actor<coordinatorState>* self, actor_system* sys_ptr){
    auto& mm = sys_ptr->middleman();
//    mm.publish(self,COORD_PORT);

    self->set_down_handler([=](const down_msg& dm) {
        if (dm.source == self->state.client) {
            aout(self) << "coordinator: client down" << endl;
            for(auto manager:self->state.managers){
                self->send(manager, cancel_atom::value);
            }
        }else{
            bool identified=false;
            for(auto manager: self->state.managers){
                if(manager==dm.source){
                    identified=true;
                    aout(self) << "coordinator: manager down" << endl;
                    self->state.managers.erase(std::remove(self->state.managers.begin(),
                                                           self->state.managers.end(), dm.source), self->state.managers.end());
                }
            }
            if(!identified){
                aout(self) << "coordinator: can't determine message source" << endl;
            }
        }
    });

    return {
            [&](add_client_atom, string ip, uint16_t port, int512_t N) {
                aout(self) << "got new client @" << ip << ":"<< port << endl;
                auto clnt_hndl = mm.remote_actor(ip,port);
                if (!clnt_hndl) {
                    cerr << "unable to connect to client: " << sys_ptr->render(clnt_hndl.error()) << "\n";
                }else {
                    self->state.client=*clnt_hndl;
                    self->monitor(*clnt_hndl);
                    self->send(self->state.client,ack_atom::value);
                }
            },
            [&](add_mngr_atom,string ip, uint16_t port){
                aout(self) << "got new manager @" << ip << ":"<< port << endl;
                auto mngr_hndl = mm.remote_actor(ip,port);
                if (!mngr_hndl) {
                    cerr << "unable to connect to new manager: " << sys_ptr->render(mngr_hndl.error()) << "\n";
                }else {
                    self->state.managers.emplace_back(*mngr_hndl);
                    self->send(*mngr_hndl,ack_atom::value);
                }
            },
            [&](request_atom a,int512_t N){
                if(self->state.managers.size()==0)aout(self)<<"no managers available"<< endl;
                //if (self->mailbox().empty()) cout << "has no job";
                for(auto manager:self->state.managers){
                    self->send(manager,new_job_atom::value ,self->state.N);
                }
            },
            [&](deliver_atom a,int512_t N){
                //if (self->mailbox().empty()) cout << "has no job";

                //stub
                self->state.N=evaluate(N);
                if(self->state.N==0) {
                    self->send(self->state.client, reply_atom::value, N );
                }else{
                    for(auto manager:self->state.managers){
                        self->send(manager,new_job_atom::value ,self->state.N);
                    }
                }
            }
    };
}

void run_server(actor_system& sys, const config& cfg) {
    auto& mm = sys.middleman();
    //launching server as coordinator
    mm.publish(sys.spawn(coordinator, &sys),SERVER_PORT);
    cout << "run_server" << endl;
}

void run_manager(actor_system& sys, const config& cfg) {
    cout << "run_manager" << endl;
    auto& mm = sys.middleman();
    mm.remote_actor(SERVER_IP,SERVER_PORT);
    mm.publish(sys.spawn(manager,NO_WORKERS),MANAGER_PORT);

}

void run_client(actor_system& sys, const config& cfg) {
    cout << "run_client" << endl;
    int512_t N=500;
    string server_ip="localhost";
    uint16_t server_port=9900;
    auto& mm = sys.middleman();
    //server handle
    auto serv_hndl = mm.remote_actor(server_ip,server_port);

    scoped_actor self{sys};
    self->request(*serv_hndl, infinite, add_client_atom::value, server_ip, server_port).receive(
            [&](ack_atom) {
                cout << "success: reached server";
            },
            [&](error& err) {
                cout << "Error: " << sys.render(err) << "\n";
            }
    );
    self->request(*serv_hndl, infinite, request_atom::value,N).receive(
            [&](int512_t p) {
                cout << "result: "<< p << endl;
            },
            [&](error& err) {
                cout << "Error: " << sys.render(err) << "\n";
            }
    );
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
      else{
    cerr << "*** invalid mode specified" << endl;
}

} // namespace <anonymous>

CAF_MAIN(io::middleman)
