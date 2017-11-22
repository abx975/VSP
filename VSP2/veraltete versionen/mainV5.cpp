/**
 * @author Nils Eggebrecht
 * @author Lennart Hartmann
 * @version 11.11.2017
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
#include <random>

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
#define NUM_WORKERS 4
#define MODE "server" //"manager" //"client"
#define TERMINATING_VALUE 1

using add_mngr_atom = atom_constant<atom("add_mngr")>;
using add_client_atom = atom_constant<atom("add_client")>;
using add_worker_atom = atom_constant<atom("add_worker")>;
using ack_atom = atom_constant<atom("ack")>;
using request_atom = atom_constant<atom("request")>;
using reply_atom = atom_constant<atom("reply")>;
using reset_atom = atom_constant<atom("reset")>;
using new_job_atom = atom_constant<atom("new_job")>;
using deliver_atom = atom_constant<atom("deliver")>;


namespace {

    struct config : actor_system_config {
        string host = SERVER_IP;
        uint16_t port = SERVER_PORT;
        size_t num_workers = NUM_WORKERS;
        string mode = MODE;

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

    //int512_t evaluate(int512_t N) {
    int evaluate(int N) {
        return N;
    }

    //worker behaviour
    behavior worker(event_based_actor *self) {
        return {
                //stub
                //[=](int512_t N) {
                [=](int N) {
                    if (!self->mailbox().empty()){
                        cout << "got new job => quit worker" << endl;
                        self->quit();
                    }
                    //for testing purposes
                    std::default_random_engine generator;
                    std::uniform_int_distribution<int> distribution(500,9000000);
                    int rand_number = distribution(generator);
                    for(int i=rand_number;i>0;i--);

                    //stub
                    aout(self) << "worker: N="<< N << endl;
                    return N-1;
                },
        };
    }


    //persistant memory for managers
    struct managerState {
        vector<actor> workers;
        actor server;
        int noWorkers = 5;
        //int512_t prev_N;
        int prev_N;
    };

    //manager behaviour
    behavior manager(stateful_actor<managerState> *self, actor_system* sys_ptr, string host, uint16_t port, int num_workers) {
        aout(self) << "manager:ready" << endl;
        auto &mm = sys_ptr->middleman();
        auto server = mm.remote_actor(host, port);
        if(!server){
            aout(self) << "manager:server not found" << endl;
        }else{
            self->state.server=*server;
        }
        //spawn specified number of workers
        for (int i = 0; i < num_workers; i++) {
            self->state.workers.emplace_back(self->spawn<monitored>(worker));
        }
        self->state.noWorkers = self->state.workers.size();
        aout(self) << "manager: availlable workers: " << self->state.noWorkers << endl;

        //subscribe to server
        self->send(*server,add_mngr_atom::value);

        //respawn dead worker and repeat previous request
        self->set_down_handler([=](const down_msg &dm) {
            aout(self) << "restart worker" << endl;
            self->state.workers.erase(std::remove(self->state.workers.begin(),
                                                  self->state.workers.end(), dm.source), self->state.workers.end());
            auto replacement_worker = self->spawn<monitored>(worker);
            self->state.workers.emplace_back(replacement_worker);
            self->send(replacement_worker,self->state.prev_N);
        });
        
        return {
                //[=](new_job_atom, int512_t N) {
                [=](new_job_atom, int N) {
                    self->state.prev_N = N;
                    for (actor worker:self->state.workers){
                        self->send(worker, N);
                    }
                },
                //[=](deliver_atom,int512_t p) {
                [=](int p) {
                    for (actor worker:self->state.workers) {
                        self->send(self->state.server,deliver_atom::value,p);
                    }
                },
                [=](reset_atom) {   //todo: send value that immediately terminates workers
                    for (actor worker:self->state.workers) {
                        self->send(self->state.server,deliver_atom::value,TERMINATING_VALUE);
                    }
                },
                [=](ack_atom) {   //todo: send value that immediately terminates workers
                    cout << "manager:connect successful" << endl;
                }
        };
    }

    //persistant memory for coordinators
    struct coordinatorState {
        vector<actor> managers;
        actor client;
        //int512_t N;
        int N;
        bool done = false;
    };

    //coordinator behaviour
    behavior coordinator(stateful_actor<coordinatorState> *self) {
        aout(self) << "server:coordinator ready"  << endl;

        self->set_down_handler([=](const down_msg &dm) {
            if (dm.source == self->state.client) {
                aout(self) << "coordinator: client down" << endl;
                for (auto manager:self->state.managers) {
                    self->send(manager, reset_atom::value);
                }
            } else {
                bool identified = false;
                for (auto manager: self->state.managers) {
                    if (manager == dm.source) {
                        identified = true;
                        aout(self) << "coordinator: manager down" << endl;
                        self->state.managers.erase(std::remove(self->state.managers.begin(),
                                                               self->state.managers.end(), dm.source),
                                                   self->state.managers.end());
                    }
                }
                if (!identified) {
                    aout(self) << "coordinator: can't determine down_msg source" << endl;
                }
            }
        });

        return {
                [=](add_client_atom) {
                    aout(self) << "got new client" << endl;
                    auto clnt = actor_cast<actor>(self->current_sender());
                    if (!clnt) {
                        cerr << "unable to register client" << endl;
                    } else {
                        self->state.client = clnt;
                        self->monitor(clnt);
                    }
                },
                [=](add_mngr_atom) {
                    //int512_t start_val=64;
                    aout(self) << "got new manager" << endl;
                    auto mngr = actor_cast<actor>(self->current_sender());
                    if (!mngr) {
                        cerr << "unable to register new manager" << endl;
                    } else {
                        self->state.managers.emplace_back(mngr);
                        self->monitor(mngr);
                        self->send(mngr, ack_atom::value);
                    }
                },
                //[=](request_atom, int512_t N) {
                [=](request_atom, int N) {
                    self->state.N=N;
                    if (self->state.managers.size() == 0){
                        aout(self) << "no managers available" << endl;
                    }else {
                        //memorize client
                        self->state.client = actor_cast<actor>(self->current_sender());
                        //initial distribution
                        for (auto manager:self->state.managers) {
                            self->send(manager, new_job_atom::value, self->state.N);
                        }
                    }
                },
                //redistribution after receiving result
                //[=](deliver_atom, int512_t N) {
                [=](deliver_atom, int p) {
                    aout(self) << "coordinator got: " << p << endl;
                    //stub
                    self->state.N = evaluate(p);
                    if (self->state.N < 2) {
                        self->send(self->state.client, reply_atom::value, p);
                    } else {
                        for (auto manager:self->state.managers) {
                            self->send(manager, new_job_atom::value, self->state.N);
                        }
                    }
                }
        };
    }

    void run_server(actor_system &sys, const config &cfg) {
        cout << "run_server" << endl;
        auto &mm = sys.middleman();
        //launching server as coordinator
        mm.publish(sys.spawn(coordinator), cfg.port);

    }

    void run_manager(actor_system &sys, const config &cfg) {
        cout << "run_manager" << endl;
        sys.spawn(manager,&sys,cfg.host,cfg.port,cfg.num_workers);
    }

    void run_client(actor_system &sys, const config &cfg) {
        cout << "run_client" << endl;
        //int512_t N = 500;
        int N = 20;
        auto &mm = sys.middleman();
        //server handle
        auto serv_hndl = mm.remote_actor(cfg.host, cfg.port);
        if(!serv_hndl){
            cout << "could not connect to server" << endl;
        }else{
            scoped_actor self{sys};
            self->send(*serv_hndl,request_atom::value,N);
            self->receive([&](reply_atom, int p) {
                cout << "done! result: " << p << endl;
            });
        }
    }

    // dispatches to run_* function depending on selected mode
    void caf_main(actor_system &sys, const config &cfg) {
        using map_t = unordered_map<string, void (*)(actor_system &, const config &)>;
        map_t modes{
                {"server",  run_server},
                {"manager", run_manager},
                {"client",  run_client}
        };
        auto i = modes.find(cfg.mode);
        if (i != modes.end()) {
            (i->second)(sys, cfg);
        } else {
            cerr << "*** invalid mode specified" << endl;
        }

    } // namespace <anonymous>
}
CAF_MAIN(io::middleman)
