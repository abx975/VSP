/**
 * @author Nils Eggebrecht
 * @author Lennart Hartmann
 * @version 5.11.2017
 */
#define NR_OF_WORKERS 4
#define IP "localhost"
#define PORT 9900
#define IS_SERVER true
#define IS_CLIENT true


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
#include "caf/all.hpp"
#include "caf/io/all.hpp"
#include "caf/all.hpp"

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
#include "int512_serialization.hpp"

using namespace caf;
using namespace std;
using namespace boost::multiprecision;
using namespace boost::multiprecision::literals;
using std::cin;
using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::vector;
using std::pair;
using std::unordered_map;
using add_atom = atom_constant<atom("add")>;
using rmv_atom = atom_constant<atom("rmv")>;
using quit_atom = atom_constant<atom("quit")>;

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
                //////////// angebeblich atomar und "threadsafe", aber ...??? ///////////////
            },[=](add_atom,actor_system& sys) {
                self->state.workers.emplace_back(sys.spawn(workerBeh));
                aout(self) << "add ... workers: " << self->state.workers.size();
            },
            [=](rmv_atom) {
                if(!self->state.workers.empty())self->state.workers.pop_back();
                aout(self) << "rmv done ... workers: " << self->state.workers.size();
            },
            [=](quit_atom){
                self->quit();
            }
    };
}


//Add a new server ////////////// stürzt bei Aufruf aus caf_main ab ////////////////////
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

//remove a server ////////////// stürzt bei Aufruf aus caf_main ab ////////////////////
void rmvManager(vector<actor>& managers){
    cout << "removing manager"<< endl;
    managers.pop_back();
}


void caf_main(actor_system& sys, const config &cfg) {
    auto &mm = sys.middleman();

//#if IS_SERVER
    if (cfg.mode == "server") {
        cout << "running as server" << endl;

        //UI-actor
        scoped_actor UI_self{sys};

        //server-actor
        actor manager = sys.spawn(managerBeh, NR_OF_WORKERS);
        //auto ba= sys.spawn(blocking_calculator_fun);

        //publish manager
        auto p = mm.publish(manager, PORT);
        if (!p) {
            cerr << "unable to publish actor: " << sys.render(p.error()) << "\n";
        } else {
            cout << "math actor published at port " << *p << "\n";
        }

        char input;
        while (true) {
            cout << "UI erreicht" << endl;
            cout << "add('+'), remove('-') manger or quit('q')? Type + or - \n >> ";
            cin >> input;
            if (input == 'q') {
                break;
            }else if(input=='+') {
                cout << "adding worker" << endl;
                UI_self->send(manager,add_atom::value);
            }else if(input=='-'){
                cout << "adding worker" << endl;
                UI_self->send(manager,rmv_atom::value);
            }
        }
        cout << "Quit UI -> terminating manager" << endl;
        UI_self->send(manager,quit_atom::value);
    }
//#endif

//#if IS_CLIENT == true
    if(cfg.mode=="client") {
        cout << "running as client" << endl;
        // scoped actor for ad hoc communication
        scoped_actor self{sys};

        //managers for coordinator to address
        std::vector<actor> managers;

        //available servers
        vector<pair<string, uint16_t>> serverList;
        pair<string, uint16_t> server1(IP, PORT);
        serverList.emplace_back(server1);

        //lookup managers
        for (pair<string, uint16_t> server : serverList) {
            //auto remotePtr = mm.remote_actor(IP, PORT);
            auto remotePtr = mm.remote_actor(server.first, server.second);
            if (!remotePtr) {
                cerr << "unable to connect to server: " << sys.render(remotePtr.error()) << "\n";
            } else {
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


        while (n > 0) {
            for (auto &manager : managers) {
                self->request(manager, infinite, n).receive(
                        [&](int p) {
                            cout << "got p=" << p << "\n";
                            n = p;
                        },
                        [&](error &err) {
                            cout << "Error: " << sys.render(err) << "\n";
                        }
                );
            }
        }
    }
//#endif
}

CAF_MAIN(io::middleman)

