#include <iostream>         // cin, cout
#include <string>           // string
#include <tuple>            // tuple
#include <gmpxx.h>          // gmp (mpz_class)
#include <random>           // random_device (for seeding gmp_randclass)
#include <vector>           // vector

#include "main.h"
#include "keygen.h"
#include "encryptDecrypt.h"

#define RANGE_MIN 0
#define RANGE_MAX 9

#define NUM_BITS 1024

using namespace std;

int main()
{
    string nothing;

    // generate keys for Alice and Bob

    tuple<tuple<mpz_class, mpz_class>, tuple<mpz_class, mpz_class>> bobKeypair;
    tuple<tuple<mpz_class, mpz_class>, tuple<mpz_class, mpz_class>> aliceKeypair;

    bobKeypair = generateKeyForUser();
    aliceKeypair = generateKeyForUser();

    // generate numbers of millions for Alice and Bob

    // initialize the class for random number generation
    gmp_randclass * rand = new gmp_randclass(gmp_randinit_default);
    random_device rd{};
    unsigned long int seed = rd();
    rand->seed(seed);

    // GENERATE RAND IN RANGE FOR ALICE
    mpz_class aliceMillions = rand->get_z_range(
        mpz_class((RANGE_MAX - RANGE_MIN) + 1) );
    
    mpz_add_ui(
        aliceMillions.get_mpz_t(),
        aliceMillions.get_mpz_t(),
        RANGE_MIN );
    
    // GENERATE RAND IN RANGE FOR BOB
    mpz_class bobMillions = rand->get_z_range(
        mpz_class((RANGE_MAX - RANGE_MIN) + 1) );
    
    mpz_add_ui(
        bobMillions.get_mpz_t(),
        bobMillions.get_mpz_t(),
        RANGE_MIN );

    cout << "Alice has " << aliceMillions << " millions\n";
    cout << "Bob has " << bobMillions << " millions\n";

    
    //  -----------------------------------------------------------------------
    //                      BEGIN 2-PARTY SMPC PROTOCOL
    //  -----------------------------------------------------------------------

    // Step 1:
    // Bob picks a random N-bit integer and encrypts it with Alice's public key

    // generate Bob's random number
    mpz_class bobRand = randNBits(NUM_BITS, rand);

    cout << "=========================BOB STARTS======================\n";
    
    cout << "Bob's Rand: [" << shorten_mpz(bobRand) << "]\n";
    getline(cin, nothing);

    // encrypt it with Alice's public key
    mpz_class k = encrypt(bobRand, get<0>(aliceKeypair));

    cout << "Bob's Rand Encrypted: ["
        << shorten_mpz(k) << "]\n";
    getline(cin, nothing);

    
    // Step 2:
    // Bob sends Alice (k - <bobMillions> + 1)

    mpz_class bob2alice_1;
    mpz_sub(
        bob2alice_1.get_mpz_t(),
        k.get_mpz_t(),
        bobMillions.get_mpz_t() );
    
    mpz_add_ui(bob2alice_1.get_mpz_t(), bob2alice_1.get_mpz_t(), 1);

    cout << "Bob sends Alice: ["
        << shorten_mpz(k) << "] - "
        << bobMillions.get_str()
        << " + 1: ["
        << shorten_mpz(bob2alice_1) << "]\n";
    getline(cin, nothing);


    // Step 3:
    // Alice computes privately the values of y_{u} = D_{a}(k - j + u) for
    //      u = range(RANGE_MIN, RANGE_MAX)

    // instantiate vector to hold values
    vector<mpz_class> yVector;

    // for each u in range...
    for (int u = RANGE_MIN; u <= RANGE_MAX; u++)
    {
        // compute (k - j + u) using the number that Bob sent Alice
        mpz_class yEnc;

        // edge case
        if (u == 0)
        {
            mpz_sub_ui(yEnc.get_mpz_t(), bob2alice_1.get_mpz_t(), 1);
        }
        else
        {
            mpz_add_ui(yEnc.get_mpz_t(), bob2alice_1.get_mpz_t(), (u - 1));
        }

        // add encrypted number to the vector
        yVector.push_back(yEnc);
    }

    cout << "==============ALICE DOES COMPUTATION=====================\n";
    cout << "Alice creates vector by adding (index - 1) to each element: \n";
    for (int i = 0; i < yVector.size(); i++)
    {
        if (i == mpz_get_ui(bobMillions.get_mpz_t()) - RANGE_MIN)
        {
            cout << "*";
        }
        cout << "["
            << shorten_mpz(yVector.at(i))
            << "]";
        
        if (i == (yVector.size() - 1))
        {
            cout << "\n";
        }
        else
        {
            cout << ", ";
        }
    }
    getline(cin, nothing);

    for (mpz_class &y_u : yVector)
    {
        // decrypt (k - j + u) with Alice's private key
        y_u = decrypt(y_u, get<1>(aliceKeypair));
    }

    cout << "Alice decrypts each element: \n";
    for (int i = 0; i < yVector.size(); i++)
    {
        if (i == mpz_get_ui(bobMillions.get_mpz_t()) - RANGE_MIN)
        {
            cout << "*";
        }
        cout << "["
            << shorten_mpz(yVector.at(i))
            << "]";
        
        if (i == (yVector.size() - 1))
        {
            cout << "\n";
        }
        else
        {
            cout << ", ";
        }
    }
    getline(cin, nothing);


    // Step 4:
    // Alice generates a prime p of N/2 bits and computes the values
    //      z_{u} = y_{u} (mod p) for all u; if all z_{u} differ by at least 2
    //      in the (mod p) sense, stop; otherwise generates a new prime and
    //      repeat the process until all z_{u} differ by at least 2

    // instantiate vector to hold values
    vector<mpz_class> zVector;
    // mpz_class to hold the prime once critera are met
    mpz_class p;

    bool differsBy2 = false;

    while (!differsBy2)
    {
        // generate a prime of N/2 bits
        mpz_class pTemp = findRandPrime((NUM_BITS / 2), rand);

        //DEBUG
        //cout << "DEBUG: found rand prime\n";

        // instantiate vector to hold values for testing
        vector<mpz_class> tempZVector;

        // for each element in yVector...
        for (mpz_class &y_u : yVector)
        {
            // compute z_{u} = y_{u} (mod p)
            mpz_class z_u;
            mpz_mod(z_u.get_mpz_t(), y_u.get_mpz_t(), pTemp.get_mpz_t());

            // add it to the temp vector
            tempZVector.push_back(z_u);
        }

        //DEBUG
        //for (auto i : tempZVector)
        //    cout << i << " ";

        // check that each value differs by 2

        //DEBUG
        //cout << "DEBUG: begin checking difby2...\n";

        // start by assuming that they do
        differsBy2 = true;

        // for each element in the list, iterate over each and compare them
        for (int i = 0; i < tempZVector.size(); i++)
        {
            for (int j = i+1; j < tempZVector.size(); j++)
            {
                // subtract tempZVector[j] from tempZVector[i]
                mpz_class cmp;
                mpz_sub(
                    cmp.get_mpz_t(),
                    tempZVector.at(i).get_mpz_t(),
                    tempZVector.at(j).get_mpz_t() );

                //DEBUG
                //cout << cmp << "\n";
                
                // compute absolute value of difference
                mpz_abs(cmp.get_mpz_t(), cmp.get_mpz_t());

                // mpz_cmp returns 0 if the quantities are equal,
                //      and negative if op2 > op1
                if ( mpz_cmp_ui(cmp.get_mpz_t(), 2) <= 0 )
                {
                    differsBy2 = false;
                    //DEBUG
                    //cout << "DEBUG: didnt pass difby2\nfound a dif of " << cmp << "\n";
                }
            }
        }

        //DEBUG
        //cout << "DEBUG: complete checking difby2.\n";

        // if we got through this entire nested for-loop and differsBy2
        //      is still True, that means the entire vector differs by
        //      at least 2. the while-loop will end
        if (differsBy2)
        {
            p = pTemp;
            zVector = tempZVector;
        }
    }

    cout << "Alice generates prime: [" << shorten_mpz(p) << "]\n\n";

    cout << "Alice modulus each number in the vector by prime: \n";
    for (int i = 0; i < zVector.size(); i++)
    {
        if (i == mpz_get_ui(bobMillions.get_mpz_t()) - RANGE_MIN)
        {
            cout << "*";
        }
        cout << "["
            << shorten_mpz(zVector.at(i))
            << "]";
        
        if (i == (zVector.size() - 1))
        {
            cout << "\n";
        }
        else
        {
            cout << ", ";
        }
    }
    getline(cin, nothing);

    
    // Step 5:
    // Alice sends the prime p to Bob as well as a modified z_{u} vector:
    //      every number past the index of Alices number in the vector
    //      is equal to z_{u} + 1

    // if the range doesn't start with 0, we need to find the offset of
    // where to find Alice's index in the list
    int indexAlice = mpz_get_ui(aliceMillions.get_mpz_t()) - RANGE_MIN;

    // instantiate vector to hold numbers to send to Bob
    vector<mpz_class> toBobVector;

    for (int i = 0; i < zVector.size(); i++)
    {
        if (i <= indexAlice)
        {
            toBobVector.push_back(zVector.at(i));
        }
        else
        {
            mpz_class addToVec;
            mpz_add_ui(
                addToVec.get_mpz_t(),
                zVector.at(i).get_mpz_t(),
                1 );
            
            toBobVector.push_back(addToVec);
        }
    }

    cout << "Alice adds 1 to each number after index "
        << to_string(indexAlice) << "\n";
    getline(cin, nothing);

    // send Bob p followed by the modified vector
    tuple<mpz_class, vector<mpz_class>> alice2bob_1;
    alice2bob_1 = make_tuple(p, toBobVector);

    cout << "Alice sends bob prime: [" << shorten_mpz(p) << "]\n\n";

    cout << "Alice sends vector with modified values: \n";
    for (int i = 0; i < toBobVector.size(); i++)
    {
        if (i == mpz_get_ui(bobMillions.get_mpz_t()) - RANGE_MIN)
        {
            cout << "*";
        }
        cout << "["
            << shorten_mpz(toBobVector.at(i))
            << "]";
        
        if (i == (toBobVector.size() - 1))
        {
            cout << "\n";
        }
        else
        {
            cout << ", ";
        }
    }
    getline(cin, nothing);


    // Step 6:
    // Bob looks at the j-th number (the number at the index of his number)
    //      in the list that Alice sent, and decides that:
    //          i >= j if it is equal to x mod p (Alice has the same or higher)
    //          i < j otherwise (Bob has a bigger number)

    // calcuate x (mod p)
    mpz_class xModP;
    mpz_mod(
        xModP.get_mpz_t(),
        bobRand.get_mpz_t(),
        get<0>(alice2bob_1).get_mpz_t() );

    cout << "==============BOB DOES COMPARISON=====================\n";

    cout << "Bob calculates his random number, mod p: ["
        << shorten_mpz(xModP) << "]\n";
    getline(cin, nothing);
    
    // if the range doesn't start with 0, we need to find the offset of
    // where to find Bob's index in the list
    int indexBob = mpz_get_ui(bobMillions.get_mpz_t()) - RANGE_MIN;
    
    // compare x (mod p) to the element at j in the list
    mpz_class element_j = (
        get<1>(alice2bob_1).at(indexBob) );

    cout << "Bob compares this to the number in the vector Alice sent "
        << "at his index " << to_string(indexBob) << ": ["
        << shorten_mpz(element_j) << "]\n";
    getline(cin, nothing);
    
    if (mpz_cmp(xModP.get_mpz_t(), element_j.get_mpz_t()) == 0)
    {
        cout << "Alice has the same or more millions!\n";
    }
    else
    {
        cout << "Bob has more millions!\n";
    }

    exit(0);
}

/// @brief implementation of generateKeyForUser(), see "main.h"
tuple<
    tuple<mpz_class, mpz_class>,
    tuple<mpz_class, mpz_class> > generateKeyForUser()
{
    // initialize the class for random number generation
    gmp_randclass * rand = new gmp_randclass(gmp_randinit_default);
    random_device rd{};
    unsigned long int seed = rd();
    rand->seed(seed);

    // get n, e, d
    tuple<mpz_class, mpz_class, mpz_class> key = genRSAKey(rand);

    // public key tuple
    tuple<mpz_class, mpz_class> publicKey;
    publicKey = make_tuple(get<0>(key), get<1>(key));

    // private key tuple
    tuple<mpz_class, mpz_class> privateKey;
    privateKey = make_tuple(get<0>(key), get<2>(key));

    tuple<tuple<mpz_class, mpz_class>, tuple<mpz_class, mpz_class>> keypair;
    keypair = make_tuple(publicKey, privateKey);

    return keypair;
}
