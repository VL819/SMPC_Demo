#include <tuple>        // tuple
#include <fstream>      // ifstream
#include <gmpxx.h>      // gmp (mpz_class)
#include <string>

#include "keygen.h"     // keygen function headers

/// @brief implementation of modPow(), see "keygen.h"
mpz_class modPow(mpz_class m, mpz_class e, mpz_class n)
{
    // TODO: is this correct???

    // return value
    mpz_class ret = 1_mpz;

    // edge case
    if (n == 0_mpz)
    {
        return 0_mpz;
    }

    m = m % n;

    // loop while exponent is greater than 0
    while(e > 0_mpz)
    {
        // if exp is odd...
        if( (e % 2) != 0 )
        {
            // ret = (ret * base) % mod
            ret = (ret * m) % n;
        }

        // bit shift exponent right 1 bit
        mpz_fdiv_q_2exp(e.get_mpz_t(), e.get_mpz_t(), 1);

        // base = (base * base) % mod
        m = (m * m) % n;
    }
    
    // return computed value
    return ret;
}

/// @brief implementation of randNBits(), see "keygen.h"
mpz_class randNBits(int n, gmp_randclass * rand)
{
    // get random number between 0 and (2^1024 - 1)
    mpz_class ret = rand->get_z_bits(n);
    // might not ACTUALLY contain a 1024-bit number (most sig bit could be 0)

    // set most significant bit to 1 to ensure that it is a 1024 bit number
    mpz_setbit(ret.get_mpz_t(), (n-1));

    // return the random number
    return ret;
}

/// @brief implementation of ssPrimality, see "keygen.h"
bool ssPrimality(mpz_class n, gmp_randclass * rand)
{
    // number of times to do Solovay-Strassen primality test
    int k = 55;
    // counter for number of times n has passed Solovay-Strassen
    int numLoops = 0;

    bool isPrime = true;
    while ( (isPrime) && (numLoops < k) )
    {
        // randomly choose a number between 0 and n-2
        mpz_class a = rand->get_z_range( (n - 1_mpz) );
        // add 1 to get a number between 1 and n-1
        a = a + 1_mpz;

        //DEBUG:
        //std::cout << "a=" << a << "\n";
        //std::cout << "numLoops=" << numLoops << "\n";

        // first test
        if (gcd(a, n) != 1_mpz)
        {
            //DEBUG:
            //std::cout << "failed TEST_1\n";

            isPrime = false;
        }

        // second test
        mpz_class exp = (n - 1_mpz) / 2_mpz;
        mpz_class test2 = modPow(a, exp, n);

        if ( (test2 != 1) && (test2 != (n - 1_mpz)) )
        {
            //DEBUG:
            //std::cout << "failed TEST_2\n";

            isPrime = false;
        }

        // third test
        int jacobi = mpz_jacobi(a.get_mpz_t(), n.get_mpz_t());

        mpz_class jacobi_mpz(jacobi);
        if (jacobi_mpz == -1)
        {
            jacobi_mpz = (n - 1_mpz);
        }

        if ( (test2 != jacobi_mpz) )
        {
            //DEBUG:
            //std::cout << "failed TEST_3\n";

            isPrime = false;
        }

        // increment numLoops
        numLoops = numLoops + 1;
    }

    if (isPrime == true)
    {
        //std::cout <<
        //    "\n\n***************************************************\n\n" <<
        //    "Used 55 values for a to determine primality\n" <<
        //    "The probability that " << n <<
        //    " is NOT prime is roughly 1 in 1000000000000, or " <<
        //    "0.000000000001%" <<
        //    "\n\n***************************************************\n\n";
    }

    return isPrime;
}

/// @brief implemnetation of findRandPrime(), see "keygen.h"
mpz_class findRandPrime(int numBits, gmp_randclass * rand)
{
    bool isPrime = false;
    mpz_class p;

    int numTested = 0;

    while (!isPrime)
    {
        mpz_class n = 4_mpz;
        while(n % 2 == 0)
        {
            n = randNBits(numBits, rand);
        }
        //DEBUG:
        //std::cout << "n=" << n << "\n";
        
        isPrime = ssPrimality(n, rand);

        if(isPrime)
        {
            p = n;
        }

        numTested = numTested + 1;
    }

    // num tested before finding prime
    //std::cout << "\n\n##################\ntested " << std::to_string(numTested)
    //    << " numbers before finding a prime\n##################\n\n";

    // p is PROBABLY prime
    return p;
}

/// @brief implementation of genRSAKey(), see "keygen.h"
std::tuple<mpz_class, mpz_class, mpz_class> genRSAKey(gmp_randclass * rand)
{
    std::tuple<mpz_class, mpz_class, mpz_class> key;

    // get p and q
    mpz_class p = findRandPrime(1024, rand);
    mpz_class q = findRandPrime(1024, rand);
    // ensure p != q
    while (p == q)
    {
        q = findRandPrime(1024, rand);
    }

    mpz_class n = p * q;
    mpz_class phiN = (p - 1_mpz) * (q - 1_mpz);

    // chose a number e that is relatively prime to phi(n)
    mpz_class e;
    do
    {
        e = rand->get_z_bits(2048);
    }
    while (gcd(e, phiN) != 1_mpz);

    // calculate d
    mpz_class d;
    mpz_invert(d.get_mpz_t(), e.get_mpz_t(), phiN.get_mpz_t());

    key = std::make_tuple(n, e, d);

    return key;
}

std::string shorten_mpz(mpz_class mpz)
{
    std::string ret;

    std::string mpz_str = mpz.get_str();

    ret = ret + mpz_str.substr(0, 3);

    ret = ret + "...";

    ret = ret + mpz_str.substr((strlen(mpz_str.c_str()) - 3), 3);

    return ret;
}