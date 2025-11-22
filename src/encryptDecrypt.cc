#include <gmpxx.h>      // gmp (mpz_class)
#include <tuple>        // tuple

#include "keygen.h"     // modPow

/// @brief implementation of encrypt(), see "encryptDecrypt.h"
mpz_class encrypt(
    mpz_class message,
    std::tuple<mpz_class, mpz_class> publicKey )
{
    // calculate m^e (mod n)
    mpz_class enc = modPow(
        message,
        get<1>(publicKey),
        get<0>(publicKey) );
    
    return enc;
}

/// @brief implementation of decrypt(), see "encryptDecrypt.h"
mpz_class decrypt(
    mpz_class encryptedMessage,
    std::tuple<mpz_class, mpz_class> privateKey )
{
    // calculate c^d (mod n)
    mpz_class dec = modPow(
        encryptedMessage,
        get<1>(privateKey),
        get<0>(privateKey) );

    return dec;
}