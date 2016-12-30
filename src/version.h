// Copyright (c) 2012 The DACRS developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#ifndef DACRS_VERSION_H_
#define DACRS_VERSION_H_

#include "clientversion.h"
#include <string>

static const int g_sClientVersion =
                           1000000 * CLIENT_VERSION_MAJOR
                         +   10000 * CLIENT_VERSION_MINOR
                         +     100 * CLIENT_VERSION_REVISION
                         +       1 * CLIENT_VERSION_BUILD;

extern const std::string g_strClientName;
extern const std::string g_strClientBuild;
extern const std::string g_strClientDate;

//
// network protocol versioning
//

static const int g_sProtocolVersion = 10007;

// intial proto version, to be increased after version/verack negotiation
static const int g_sInitProtoVersion = 10001;

// disconnect from peers older than this proto version
static const int g_sMinPeerProtoVersion = 10006;

// nTime field added to CAddress, starting with this version;
// if possible, avoid requesting addresses nodes older than this
//static const int CADDR_TIME_VERSION = 31402;

// only request blocks from nodes outside this range of versions
//static const int NOBLKS_VERSION_START = 32000;
//static const int NOBLKS_VERSION_END = 32400;

// BIP 0031, pong message, is enabled for all versions AFTER this one
//static const int BIP0031_VERSION = 60000;

// "mempool" command, enhanced "getdata" behavior starts with this version:
//static const int MEMPOOL_GD_VERSION = 60002;

#endif
