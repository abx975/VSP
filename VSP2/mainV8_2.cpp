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

using std::cerr;
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
#define NUM_WORKERS 1
#define MODE "server" // "server" // "manager" // "client"
#define TERMINATING_VALUE 1

using add_mngr_atom = atom_constant<atom("add_mngr")>;
//using add_client_atom = atom_constant<atom("add_client")>;
using add_worker_atom = atom_constant<atom("add_worker")>;
using ack_atom = atom_constant<atom("ack")>;
using request_atom = atom_constant<atom("request")>;
using reply_atom = atom_constant<atom("reply")>;
using reset_atom = atom_constant<atom("reset")>;
using new_job_atom = atom_constant<atom("new_job")>;
using deliver_atom = atom_constant<atom("deliver")>;
using update_cycles_atom = atom_constant<atom("upd_cycles")>;



namespace
{


    struct numberToSplit
    {
        int512_t N = 0;
        int512_t p = 0;
        bool isprim = false;
        double deltaT=0;
        int512_t cycles = 0;
    };

//persistant memory for coordinators
    struct coordinatorState
    {
        vector<actor> managers;
        actor client;
        int512_t N;
        bool done = true;
        struct numberToSplit num;
        double wallStartTime;
        set<int512_t> setOfPrimFactors;
        set<int512_t> setOfFactors;
        int512_t lastSended_N;
        ///////////////////////
        double cpu_time_total;
        int512_t cycles_total;
    };

//persistant memory for managers
    struct managerState
    {
        vector<actor> workers;
        actor server;
        int noWorkers = 2;
        numberToSplit prev_Nts;
        double tPrev;
      //  int512_t cycles_total = 0;
        int512_t prev_cycles=0;
        //int512_t cycles_last = 0;// todo: toter Code?
    };

    template <class Inspector>
    typename Inspector::result_type inspect(Inspector& f, numberToSplit& p)
    {
        return f(meta::type_name("numberToSplit"), p.N, p.p, p.isprim, p.cycles);
    }

    struct config : actor_system_config
    {
        string host = SERVER_IP;
        uint16_t port = SERVER_PORT;
        size_t num_workers = NUM_WORKERS;
        string mode = MODE;

        config()
        {
            //add_message_type<int512_t>("add_client");
            add_message_type<numberToSplit>("numberToSplit");
            add_message_type<int512_t>("int512_t");
            add_message_type<vector<int512_t>>("vector<int512_t>");
            opt_group{custom_options_, "global"}
                    .add(host, "host,H", "server host (ignored in server mode)")
                    .add(port, "port,p", "port")
                    .add(num_workers, "num-workers,w", "number of workers (in manager mode)")
                    .add(mode, "mode,m", "one of 'server', 'manager' or 'client'");
        }
    };

    numberToSplit PollardRho(int512_t N, event_based_actor* self);
    inline bool IsPrime( int512_t number );
    int512_t power(int512_t a, int512_t n, int512_t mod);
    int512_t modular_pow(int512_t base, int512_t exponent, int512_t modulus);
    void printSet(set<int512_t> setToPrint);
    double get_wall_time();
    double get_cpu_time();


    int512_t errorCode = -99;
    static int numPRoh = 0;

    int512_t Z0 = 210;
    int512_t Z1 = 8806715679; // 3 * 29 * 29 * 71 * 211 * 233
    int512_t Z2 = 0x826efbb5b4c665b9_cppui512; // 9398726230209357241; // 443 * 503 * 997 * 1511 * 3541 * 7907
    int512_t Z3 = 0xc72af6a83cc2d3984fedbe6c1d15e542556941e7_cppui512;
/*
Z3 = 1137047281562824484226171575219374004320812483047 = 0xc72af6a83cc2d3984fedbe6c1d15e542556941e7 =
7907 * 12391 * 12553 * 156007 * 191913031 * 4302407713 * 7177162612387
*/
    int512_t Z4 =
            0x1aa0d675bd49341ccc03fff7170f29cd7048bf40430c22ced5a391d015d19677bde78a7b95b5d59b6b26678238fa7_cppui512;
/*
Z4 = 1000602106143806596478722974273666950903906112131794745457338659266842446985022076792112309173975243506969710503
   = 0xc72af6a83cc2d3984fedbe6c1d15e542556941e7 =
   10657 * 11657 * 13264529 * 10052678938039 * 2305843009213693951 *
   (26196077648981785233796902948145280025374347621094198197324282087) =
*/


    numberToSplit PollardRho(int512_t N, event_based_actor* self)
    {
        /* initialize random seed */
        timeval start;
        int useconds;
        gettimeofday(&start, NULL);
        useconds = start.tv_usec;
        srand(useconds);
        //srand (time(NULL));
        struct numberToSplit num;
        num.N = N;

        /* no prime divisor for 1 */
        if (N==1)
        {
            num.p = 1;
            return num;
        }

        /* even number means one of the divisors is 2 */
        if (N % 2 == 0)
        {
            num.p = 2;
            return num;
        }
        /* we will pick from the range [1, N) */
        int512_t x = (rand()% N + 1);

        int512_t y = x;

        /* the constant in f(x).
         * Algorithm can be re-run with a different a
         * if it throws failure for a composite. */
        int512_t a = (rand()%(N-1))+1;

        /* Initialize Difference */
        int512_t d = 0;

        /* Initialize candidate divisor (or result) */
        int512_t p = 1;

        /* until the prime factor isn't obtained.
           If N is prime, return N */
        int checkMailboxValue = 0;
        int numberOfRuns = 0;
        int512_t M = (int512_t) sqrt(sqrt(N));
        M = (int512_t) (M * 118);
        M = (int512_t) (M / 100);

        do
        {
            // If Pollard is running longer than Middle M 1.18 * sqrt(sqrt(N)): start with a new random number
            if (numberOfRuns >  M)
            {
                cerr << "Pollard is running longer than Middle M 1.18 * sqrt(sqrt(N)):" << M << endl;
                return PollardRho(N, self);
            }
            if (checkMailboxValue >= 5000)
            {

                //checkmailbox;
                if( !self->mailbox().empty() )
                {
                    cerr << "worker got a new job" << endl;
                    self->quit(sec::request_receiver_down);
                }
            }

            /* Tortoise Move: x= (x² + a) mod N */
            x = (x * x + a) % N;

            /* Hare Move: y(i+1) = f(f(y(i))) */
            y = (y * y + a) % N;
            y = (y * y + a) % N;

            /* Difference between (y - x) % N*/
            d = (y - x) % N;

            /* check gcd of |x-y| and N */
            p = __gcd(d, N);
            /* make the divisor positive */
            if (p < 0)
            {
                p *= -1;
            }
            checkMailboxValue++;
            numberOfRuns++;
            numPRoh++;
        }
        while (p == 1);

        /* retry if the algorithm fails to find prime factor
         * with chosen x and a */
        if (p!=N)
        {
            struct numberToSplit num;
            num.N = N;
            num.p = p;
            num.cycles = numPRoh;
            return num;
        }
        else
        {
            num.p = errorCode;
            return num;
        }
    }


// Check Prime
    int512_t power(int512_t a, int512_t n, int512_t mod)
    {
        int512_t power = a,result=1;

        while(n)
        {
            if(n&1)
                result=(result*power)%mod;
            power=(power*power)%mod;
            n>>=1;
        }
        return result;
    }

    bool witness(int512_t a, int512_t n)
    {
        int512_t t,u,i;
        int512_t prev,curr;

        u=n/2;
        t=1;
        while(!(u&1))
        {
            u/=2;
            ++t;
        }

        prev=power(a,u,n);
        for(i=1; i<=t; ++i)
        {
            curr=(prev*prev)%n;
            if((curr==1)&&(prev!=1)&&(prev!=n-1))
                return true;
            prev=curr;
        }
        if(curr!=1)
            return true;
        return false;
    }

    inline bool IsPrime( int512_t number )
    {
        if ( ( (!(number & 1)) && number != 2 ) || (number < 2) || (number % 3 == 0 && number != 3) )
            return (false);

        if(number<1373653)
        {
            for( int k = 1; 36*k*k-12*k < number; ++k)
                if ( (number % (6*k+1) == 0) || (number % (6*k-1) == 0) )
                    return (false);
            return true;
        }

        if(number < 9080191)
        {
            if(witness(31,number)) return false;
            if(witness(73,number)) return false;
            return true;
        }


        if(witness(2,number)) return false;
        if(witness(7,number)) return false;
        if(witness(61,number)) return false;
        return true;
    }

    void printSet(set<int512_t> setToPrint)
    {
        if(setToPrint.size() > 0)
        {
            set<int512_t>::iterator iter;
            for(iter=setToPrint.begin(); iter!=setToPrint.end(); ++iter)
            {
                cerr << *iter << " " ;
            }
            cerr << endl;
        }
        else
        {
            cerr << "The Set is empty" << endl;
        }
    }

    double get_wall_time()
    {
        struct timeval time;
        if (gettimeofday(&time,NULL))
        {
            //  Handle error
            return 0;
        }
        return (double)time.tv_sec + (double)time.tv_usec * .000001;
    }


    void evaluate(coordinatorState& state)
    {
        if(state.num.isprim == true)
        {
            state.setOfPrimFactors.insert(state.num.p);
            //                cerr << " Neue Primzahl gefunden: The Primfactors are: ";
            //                printSet( self->state.setOfPrimFactors);
            //cerr << "Found new Primfaktor: The new Primfaktor is: " << state.num.p << endl;
        }

            //wenn nicht dann insert p in Faktoren
        else
        {
            state.setOfFactors.insert(state.num.p);
        }

        // für jede zahl in Fakroten:
        set<int512_t>::iterator it;

        // für alle fakroten testen ob sie durch die gefundene Primzahl teilbar sind

        for(it= state.setOfFactors.begin(); it!= state.setOfFactors.end(); ++it)
        {
            int512_t zwischenspeicher = *it;

            // dividiere durch num.p bis != 0
            while (zwischenspeicher % state.num.p == 0)
            {
                zwischenspeicher/= state.num.p;
            }
            // insert das neue N

            bool zwiIsprime = IsPrime(zwischenspeicher);
            if(zwischenspeicher > 1 && !zwiIsprime)
            {
//                                    cerr<< "INSERT  self->state.setOfFactors.insert(" << zwischenspeicher << ");" << endl;
                state.setOfFactors.insert(zwischenspeicher);
            }
            else if(zwischenspeicher > 1 && zwiIsprime)
            {
//                                    cerr<< "INSERT  self->state.setOfPrimFactors.insert(" << zwischenspeicher << ");" << endl;
                state.setOfPrimFactors.insert(zwischenspeicher);
                state.setOfFactors.erase(zwischenspeicher);

//                                    cerr << "Found new Primfaktor: The new Primfaktor is: " << zwischenspeicher << endl;
            }
        }
//                            cerr<< "ERASE  self->state.setOfFactors.erase(" << self->state.num.N << ");" << endl;
        state.setOfFactors.erase(state.num.N);
    }

//worker behaviour
    behavior worker(event_based_actor *self)
    {
        return
                {
                        //stub
                        [=](numberToSplit nts) -> numberToSplit {
                            if (nts.N != TERMINATING_VALUE)
                            {
                                cerr << "worker erreicht" << endl;
                                struct numberToSplit num;
                                num = nts;
                                num = PollardRho(num.N, self);
                                {
                                    while (num.p == errorCode)
                                        num = PollardRho(num.N, self);
                                }

                                if (IsPrime(num.p))
                                {
                                    num.isprim = true;
                                }
                                else
                                {
                                    num.isprim = false;
                                }

                                cerr << "worker " << self->address().id() << " läuft durch; num.N: " << num.N
                                     << " num.p: " << num.p << endl;

                                return num;
                            }
                        }
                };
    }

//manager behaviour
    behavior manager(stateful_actor<managerState> *self, actor_system* sys_ptr, string host, uint16_t port, int num_workers)
    {
        cerr << "manager:ready" << endl;
        auto &mm = sys_ptr->middleman();
        auto server = mm.remote_actor(host, port);
        if(!server)
        {
            cerr << "manager:server not found" << endl;
        }
        else
        {
            self->state.server=*server;
        }
        //spawn specified number of workers
        for (int i = 0; i < num_workers; i++)
        {
            self->state.workers.emplace_back(self->spawn<monitored>(worker));
        }
        self->state.noWorkers = self->state.workers.size();
        cerr << "manager: availlable workers: " << self->state.noWorkers << endl;

        //subscribe to server
        self->send(*server,add_mngr_atom::value);

        //respawn dead worker and repeat previous request
        self->set_down_handler([=](const down_msg &dm)
                               {

                                   cerr << "restart worker" << endl;

//                                   self->state.cycles_total +=  self->state.cycles_last;

                                   self->state.workers.erase(std::remove(self->state.workers.begin(),
                                                                         self->state.workers.end(), dm.source), self->state.workers.end());
                                   auto replacement_worker = self->spawn<monitored>(worker);
                                   self->state.workers.emplace_back(replacement_worker);
                                   self->send(replacement_worker,self->state.prev_Nts);
                                   ////////////////////////////////////////////////////////
                                   self->send(self->state.server, update_cycles_atom::value, self->state.prev_cycles);
                               });

        return
                {
                        [=](new_job_atom, numberToSplit nts)
                        {

                            cerr << "!!!manager got a new job with nts.n = " << nts.N << endl;
                            self->state.prev_Nts = nts;
                            ////////////////////////////////
                            self->state.tPrev=get_wall_time();
                            self->state.prev_cycles=0;
                            ////////////////////////////////
                            for (actor worker:self->state.workers)
                            {
                                self->send(worker, nts);
                            }
                        },
                        [=](numberToSplit nts)
                        {
                            if(nts.p != TERMINATING_VALUE)
                            {
                                /////////////////////////////////////////
                                self->state.prev_cycles = nts.cycles;
                                /////////////////////////////////////////
                                nts.deltaT = get_wall_time()-self->state.tPrev;
                                self->state.tPrev = get_wall_time();
                                cerr << "delivering ... wall time: " <<  nts.deltaT << endl;
                                self->send(self->state.server, deliver_atom::value, nts);
                            }
                        },
                        [=](reset_atom)
                        {
                            numberToSplit nts;
                            nts.N = TERMINATING_VALUE;
                            cerr << "resetting workers" << endl;
                            for (actor worker:self->state.workers)
                            {
                                self->send(worker,nts);
                            }
                        },
                        [=](ack_atom)
                        {
                            cerr << "manager:connect successful" << endl;
                        }
                };
    }

//coordinator behaviour
    behavior coordinator(stateful_actor<coordinatorState> *self)
    {
        cerr << "server:coordinator ready"  << endl;
        self->set_down_handler([=](const down_msg &dm)
                               {
                                   if (dm.source == self->state.client)
                                   {
                                       cerr << "coordinator: client down" << endl;
                                       for (auto manager:self->state.managers)
                                       {
                                           self->send(manager, reset_atom::value);
                                       }
                                   }
                                   else
                                   {
                                       bool identified = false;
                                       for (auto manager: self->state.managers)
                                       {
                                           if (manager == dm.source)
                                           {
                                               identified = true;
                                               cerr << "coordinator: manager down" << endl;
                                               self->state.managers.erase(std::remove(self->state.managers.begin(),
                                                                                      self->state.managers.end(), dm.source), self->state.managers.end());
                                           }
                                       }
                                       if (!identified)
                                       {
                                           cerr << "coordinator: can't determine down_msg source" << endl;
                                       }
                                   }
                               });

        return
                {
                        ////////////////////////////////////////////
                        [=](update_cycles_atom,int512_t manager_cycles){
                            self->state.cycles_total+=  manager_cycles;                      },
                        ////////////////////////////////////////////
                        [=](add_mngr_atom)
                        {
                            cerr << "got new manager" << endl;
                            auto mngr = actor_cast<actor>(self->current_sender());
                            if (!mngr)
                            {
                                cerr << "unable to register new manager" << endl;
                            }
                            else
                            {
                                self->state.managers.emplace_back(mngr);
                                self->monitor(mngr);
                                self->send(mngr, ack_atom::value);
                            }
                        },
                        [=](request_atom, int512_t N)
                        {
                            self->state.done = false;
                            self->state.N=N;

                            // client sendet an den Server das N und speichert es in self->state.N

                            self->state.N = N;
                            self->state.num.N = N;


                            //  Start Timer
                            self->state.wallStartTime = get_wall_time();
                            self->state.cpu_time_total=0.0;
                            self->state.cycles_total=0;
                            cerr << "////////////////////////////////////////////////////////////////////" << endl;
                            cerr << "got request... cycles_total: " << self->state.cycles_total << endl;
                            cerr << "////////////////////////////////////////////////////////////////////" << endl;
                            cerr << "N: " << self->state.num.N << endl;

                            if(IsPrime(self->state.num.N) || self->state.num.N == 1)
                            {
                                self->state.num.p = 1;
                                self->state.num.isprim = true;
                                self->state.client = actor_cast<actor>(self->current_sender());
                                stringstream s;// string Stream erzeugen
                                // Print setOfPrimFactors
                                s << N << " is a prim";
                                string retval = s.str(); // string zurückgeben
                                self->send(self->state.client, reply_atom::value, retval);
                            }
                            else
                            {
                                self->state.setOfFactors.insert(self->state.num.N);

                                //ende : client sendet an den Server das N und spwichert es in self->state.N
                                if (self->state.managers.size() == 0)
                                {
                                    cerr << "no managers available" << endl;
                                }
                                else
                                {
                                    //memorize client
                                    self->state.client = actor_cast<actor>(self->current_sender());
                                    //initial distribution
                                    self->state.lastSended_N = self->state.num.N;

                                    for (auto manager:self->state.managers)
                                    {
                                        cerr << "sever sendet; num.N:" << self->state.num.N << endl;
                                        self->send(manager, new_job_atom::value, self->state.num);
                                    }

                                }

                            }
                        },
                        //redistribution after receiving result
                        [=](deliver_atom, numberToSplit nts)
                        {
                            ///////////////////////////////////////////////////////
                            self->state.cycles_total=self->state.cycles_total+nts.cycles;
                            cerr << "new cycles: " << nts.cycles << "    cycles_total: " << self->state.cycles_total << endl;
                            self->state.cpu_time_total+=nts.deltaT;
                            cerr << "new delta: " << nts.deltaT << "    cpu_time_total: " << self->state.cpu_time_total << endl;
                            ///////////////////////////////////////////////////////
                            cerr << "coordinator got  nts.p : " << nts.p << " nts.n = " << nts.N << endl;
                            //stub
                            cerr << "Before evaluate: " << endl;
                            cerr << "The Factors for N are: " << endl;
                            printSet(self->state.setOfFactors);
                            cerr << "The Primfactors for N are: " << endl;
                            printSet(self->state.setOfPrimFactors);

                            self->state.num = nts;
                            evaluate(self->state);

                            cerr << "After evaluate: " << endl;
                            cerr << "The Factors for N are: " << endl;
                            printSet(self->state.setOfFactors);
                            cerr << "The Primfactors for N are: " << endl;
                            printSet(self->state.setOfPrimFactors);

                            if ( self->state.setOfFactors.empty())
                            {
                                    //  Stop timers
                                    self->state.done = true;
                                    double wallEndTime = get_wall_time();
                                    double wallTime = wallEndTime - self->state.wallStartTime;
                                    double cpuTime = self->state.cpu_time_total;

                                    stringstream s;// string Stream erzeugen
                                    s << "Number of Runs: " << self->state.cycles_total << endl;
                                    s << "Wall Time = " << wallTime << endl;
                                    s << "CPU Time  = " << cpuTime << endl;
                                    self->state.cycles_total=0;
                                    self->state.num.cycles = 0;

                                    // Print setOfPrimFactors
                                    s << "The Primfactors for N are: ";
                                    set<int512_t>::iterator iter;
                                    for(iter=self->state.setOfPrimFactors.begin(); iter!=self->state.setOfPrimFactors.end(); ++iter)
                                    {
                                        s << *iter << " " ;
                                    }


                                    string retval =s.str(); // string zurückgeben
                                    cerr << retval << endl;
                                    //self->send(self->state.client, reply_atom::value, self->state.num);
                                    self->send(self->state.client, reply_atom::value, retval);
                            }
                            else
                            {
                                set<int512_t>::iterator iter =  self->state.setOfFactors.begin();
                                set<int512_t>::iterator iterend =  self->state.setOfFactors.end();
                                // der reihe nach wird jeder faktor verteilt, wenn keine Faktoren mehr da sind beginnt er von vorn
                                int i= 0;
                                self->state.num.N = *iter;
                                if (self->state.lastSended_N != self->state.num.N)
                                {


                                    for (auto manager:self->state.managers)
                                    {
                                        i++;
                                        cerr << "Anzahl stringgesenderter nachrichten:  i = " << i << endl;
                                        cerr << "setOfPrimFactors: " << endl;
                                        printSet(self->state.setOfPrimFactors);
                                        cerr << "setOfFactors: " << endl;
                                        printSet(self->state.setOfFactors);

                                        self->state.num.N = *iter;
                                        cerr << "self->state.num.N: " <<  self->state.num.N << endl;
                                        cerr << "lastSended_N: " <<  self->state.lastSended_N << endl;


                                        self->state.lastSended_N = *iter;
                                        cerr << "sever sendet new_job; num.N:" << self->state.num.N << endl;
                                        self->send(manager, new_job_atom::value, self->state.num);

                                        if(i %  self->state.setOfFactors.size() == 0)
                                        {
                                            iter = self->state.setOfFactors.begin();
                                        }
                                        else
                                        {
                                            ++iter;
                                        }
                                    }
                                }
                            }
                        }
                };

    }
    void run_server(actor_system &sys, const config &cfg)
    {
        cerr << "run_server" << endl;
        auto &mm = sys.middleman();
        //launching server as coordinator
        mm.publish(sys.spawn(coordinator), cfg.port);

    }

    void run_manager(actor_system &sys, const config &cfg)
    {
        cerr << "run_manager" << endl;
        sys.spawn(manager,&sys,cfg.host,cfg.port,cfg.num_workers);
    }

    void run_client(actor_system &sys, const config &cfg)
    {
        cerr << "run_client" << endl;
        int512_t N = 210;
        //int N = 20;
        auto &mm = sys.middleman();
        //server handle
        auto serv_hndl = mm.remote_actor(cfg.host, cfg.port);
        if(!serv_hndl)
        {
            cerr << "could not connect to server" << endl;
        }
        else
        {
            scoped_actor self{sys};
            self->send(*serv_hndl,request_atom::value,N);
//        self->receive([&](reply_atom, numberToSplit num)
//        {
//            cerr << "done! result: " << num.p << endl;
//        });
            self->receive([&](reply_atom, string s)
                          {
                              cerr << "done! result: " << endl << s << endl;
                          });
        }
    }

// dispatches to run_* function depending on selected mode
    void caf_main(actor_system &sys, const config &cfg)
    {
        using map_t = unordered_map<string, void (*)(actor_system &, const config &)>;
        map_t modes
                {
                        {"server",  run_server},
                        {"manager", run_manager},
                        {"client",  run_client}
                };
        auto i = modes.find(cfg.mode);
        if (i != modes.end())
        {
            (i->second)(sys, cfg);
        }
        else
        {
            cerr << "*** invalid mode specified" << endl;
        }
    } // namespace <anonymous>
}
CAF_MAIN(io::middleman)

