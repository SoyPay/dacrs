/*
 * CRPCRequest2.cpp
 *
 *  Created on: Dec 9, 2014
 *      Author: ranger.shi
 */

#include "systestbase.h"



void DetectShutdownThread(boost::thread_group* threadGroup) {
	bool bShutdown = ShutdownRequested();
	// Tell the main threads to shutdown.
	while (!bShutdown) {
		MilliSleep(200);
		bShutdown = ShutdownRequested();
	}

	if (threadGroup) {
		threadGroup->interrupt_all();
		threadGroup->join_all();
	}
	CUIServer::StopServer();
}

bool PrintTestNotSetPara() {
	bool bFlag = false;
	if (1 == SysCfg().GetArg("-listen", bFlag)) {
		if (SysCfg().GetDefaultPort() == SysCfg().GetArg("-port", SysCfg().GetDefaultPort())) {
			cout << "Waring if config file seted the listen strParam must be true, and port can't be default port" << endl;
			MilliSleep(500);
			exit(0);
		}
	}
	string str("");
	string strConnect = SysCfg().GetArg("-connect", str);
	if (str != strConnect) {
		cout << "Waring the test of the config file the connect param must be false" << endl;
		MilliSleep(500);
		exit(0);
	}
	if (SysCfg().GetArg("-iscutmine", bFlag)) {
		cout << "Waring the test of config file the iscutmine param must be false" << endl;
		MilliSleep(500);
		exit(0);
	}
	if (!SysCfg().GetArg("-isdbtraversal", bFlag)) {
		cout << "Waring the test of config file the isdbtraversal param must be true" << endl;
		MilliSleep(500);
		exit(0);
	}

	if (!SysCfg().GetArg("-regtest", bFlag)) {
		cout << "Waring the test of config file the regtest param must be 1" << endl;
		MilliSleep(500);
		exit(0);
	}

	if (filesystem::exists(GetDataDir() / "blocks")) {
		cout << "Waring the test of must del " << (GetDataDir() / "blocks").string() << endl;
		MilliSleep(500);
		exit(0);
	}

	return true;
}

bool AppInit(int argc, char* argv[],boost::thread_group &threadGroup) {
	bool bRet = false;
	try {
		CBaseParams::IntialParams(argc, argv);
		SysCfg().InitalConfig();
		PrintTestNotSetPara();
		if (SysCfg().IsArgCount("-?") || SysCfg().IsArgCount("--help")) {
			// First part of help message is specific to Dacrsd / RPC client
			std::string strUsage = _("Bitcoin Core Daemon") + " " + _("version") + " " + FormatFullVersion() + "\n\n"
					+ _("Usage:") + "\n" + "  Dacrsd [options]                     " + _("Start Bitcoin Core Daemon")
					+ "\n" + _("Usage (deprecated, use Dacrs-cli):") + "\n"
					+ "  Dacrsd [options] <command> [params]  " + _("Send command to Bitcoin Core") + "\n"
					+ "  Dacrsd [options] help                " + _("List commands") + "\n"
					+ "  Dacrsd [options] help <command>      " + _("Get help for a command") + "\n";

			strUsage += "\n" + HelpMessage(EM_HMM_COIND);
			strUsage += "\n" + HelpMessageCli(false);

			fprintf(stdout, "%s", strUsage.c_str());
			return false;
		}

		// Command-line RPC
		bool bCommandLine = false;
		for (int i = 1; i < argc; i++)
			if (!IsSwitchChar(argv[i][0]) && !boost::algorithm::istarts_with(argv[i], "Dacrs:")) {
				bCommandLine = true;
			}
		if (bCommandLine) {
			int ret = CommandLineRPC(argc, argv);
			exit(ret);
		}

		SysCfg().SoftSetBoolArg("-server", true);

		bRet = AppInit2(threadGroup);
	} catch (std::exception& e) {
		PrintExceptionContinue(&e, "AppInit()");
	} catch (...) {
		PrintExceptionContinue(NULL, "AppInit()");
	}

	return bRet;
}

std::tuple<bool, boost::thread*> RunDacrs(int argc, char* argv[]) {
	boost::thread* detectShutdownThread = NULL;
	static boost::thread_group threadGroup;
	SetupEnvironment();

	bool bRet = false;

	// Connect Dacrsd signal handlers
	noui_connect();

	bRet = AppInit(argc, argv, threadGroup);

	detectShutdownThread = new boost::thread(boost::bind(&DetectShutdownThread, &threadGroup));

	if (!bRet) {
		if (detectShutdownThread)
			detectShutdownThread->interrupt();

		threadGroup.interrupt_all();
	}
	return std::make_tuple(bRet, detectShutdownThread);
}

SysTestBase::SysTestBase() {
	// todo Auto-generated constructor stub
}

SysTestBase::~SysTestBase() {
	// todo Auto-generated destructor stub
}

bool SysTestBase::ImportAllPrivateKey() {

	const char* pszKey[] = {
			        /*for bess test*/
					"cUa4v77hiXteMFkHoyuPVVbCCULS1CnFBhU1MhgKHEGRTHmd4BC5",// addr:  dkJwhBs2P2SjbQWt5Bz6vzjqUhXTymvsGr
					"cTAqnCwjuLwXqHxGe5c6KrGqQw5yjHH6Na6yYRQCgKKnf6cJBPxF",// addr:  ddEaChh3846J6xLkeyNaXwo6tMMZdHUTx6
					"cVFWoy8jmJVVSNnMs3YRizkR7XEekMTta4MzvuRshKuQEEJ4kbNg",// addr:  dggsWmQ7jH46dgtA5dEZ9bhFSAK1LASALw
					"cNcJkU44oG3etbWoEvY46i5qWPeE8jVb7K44keXxEQxsXUZ85MKU",//"address" : "dk2NNjraSvquD9b4SQbysVRQeFikA55HLi",
					"cStrXy6NowsDyaLRJMhQCJu4WnP6WR6SMC1c3dmxDeeLKFcYHDsQ",//"address" : "dpriG5eCE3uxMcGuN9hnLGm4fSkikZYnin",


					/*for yang test*/
					"cSu84vACzZkWqnP2LUdJQLX3M1PYYXo2gEDDCEKLWNWfM7B4zLiP",// addr:  dtKsuK9HUvLLHtBQL8Psk5fUnTLTFC83GS
					"cSVY69D9aUo4MugzUG9rM14DtV21cBAbZUVXmgAC2RpJwtZRUbsM",// addr:  dejEcGCsBkwsZaiUH1hgMapjbJqPdPNV9U
					"cTCcDyQvX6ucP9NEjhyHfTixamKQHQkFiSyfupm4CGZZYV7YYnf8",// addr:  dkoEsWuW3aoKaGduFbmjVDbvhmjxFnSbyL

					/*for franklin test*/
					"cUwPkEYdg3d3CmNctg2aegdyeq7dbLta1HAVHcGQTp33kWqzMSuT ",//addr:  dsGb9GyDGYnnHdjSvRfYbj9ox2zPbtgtpo
					/*for spark test*/
					"cPqVgscsWpPgkLHZP3pKJVSU5ZTCCvVhkd5cmXVWVydXdMTtBGj7",// addr:  doym966kgNUKr2M9P7CmjJeZdddqvoU5RZ
					"cU1dxQgvyKt8yEqqkKiNLK9jfyW498RKi8y2evqzjtLXrLD4fBMs",// addr:  dd936HZcwj9dQkefHPqZpxzUuKZZ2QEsbN
					"cRYYMN1EFd9X4sGqEkUkWLi38GCFyAccKQEuF1WiYFwUWsqBGwHe",// addr:  e21rEzVwkPFQYfgxcg7xLp7DKeYrW4Fpoz
					/*for ranger test*/
					"cR5wPiv3Vp4sQmww2gWzShkDUaamYrJ6QHHtDd1Pm4nVJFTxnksC",// addr:  dsjkLDFfhenmx2JkFMdtJ22TYDvSGgmJem
					"cT1BuRbx5Cvmvic2dX2aq3ep2fu75CDwYk8fCQPtrftKiBEQiPJm",// addr:  dps9hqUmBAVGVg7ijLGPcD9CJz9HHiTw6H
					/*for server 169 */
					"cQXpVRxwXqeh8FjSxkGE7sYrzXLXPdoHeUQCdJk9uLy17F3WKbPM",//"address" : "duNuYXNL1bXb9ay6Hg25oXC7DR2FZQQMfh",
					"cVNeGiYHhtaVSvmCswUs8jootYPFJisVwx6gqBbkeWSftkXeaHbC",//"address" : "dcszJChVG9NgndfGHJ4G2CUHJVyJoSiAme",
					"cNjb55M6fqNVuhKmNE95C8weWYr6iD2yW6QqifYWSvuVGKUJRTt9",//"address" : "dcmoFWD7sjPUBz4heykq1t3EJCDLWRTCu5",

					/*remain for test*/
				//	"cNcJkU44oG3etbWoEvY46i5qWPeE8jVb7K44keXxEQxsXUZ85MKU",//"address" : "dk2NNjraSvquD9b4SQbysVRQeFikA55HLi",
				//	"cStrXy6NowsDyaLRJMhQCJu4WnP6WR6SMC1c3dmxDeeLKFcYHDsQ",//"address" : "dpriG5eCE3uxMcGuN9hnLGm4fSkikZYnin",
					"cN2xNMvvNCtqh1K87J9o35cHHQttdZi1MYgUj8FYZPdtaFTtxbtd",//"address" : "ddMuEBkAwhcb5K5QJ83MqQHrgHRn4EbRdh",
					"cU8kr9JvCXotPoBQZ4TPxkD2S98ZFz2AKLDupMt8hgNG4JLQ1b2x",//"address" : "duKfNyq6zsuy2CMGMFVrQLuGi95UC7w6DV",
					"cSvRSiQGS6d11CbY4Mac1sCN84YHyNsFvNZ5xgBM1FHUSi7fgcaA",//"address" : "dv941EwdSsjpT6Fe5tzxzdMuSCx7s6oeqR",
					"cMv5HP4EPsX4Fmvj2Zgtpq5MfAbVRxwunqNtK2qjVrMdfvv36Zr3",//"address" : "dp3iSequpX4Jaax8kJ72LDnwTPZZWsDLvg",

	};

	int nCount = sizeof(pszKey) / sizeof(char*);
	for (int i = 0; i < nCount; i++) {
		const char *argv2[] = { "rpctest", "importprivkey", pszKey[i] };

		Value value;
		if (!CommandLineRPC_GetValue(sizeof(argv2) / sizeof(argv2[0]), argv2, value)) {
			return false;
		}
	}
	return true;
}

bool SysTestBase::ResetEnv() {
	const char *argv[] = { "rpctest", "resetclient" };
	Value value;
	if (!CommandLineRPC_GetValue(sizeof(argv) / sizeof(argv[0]), argv, value)) {
		return false;
	}
	if (ImportAllPrivateKey() == false) {
		return false;
	}
	return true;
}

int SysTestBase::GetRandomFee() {
	srand(time(NULL));
	int r = (rand() % 1000000);
	return r;
}

int SysTestBase::GetRandomMoney() {
	srand(time(NULL));
	int r = (rand() % 1000) + 1000;
	return r;
}

Value SysTestBase::CreateRegAppTx(const string& strAddress, const string& strScript, bool bRigsterScript, int nFee,
		int nHeight) {

	string strFilePath = SysCfg().GetDefaultTestDataPath() + strScript;
	if (!boost::filesystem::exists(strFilePath)) {
		BOOST_CHECK_MESSAGE(0, strFilePath + " not exist");
		return false;
	}

	string strFee = strprintf("%d",nFee);
	string strHeight = strprintf("%d",nHeight);

	const char *argv[] = { "rpctest", "registerapptx", (char*) strAddress.c_str(), (char*) strFilePath.c_str(),
			(char*) strFee.c_str(), (char*) strHeight.c_str(), "this is description" };
	int argc = sizeof(argv) / sizeof(char*);

	Value value;

	if (CommandLineRPC_GetValue(argc, argv, value)) {
		LogPrint("test_miners", "RegScriptTx:%s\r\n", write_string(value, true));
		return value;
	}
	return value;
}

Value SysTestBase::GetAccountInfo(const string& strID) {
	const char *argv[] = { "rpctest", "getaccountinfo", strID.c_str() };

	Value value;
	if (CommandLineRPC_GetValue(sizeof(argv) / sizeof(argv[0]), argv, value)) {
		return value;
	}
	return value;
}

Value SysTestBase::GetAppAccountInfo(const string& strScriptId, const string& strAddr) {
	const char *argv[] = { "rpctest", "getappaccinfo", (char*) strScriptId.c_str(), (char*) strAddr.c_str() };
	int argc = sizeof(argv) / sizeof(char*);

	Value value;
	if (CommandLineRPC_GetValue(argc, argv, value)) {
		LogPrint("test_miners", "GetAppAccountInfo:%s\r\n", write_string(value, true));
		return value;
	}
	return value;
}

bool SysTestBase::CommandLineRPC_GetValue(int argc, const char *argv[], Value &value) {
	string strPrint;
	bool bRes = false;
	try {
		// Skip switches
		while (argc > 1 && IsSwitchChar(argv[1][0])) {
			argc--;
			argv++;
		}

		// Method
		if (argc < 2) {
			throw runtime_error("too few parameters");
		}
		string strMethod = argv[1];

		// Parameters default to strings
		std::vector<std::string> strParams(&argv[2], &argv[argc]);
		Array params = RPCConvertValues(strMethod, strParams);

		// Execute
		Object reply = CallRPC(strMethod, params);

		// Parse reply
		const Value& result = find_value(reply, "result");
		const Value& error = find_value(reply, "error");

		if (error.type() != null_type) {
			// Error
			strPrint = "error: " + write_string(error, false);
//			int code = find_value(error.get_obj(), "code").get_int();
		} else {
			value = result;
			// Result
			if (result.type() == null_type)
				strPrint = "";
			else if (result.type() == str_type)
				strPrint = result.get_str();
			else
				strPrint = write_string(result, true);
			bRes = true;
		}
	} catch (boost::thread_interrupted) {
		throw;
	} catch (std::exception& e) {
		strPrint = string("error: ") + e.what();
	} catch (...) {
		PrintExceptionContinue(NULL, "CommandLineRPC()");
		throw;
	}

	if (strPrint != "") {
		if (false == bRes) {
//			cout<<strPrint<<endl;
		}
//	    fprintf((nRes == 0 ? stdout : stderr), "%s\n", strPrint.c_str());
	}

	return bRes;
}

bool SysTestBase::IsScriptAccCreated(const string& strScript) {
	Value valueRes = GetAccountInfo(strScript);
	if (valueRes.type() == null_type) {
		return false;
	}
	Value result = find_value(valueRes.get_obj(), "KeyID");
	if (result.type() == null_type) {
		return false;
	}
	return true;
}

uint64_t SysTestBase::GetBalance(const string& strID) {
	Value valueRes = GetAccountInfo(strID);
	BOOST_CHECK(valueRes.type() != null_type);
	Value result = find_value(valueRes.get_obj(), "Balance");
	BOOST_CHECK(result.type() != null_type);

	uint64_t ullMoney = result.get_int64();
	return ullMoney;
}

bool SysTestBase::GetNewAddr(std::string &strAddr, bool bFlag) {
	//CommanRpc
	string strParam = "false";
	if (bFlag) {
		strParam = "true";
	}
	const char *argv[] = { "rpctest", "getnewaddress", strParam.c_str() };
	int argc = sizeof(argv) / sizeof(char*);

	Value value;

	if (CommandLineRPC_GetValue(argc, argv, value)) {
		strAddr = "strAddr";
		return GetStrFromObj(value, strAddr);
	}
	return false;
}

bool SysTestBase::GetMemPoolSize(int &size) {
	const char *argv[] = { "rpctest", "getrawmempool" };
	int argc = sizeof(argv) / sizeof(char*);
	Value value;
	if (CommandLineRPC_GetValue(argc, argv, value)) {
		Array arry = value.get_array();
		size = arry.size();
		return true;
	}
	return false;
}

bool SysTestBase::GetBlockHeight(int &nHeight) {
	const char *argv[] = { "rpctest", "getinfo" };
	int argc = sizeof(argv) / sizeof(char*);

	Value value;
	if (CommandLineRPC_GetValue(argc, argv, value)) {
		Object obj = value.get_obj();
		nHeight = find_value(obj, "blocks").get_int();
		LogPrint("test_miners", "GetBlockHeight:%d\r\n", nHeight);
		return true;
	}
	return false;
}

Value SysTestBase::CreateNormalTx(const std::string &strSrcAddr, const std::string &strDesAddr, uint64_t ullMoney) {
	//CommanRpc
	char arrchSrc[64] = { 0 };
	strncpy(arrchSrc, strSrcAddr.c_str(), sizeof(arrchSrc) - 1);

	char arrchDest[64] = { 0 };
	strncpy(arrchDest, strDesAddr.c_str(), sizeof(arrchDest) - 1);

	string strMoney = strprintf("%ld", ullMoney);

	const char *argv[] = { "rpctest", "sendtoaddress", arrchSrc, arrchDest, (char*) strMoney.c_str() };
	int argc = sizeof(argv) / sizeof(char*);

	Value value;
	if (CommandLineRPC_GetValue(argc, argv, value)) {
		//LogPrint("test_miners", "CreateNormalTx:%s\r\n", value.get_str().c_str());
		return value;
	}
	return value;
}

Value SysTestBase::CreateNormalTx(const std::string &strDesAddr, uint64_t ullMoney) {

	char arrchDest[64] = { 0 };
	strncpy(arrchDest, strDesAddr.c_str(), sizeof(arrchDest) - 1);

	string strMoney = strprintf("%ld", ullMoney);

	const char *argv[] = { "rpctest", "sendtoaddress", arrchDest, (char*) strMoney.c_str() };
	int argc = sizeof(argv) / sizeof(char*);

	Value value;
	if (CommandLineRPC_GetValue(argc, argv, value)) {
		//LogPrint("test_miners", "CreateNormalTx:%s\r\n", value.get_str().c_str());
		return value;
	}
	return value;
}

Value SysTestBase::RegistAccountTx(const std::string &strAddr, const int nfee) {
	//CommanRpc
	char arrchAddr[64] = { 0 };
	strncpy(arrchAddr, strAddr.c_str(), sizeof(arrchAddr) - 1);

	m_llCurFee = nfee;
	if (nfee == 0) {
		m_llCurFee = GetRandomFee();
	}
	string strFee = strprintf("%ld", m_llCurFee);

	const char *argv[] = { "rpctest", "registaccounttx", arrchAddr, (char*) strFee.c_str() };
	int argc = sizeof(argv) / sizeof(char*);

	Value value;
	if (CommandLineRPC_GetValue(argc, argv, value)) {
		//	LogPrint("test_miners", "RegisterSecureTx:%s\r\n", value.get_str().c_str());
		return value;
	}
	return value;
}

Value SysTestBase::CreateContractTx(const std::string &strScriptid, const std::string &strAddrs,
		const std::string &strContract, int nHeight, int nFee, uint64_t ullMoney) {

	if (0 == nFee) {
		int nfee = GetRandomFee();
		m_llCurFee = nfee;
	} else {
		m_llCurFee = nFee;
	}

	string strFee = strprintf("%d",m_llCurFee);
	string strHeight = strprintf("%d", nHeight);
	string strMoney = strprintf("%ld", ullMoney);

	const char *argv[] =
			{ "rpctest", "createcontracttx", (char *) (strAddrs.c_str()), (char *) (strScriptid.c_str()),
					(char *) strMoney.c_str(), (char *) (strContract.c_str()), (char*) strFee.c_str(),
					(char*) strHeight.c_str() };
	int argc = sizeof(argv) / sizeof(char*);

	Value value;
	if (CommandLineRPC_GetValue(argc, argv, value)) {
		return value;
	}
	return value;
}

Value SysTestBase::RegisterAppTx(const string& strAddress, const string& strScript, int nHeight, int nFee) {
	return CreateRegAppTx(strAddress, strScript, true, nFee, nHeight);
}

Value SysTestBase::SignSecureTx(const string &strSecuretx) {
	//CommanRpc
	char arrchSecureTx[10 * 1024] = { 0 };
	strncpy(arrchSecureTx, strSecuretx.c_str(), sizeof(arrchSecureTx) - 1);

	const char *argv[] = { "rpctest", "signcontracttx", arrchSecureTx };
	int argc = sizeof(argv) / sizeof(char*);

	Value value;
	if (CommandLineRPC_GetValue(argc, argv, value)) {
		//LogPrint("test_miners", "SignSecureTx:%s\r\n", value.get_str().c_str());
		return value;
	}
	return value;
}

bool SysTestBase::IsAllTxInBlock() {
	const char *argv[] = { "rpctest", "listunconfirmedtx" };
	int argc = sizeof(argv) / sizeof(char*);

	Value value;
	if (CommandLineRPC_GetValue(argc, argv, value)) {
		value = find_value(value.get_obj(), "UnConfirmTx");
		if (0 == value.get_array().size()) {
			return true;
		}
	}
	return false;
}

bool SysTestBase::GetBlockHash(const int nHeight, std::string &strBlockHash) {
	char arrchHeight[16] = { 0 };
	sprintf(arrchHeight, "%d", nHeight);

	const char *argv[] = { "rpctest", "getblockhash", arrchHeight };
	int argc = sizeof(argv) / sizeof(char*);

	Value value;
	if (CommandLineRPC_GetValue(argc, argv, value)) {
		strBlockHash = find_value(value.get_obj(), "hash").get_str();
		LogPrint("test_miners", "GetBlockHash:%s\r\n", strBlockHash.c_str());
		return true;
	}
	return false;
}

bool SysTestBase::GetBlockMinerAddr(const std::string &strBlockHash, std::string &strAddr) {
	char arrchBlockHash[80] = { 0 };
	strncpy(arrchBlockHash, strBlockHash.c_str(), sizeof(arrchBlockHash) - 1);

	const char *argv[] = { "rpctest", "getblock", arrchBlockHash };
	int argc = sizeof(argv) / sizeof(char*);

	Value value;
	if (CommandLineRPC_GetValue(argc, argv, value)) {
		Array txs = find_value(value.get_obj(), "tx").get_array();
		strAddr = find_value(txs[0].get_obj(), "addr").get_str();
		LogPrint("test_miners", "GetBlockMinerAddr:%s\r\n", strAddr.c_str());
		return true;
	}
	return false;
}

boost::thread*SysTestBase::pThreadShutdown = NULL;

bool SysTestBase::GenerateOneBlock() {
	const char *argv[] = { "rpctest", "setgenerate", "true", "1" };
	int argc = sizeof(argv) / sizeof(char*);
	int nHigh = 0;
	GetBlockHeight(nHigh);
	Value value;
	if (CommandLineRPC_GetValue(argc, argv, value)) {
		int nHeight = 0;
		int nConter = 0;
		do {
			MilliSleep(1000);
			GetBlockHeight(nHeight);
			if (nConter++ > 80) {
				break;
			}
		} while (nHigh + 1 > nHeight);
		BOOST_CHECK(nConter < 80);
		return true;
	}
	return false;
}

bool SysTestBase::SetAddrGenerteBlock(const char *pszAddr) {
	const char *argv[] = { "rpctest", "generateblock", pszAddr };
	int argc = sizeof(argv) / sizeof(char*);

	Value value;
	if (CommandLineRPC_GetValue(argc, argv, value)) {
		return true;
	}
	return false;
}

bool SysTestBase::DisConnectBlock(int nNum) {
	int nFirstHeight = 0;
	GetBlockHeight(nFirstHeight);
	if (nFirstHeight <= 0) {
		return false;
	}
	BOOST_CHECK(nNum > 0 && nNum <= nFirstHeight);

	string strNum = strprintf("%d",nNum);
	const char *argv[3] = { "rpctest", "disconnectblock", strNum.c_str() };
	int argc = sizeof(argv) / sizeof(char*);

	Value value;
	if (CommandLineRPC_GetValue(argc, argv, value)) {
		int nHeightAfterDis = 0;
		GetBlockHeight(nHeightAfterDis);
		BOOST_CHECK(nHeightAfterDis == nFirstHeight - nNum);
		return true;
	}
	return false;
}

void SysTestBase::StartServer(int argc, const char* argv[]) {
//		int argc = 2;
//		char* argv[] = {"D:\\cppwork\\Dacrs\\src\\Dacrsd.exe","-datadir=d:\\bitcoin" };
	assert(pThreadShutdown == NULL);
	{
		std::tuple<bool, boost::thread*> ret = RunDacrs(argc, const_cast<char **>(argv));
		pThreadShutdown = std::get<1>(ret);
	}
}

//void StartShutdown()
//{
//    fRequestShutdown = true;
//}
void SysTestBase::StopServer() {
	StartShutdown();
	assert(pThreadShutdown != NULL);
	if (pThreadShutdown) {
		pThreadShutdown->join();
		delete pThreadShutdown;
		pThreadShutdown = NULL;
	}
	Shutdown();
}

bool SysTestBase::GetStrFromObj(const Value& valueRes, string& str) {
	if (valueRes.type() == null_type) {
		return false;
	}

	const Value& result = find_value(valueRes.get_obj(), str);
	if (result.type() == null_type) {
		return false;
	}
	if (result.type() == str_type) {
		str = result.get_str();
	}
	return true;
}

bool SysTestBase::ImportWalletKey(const char**address, int nCount) {
	for (int i = 0; i < nCount; i++) {
		const char *argv2[] = { "rpctest", "importprivkey", address[i] };

		Value value;
		if (!CommandLineRPC_GetValue(sizeof(argv2) / sizeof(argv2[0]), argv2, value)) {
			continue;
		}
	}

	return true;
}

uint64_t SysTestBase::GetRandomBetfee() {
	srand(time(NULL));
	int r = (rand() % 1000000) + 100000000;
	return r;
}

bool SysTestBase::GetKeyId(string const &strAddr, CKeyID &cKeyId) {
	if (!CRegID::GetKeyID(strAddr, cKeyId)) {
		cKeyId = CKeyID(strAddr);
		if (cKeyId.IsEmpty()) {
			return false;
		}
	}
	return true;
}

bool SysTestBase::IsTxInMemorypool(const uint256& cTxHash) {
	for (const auto& entry : g_cTxMemPool.m_mapTx) {
		if (entry.first == cTxHash) {
			return true;
		}
	}
	return false;
}

bool SysTestBase::IsTxUnConfirmdInWallet(const uint256& cTxHash) {
	for (const auto &item : g_pwalletMain->m_mapUnConfirmTx) {
		if (cTxHash == item.first) {
			return true;
		}
	}
	return false;
}

bool SysTestBase::GetRegID(string& strAddr,string& strRegID){
	Value value = GetAccountInfo(strAddr);
	strRegID = "RegID";
	return GetStrFromObj(value,strRegID);
}

bool SysTestBase::IsTxInTipBlock(const uint256& cTxHash) {
	CBlockIndex* pcIndex = g_cChainActive.Tip();
	CBlock cBlock;
	if (!ReadBlockFromDisk(cBlock, pcIndex)) {
		return false;
	}
	cBlock.BuildMerkleTree();
	std::tuple<bool, int> ret = cBlock.GetTxIndex(cTxHash);
	if (!std::get<0>(ret)) {
		return false;
	}

	return true;
}

bool SysTestBase::GetRegID(string& strAddr, CRegID& cRegID) {
	CAccount cAccount;
	CKeyID cKeyid;
	if (!GetKeyId(strAddr, cKeyid)) {
		return false;
	}

	CUserID userId = cKeyid;

	LOCK(g_cs_main);
	CAccountViewCache accView(*g_pAccountViewTip, true);
	if (!accView.GetAccount(userId, cAccount)) {
		return false;
	}
	if ((!cAccount.IsRegister()) || cAccount.m_cRegID.IsEmpty()) {
		return false;
	}

	cRegID = cAccount.m_cRegID;
	return true;
}

bool SysTestBase::GetTxOperateLog(const uint256& cTxHash, vector<CAccountLog>& vcLog) {
	if (!GetTxOperLog(cTxHash, vcLog)) {
		return false;
	}
	return true;
}

bool SysTestBase::PrintLog() {
	const char *argv2[] = { "rpctest", "printblokdbinfo" };

	Value value;
	if (!CommandLineRPC_GetValue(sizeof(argv2) / sizeof(argv2[0]), argv2, value)) {
		return true;
	}
	return false;
}

bool SysTestBase::IsMemoryPoolEmpty() {
	//return g_cTxMemPool.mapTx.empty();
	const char *argv[] = { "rpctest", "getrawmempool" };
	int argc = sizeof(argv) / sizeof(char*);

	Value value;
	if (CommandLineRPC_GetValue(argc, argv, value)) {
		json_spirit::Value::Array narray = value.get_array();
		return narray.size() > 0 ? false : true;
	}
	return true;
}
