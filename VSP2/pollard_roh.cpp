/* a++ program to find a prime factor of composite using
   Pollard's Rho algorithm */
#include<bits/stdc++.h>
#include <boost/multiprecision/cpp_int.hpp>
#include <time.h>
#include <sys/time.h>


using namespace std;
using namespace boost::multiprecision;
using namespace boost::multiprecision::literals;

int512_t PollardRho(int512_t N);
inline bool IsPrime( int512_t number );
int512_t power(int512_t a, int512_t n, int512_t mod);
int512_t modular_pow(int512_t base, int512_t exponent, int512_t modulus);
void printSet(set<int512_t> setToPrint);
double get_wall_time();
double get_cpu_time();


set<int512_t> setOfPrimFactors;
set<int512_t> setOfFactors;
int512_t errorCode = -99;


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
int512_t PollardRho(int512_t N)
{
    /* initialize random seed */
    srand (time(NULL));

    /* no prime divisor for 1 */
    if (N==1) return N;

    /* even number means one of the divisors is 2 */
    if (N % 2 == 0) return 2;

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
    cout << "M = (int512_t) 1.18 * sqrt(sqrt(N)): " << M << endl;
    do
    {

        if (numberOfRuns >  M)
        {
            cout << "Stop and start again" << endl;
            return PollardRho(N);
        }
        if (checkMailboxValue == 2000)
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
    }
    while (p == 1);
    cout << "wie lange dauert das berechnen einer zahl: " << checkMailboxValue << endl;


    /* retry if the algorithm fails to find prime factor
     * with chosen x and a */
    if (p!=N)
    {
        return p;
    }
    else
    {
        //error code
        return errorCode;
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


double get_wall_time(){
    struct timeval time;
    if (gettimeofday(&time,NULL)){
        //  Handle error
        return 0;
    }
    return (double)time.tv_sec + (double)time.tv_usec * .000001;
}
double get_cpu_time(){
    return (double)clock() / CLOCKS_PER_SEC;
}



struct numberToSplit
{
    int512_t N;
    int512_t p;
    bool isprim = false;
};

/* driver function */
int main()
{

    //int512_t N = 210;


    int512_t N = Z3;

    cout << "Z1: " << Z1 << endl;
    cout << "Z2: " << Z2 << endl;
    cout << "Z3: " << Z3 << endl;
    cout << "Z4: " << Z4 << endl;
    cout << "N: " << N << endl;


//  Start Timers
    double wall0 = get_wall_time();
    double cpu0  = get_cpu_time();


    struct numberToSplit num;
    num.N = N;

    if(IsPrime(num.N))
    {
        return 0;
    }

    setOfFactors.insert(N);

    //while(N != 1 || !setOfFactors.empty())
    while(!setOfFactors.empty())
    {
        set<int512_t>::iterator iter = setOfFactors.begin();
        set<int512_t>::iterator iterend = setOfFactors.end();

        //TODO: Manager.size() nutzen nicht int anzahlManager nutzen
        int anzahlManager = 1;

        // weniger Manager als Faktoren
        if (anzahlManager <= setOfFactors.size())
        {
            // jeder manager bekommt einen andern Faktor
            for (int i = 0; i < anzahlManager; i++)
            {
                //TODO: *iter an Manager[i] senden
                num.p = PollardRho(*iter);
                num.N = *iter;
                ++iter;
            }
        }
        // mehr Manager als Faktoren
        else
        {
            // der reihe nach wird jeder faktor verteilt, wenn keine Faktoren mehr da sind beginnt er von vorn
            for (int i = 1; i <= anzahlManager; i++)
            {
                //TODO: *iter an Manager[i] senden
//                cout << "i: " << i << endl;
//                cout << "p = PollardRho(*iter); " << *iter << endl;
                num.p = PollardRho(*iter);
                num.N = *iter;



                if(i % setOfFactors.size() == 0)
                {
                    iter= setOfFactors.begin();
                }
                else
                {
                    ++iter;
                }
            }
        }


        if(num.p == errorCode)
        {
            num.p = PollardRho(num.N);
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

            //TODO: worker: num zurücksenden an Koordiantor



            //todo: Koordinator
            // num.N aus den Faktoren Löschen

            printSet(setOfFactors);

            //wenn num.p is prim insert in Primzahlen
            if(num.isprim == true)
            {
                setOfPrimFactors.insert(num.p);
//                cout << " Neue Primzahl gefunden: The Primfactors are: ";
//                printSet(setOfPrimFactors);
                cout << " Found new Primfaktor: The new Primfaktor is: " << num.p << endl;
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
                    cout << " Found new Primfaktor: The new Primfaktor is: " << zwischenspeicher << endl;
                }
            }
            setOfFactors.erase(num.N);
        }
    }


    //  Stop timers
    double wall1 = get_wall_time();
    double cpu1  = get_cpu_time();

    cout << "Wall Time = " << wall1 - wall0 << endl;
    cout << "CPU Time  = " << cpu1  - cpu0  << endl;
    // Print setOfPrimFactors
    cout << "N: " << N << endl;
    cout << "The Primfactors for N are: ";
    printSet(setOfPrimFactors);

    return 0;
}



///* a++ program to find a prime factor of composite using
//   Pollard's Rho algorithm */
//#include<bits/stdc++.h>
//#include <boost/multiprecision/cpp_int.hpp>
//
//using namespace std;
//using namespace boost::multiprecision;
//
//int512_t PollardRho(int512_t N);
//inline bool IsPrime( int512_t number );
//int512_t power(int512_t a, int512_t n, int512_t mod);
//int512_t modular_pow(int512_t base, int512_t exponent, int512_t modulus);
//void printSet(set<int512_t> setToPrint);
//
//
//
///* method to return prime divisor for N */
//int512_t PollardRho(int512_t N)
//{
//    /* initialize random seed */
//    srand (time(NULL));
//
//    if(IsPrime(N))
//    {
//        return N;
//    }
//
//    /* no prime divisor for 1 */
//    if (N==1) return N;
//
//    /* even number means one of the divisors is 2 */
//    if (N % 2 == 0) return 2;
//
//    /* we will pick from the range [1, N) */
//    int512_t x = (rand()% N + 1);
//
//    int512_t y = x;
//
//    /* the constant in f(x).
//     * Algorithm can be re-run with a different a
//     * if it throws failure for a composite. */
//    int512_t a = (rand()%(N-1))+1;
//
//    /* Initialize Difference */
//    int512_t d = 0;
//
//    /* Initialize candidate divisor (or result) */
//    int512_t p = 1;
//
//    /* until the prime factor isn't obtained.
//       If N is prime, return N */
//
//    do
//    {
//        /* Tortoise Move: x= (x² + a) mod N */
//        x = (x * x + a) % N;
//
//        /* Hare Move: y(i+1) = f(f(y(i))) */
//        y = (y * y + a) % N;
//        y = (y * y + a) % N;
//
//        /* Difference between (y - x) % N*/
//        d = (y - x) % N;
//
//        /* check gcd of |x-y| and N */
//        p = __gcd(d, N);
//        /* make the divisor positive */
//        if (p < 0)
//        {
//            p *= -1;
//        }
//
//
//    }
//    while (p == 1 || p == -1 );
//
//    /* retry if the algorithm fails to find prime factor
//     * with chosen x and a */
//    if (p!=N)
//    {
//        return p;
//
//    }
//}
//
//
//// Check Prime
/////////////////////////
//int512_t power(int512_t a, int512_t n, int512_t mod)
//{
//    int512_t power = a,result=1;
//
//    while(n)
//    {
//        if(n&1)
//            result=(result*power)%mod;
//        power=(power*power)%mod;
//        n>>=1;
//    }
//    return result;
//}
//
//bool witness(int512_t a, int512_t n)
//{
//    int512_t t,u,i;
//    int512_t prev,curr;
//
//    u=n/2;
//    t=1;
//    while(!(u&1))
//    {
//        u/=2;
//        ++t;
//    }
//
//    prev=power(a,u,n);
//    for(i=1; i<=t; ++i)
//    {
//        curr=(prev*prev)%n;
//        if((curr==1)&&(prev!=1)&&(prev!=n-1))
//            return true;
//        prev=curr;
//    }
//    if(curr!=1)
//        return true;
//    return false;
//}
//
//inline bool IsPrime( int512_t number )
//{
//    if ( ( (!(number & 1)) && number != 2 ) || (number < 2) || (number % 3 == 0 && number != 3) )
//        return (false);
//
//    if(number<1373653)
//    {
//        for( int k = 1; 36*k*k-12*k < number; ++k)
//            if ( (number % (6*k+1) == 0) || (number % (6*k-1) == 0) )
//                return (false);
//
//        return true;
//    }
//
//    if(number < 9080191)
//    {
//        if(witness(31,number)) return false;
//        if(witness(73,number)) return false;
//        return true;
//    }
//
//
//    if(witness(2,number)) return false;
//    if(witness(7,number)) return false;
//    if(witness(61,number)) return false;
//    return true;
//
//    /*WARNING: Algorithm deterministic only for numbers < 4,759,123,141 (unsigned int's max is 4294967296)
//      if n < 1,373,653, it is enough to test a = 2 and 3.
//      if n < 9,080,191, it is enough to test a = 31 and 73.
//      if n < 4,759,123,141, it is enough to test a = 2, 7, and 61.
//      if n < 2,152,302,898,747, it is enough to test a = 2, 3, 5, 7, and 11.
//      if n < 3,474,749,660,383, it is enough to test a = 2, 3, 5, 7, 11, and 13.
//      if n < 341,550,071,728,321, it is enough to test a = 2, 3, 5, 7, 11, 13, and 17.*/
//}
//
///////////////////////////
//
//
//void printSet(set<int512_t> setToPrint)
//{
//    set<int512_t>::iterator iter;
//    for(iter=setToPrint.begin(); iter!=setToPrint.end(); ++iter)
//    {
//        cout << *iter << " " ;
//    }
//    cout << endl;
//
//}
//
///* driver function */
//int main()
//{
//
//    set<int512_t> setOfPrimFactors;
//    set<int512_t> setOfFactors;
//
//    //int512_t N = 210;
//    int512_t N = 8806715679;
//    int512_t p = 1 ;
//
//    setOfFactors.insert(N);
//    while(N != 1 || !setOfFactors.empty())
//    {
//        p = PollardRho(N);
//
//        if(IsPrime(p))
//        {
//
//        //TODO: send p to Koordinator
//            setOfPrimFactors.insert(p);
//         //TODO: send N to Koordinator? oder weiß der
//            setOfFactors.erase(N);
//
//            //Check what we is to do
//            if(!setOfFactors.empty())
//            {
//                set<int512_t>::iterator oneFactor = setOfFactors.begin();
//                //TODO: send N to worker
//                N = *oneFactor;
//
//            }
//        }
//        else if (N % p == 0)
//        {
//            setOfFactors.insert(p);
//        }
//        while (N % p == 0)
//        {
//            N /= p;
//        }
//
//        if (N > 1 )
//        {
//            setOfFactors.insert(N);
//        }
//            cout << "N: " << N << endl;
//
//    }
//
//    // Print setOfPrimFactors
//    cout << "The Primfactors are: ";
//    printSet(setOfPrimFactors);
//
//    return 0;
//}

