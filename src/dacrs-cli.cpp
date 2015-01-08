// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2013 The Dacrs developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "util.h"
#include "init.h"
#include "rpcclient.h"
#include "rpcprotocol.h"
#include "ui_interface.h" /* for _(...) */
#include "chainparams.h"

#include <boost/filesystem/operations.hpp>

//////////////////////////////////////////////////////////////////////////////
//
// Start
//
static bool AppInitRPC(int argc, char* argv[])
{
    //
    // Parameters
    //
	CBaseParams::IntialParams(argc, argv);
	SysCfg().InitalConfig();

    if (argc<2 || SysCfg().IsArgCount("-?") || SysCfg().IsArgCount("--help"))
    {
        // First part of help message is specific to RPC client
        string strUsage = _("Dacrs Core RPC client version") + " " + FormatFullVersion() + "\n\n" +
            _("Usage:") + "\n" +
              "  Dacrs-cli [options] <command> [params]  " + _("Send command to Dacrs Core") + "\n" +
              "  Dacrs-cli [options] help                " + _("List commands") + "\n" +
              "  Dacrs-cli [options] help <command>      " + _("Get help for a command") + "\n";

        strUsage += "\n" + HelpMessageCli(true);

        fprintf(stdout, "%s", strUsage.c_str());
        return false;
    }
    return true;
}

int main(int argc, char* argv[])
{
    SetupEnvironment();

    try
    {
        if(!AppInitRPC(argc, argv))
            return abs(RPC_MISC_ERROR);
    }
    catch (exception& e) {
        PrintExceptionContinue(&e, "AppInitRPC()");
        return abs(RPC_MISC_ERROR);
    } catch (...) {
        PrintExceptionContinue(NULL, "AppInitRPC()");
        return abs(RPC_MISC_ERROR);
    }

    int ret = abs(RPC_MISC_ERROR);
    try
    {
        ret = CommandLineRPC(argc, argv);
    }
    catch (exception& e) {
        PrintExceptionContinue(&e, "CommandLineRPC()");
    } catch (...) {
        PrintExceptionContinue(NULL, "CommandLineRPC()");
    }
    return ret;
}
