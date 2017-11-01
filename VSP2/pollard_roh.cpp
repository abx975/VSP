/* a++ program to find a prime factor of composite using
   Pollard's Rho algorithm */
#include<bits/stdc++.h>
#include <boost/multiprecision/cpp_int.hpp>

using namespace std;
using namespace boost::multiprecision;

int512_t PollardRho(int512_t N);
inline bool IsPrime( int512_t number );
int512_t power(int512_t a, int512_t n, int512_t mod);
int512_t modular_pow(int512_t base, int512_t exponent, int512_t modulus);



/* method to return prime divisor for N */
int512_t PollardRho(int512_t N)
{
    /* initialize random seed */
    srand (time(NULL));

    if(IsPrime(N))
    {
        return N;
    }

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

    do
    {
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


    }
    while (p == 1 || p == -1 );

    /* retry if the algorithm fails to find prime factor
     * with chosen x and a */
    if (p!=N)
    {
        return p;

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

/* driver function */
int main()
{

    set<int512_t> setOfPrimFactors;
    set<int512_t> setOfFactors;

    //int512_t N = 210;
    int512_t N = 8806715679;

    int512_t p = 0 ;
    int512_t oldp= 0;

    setOfFactors.insert(N);
    while(N != 1 || !setOfFactors.empty())
    {
        p = PollardRho(N);

        if(IsPrime(p) && p != oldp)
        {
            setOfPrimFactors.insert(p);
            oldp = p;
            setOfFactors.erase(N);
            if(!setOfFactors.empty())
            {
                set<int512_t>::iterator setOfFactorsElem = setOfFactors.begin();
                N = *setOfFactorsElem;
                setOfFactors.erase(N);
            }
        }
        else if (N % p == 0)
        {
            setOfFactors.insert(p);
            setOfFactors.insert(N/p);
        }
        while (N % p == 0)
        {
            {
                N /= p;
            }
        }

        if (N > 1){
        setOfFactors.insert(N);
        }
    }
/// Print setOfPrimFactors
    set<int512_t>::iterator iter;
    cout << "The Primfactors are: ";
    for(iter=setOfPrimFactors.begin(); iter!=setOfPrimFactors.end(); ++iter)
    {
        cout << *iter << " " ;
    }
    cout << endl;
    return 0;
}

