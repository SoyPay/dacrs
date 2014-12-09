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

bool AppInit(int argc, char* argv[],boost::thread_group &threadGroup) {
	bool fRet = false;
	try {
		CBaseParams::IntialParams(argc, argv);
		SysCfg().InitalConfig();

		if (SysCfg().IsArgCount("-?") || SysCfg().IsArgCount("--help")) {
			// First part of help message is specific to soypayd / RPC client
			std::string strUsage = _("Bitcoin Core Daemon") + " " + _("version") + " " + FormatFullVersion() + "\n\n"
					+ _("Usage:") + "\n" + "  soypayd [options]                     " + _("Start Bitcoin Core Daemon")
					+ "\n" + _("Usage (deprecated, use soypay-cli):") + "\n"
					+ "  soypayd [options] <command> [params]  " + _("Send command to Bitcoin Core") + "\n"
					+ "  soypayd [options] help                " + _("List commands") + "\n"
					+ "  soypayd [options] help <command>      " + _("Get help for a command") + "\n";

			strUsage += "\n" + HelpMessage(HMM_BITCOIND);
			strUsage += "\n" + HelpMessageCli(false);

			fprintf(stdout, "%s", strUsage.c_str());
			return false;
		}

		// Command-line RPC
		bool fCommandLine = false;
		for (int i = 1; i < argc; i++)
			if (!IsSwitchChar(argv[i][0]) && !boost::algorithm::istarts_with(argv[i], "soypay:"))
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

std::tuple<bool, boost::thread*> RunSoyPay(int argc, char* argv[]) {
	boost::thread* detectShutdownThread = NULL;
	static boost::thread_group threadGroup;
	SetupEnvironment();

	bool fRet = false;

	// Connect soypayd signal handlers
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
			BOOST_CHECK_MESSAGE(0, strScript + "not exist");
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
	char *argv[13] = {						//
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
	char *argv[] = { "rpctest", "getaccountinfo", (char*) strID.c_str() };
	int argc = sizeof(argv) / sizeof(char*);

	Value value;
	if (CommandLineRPC_GetValue(sizeof(argv) / sizeof(argv[0]), argv, value)) {
		return value;
	}
	return value;
}

bool SysTestBase::CommandLineRPC_GetValue(int argc, char *argv[], Value &value) {
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
			int code = find_value(error.get_obj(), "code").get_int();
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
		// fprintf((nRet == 0 ? stdout : stderr), "%s\n", strPrint.c_str());
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

bool SysTestBase::GetOneAddr(std::string &addr, char *pStrMinMoney, char *bpBoolReg) {
	//CommanRpc
	char *argv[] = { "rpctest", "getoneaddr", pStrMinMoney, bpBoolReg };
	int argc = sizeof(argv) / sizeof(char*);

	Value value;
	if (CommandLineRPC_GetValue(sizeof(argv) / sizeof(argv[0]), argv, value)) {
		addr = value.get_str();
		LogPrint("test_miners", "GetOneAddr:%s\r\n", addr.c_str());
		return true;
	}
	return false;
}

bool SysTestBase::GetOneScriptId(std::string &regscriptid) {
	//CommanRpc
	char *argv[] = { "rpctest", "listscriptregid" };
	int argc = sizeof(argv) / sizeof(char*);

	Value value;
	if (CommandLineRPC_GetValue(argc, argv, value)) {
		Object &Oid = value.get_obj();
		regscriptid = Oid[0].value_.get_str();
		LogPrint("test_miners", "GetOneAddr:%s\r\n", regscriptid.c_str());
		return true;
	}
	return false;
}

bool SysTestBase::GetNewAddr(std::string &addr) {
	//CommanRpc
	char *argv[] = { "rpctest", "getnewaddress" };
	int argc = sizeof(argv) / sizeof(char*);

	Value value;
	int ret = CommandLineRPC_GetValue(argc, argv, value);
	if (!ret) {
		addr = value.get_str();
		LogPrint("test_miners", "GetNewAddr:%s\r\n", addr.c_str());
		return true;
	}
	return false;
}

bool SysTestBase::GetAccState(const std::string &addr, AccState &accstate) {
	//CommanRpc
	char temp[64] = { 0 };
	strncpy(temp, addr.c_str(), sizeof(temp) - 1);

	char *argv[] = { "rpctest", "getaddramount", temp };
	int argc = sizeof(argv) / sizeof(char*);
	Value value;
	int ret = CommandLineRPC_GetValue(argc, argv, value);
	if (!ret) {
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
	char *argv[] = { "rpctest", "getinfo", };
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

	char *argv[] = { "rpctest", "createnormaltx", src, dest, money, fee, height };
	int argc = sizeof(argv) / sizeof(char*);

	Value value;
	int ret = CommandLineRPC_GetValue(argc, argv, value);
	if (!ret) {
		LogPrint("test_miners", "CreateNormalTx:%s\r\n", value.get_str().c_str());
		return true;
	}
	return false;
}

bool SysTestBase::CreateFreezeTx(const std::string &addr, const int nHeight) {
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

	char *argv[] = { "rpctest", "createfreezetx", caddr, money, fee, height, freeheight };
	int argc = sizeof(argv) / sizeof(char*);

	Value value;
	if (CommandLineRPC_GetValue(argc, argv, value)) {
		LogPrint("test_miners", "CreateFreezeTx:%s\r\n", value.get_str().c_str());
		return true;
	}
	return false;
}

bool SysTestBase::RegisterAccountTx(const std::string &addr, const int nHeight) {
	//CommanRpc
	char caddr[64] = { 0 };
	strncpy(caddr, addr.c_str(), sizeof(caddr) - 1);

	char fee[64] = { 0 };
	int nfee = GetRandomFee();
	sprintf(fee, "%d", nfee);
	nCurFee = nfee;

	char height[16] = { 0 };
	sprintf(height, "%d", nHeight);

	char *argv[] = { "rpctest", "registeraccounttx", caddr, fee, height };
	int argc = sizeof(argv) / sizeof(char*);

	Value value;
	int ret = CommandLineRPC_GetValue(argc, argv, value);
	if (!ret) {
		LogPrint("test_miners", "RegisterSecureTx:%s\r\n", value.get_str().c_str());
		return true;
	}
	return false;
}

bool SysTestBase::CreateContractTx(const std::string &scriptid, const std::string &addrs, const std::string &contract,
		const int nHeight) {
	char cscriptid[1024] = { 0 };
//		vector<char> te(scriptid.begin(),scriptid.end());
//		&te[0]
//		strncpy(cscriptid, scriptid.c_str(), sizeof(scriptid)-1);

//		char caddr[1024] = { 0 };
//		strncpy(caddr, addrs.c_str(), sizeof(addrs)-1);

//		char ccontract[128*1024] = { 0 };
//		strncpy(ccontract, contract.c_str(), sizeof(contract)-1);

	char fee[64] = { 0 };
	int nfee = GetRandomFee();
	sprintf(fee, "%d", nfee);
	nCurFee = nfee;

	char height[16] = { 0 };
	sprintf(height, "%d", nHeight);

	char *argv[] = { "rpctest", "createcontracttx", (char *) (scriptid.c_str()), (char *) (addrs.c_str()),
			(char *) (contract.c_str()), fee, height };
	int argc = sizeof(argv) / sizeof(char*);

	Value value;
	if (CommandLineRPC_GetValue(argc, argv, value)) {
		LogPrint("test_miners", "createcontracttx:%s\r\n", value.get_str().c_str());
		return true;
	}
	return false;
}

Value SysTestBase::RegisterScriptTx(const string& strAddress, const string& strScript, int nHeight, int nFee) {
	return CreateRegScriptTx(strAddress, strScript, true, nFee, nHeight, CNetAuthorizate());
}

Value SysTestBase::ModifyAuthor(const string& strAddress, const string& strScript, int nHeight, int nFee,
		const CNetAuthorizate& author) {
	return CreateRegScriptTx(strAddress, strScript, false, nFee, nHeight, author);
}

bool SysTestBase::CreateSecureTx(const string &scriptid, const vector<string> &obaddrs, const vector<string> &addrs,
		const string&contract, const int nHeight) {
	//CommanRpc
	char cscriptid[64] = { 0 };
	strncpy(cscriptid, scriptid.c_str(), sizeof(cscriptid) - 1);

	char cobstr[512] = { 0 };
	{
		Array array;
		array.clear();
		for (const auto &str : obaddrs) {
			array.push_back(str);
		}
		string arraystr = write_string(Value(array), false);
		strncpy(cobstr, arraystr.c_str(), sizeof(cobstr) - 1);
	}
	char addrstr[512] = { 0 };
	{
		Array array;
		array.clear();
		for (const auto &str : addrs) {
			array.push_back(str);
		}
		string arraystr = write_string(Value(array), false);
		strncpy(addrstr, arraystr.c_str(), sizeof(addrstr) - 1);
	}

	char ccontract[10 * 1024] = { 0 };
	strncpy(ccontract, contract.c_str(), sizeof(ccontract) - 1);

	char height[16] = { 0 };
	sprintf(height, "%d", nHeight);

	char *argv[] = { "rpctest", "createsecuretx", cscriptid, cobstr, addrstr, ccontract, "1000000", height };
	int argc = sizeof(argv) / sizeof(char*);

	Value value;
	int ret = CommandLineRPC_GetValue(argc, argv, value);
	if (!ret) {
		LogPrint("test_miners", "CreateSecureTx:%s\r\n", value.get_str().c_str());
		return true;
	}
	return false;
}

bool SysTestBase::SignSecureTx(const string &securetx) {
	//CommanRpc
	char csecuretx[10 * 1024] = { 0 };
	strncpy(csecuretx, securetx.c_str(), sizeof(csecuretx) - 1);

	char *argv[] = { "rpctest", "signsecuretx", csecuretx };
	int argc = sizeof(argv) / sizeof(char*);

	Value value;
	if (CommandLineRPC_GetValue(argc, argv, value)) {
		LogPrint("test_miners", "SignSecureTx:%s\r\n", value.get_str().c_str());
		return true;
	}
	return false;
}

bool SysTestBase::IsAllTxInBlock() {
	char *argv[] = { "rpctest", "listunconfirmedtx" };
	int argc = sizeof(argv) / sizeof(char*);

	Value value;
	int ret = CommandLineRPC_GetValue(argc, argv, value);
	if (!ret) {
		Array array = value.get_array();
		if (array.size() == 0)
			return true;
	}
	return false;
}

bool SysTestBase::GetBlockHash(const int nHeight, std::string &blockhash) {
	char height[16] = { 0 };
	sprintf(height, "%d", nHeight);

	char *argv[] = { "rpctest", "getblockhash", height };
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

	char *argv[] = { "rpctest", "getblock", cblockhash };
	int argc = sizeof(argv) / sizeof(char*);

	Value value;
	int ret = CommandLineRPC_GetValue(argc, argv, value);
	if (!ret) {
		Array txs = find_value(value.get_obj(), "tx").get_array();
		addr = find_value(txs[0].get_obj(), "addr").get_str();
		LogPrint("test_miners", "GetBlockMinerAddr:%s\r\n", addr.c_str());
		return true;
	}
	return false;
}
boost::thread*SysTestBase::pThreadShutdown = NULL;
bool SysTestBase::GenerateOneBlock() {
	char *argv[] = { "rpctest", "setgenerate", "true" };
	int argc = sizeof(argv) / sizeof(char*);

	Value value;
	if (CommandLineRPC_GetValue(argc, argv, value)) {
		return true;
	}
	return false;
}


void SysTestBase::StartServer(int argc,char* argv[]) {
//		int argc = 2;
//		char* argv[] = {"D:\\cppwork\\soypay\\src\\soypayd.exe","-datadir=d:\\bitcoin" };
	assert(pThreadShutdown == NULL);
	{
	std::tuple<bool, boost::thread*> ret = RunSoyPay(argc, argv);
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
