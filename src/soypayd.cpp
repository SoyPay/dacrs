// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2013 The Dacrs developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "rpcserver.h"
#include "rpcclient.h"
#include "init.h"
#include "main.h"
#include "noui.h"
#include "ui_interface.h"
#include "util.h"
#include "cuiserve.h"
#include <boost/algorithm/string/predicate.hpp>
#include <boost/filesystem.hpp>

/* Introduction text for doxygen: */

/*! \mainpage Developer documentation
 *
 * \section intro_sec Introduction
 *
 * This is the developer documentation of the reference client for an experimental new digital currency called Dacrs (http://www.Dacrs.org/),
 * which enables instant payments to anyone, anywhere in the world. Dacrs uses peer-to-peer technology to operate
 * with no central authority: managing transactions and issuing money are carried out collectively by the network.
 *
 * The software is a community-driven open source project, released under the MIT license.
 *
 * \section Navigation
 * Use the buttons <code>Namespaces</code>, <code>Classes</code> or <code>Files</code> at the top of the page to start navigating the code.
 */

static bool fDaemon =false;

void DetectShutdownThread(boost::thread_group* threadGroup) {
	bool fShutdown = ShutdownRequested();
	// Tell the main threads to shutdown.
	while (!fShutdown) {
		MilliSleep(200);
		fShutdown = ShutdownRequested();
	}
	CUIServer::StopServer();
	if (threadGroup) {
		threadGroup->interrupt_all();
		threadGroup->join_all();
	}
}

//////////////////////////////////////////////////////////////////////////////
//
// Start
//
bool AppInit(int argc, char* argv[],boost::thread_group &threadGroup) {
//	boost::thread* detectShutdownThread = NULL;

	bool fRet = false;
	try {
		//
		// Parameters
		//
		// If Qt is used, parameters/Dacrs.conf are parsed in qt/Dacrs.cpp's main()
		CBaseParams::IntialParams(argc, argv);
		SysCfg().InitalConfig();

		if (SysCfg().IsArgCount("-?") || SysCfg().IsArgCount("--help")) {
			// First part of help message is specific to Dacrsd / RPC client
			std::string strUsage = _("Dacrs Core Daemon") + " " + _("version") + " " + FormatFullVersion() + "\n\n"
					+ _("Usage:") + "\n" + "  Dacrsd [options]                     " + _("Start Dacrs Core Daemon")
					+ "\n" + _("Usage (deprecated, use Dacrs-cli):") + "\n"
					+ "  Dacrsd [options] <command> [params]  " + _("Send command to Dacrs Core") + "\n"
					+ "  Dacrsd [options] help                " + _("List commands") + "\n"
					+ "  Dacrsd [options] help <command>      " + _("Get help for a command") + "\n";

			strUsage += "\n" + HelpMessage(HMM_BITCOIND);
			strUsage += "\n" + HelpMessageCli(false);

			fprintf(stdout, "%s", strUsage.c_str());
			return false;
		}

		// Command-line RPC
		bool fCommandLine = false;
		for (int i = 1; i < argc; i++)
			if (!IsSwitchChar(argv[i][0]) && !boost::algorithm::istarts_with(argv[i], "Dacrs:"))
				fCommandLine = true;

		if (fCommandLine) {
			int ret = CommandLineRPC(argc, argv);
			exit(ret);
		}
#ifndef WIN32
		fDaemon = SysCfg().GetBoolArg("-daemon", false);
		if (fDaemon)
		{
			fprintf(stdout, "Dacrs server starting\n");

			// Daemonize
			pid_t pid = fork();
			if (pid < 0)
			{
				fprintf(stderr, "Error: fork() returned %d errno %d\n", pid, errno);
				return false;
			}
			if (pid > 0) // Parent process, pid is child process id
			{
				CreatePidFile(GetPidFile(), pid);
				return true;
			}
			// Child process falls through to rest of initialization

			pid_t sid = setsid();
			if (sid < 0)
			fprintf(stderr, "Error: setsid() returned %d errno %d\n", sid, errno);
		}
#endif
		SysCfg().SoftSetBoolArg("-server", true);


		fRet = AppInit2(threadGroup);
	} catch (std::exception& e) {
		PrintExceptionContinue(&e, "AppInit()");
	} catch (...) {
		PrintExceptionContinue(NULL, "AppInit()");
	}



	return fRet;
}
std::tuple<bool, boost::thread*> RunDacrs(int argc, char* argv[])
{
	boost::thread* detectShutdownThread = NULL;
	static boost::thread_group threadGroup;
	SetupEnvironment();

	bool fRet = false;

	// Connect Dacrsd signal handlers
	noui_connect();

	fRet = AppInit(argc, argv,threadGroup);

	detectShutdownThread = new boost::thread(boost::bind(&DetectShutdownThread, &threadGroup));

	if (!fRet) {
		if (detectShutdownThread)
			detectShutdownThread->interrupt();

		threadGroup.interrupt_all();

		// threadGroup.join_all(); was left out intentionally here, because we didn't re-test all of
		// the startup-failure cases to make sure they don't result in a hang due to some
		// thread-blocking-waiting-for-another-thread-during-startup case
	}
  return std::make_tuple (fRet,detectShutdownThread);
}

int main(int argc, char* argv[]) {

	std::tuple<bool, boost::thread*> ret = RunDacrs(argc,argv);

	boost::thread* detectShutdownThread  = std::get<1>(ret);

	bool fRet = std::get<0>(ret);

	if (detectShutdownThread) {
		detectShutdownThread->join();
		delete detectShutdownThread;
		detectShutdownThread = NULL;
	}

	Shutdown();

#ifndef WIN32
		fDaemon = SysCfg().GetBoolArg("-daemon", false);
#endif

	if (fRet && fDaemon)
		return 0;

	return (fRet ? 0 : 1);
}
