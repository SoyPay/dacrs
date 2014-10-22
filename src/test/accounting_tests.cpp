// Copyright (c) 2012-2014 The Bitcoin Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "wallet.h"
#include "walletdb.h"

#include <stdint.h>

#include <boost/foreach.hpp>
#include <boost/test/unit_test.hpp>

extern CWallet* pwalletMain;

BOOST_AUTO_TEST_SUITE(accounting_tests)

static void
GetResults(CWalletDB& walletdb, map<int64_t, CAccountingEntry>& results)
{
    list<CAccountingEntry> aes;

    results.clear();
    BOOST_CHECK(walletdb.ReorderTransactions(pwalletMain) == DB_LOAD_OK);
    walletdb.ListAccountCreditDebit("", aes);
    for (auto& ae : aes)
    {
        results[ae.nOrderPos] = ae;
    }
}

BOOST_AUTO_TEST_CASE(acc_orderupgrade)
{
    CWalletDB walletdb(pwalletMain->strWalletFile);
}

BOOST_AUTO_TEST_SUITE_END()
