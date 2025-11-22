#ifndef VINSMPCMAIN_H
#define VINSMPCMAIN_H

#include <string>
#include <tuple>
#include <gmpxx.h>

// function headers for main

/// @brief generates a keypair (<public key>, <private key>)
/// @return keypair
std::tuple<
    std::tuple<mpz_class, mpz_class>,
    std::tuple<mpz_class, mpz_class> > generateKeyForUser();

#endif