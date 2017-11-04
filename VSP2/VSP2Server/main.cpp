
#include <string>
#include <iostream>
#include <stdexcept>
#include <vector>

#include "caf/all.hpp"
#include "caf/io/all.hpp"
#include<bits/stdc++.h>
#include <boost/multiprecision/cpp_int.hpp>
#include <time.h>
#include <sys/time.h>
#include "int512_serialization.hpp"

using std::cout;
using std::endl;
using std::cerr;
using std::endl;
using namespace caf;
using namespace std;
using namespace boost::multiprecision;
using namespace boost::multiprecision::literals;

//using add_atom = atom_constant<atom("add")>;

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
typename Inspector::result_type inspect(Inspector& f, numberToSplit& p) {
  return f(meta::type_name("numberToSplit"), p.N, p.p, p.isprim, p.numpPRohPerWorker, p.wallStartTime, p.cpuStartTime);
}

struct config : actor_system_config {
  std::string host = "localhost";
  uint16_t port = 9900;
  size_t num_workers = 1;
  bool server = true;
  config() {
    add_message_type<int512_t>("int512_t");
    add_message_type<numberToSplit>("numberToSplit");
    opt_group{custom_options_, "global"}
    .add(host, "host,H", "hostname of server")
    .add(port, "port,p", "IP port for publish/remote_actor")
    .add(num_workers, "num-workers,w", "number of workers (in manager mode)")
    .add(server, "server,s", "run as server");
    //.add_message_type<numberToSplit>("numberToSplit");
  }
};

struct state {
  actor worker;
  std::vector<actor> workers;
};


numberToSplit PollardRho(int512_t N);
inline bool IsPrime( int512_t number );
int512_t power(int512_t a, int512_t n, int512_t mod);
int512_t modular_pow(int512_t base, int512_t exponent, int512_t modulus);
void printSet(set<int512_t> setToPrint);
double get_wall_time();
double get_cpu_time();
numberToSplit worker(int512_t N);


set<int512_t> setOfPrimFactors;
set<int512_t> setOfFactors;
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

int512_t Z4 = 0x1aa0d675bd49341ccc03fff7170f29cd7048bf40430c22ced5a391d015d19677bde78a7b95b5d59b6b26678238fa7_cppui512;
/*
Z4 = 1000602106143806596478722974273666950903906112131794745457338659266842446985022076792112309173975243506969710503
   = 0xc72af6a83cc2d3984fedbe6c1d15e542556941e7 =
   10657 * 11657 * 13264529 * 10052678938039 * 2305843009213693951 * 26196077648981785233796902948145280025374347621094198197324282087 =
*/

/* method to return prime divisor for N */
numberToSplit PollardRho(int512_t N)
{
    /* initialize random seed */
    srand (time(NULL));
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

        if (numberOfRuns >  M)
        {
            cout << "Pollard is running longer than Middle M 1.18 * sqrt(sqrt(N)):" << M << endl;
            return PollardRho(N);
        }
        if (checkMailboxValue == 50000)
        {

            //checkmailbox;

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
    cout << "wie lange dauert das berechnen einer zahl: " << checkMailboxValue << endl;


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
        //error code
        return num;
    }
}


// Check Prime
///////////////////////
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

    /*WARNING: Algorithm deterministic only for numbers < 4,759,123,141 (unsigned int's max is 4294967296)
      if n < 1,373,653, it is enough to test a = 2 and 3.
      if n < 9,080,191, it is enough to test a = 31 and 73.
      if n < 4,759,123,141, it is enough to test a = 2, 7, and 61.
      if n < 2,152,302,898,747, it is enough to test a = 2, 3, 5, 7, and 11.
      if n < 3,474,749,660,383, it is enough to test a = 2, 3, 5, 7, 11, and 13.
      if n < 341,550,071,728,321, it is enough to test a = 2, 3, 5, 7, 11, 13, and 17.*/
}

/////////////////////////

void printSet(set<int512_t> setToPrint)
{
    if(setToPrint.size() > 0)
    {
        set<int512_t>::iterator iter;
        for(iter=setToPrint.begin(); iter!=setToPrint.end(); ++iter)
        {
            cout << *iter << " " ;
        }
        cout << endl;
    }
    else
    {
        cout << "The Set is empty" << endl;
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


//Verhalten der Worker
behavior workerBeh(event_based_actor* self) {
  return {
      [=](int512_t N) -> numberToSplit {
      aout(self) << "worker erreicht" <<endl;
       struct numberToSplit num;

    num = PollardRho(N);
    
    if(num.p == errorCode)
    {
        num = PollardRho(N);
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

        cerr << "worker läuft durch; num.N: " << num.N <<" num.p: "<< num.p <<   endl;
        
        return num;

    }
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
    [=](int512_t N) {
      
      aout(self) << "manager erreicht" <<endl;
      int i=0;
      for(auto& worker: self->state.workers){
        self->delegate(worker, N);
        cout << "sende " << N <<" an worker " << i << endl;

        i++;
      } 
    }
  };
}


void coordBeh(event_based_actor* self, std::vector<actor> managers) {  
 static struct numberToSplit num;
int512_t N = Z0;

//    cout << "Z1: " << Z1 << endl;
//    cout << "Z2: " << Z2 << endl;
//    cout << "Z3: " << Z3 << endl;
//    cout << "Z4: " << Z4 << endl;
    cout << "N: " << N << endl;

//  Start Timers
    double wallStartTime = get_wall_time();
    double cpuStartTime  = get_cpu_time();

    num.N = N;

    if(IsPrime(num.N) || num.N == 1)
    {
        cout << N << " is a prime" << endl;
        //todo: programm beenden
        //return 0;
    }

    setOfFactors.insert(N);

    //while(N != 1 || !setOfFactors.empty())
    while(!setOfFactors.empty())
    {
        set<int512_t>::iterator iter = setOfFactors.begin();
        set<int512_t>::iterator iterend = setOfFactors.end();

        //TODO: Manager.size() nutzen nicht int anzahlManager nutzen
        int anzahlManager = 1;

        // der reihe nach wird jeder faktor verteilt, wenn keine Faktoren mehr da sind beginnt er von vorn
        int i= 0;
        for (auto manager: managers)
        {
           i++;
            //TODO: *iter an Manager[i] senden
//                cout << "i: " << i << endl;
//                cout << "p = PollardRho(*iter); " << *iter << endl;
            
//////////////////// request/receive /////////////////////////////////////////////////////            
            
   //         num = worker(*iter);
     self->request(manager,infinite,*iter).then(   ///  numberToSplit = manager(N);
        [=](numberToSplit nts) {
        num = nts;
        num.N = nts.N;
        num.p = nts.p;
        num.isprim = nts.isprim;
        num.numpPRohPerWorker = nts.numpPRohPerWorker;
        num.wallStartTime = nts.wallStartTime;
        num.cpuStartTime = nts.cpuStartTime;
         aout(self) << " got num.p:  " << num.p << endl;
         cout << " got num.p:  " << num.p << endl;
         aout(self) << " got nts.p:  " << nts.p << endl;
         cout << " got nts.p:  " << nts.p << endl;
      });
         cerr << "got num; num.N: " << num.N <<" num.p: "<< num.p << endl;


////////////////////////////////////////////////////////////////////////////////////////            
            cout << "i % setOfFactors.size() == 0; i= "  << i << "setOfFactors.size(): " << setOfFactors.size() << endl;
            if(i % setOfFactors.size() == 0)
            {
                iter = setOfFactors.begin();
            }
            else
            {
                ++iter;
            }
        }

        //todo: Koordinator
        // num.N aus den Faktoren Löschen


        //wenn num.p is prim insert in Primzahlen
        if(num.isprim == true)
        {
            setOfPrimFactors.insert(num.p);
//                cout << " Neue Primzahl gefunden: The Primfactors are: ";
//                printSet(setOfPrimFactors);
            cout << "\tFound new Primfaktor: The new Primfaktor is: " << num.p << endl;
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
                cout << "\tFound new Primfaktor: The new Primfaktor is: " << zwischenspeicher << endl;
            }
        }
        setOfFactors.erase(num.N);
    }

    //  Stop timers
    double wallEndTime = get_wall_time();
    double cpuEndTime  = get_cpu_time();

    //TODO: anzahlworker ersetzen
    int numOfWorker = 1;
    cout << "Number of Runs: " << num.numpPRohPerWorker * numOfWorker << endl;
    cout << "Wall Time = " << wallEndTime - wallStartTime << endl;
    cout << "CPU Time  = " << (cpuEndTime  - cpuStartTime) * numOfWorker  << endl;
    // Print setOfPrimFactors
    cout << "N: " << N << endl;
    cout << "The Primfactors for N are: ";
    printSet(setOfPrimFactors);
}

///////////////////////////////////////////////////////////////////////////////////
//todo: if/else einkommentieren für Prozesse auf separaten Rechnern
void caf_main(actor_system& sys, const config& cfg) {

  auto& mm = sys.middleman();
  if (cfg.server) {
    auto p = mm.publish(sys.spawn(managerBeh,cfg.num_workers), cfg.port);
    if (!p){
      cerr << "unable to publish actor: " << sys.render(p.error()) << "\n";
    }else{
      cout << "math actor published at port " << *p << "\n";
    }
  }else{
    auto x = mm.remote_actor(cfg.host, cfg.port);
    if (!x){
      cerr << "unable to connect to server: " << sys.render(x.error()) << "\n";
    }else {
      std::vector<actor> managers;
      managers.emplace_back(*x);
      auto coordinator = sys.spawn(coordBeh, managers); 
    }
  }      
}


CAF_MAIN(io::middleman)
    
    



