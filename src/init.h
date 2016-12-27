// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2013 The DACRS developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef DACRS_INIT_H_
#define DACRS_INIT_H_

#include <string>
using std::string;
class CWallet;

namespace boost {
    class thread_group;
};

extern CWallet* g_pwalletMain;

void StartShutdown();
bool ShutdownRequested();
void Shutdown();
bool AppInit2(boost::thread_group& threadGroup);

/* The help message mode determines what help message to show */
enum emHelpMessageMode
{
    EM_HMM_COIND,
    EM_HMM_COIN_QT
};

string HelpMessage(emHelpMessageMode mode);

#endif
