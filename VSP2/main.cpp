/**
 * @author Nils Eggebrecht
 * @author Lennart Hartmann
 * @version 25.11.2017
 */

// C++ standard library includes
#include <string>
#include <vector>
#include <sstream>
#include <iostream>
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
#include "int512_serialization.hpp"

using namespace caf;
using namespace std;
using namespace boost::multiprecision;
using namespace boost::multiprecision::literals;

#define SERVER_IP "localhost"
#define SERVER_PORT 9900
#define NUM_WORKERS 2
#define MODE "server" // "server" // "manager" // "client"
#define TERMINATING_VALUE 1

using add_mngr_atom = atom_constant<atom("add_mngr")>;
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
    double deltaT = 0;
    int512_t cycles = 0;
};

//persistant memory for coordinators
struct coordinatorState
{
    vector<actor> managers;
    actor client;
    int512_t N;
    int512_t cycles_total;
    int512_t lastSended_N;
    struct numberToSplit num;
    double walltimeTime;
    double cpu_time_total;
    bool iAmBusy = false;
    set<int512_t> setOfPrimFactors;
    set<int512_t> setOfFactors;
    set<int512_t> setOfUsedN;
};

//persistant memory for managers
struct managerState
{
    vector<actor> workers;
    actor server;
    int noWorkers = 2;
    numberToSplit prev_Nts;
    double tPrev;
    int512_t prev_cycles=0;
};

template <class Inspector>
typename Inspector::result_type inspect(Inspector& f, numberToSplit& p)
{
    return f(meta::type_name("numberToSplit"), p.N, p.p, p.isprim, p.deltaT, p.cycles);
}

struct config : actor_system_config
{
    string host = SERVER_IP;
    uint16_t port = SERVER_PORT;
    size_t num_workers = NUM_WORKERS;
    string mode = MODE;

    config()
    {
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

//funktion to calculate Pollard Roh algorithm
numberToSplit PollardRho(int512_t N, event_based_actor* self);
// function to check if a number is a prime
inline bool IsPrime( int512_t number );
// power of two int_512t numbers
int512_t power(int512_t a, int512_t n, int512_t mod);
// funktion to print a set
void printSet(set<int512_t> setToPrint);
//funktion to get the wall time
double get_wall_time();

// errorCode if PollardRho don't found a factor of N
int512_t errorCode = -99;
//number of loop-through runs of Pollard Roh per worker per Factor
static int numPRoh = 0;


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

//funktion to calculate Pollard Roh algorithm
numberToSplit PollardRho(int512_t N, event_based_actor* self)
{
    /* initialize random seed */
    timeval time;
    int useconds;
    gettimeofday(&time, NULL);
    useconds = time.tv_usec;
    srand(useconds);
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
     * if iter_factor throws failure for a composite. */
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
        // If Pollard is running longer than Middle M 1.18 * sqrt(sqrt(N)): time with a new random number
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
            checkMailboxValue = 0;
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
        numPRoh = 0;
        return num;
    }
    else
    {
        num.p = errorCode;
        return num;
    }
}


// power of two int_512t numbers
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

// help funktion for IsPrime
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

// function to check if a number is a prime
inline bool IsPrime( int512_t number )
{
    if ( ( (!(number & 1)) && number != 2 ) || (number < 2) || (number % 3 == 0 && number != 3) )
        return (false);

    if(number < 1373653)
    {
        for( int k = 1; 36*k*k-12*k < number; ++k)
            if ( (number % (6*k+1) == 0) || (number % (6*k-1) == 0) )
                return (false);
        return true;
    }

    if(number < 9080191)
    {
        set<int512_t> setOfUsedN;

        if(witness(31,number)) return false;
        if(witness(73,number)) return false;
        return true;
    }

    if(witness(2,number)) return false;
    if(witness(7,number)) return false;
    if(witness(61,number)) return false;
    return true;
}

// funktion to print a set
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
//funktion to get the wall time
double get_wall_time()
{
    struct timeval time;
    if (gettimeofday(&time,NULL))
    {
        return 0;
    }
    return (double)time.tv_sec + (double)time.tv_usec * .000001;
}

//function to evaluate the Result of the worker
void evaluate(coordinatorState& state)
{
    //if factor prime add to setOfPrimFactors
    if(state.num.isprim == true)
    {
        state.setOfPrimFactors.insert(state.num.p);
    }

    //if not add to setOfFactors
    else
    {
        state.setOfFactors.insert(state.num.p);
    }


    set<int512_t>::iterator iter_factor;

    // check for all faktors whether it is a divisor of the number
    for(iter_factor= state.setOfFactors.begin(); iter_factor!= state.setOfFactors.end(); ++iter_factor)
    {
        int512_t cache = *iter_factor;

        while (cache % state.num.p == 0)
        {
            cache/= state.num.p;
        }

        bool zwiIsprime = IsPrime(cache);
        if(cache > 1 && !zwiIsprime)
        {
            state.setOfFactors.insert(cache);
        }
        else if(cache > 1 && zwiIsprime)
        {
            state.setOfPrimFactors.insert(cache);
            state.setOfFactors.erase(cache);
        }
    }
    state.setOfFactors.erase(state.num.N);
}

//worker behaviour
behavior worker(event_based_actor *self)
{
    return
    {
        [=](numberToSplit nts) -> numberToSplit {
            if (nts.N != TERMINATING_VALUE)
            {
                cerr << "worker erreicht" << endl;
                struct numberToSplit num;
                num = nts;

                //reset old cycles
                nts.cycles = 0;
                //run to find a facor of num.N
                num = PollardRho(num.N, self);
                {
                    while (num.p == errorCode)
                        num = PollardRho(num.N, self);
                }
                // set is_prim value
                if (IsPrime(num.p))
                {
                    num.isprim = true;
                }
                else
                {
                    num.isprim = false;
                }
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
        cerr << "retime worker" << endl;
        self->state.workers.erase(std::remove(self->state.workers.begin(),
                                              self->state.workers.end(), dm.source), self->state.workers.end());
        auto replacement_worker = self->spawn<monitored>(worker);
        self->state.workers.emplace_back(replacement_worker);
        self->send(replacement_worker,self->state.prev_Nts);
        self->send(self->state.server, update_cycles_atom::value, self->state.prev_cycles);
    });

    return
    {
        // handle a new job and send it to his workers
        [=](new_job_atom, numberToSplit nts)
        {
            cerr << "manager got a new job with nts.n = " << nts.N << endl;
            self->state.prev_Nts = nts;
            self->state.tPrev=get_wall_time();
            self->state.prev_cycles = 0;

            for (actor worker:self->state.workers)
            {
                self->send(worker, nts);
            }
        },
        // handle the answer of a worker
        [=](numberToSplit nts)
        {
            if(nts.p != TERMINATING_VALUE)
            {
                self->state.prev_cycles = nts.cycles;
                nts.deltaT = get_wall_time()-self->state.tPrev;
                self->state.tPrev = get_wall_time();
                self->send(self->state.server, deliver_atom::value, nts);
                nts.cycles = 0;
            }
        },
        //reset all workers of an manager
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
        // ack manager connect successful
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
            self->state.iAmBusy = false;
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
            self->state.iAmBusy = false;
            string retval = "I have no manager, try again later!";
            self->send(self->state.client, reply_atom::value, retval);

            if (!identified)
            {
                cerr << "coordinator: can't determine down_msg source" << endl;
            }
        }
    });

    return
    {
        // count the cycles of the worker
        [=](update_cycles_atom,int512_t manager_cycles)
        {
            self->state.cycles_total+=  manager_cycles;
        },
        //add a manager
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
        //request from a Client and handle the request
        [=](request_atom, int512_t N)
        {
            //if busy handling
            if(self->state.iAmBusy == true)
            {
                string retval = "I am busy, try again later!";
                self->send(actor_cast<actor>(self->current_sender()), reply_atom::value, retval);
            }
            //reset all before start the new task
            else
            {
                self->state.iAmBusy = true;
                self->state.setOfFactors.clear();
                self->state.setOfPrimFactors.clear();
                self->state.setOfUsedN.clear();
                self->state.cpu_time_total=0.0;
                self->state.cycles_total=0;
                self->state.N = N;
                self->state.num.N = N;
                cerr << "N: " << self->state.num.N << endl;

                //  start Timer
                self->state.walltimeTime = get_wall_time();

                //if the task is a prime answer don*t start the worker and return directly
                if(IsPrime(self->state.num.N) || self->state.num.N == 1)
                {
                    self->state.num.p = 1;
                    self->state.num.isprim = true;
                    self->state.client = actor_cast<actor>(self->current_sender());

                    stringstream s;
                    s << N << " is a prim";
                    string retval = s.str();
                    self->state.iAmBusy = false;
                    self->send(self->state.client, reply_atom::value, retval);
                }
                else
                {
                    self->state.setOfFactors.insert(self->state.num.N);
                    if (self->state.managers.size() == 0)
                    {
                        self->state.iAmBusy = false;
                        cerr << "no managers available" << endl;
                        string retval = "I have no manager, try again later!";
                        self->send(actor_cast<actor>(self->current_sender()), reply_atom::value, retval);
                    }
                    else
                    {
                        //memorize client
                        self->state.client = actor_cast<actor>(self->current_sender());
                        //initial distribution
                        self->state.lastSended_N = self->state.num.N;
                        self->state.num.cycles = 0;
                        for (auto manager:self->state.managers)
                        {
                            cerr << "sever sendet; num.N:" << self->state.num.N << endl;
                            self->send(manager, new_job_atom::value, self->state.num);
                        }
                    }
                }
            }
        },
        //redistribution after receiving result
        [=](deliver_atom, numberToSplit nts)
        {
            bool isFirstN = true;
            set<int512_t>::iterator iter;
            for(iter=self->state.setOfUsedN.begin(); iter!=self->state.setOfUsedN.end(); ++iter)
            {
                if(*iter == nts.N)
                {
                    isFirstN = false;
                }
            }
            if (isFirstN == true)
            {
                self->state.setOfUsedN.insert(nts.N);
                self->state.cycles_total=self->state.cycles_total+nts.cycles;
                self->state.cpu_time_total+=nts.deltaT;
                self->state.num = nts;
                evaluate(self->state);

                if ( self->state.setOfFactors.empty())
                {
                    //  Stop timers
                    double wallEndTime = get_wall_time();
                    double wallTime = wallEndTime - self->state.walltimeTime;
                    double cpuTime = self->state.cpu_time_total;

                    stringstream s;
                    s << "Number of Runs: " << self->state.cycles_total << endl;
                    s << "Wall Time = " << wallTime << endl;
                    s << "CPU Time  = " << cpuTime << endl;
                    self->state.cycles_total = 0;
                    self->state.num.cycles = 0;

                    // add setOfPrimFactors to the output string
                    s << "The Primfactors for N are: ";
                    set<int512_t>::iterator iter;
                    for(iter=self->state.setOfPrimFactors.begin(); iter!=self->state.setOfPrimFactors.end(); ++iter)
                    {
                        s << *iter << " " ;
                    }
                    string retval = s.str();
                    cerr << retval << endl;
                    self->state.iAmBusy = false;
                    self->send(self->state.client, reply_atom::value, retval);
                }
                else
                {
                    set<int512_t>::iterator iter =  self->state.setOfFactors.begin();
                    // every factor is distributed in turn,
                    // when there are no more factors, it starts all over again
                    int i = 0;
                    self->state.num.N = *iter;
                    if (self->state.lastSended_N != self->state.num.N)
                    {
                        for (auto manager:self->state.managers)
                        {

                            self->state.num.N = *iter;
                            self->state.lastSended_N = *iter;
                            self->state.num.cycles = 0;
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
            else
            {
                self->state.setOfFactors.erase(nts.N);
            }
        }
    };
}

//launching server as coordinator
void run_server(actor_system &sys, const config &cfg)
{
    cerr << "run_server" << endl;
    auto &mm = sys.middleman();
    mm.publish(sys.spawn(coordinator), cfg.port);

}
// start manager
void run_manager(actor_system &sys, const config &cfg)
{
    cerr << "run_manager" << endl;
    sys.spawn(manager,&sys,cfg.host,cfg.port,cfg.num_workers);
}
// start manager
void run_client(actor_system &sys, const config &cfg)
{
    cerr << "run_client" << endl;
    int512_t N = 0;
    string inputString = "";
    cout << "Bitte geben Sie eine Zahl ein!" << endl;
    cin >> N;
    if(N == -1)
    {
        N = Z1;
    }
    else if(N == -2)
    {
        N = Z2;
    }
    else if(N == -3)
    {
        N = Z3;
    }
    else if(N == -4)
    {
        N = Z4;
    }


    cout << "Bitte warten bis die Zahl zerlegt wurde." << endl;


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
        //send request to server and print the answer of the server
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
}
}
CAF_MAIN(io::middleman)


