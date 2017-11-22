/**
 * @author Nils Eggebrecht
 * @author Lennart Hartmann
 * @version 5.11.2017
 */
#include <string>
#include <iostream>
#include <vector>
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

//persistent memory for manangers
struct state
{
    actor worker;
    std::vector<actor> workers;
};

struct numberToSplit
{
    int512_t N = 0;
    int512_t p = 0;
    bool isprim = false;
    int512_t numpPRohPerWorker = 0;
    double wallStartTime = 0;
    double cpuStartTime = 0;
};

template <class Inspector>
typename Inspector::result_type inspect(Inspector& f, numberToSplit& p)
{
    return f(meta::type_name("numberToSplit"), p.N, p.p, p.isprim, p.numpPRohPerWorker, p.wallStartTime, p.cpuStartTime);
}

numberToSplit PollardRho(int512_t N, event_based_actor* self); 
inline bool IsPrime( int512_t number );
int512_t power(int512_t a, int512_t n, int512_t mod);
int512_t modular_pow(int512_t base, int512_t exponent, int512_t modulus);
void printSet(set<int512_t> setToPrint);
double get_wall_time();
double get_cpu_time();
numberToSplit worker(int512_t N);


set<int512_t> setOfPrimFactors;
set<int512_t> setOfFactors;
///////////// negativ => absturz ////////////////////////////////
int512_t errorCode = 1111111111111111111;  //-99;   
//////////////////////////////////////////////////////////////////
static int numPRoh = 0;


int512_t Z0 = 21;//210;
int512_t Z1 = 64;//8806715679; // 3 * 29 * 29 * 71 * 211 * 233
int512_t Z2 = 128;//0x826efbb5b4c665b9_cppui512; // 9398726230209357241; // 443 * 503 * 997 * 1511 * 3541 * 7907
int512_t Z3 = 210;//0xc72af6a83cc2d3984fedbe6c1d15e542556941e7_cppui512;
/*
Z3 = 1137047281562824484226171575219374004320812483047 = 0xc72af6a83cc2d3984fedbe6c1d15e542556941e7 =
7907 * 12391 * 12553 * 156007 * 191913031 * 4302407713 * 7177162612387
*/
int512_t Z4 = 8806715679; // 3 * 29 * 29 * 71 * 211 * 233 0x1aa0d675bd49341ccc03fff7170f29cd7048bf40430c22ced5a391d015d19677bde78a7b95b5d59b6b26678238fa7_cppui512;
/*
Z4 = 1000602106143806596478722974273666950903906112131794745457338659266842446985022076792112309173975243506969710503
   = 0xc72af6a83cc2d3984fedbe6c1d15e542556941e7 =
   10657 * 11657 * 13264529 * 10052678938039 * 2305843009213693951 * 26196077648981785233796902948145280025374347621094198197324282087 =
*/

/* method to return prime divisor for N */
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
        if (checkMailboxValue >= 0)//50000)
        {

            //checkmailbox;
          if( !self->mailbox().empty() ){ 
            cerr << "worker got a new job" << endl;  
            self->quit(sec::request_receiver_down);  
          }else{
            cerr << "ok"<< endl;          
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
        num.numpPRohPerWorker = numPRoh;
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
double get_cpu_time()
{
    return (double)clock() / CLOCKS_PER_SEC;
}

//behavior of workers
behavior workerBeh(event_based_actor* self)
{
    return
    {
        [=](int512_t N) -> numberToSplit {
            cerr << "worker erreicht" <<endl;
            struct numberToSplit num;

            num = PollardRho(N, self);
            if(num.p == errorCode)
            {
                num = PollardRho(N,self);
            }
            else
            {
                //TODO: worker nachdem er num.p für num.N berechnet hat
                if(IsPrime(num.p))
                {
                    num.isprim = true;
                }
                else
                {
                    num.isprim = false;
                }

                cerr << "worker " << self->address().id() <<  " läuft durch; num.N: " << num.N <<" num.p: "<< num.p <<   endl;

                return num;
            }
        }
    };
}

//behaviour of managers
behavior managerBeh(stateful_actor<state>* self, int noOfWorkers)
{
    //initialisation
    for (auto i = 0; i < noOfWorkers; ++i)
    {
      //self->state.workers.emplace_back(self->spawn(workerBeh));
      auto worker=self->spawn(workerBeh);
      self->monitor(worker);
      self->state.workers.emplace_back(worker);
    }

    //setting bahavior for case server is down
    self->set_down_handler([=](const down_msg& dm) {
      cout << "starte worker neu\n";
      self->state.workers.erase(std::remove(self->state.workers.begin(), 
      self->state.workers.end(), dm.source), self->state.workers.end());
    // <monitored> is a shortcut for calling monitor() afterwards
      self->state.workers.emplace_back( self->spawn<monitored>(workerBeh));  
    });    
 

    //actual bahaviour
    return
    {
        [=](int512_t N)
        {
            cerr << "manager erreicht" <<endl;
            int i=0;
            for(auto& worker: self->state.workers)
            {
                self->delegate(worker, N);
                cerr << "sende " << N <<" an worker " << i << endl;
                i++;
            }
        }
    };
}

void caf_main(actor_system& sys)
{
    auto& mm = sys.middleman();

#if IS_SERVER==true
    //server-actor
    actor manager = sys.spawn(managerBeh,NR_OF_WORKERS);

    //publish manager
    auto p = mm.publish(manager, PORT);
    if (!p)
    {
        cerr << "unable to publish actor: " << sys.render(p.error()) << "\n";
    }
    else
    {
        cerr << "math actor published at port " << *p << "\n";
    }
#endif

#if IS_CLIENT==true
    // scoped actor for ad hoc communication
    scoped_actor self{sys};

    //managers for coordinator to address
    std::vector<actor> managers;

    //lookup managers
    auto remotePtr = mm.remote_actor(IP, PORT);
    if (!remotePtr)
    {
        cerr << "unable to connect to server: " << sys.render(remotePtr.error()) << "\n";
    }
    else
    {
        managers.emplace_back(*remotePtr);
    }

////////////////// coordinator behaviour ///////////////////////

    cerr << "launching coordinator" << endl;
    static struct numberToSplit num;
    int512_t N = Z4;

//    cerr << "Z1: " << Z1 << endl;
//    cerr << "Z2: " << Z2 << endl;
//    cerr << "Z3: " << Z3 << endl;
//    cerr << "Z4: " << Z4 << endl;
    cerr << "N: " << N << endl;

//  Start Timers
    double wallStartTime = get_wall_time();
    double cpuStartTime  = get_cpu_time();

    num.N = N;

    if(IsPrime(num.N) || num.N == 1)
    {
        cerr << N << " is a prime" << endl;
        //todo: programm beenden
        //return 0;
    }

    setOfFactors.insert(N);

    //while(N != 1 || !setOfFactors.empty())


    while(!setOfFactors.empty())
    {
        set<int512_t>::iterator iter = setOfFactors.begin();
        set<int512_t>::iterator iterend = setOfFactors.end();
        // der reihe nach wird jeder faktor verteilt, wenn keine Faktoren mehr da sind beginnt er von vorn
        int i= 0;
        for (auto manager: managers)
        {
            i++;
            self->request(manager, infinite, *iter).receive(
                [&](numberToSplit nts)
            {
                num = nts;
            },
            [&](error& err)
            {
                cerr << "Error: " << sys.render(err) << "\n";
            });

            if(i % setOfFactors.size() == 0)
            {
                iter = setOfFactors.begin();
            }
            else
            {
                ++iter;
            }
        }
        //wenn num.p is prim insert in Primzahlen
        if(num.isprim == true)
        {
            setOfPrimFactors.insert(num.p);
//                cerr << " Neue Primzahl gefunden: The Primfactors are: ";
//                printSet(setOfPrimFactors);
            cerr << "Found new Primfaktor: The new Primfaktor is: " << num.p << endl;
        }

        //wenn nicht dann insert p in Faktoren
        else
        {
            setOfFactors.insert(num.p);
        }

        // für jede zahl in Fakroten:
        set<int512_t>::iterator it;

        // für alle fakroten testen ob sie durch die gefundene Primzahl teilbar sind

        for(it=setOfFactors.begin(); it!=setOfFactors.end(); ++it)
        {
            int512_t zwischenspeicher = *it;

            // dividiere durch num.p bis != 0
            while (zwischenspeicher % num.p == 0)
            {
                zwischenspeicher/= num.p;
            }
            // insert das neue N

            bool zwiIsprime = IsPrime(zwischenspeicher);
            if(zwischenspeicher > 1 && !zwiIsprime)
            {
                setOfFactors.insert(zwischenspeicher);
            }
            else if(zwischenspeicher > 1 && zwiIsprime)
            {
                setOfPrimFactors.insert(zwischenspeicher);
                cerr << "Found new Primfaktor: The new Primfaktor is: " << zwischenspeicher << endl;
            }
        }
        setOfFactors.erase(num.N);
    }
    //  Stop timers
    double wallEndTime = get_wall_time();
    double cpuEndTime  = get_cpu_time();

    //TODO: anzahlworker ersetzen
    cerr << "Number of Runs: " << num.numpPRohPerWorker * NR_OF_WORKERS << endl;
    cerr << "Wall Time = " << wallEndTime - wallStartTime << endl;
    cerr << "CPU Time  = " << (cpuEndTime  - cpuStartTime) * NR_OF_WORKERS  << endl;

    // Print setOfPrimFactors
    cerr << "N: " << N << endl;
    cerr << "The Primfactors for N are: ";
    printSet(setOfPrimFactors);

#endif
}

CAF_MAIN(io::middleman)


