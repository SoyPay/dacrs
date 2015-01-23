/*
 * CRPCRequest2.cpp
 *
 *  Created on: Dec 9, 2014
 *      Author: ranger.shi
 */

#include "SysTestBase.h"



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

bool PrintTestNotSetPara()
{
	bool flag = false;
	if(1 == SysCfg().GetArg("-listen",flag))
	{
		if (SysCfg().GetDefaultPort() == SysCfg().GetArg("-port", SysCfg().GetDefaultPort())) {
			cout << "Waring if config file seted the listen param must be true, and port can't be default port" << endl;
			MilliSleep(500);
			exit(0);
		}
	}
	string str("");
	string strconnect = SysCfg().GetArg("-connect",str);
	if (str != strconnect)
	{
		cout<<"Waring the test of the config file the connect param must be false"<<endl;
		MilliSleep(500);
		exit(0);

	}
	if(SysCfg().GetArg("-iscutmine",flag))
	{
		cout<<"Waring the test of config file the iscutmine param must be false"<<endl;
		MilliSleep(500);
		exit(0);
	}
	if(!SysCfg().GetArg("-isdbtraversal",flag))
	{
		cout<<"Waring the test of config file the isdbtraversal param must be true"<<endl;
		MilliSleep(500);
		exit(0);
	}

	return true;
}
bool AppInit(int argc, char* argv[],boost::thread_group &threadGroup) {
	bool fRet = false;
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

		SysCfg().SoftSetBoolArg("-server", true);

		fRet = AppInit2(threadGroup);
	} catch (std::exception& e) {
		PrintExceptionContinue(&e, "AppInit()");
	} catch (...) {
		PrintExceptionContinue(NULL, "AppInit()");
	}

	return fRet;
}

std::tuple<bool, boost::thread*> RunDacrs(int argc, char* argv[]) {
	boost::thread* detectShutdownThread = NULL;
	static boost::thread_group threadGroup;
	SetupEnvironment();

	bool fRet = false;

	// Connect Dacrsd signal handlers
	noui_connect();

	fRet = AppInit(argc, argv, threadGroup);

	detectShutdownThread = new boost::thread(boost::bind(&DetectShutdownThread, &threadGroup));

	if (!fRet) {
		if (detectShutdownThread)
			detectShutdownThread->interrupt();

		threadGroup.interrupt_all();
	}
	return std::make_tuple(fRet, detectShutdownThread);
}



SysTestBase::SysTestBase() {
	// todo Auto-generated constructor stub
}

SysTestBase::~SysTestBase() {
	// todo Auto-generated destructor stub
}

bool SysTestBase::ResetEnv() {
	const char *argv[] = { "rpctest", "resetclient"};

	Value value;
	if (!CommandLineRPC_GetValue(sizeof(argv) / sizeof(argv[0]), argv, value)) {
		return false;
	}

	const char* pKey[] = {
			        /*for bess test*/
					"cUa4v77hiXteMFkHoyuPVVbCCULS1CnFBhU1MhgKHEGRTHmd4BC5",// addr:  mo51PMpnadiFx5JcZaeUdWBa4ngLBVgoGz
					"cTAqnCwjuLwXqHxGe5c6KrGqQw5yjHH6Na6yYRQCgKKnf6cJBPxF",// addr:  mfzdtseoKfMpTd8V9N2xETEqUSWRujndgZ
					"cVFWoy8jmJVVSNnMs3YRizkR7XEekMTta4MzvuRshKuQEEJ4kbNg",// addr:  mjSwCwMsvtKczMfta1tvr78z2FTsZA1JKw
					"cNcJkU44oG3etbWoEvY46i5qWPeE8jVb7K44keXxEQxsXUZ85MKU",//"address" : "mnnS4upLeY7RZpNnvoGMZzs9ELscQjtvqy",
					"cStrXy6NowsDyaLRJMhQCJu4WnP6WR6SMC1c3dmxDeeLKFcYHDsQ",//"address" : "mscmxFbxRfBUiH4drYNA2nCoFXuausKjd7",


					/*for yang test*/
					"cSu84vACzZkWqnP2LUdJQLX3M1PYYXo2gEDDCEKLWNWfM7B4zLiP",// addr:  mw5wbV73gXbreYy8pX4FSb7DNYVKU3LENc
					"cSVY69D9aUo4MugzUG9rM14DtV21cBAbZUVXmgAC2RpJwtZRUbsM",// addr:  mhVJJSAdPNDPvFWCmQN446GUBPzFm8aN4y
					"cTCcDyQvX6ucP9NEjhyHfTixamKQHQkFiSyfupm4CGZZYV7YYnf8",// addr:  moZJZgsGFC4qvwRdjzS7Bj3fHrtpUfEVEE

					/*for franklin test*/
					"cUwPkEYdg3d3CmNctg2aegdyeq7dbLta1HAVHcGQTp33kWqzMSuT ",//addr:  mv2eqSvyUA4JeJXBQpKvJEbYY89FqoRbX5
					/*for spark test*/
					"cPqVgscsWpPgkLHZP3pKJVSU5ZTCCvVhkd5cmXVWVydXdMTtBGj7",// addr:  mrjpqG4WsyjrCh8ssVs9Rp6JDini8suA7v
					"cU1dxQgvyKt8yEqqkKiNLK9jfyW498RKi8y2evqzjtLXrLD4fBMs",// addr:  mfu6nTXP9LR9mRSPmnVwXUSDVQiRCBDJi7
					"cRYYMN1EFd9X4sGqEkUkWLi38GCFyAccKQEuF1WiYFwUWsqBGwHe",// addr:  n4muwAThwzWvuLUh74nL3KYwujhihke1Kb
					/*for ranger test*/
					"cR5wPiv3Vp4sQmww2gWzShkDUaamYrJ6QHHtDd1Pm4nVJFTxnksC",// addr:  mvVp2PDRuG4JJh6UjkJFzXUC8K5JVbMFFA
					"cT1BuRbx5Cvmvic2dX2aq3ep2fu75CDwYk8fCQPtrftKiBEQiPJm",// addr:  msdDQ1SXNmknrLuTDivmJiavu5J9VyX9fV
					/*for server 169 */
					"cQXpVRxwXqeh8FjSxkGE7sYrzXLXPdoHeUQCdJk9uLy17F3WKbPM",//"address" : "mx8yEhL6DCo7WFkpn4gTW2dqoWB7oXpvfV",
					"cVNeGiYHhtaVSvmCswUs8jootYPFJisVwx6gqBbkeWSftkXeaHbC",//"address" : "mfe3zNfFTkeD9JSzmgidihv1tb8AxLVtRT",
					"cNjb55M6fqNVuhKmNE95C8weWYr6iD2yW6QqifYWSvuVGKUJRTt9",//"address" : "mfXrwgAt5LezYerS9NRCiPUxtHNChrxyiT",

					/*remain for test*/
				//	"cNcJkU44oG3etbWoEvY46i5qWPeE8jVb7K44keXxEQxsXUZ85MKU",//"address" : "mnnS4upLeY7RZpNnvoGMZzs9ELscQjtvqy",
				//	"cStrXy6NowsDyaLRJMhQCJu4WnP6WR6SMC1c3dmxDeeLKFcYHDsQ",//"address" : "mscmxFbxRfBUiH4drYNA2nCoFXuausKjd7",
					"cN2xNMvvNCtqh1K87J9o35cHHQttdZi1MYgUj8FYZPdtaFTtxbtd",//"address" : "mg7xvMhw9Jt7Rys8nWhjXujbGNaeFn3x4L",
					"cU8kr9JvCXotPoBQZ4TPxkD2S98ZFz2AKLDupMt8hgNG4JLQ1b2x",//"address" : "mx5j59nsCVBVNs8zqeAE6rM1JEELRnY3KX",
					"cSvRSiQGS6d11CbY4Mac1sCN84YHyNsFvNZ5xgBM1FHUSi7fgcaA",//"address" : "mxu7hQuPeV1Lom3NaHfLh8oe2J6z3dKB22",
					"cMv5HP4EPsX4Fmvj2Zgtpq5MfAbVRxwunqNtK2qjVrMdfvv36Zr3",//"address" : "mron8pog28KpwFjsEgmQ2jEg3UiRkEwqyV",

	};

	int nCount = sizeof(pKey) / sizeof(char*);
	for (int i = 0; i < nCount; i++) {
		const char *argv2[] = { "rpctest", "importprivkey", pKey[i]};

		Value value;
		if (!CommandLineRPC_GetValue(sizeof(argv2) / sizeof(argv2[0]), argv2, value)) {
			return false;
		}
	}

	return true;
}

int SysTestBase::GetRandomFee() {
	srand(time(NULL));
	int r = (rand() % 1000000) + 1000000;
	return r;
}



int SysTestBase::GetRandomMoney() {
	srand(time(NULL));
	int r = (rand() % 1000) + 1000;
	return r;
}

Value SysTestBase::CreateRegScriptTx(const string& strAddress, const string& strScript, bool bRigsterScript, int nFee,
		int nHeight, const CNetAuthorizate& author) {
	string strScriptData;
	char szType[1] = { 0 };
	if (bRigsterScript) {
		szType[0] = '0';
		strScriptData = SysCfg().GetDefaultTestDataPath() + strScript;
		if (!boost::filesystem::exists(strScriptData)) {
			BOOST_CHECK_MESSAGE(0, strScriptData + " not exist");
			return false;
		}
	} else {
		strScriptData = strScript;
		szType[0] = '1';
	}

	string strFee = strprintf("%d",nFee);
	string strHeight = strprintf("%d",nHeight);
	string strAuTime = strprintf("%d",author.GetAuthorizeTime());
	string strMoneyPerTime = strprintf("%d",author.GetMaxMoneyPerTime());
	string strMoneyTotal = strprintf("%d",author.GetMaxMoneyTotal());
	string strMoneyPerDay = strprintf("%d",author.GetMaxMoneyPerDay());

	string strUserData = HexStr(author.GetUserData());
	const char *argv[13] = {						//
			"rpctest",						//
					"registerscripttx",				//
					(char*) strAddress.c_str(), 	//
					szType, 						//
					(char*) strScriptData.c_str(),	//
					(char*) strFee.c_str(), 		//
					(char*) strHeight.c_str(), 		//
					"this is description", 			//
					(char*) strAuTime.c_str(),		//
					(char*) strMoneyPerTime.c_str(),		//
					(char*) strMoneyTotal.c_str(), 	//
					(char*) strMoneyPerDay.c_str(),	//
					(char*) strUserData.c_str() };	//

	Value value;

	if (CommandLineRPC_GetValue(sizeof(argv) / sizeof(argv[0]), argv, value)) {
		LogPrint("test_miners", "RegisterSecureTx:%s\r\n", write_string(value, true));
		return value;
	}
	return value;
}

Value SysTestBase::GetAccountInfo(const string& strID) {
	const char *argv[] = { "rpctest", "getaccountinfo", strID.c_str()};

	Value value;
	if (CommandLineRPC_GetValue(sizeof(argv) / sizeof(argv[0]), argv, value)) {
		return value;
	}
	return value;
}

bool SysTestBase::CommandLineRPC_GetValue(int argc, const char *argv[], Value &value) {
	string strPrint;
	bool nRes = false;
	try {
		// Skip switches
		while (argc > 1 && IsSwitchChar(argv[1][0])) {
			argc--;
			argv++;
		}

		// Method
		if (argc < 2)
			throw runtime_error("too few parameters");
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
			nRes = true;
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
		if (false == nRes) {
//			cout<<strPrint<<endl;
		}
//	    fprintf((nRes == 0 ? stdout : stderr), "%s\n", strPrint.c_str());
	}

	return nRes;
}

bool SysTestBase::IsScriptAccCreated(const string& strScript) {
	Value valueRes = GetAccountInfo(strScript);
	if (valueRes.type() == null_type)
		return false;

	Value result = find_value(valueRes.get_obj(), "KeyID");
	if (result.type() == null_type)
		return false;

	return true;
}

uint64_t SysTestBase::GetFreeMoney(const string& strID) {
	Value valueRes = GetAccountInfo(strID);
	BOOST_CHECK(valueRes.type() != null_type);
	Value result = find_value(valueRes.get_obj(), "FreeValues");
	BOOST_CHECK(result.type() != null_type);

	uint64_t nMoney = result.get_int64();

	result = find_value(valueRes.get_obj(), "FreedomFund");
	Array arrayFreedom = result.get_array();

	for (const auto& item : arrayFreedom) {
		result = find_value(item.get_obj(), "value");
		BOOST_CHECK(result.type() != null_type);
		nMoney += result.get_int64();
	}

	return nMoney;
}

bool SysTestBase::GetOneScriptId(std::string &regscriptid) {
	//CommanRpc
	const char *argv[] = { "rpctest", "listregscript","false" };
	int argc = sizeof(argv) / sizeof(char*);

	Value value;
	if (CommandLineRPC_GetValue(argc, argv, value)) {
		Object &Oid = value.get_obj();
		string strPrint = write_string((Value)Oid, true);
		LogPrint("test_miners", "GetOneAddr1:%s\r\n", strPrint);
		Array &Oid1 = Oid[0].value_.get_array();
		strPrint = write_string((Value)Oid1, true);
		LogPrint("test_miners", "GetOneAddr2:%s\r\n", strPrint);
		const Value& result1 = find_value(Oid1[0].get_obj(), "scriptId");
		regscriptid = result1.get_str();
		LogPrint("test_miners", "GetOneAddr:%s\r\n", regscriptid.c_str());
		return true;
	}
	return false;
}

bool SysTestBase::GetNewAddr(std::string &addr,bool flag) {
	//CommanRpc
	string param = "false";
	if(flag)
	{
		param = "true";
	}
	const char *argv[] = { "rpctest", "getnewaddress",param.c_str()};
	int argc = sizeof(argv) / sizeof(char*);

	Value value;

	if (CommandLineRPC_GetValue(argc, argv, value)) {
		addr = "addr";
		return GetStrFromObj(value,addr);
	}
	return false;
}

bool SysTestBase::GetAccState(const std::string &addr, AccState &accstate) {
	//CommanRpc
	char temp[64] = { 0 };
	strncpy(temp, addr.c_str(), sizeof(temp) - 1);

	const char *argv[] = { "rpctest", "getaddramount", temp };
	int argc = sizeof(argv) / sizeof(char*);
	Value value;

	if (CommandLineRPC_GetValue(argc, argv, value)) {
		Object obj = value.get_obj();
		double dfree = find_value(obj, "free amount").get_real();
		double dmature = find_value(obj, "mature amount").get_real();
		double dfrozen = find_value(obj, "frozen amount").get_real();
		accstate.dFreeMoney = roundint64(dfree * COIN);
		accstate.dUnmatureMoney = roundint64(dmature * COIN);
		accstate.dFrozenMoney = roundint64(dfrozen * COIN);
		LogPrint("test_miners", "addr:%s GetAccState FreeMoney:%0.8lf matureMoney:%0.8lf FrozenMoney:%0.8lf\r\n",
				addr.c_str(), dfree, dmature, dfrozen);
		return true;
	}
	return false;
}

bool SysTestBase::GetBlockHeight(int &nHeight) {
	const char *argv[] = { "rpctest", "getinfo"};
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

bool SysTestBase::CreateNormalTx(const std::string &srcAddr, const std::string &desAddr, const int nHeight) {
	//CommanRpc
	char src[64] = { 0 };
	strncpy(src, srcAddr.c_str(), sizeof(src) - 1);

	char dest[64] = { 0 };
	strncpy(dest, desAddr.c_str(), sizeof(dest) - 1);

	char money[64] = { 0 };
	int nmoney = GetRandomMoney();
	sprintf(money, "%d00000000", nmoney);
	nCurMoney = nmoney * COIN;

	char fee[64] = { 0 };
	int nfee = GetRandomFee();
	sprintf(fee, "%d", nfee);
	nCurFee = nfee;

	char height[16] = { 0 };
	sprintf(height, "%d", nHeight);

	const char *argv[] = { "rpctest", "createnormaltx", src, dest, money, fee, height };
	int argc = sizeof(argv) / sizeof(char*);

	Value value;
	if (CommandLineRPC_GetValue(argc, argv, value)) {
		LogPrint("test_miners", "CreateNormalTx:%s\r\n", value.get_str().c_str());
		return true;
	}
	return false;
}

Value SysTestBase::CreateFreezeTx(const std::string &addr, const int nHeight) {
	//CommanRpc
	char caddr[64] = { 0 };
	strncpy(caddr, addr.c_str(), sizeof(caddr) - 1);

	char money[64] = { 0 };
	int nmoney = GetRandomMoney();
	sprintf(money, "%d00000000", nmoney);
	nCurMoney = nmoney * COIN;

	char fee[64] = { 0 };
	int nfee = GetRandomFee();
	sprintf(fee, "%d", nfee);
	nCurFee = nfee;

	char height[16] = { 0 };
	sprintf(height, "%d", nHeight);

	char freeheight[16] = { 0 };
	sprintf(freeheight, "%d", nHeight + 100);

	const char *argv[] = { "rpctest", "createfreezetx", caddr, money, fee, height, freeheight };
	int argc = sizeof(argv) / sizeof(char*);

	Value value;
	if (CommandLineRPC_GetValue(argc, argv, value)) {
	//	LogPrint("test_miners", "CreateFreezeTx:%s\r\n", value.get_str().c_str());
		return value;
	}
	return value;
}

Value SysTestBase::registaccounttx(const std::string &addr, const int nHeight) {
	//CommanRpc
	char caddr[64] = { 0 };
	strncpy(caddr, addr.c_str(), sizeof(caddr) - 1);

	char fee[64] = { 0 };
	int nfee = GetRandomFee();
	sprintf(fee, "%d", nfee);
	nCurFee = nfee;

	char height[16] = { 0 };
	sprintf(height, "%d", nHeight);

	const char *argv[] = { "rpctest", "registaccounttx", caddr, fee, height };
	int argc = sizeof(argv) / sizeof(char*);

	Value value;

	if (CommandLineRPC_GetValue(argc, argv, value)) {
	//	LogPrint("test_miners", "RegisterSecureTx:%s\r\n", value.get_str().c_str());
		return value;
	}
	return value;
}
Value SysTestBase::CreateContractTx1(const std::string &scriptid, const std::string &addrs, const std::string &contract,
			const int nHeight)
{
//	char cscriptid[1024] = { 0 };

	char fee[64] = { 0 };
	int nfee =1000000;
	sprintf(fee, "%d", nfee);
	nCurFee = nfee;

	char height[16] = { 0 };
	sprintf(height, "%d", nHeight);

	const char *argv[] = { "rpctest", "createcontracttx", scriptid.c_str(), addrs.c_str(), contract.c_str(), fee, height };
	int argc = sizeof(argv) / sizeof(char*);

	Value value;
	if (CommandLineRPC_GetValue(argc, argv, value)) {
//		const Value& result = find_value(value.get_obj(), "hash");
		return value;
	}
	return value;
}
bool SysTestBase::CreateContractTx(const std::string &scriptid, const std::string &addrs, const std::string &contract,
		int nHeight,int nFee) {
//	char cscriptid[1024] = { 0 };

	string strFee;
	if (0 == nFee) {
		int nfee = GetRandomFee();
		nCurFee = nfee;
	} else {
		nCurFee = nFee;
	}

	strFee = strprintf("%d",nCurFee);

	char height[16] = { 0 };
	sprintf(height, "%d", nHeight);

	vector<unsigned char> vTemp;
	vTemp.assign(contract.begin(),contract.end());
	string strContractData = HexStr(vTemp);

	const char *argv[] = { "rpctest", "createcontracttx", (char *) (scriptid.c_str()), (char *) (addrs.c_str()),
			(char *) (strContractData.c_str()), (char*)strFee.c_str(), height };
	int argc = sizeof(argv) / sizeof(char*);

	Value value;
	if (CommandLineRPC_GetValue(argc, argv, value)) {
		return true;
	}
	return false;
}

Value SysTestBase::CreateContractTxEx(const std::string &scriptid, const std::string &addrs, const std::string &contract,
		int nHeight,int nFee) {
//	char cscriptid[1024] = { 0 };

	string strFee;
	if (0 == nFee) {
		int nfee = GetRandomFee();
		nCurFee = nfee;
	} else {
		nCurFee = nFee;
	}

	strFee = strprintf("%d",nCurFee);

	char height[16] = { 0 };
	sprintf(height, "%d", nHeight);

	vector<unsigned char> vTemp;
	vTemp.assign(contract.begin(),contract.end());
	string strContractData = HexStr(vTemp);

	const char *argv[] = { "rpctest", "createcontracttx", (char *) (scriptid.c_str()), (char *) (addrs.c_str()),
			(char *) (strContractData.c_str()), (char*)strFee.c_str(), height };
	int argc = sizeof(argv) / sizeof(char*);

	Value value;
	if (CommandLineRPC_GetValue(argc, argv, value)) {
		return value;
	}
	return value;
}

//注意，此函数为彩票测试专用，其他滥用后果自负
Value SysTestBase::PCreateContractTx(const std::string &scriptid, const std::string &addrs, const std::string &contract,
		int nHeight,int nFee) {
//	char cscriptid[1024] = { 0 };

	string strFee;
	if (0 == nFee) {
		int nfee = GetRandomFee();
		nCurFee = nfee;
	} else {
		nCurFee = nFee;
	}

	strFee = strprintf("%d",nCurFee);

	char height[16] = { 0 };
	sprintf(height, "%d", nHeight);

	const char *argv[] = { "rpctest", "createcontracttx", (char *) (scriptid.c_str()), (char *) (addrs.c_str()),
			(char *) (contract.c_str()), (char*)strFee.c_str(), height };
	int argc = sizeof(argv) / sizeof(char*);

	Value value;
	if (CommandLineRPC_GetValue(argc, argv, value)) {
		return value;
	}
	return value;
}

Value SysTestBase::RegisterScriptTx(const string& strAddress, const string& strScript, int nHeight, int nFee) {
	return CreateRegScriptTx(strAddress, strScript, true, nFee, nHeight, CNetAuthorizate());
}

Value SysTestBase::ModifyAuthor(const string& strAddress, const string& strScript, int nHeight, int nFee,
		const CNetAuthorizate& author) {
	return CreateRegScriptTx(strAddress, strScript, false, nFee, nHeight, author);
}

Value SysTestBase::SignSecureTx(const string &securetx) {
	//CommanRpc
	char csecuretx[10 * 1024] = { 0 };
	strncpy(csecuretx, securetx.c_str(), sizeof(csecuretx) - 1);

	const char *argv[] = { "rpctest", "signcontracttx", csecuretx };
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
	if (CommandLineRPC_GetValue(argc, argv, value) ) {
		value = find_value(value.get_obj(), "UnConfirmTx");
		if (0 == value.get_array().size())
			return true;
	}
	return false;
}

bool SysTestBase::GetBlockHash(const int nHeight, std::string &blockhash) {
	char height[16] = { 0 };
	sprintf(height, "%d", nHeight);

	const char *argv[] = { "rpctest", "getblockhash", height };
	int argc = sizeof(argv) / sizeof(char*);

	Value value;
	if (CommandLineRPC_GetValue(argc, argv, value)) {
		blockhash = value.get_str();
		LogPrint("test_miners", "GetBlockHash:%s\r\n", blockhash.c_str());
		return true;
	}
	return false;
}

bool SysTestBase::GetBlockMinerAddr(const std::string &blockhash, std::string &addr) {
	char cblockhash[80] = { 0 };
	strncpy(cblockhash, blockhash.c_str(), sizeof(cblockhash) - 1);

	const char *argv[] = { "rpctest", "getblock", cblockhash };
	int argc = sizeof(argv) / sizeof(char*);

	Value value;
	if (CommandLineRPC_GetValue(argc, argv, value) ) {
		Array txs = find_value(value.get_obj(), "tx").get_array();
		addr = find_value(txs[0].get_obj(), "addr").get_str();
		LogPrint("test_miners", "GetBlockMinerAddr:%s\r\n", addr.c_str());
		return true;
	}
	return false;
}
boost::thread*SysTestBase::pThreadShutdown = NULL;
bool SysTestBase::GenerateOneBlock() {
	const char *argv[] = { "rpctest", "setgenerate", "true" ,"1"};
	int argc = sizeof(argv) / sizeof(char*);
    int high= chainActive.Height();
	Value value;
	if (CommandLineRPC_GetValue(argc, argv, value)) {
		BOOST_CHECK(high+1==chainActive.Height());
		return true;
	}
	return false;
}

bool SysTestBase::DisConnectBlock(int nNum) {
	int nFirstHeight = static_cast<int>(chainActive.Height() );
	BOOST_CHECK(nNum>0 && nNum<=nFirstHeight);

	string strNum = strprintf("%d",nNum);
    const char *argv[3] = { "rpctest", "disconnectblock", strNum.c_str()};
	int argc = sizeof(argv) / sizeof(char*);

	Value value;
	if (CommandLineRPC_GetValue(argc, argv, value)) {
		int nHeightAfterDis = static_cast<int>(chainActive.Height() );
		BOOST_CHECK(nHeightAfterDis == nFirstHeight-nNum);
		return true;
	}
	return false;
}
Value SysTestBase::GetScriptID(string txhash)
{
	const char *argv[3] = { "rpctest", "getscriptid", txhash.c_str()};
	int argc = sizeof(argv) / sizeof(char*);
	Value value;
	if (CommandLineRPC_GetValue(argc, argv, value)) {
		return value;
	}
	return value;
}
void SysTestBase::StartServer(int argc,const char* argv[]) {
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
bool SysTestBase::GetStrFromObj(const Value& valueRes,string& str)
{
	if (valueRes.type() == null_type) {
				return false;
			}

			const Value& result = find_value(valueRes.get_obj(), str);
			if (result.type() == null_type){
				return false;
			}
			if (result.type() != null_type){
				str = result.get_str();
				}
			return true;
}
bool SysTestBase::ImportWalletKey(const char**address, int nCount){
	for (int i = 0; i < nCount; i++) {
		const char *argv2[] = { "rpctest", "importprivkey", address[i]};

		Value value;
		if (!CommandLineRPC_GetValue(sizeof(argv2) / sizeof(argv2[0]), argv2, value)) {
			continue;
		}
	}

	return true;
}
