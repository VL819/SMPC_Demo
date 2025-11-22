#ifndef VINSMPCENCDEC_H
#define VINSMPCENCDEC_H

#include <gmpxx.h>      // gmp (mpz_class)
#include <tuple>        // tuple

// function headers for encryptDecrypt.cc

/// @brief encrypts message using given public key
/// @param publicKey public key (n, e)
/// @return encrypted message
mpz_class encrypt(
    mpz_class message,
    std::tuple<mpz_class, mpz_class> publicKey );

/// @brief decrypts message using given private key
/// @param privateKey private key (n, d)
/// @return decrypted message
mpz_class decrypt(
    mpz_class encryptedMessage,
    std::tuple<mpz_class, mpz_class> privateKey );

#endif