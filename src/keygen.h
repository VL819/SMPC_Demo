#ifndef VINSMPCKEY_H
#define VINSMPCKEY_H

#include <gmpxx.h>      // gmp (mpz_class)
#include <tuple>        // tuple

// function headers for keygen.cc

/// @brief fast modular exponentiation
/// @param m base
/// @param e exponent
/// @param n modulus
/// @return m^e (mod n)
mpz_class modPow(mpz_class m, mpz_class e, mpz_class n);

/// @brief generates a random N-bit number between 2^(N-1) and (2^N - 1)
/// @param rand pointer to a gmp random number generator
/// @return random N-bit integer
mpz_class randNBits(int n, gmp_randclass * rand);

/// @brief computes whether or not a number is prime (using Solovay-Strassen)
/// @param n number to test primality of
/// @param rand pointer to a gmp random number generator
/// @return true if n is prime, false otherwise
bool ssPrimality(mpz_class n, gmp_randclass * rand);

/// @brief computes a random N-bit prime number
/// @param rand pointer to a gmp random number generator
/// @param numBits number of bits
/// @return random N-bit prime number
mpz_class findRandPrime(int numBits, gmp_randclass * rand);

/// @brief generates an RSA key
/// @param rand pointer to a gmp random number generator
/// @return RSA key <n, e, d>
std::tuple<mpz_class, mpz_class, mpz_class> genRSAKey(gmp_randclass * rand);

std::string shorten_mpz(mpz_class mpz);

#endif