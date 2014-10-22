// Copyright (c) 2012-2014 The Bitcoin Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "wallet.h"

#include <set>
#include <stdint.h>
#include <utility>
#include <vector>

#include <boost/foreach.hpp>
#include <boost/test/unit_test.hpp>

// how many times to run all the tests to have a chance to catch errors that only show up with particular random shuffles
#define RUN_TESTS 100

// some tests fail 1% of the time due to bad luck.
// we repeat those tests this many times and only complain if all iterations of the test fail
#define RANDOM_REPEATS 5

using namespace std;

BOOST_AUTO_TEST_SUITE(wallet_tests)

static CWallet wallet;
//static vector<COutput> vCoins;

static void add_coin(int64_t nValue, int nAge = 6*24, bool fIsFromMe = false, int nInput=0)
{
//    static int nextLockTime = 0;
//    CTransaction tx;
//    tx.nLockTime = nextLockTime++;        // so all transactions get different hashes
//    tx.vout.resize(nInput+1);
//    tx.vout[nInput].nValue = nValue;
//    CWalletTx* wtx = new CWalletTx(&wallet, tx);
//    if (fIsFromMe)
//    {
//        // IsFromMe() returns (GetDebit() > 0), and GetDebit() is 0 if vin.empty(),
//        // so stop vin being empty, and cache a non-zero Debit to fake out IsFromMe()
//        wtx->vin.resize(1);
//        wtx->fDebitCached = true;
//        wtx->nDebitCached = 1;
//    }
//    COutput output(wtx, nInput, nAge);
//    vCoins.push_back(output);
}

static void empty_wallet(void)
{
//    BOOST_FOREACH(COutput output, vCoins)
//        delete output.tx;
//    vCoins.clear();
}


BOOST_AUTO_TEST_CASE(coin_selection_tests)
{

}

BOOST_AUTO_TEST_SUITE_END()
