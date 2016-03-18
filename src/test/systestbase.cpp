/*
 * CRPCRequest2.cpp
 *
 *  Created on: Dec 9, 2014
 *      Author: ranger.shi
 */

#include "systestbase.h"



void DetectShutdownThread(boost::thread_group* threadGroup) {
	bool fShutdown = ShutdownRequested();
	// Tell the main threads to shutdown.
	while (!fShutdown) {
		MilliSleep(200);
		fShutdown = ShutdownRequested();
	}

	if (threadGroup) {
		threadGroup->interrupt_all();
		threadGroup->join_all();
	}
	CUIServer::StopServer();
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

	if(!SysCfg().GetArg("-regtest",flag))
	{
		cout<<"Waring the test of config file the regtest param must be 1"<<endl;
		MilliSleep(500);
		exit(0);
	}


    if (filesystem::exists( GetDataDir() / "blocks"))
    {
		cout<<"Waring the test of must del " <<(GetDataDir() / "blocks").string() <<endl;
		MilliSleep(500);
		exit(0);
    }

//			boost::filesystem::remove_all(p);


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

bool SysTestBase::ImportAllPrivateKey()
{

	const char* pKey[] = {
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

bool SysTestBase::ResetEnv() {
	const char *argv[] = { "rpctest", "resetclient"};

	Value value;
	if (!CommandLineRPC_GetValue(sizeof(argv) / sizeof(argv[0]), argv, value)) {
		return false;
	}

    if(ImportAllPrivateKey() == false)
    {
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

	string filepath = SysCfg().GetDefaultTestDataPath() + strScript;
		if (!boost::filesystem::exists(filepath)) {
			BOOST_CHECK_MESSAGE(0, filepath + " not exist");
			return false;
		}

	string strFee = strprintf("%d",nFee);
	string strHeight = strprintf("%d",nHeight);

	const char *argv[] = { "rpctest", "registerapptx", (char*) strAddress.c_str(), (char*) filepath.c_str(), (char*) strFee.c_str(),(char*) strHeight.c_str(),"this is description"};
	int argc = sizeof(argv) / sizeof(char*);

	Value value;

	if (CommandLineRPC_GetValue(argc, argv, value)) {
		LogPrint("test_miners", "RegScriptTx:%s\r\n", write_string(value, true));
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

Value SysTestBase::GetAppAccountInfo(const string& scriptId,const string& strAddr)
{
	const char *argv[] = { "rpctest", "getappaccinfo", (char*) scriptId.c_str(), (char*) strAddr.c_str()};
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

uint64_t SysTestBase::GetBalance(const string& strID) {
	Value valueRes = GetAccountInfo(strID);
	BOOST_CHECK(valueRes.type() != null_type);
	Value result = find_value(valueRes.get_obj(), "Balance");
	BOOST_CHECK(result.type() != null_type);

	uint64_t nMoney = result.get_int64();
	return nMoney;
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

bool SysTestBase::GetMemPoolSize(int &size) {
	const char *argv[] = { "rpctest", "getrawmempool"};
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

Value SysTestBase::CreateNormalTx(const std::string &srcAddr, const std::string &desAddr,uint64_t nMoney) {
	//CommanRpc
	char src[64] = { 0 };
	strncpy(src, srcAddr.c_str(), sizeof(src) - 1);

	char dest[64] = { 0 };
	strncpy(dest, desAddr.c_str(), sizeof(dest) - 1);

	string money =strprintf("%ld", nMoney);


	const char *argv[] = { "rpctest", "sendtoaddress", src, dest, (char*)money.c_str()};
	int argc = sizeof(argv) / sizeof(char*);

	Value value;
	if (CommandLineRPC_GetValue(argc, argv, value)) {
		//LogPrint("test_miners", "CreateNormalTx:%s\r\n", value.get_str().c_str());
		return value;
	}
	return value;
}

Value SysTestBase::CreateNormalTx(const std::string &desAddr,uint64_t nMoney){

	char dest[64] = { 0 };
	strncpy(dest, desAddr.c_str(), sizeof(dest) - 1);

	string money =strprintf("%ld", nMoney);


	const char *argv[] = { "rpctest", "sendtoaddress",dest, (char*)money.c_str()};
	int argc = sizeof(argv) / sizeof(char*);

	Value value;
	if (CommandLineRPC_GetValue(argc, argv, value)) {
		//LogPrint("test_miners", "CreateNormalTx:%s\r\n", value.get_str().c_str());
		return value;
	}
	return value;
}

Value SysTestBase::RegistAccountTx(const std::string &addr, const int nfee) {
	//CommanRpc
	char caddr[64] = { 0 };
	strncpy(caddr, addr.c_str(), sizeof(caddr) - 1);

	nCurFee = nfee;
	if(nfee == 0)
		nCurFee = GetRandomFee();;
	string fee =strprintf("%ld", nCurFee);

	const char *argv[] = { "rpctest", "registaccounttx", caddr, (char*)fee.c_str()};
	int argc = sizeof(argv) / sizeof(char*);

	Value value;
	if (CommandLineRPC_GetValue(argc, argv, value)) {
	//	LogPrint("test_miners", "RegisterSecureTx:%s\r\n", value.get_str().c_str());
		return value;
	}
	return value;
}

Value SysTestBase::CreateContractTx(const std::string &scriptid, const std::string &addrs, const std::string &contract,
		int nHeight,int nFee,uint64_t nMoney ) {

	if (0 == nFee) {
		int nfee = GetRandomFee();
		nCurFee = nfee;
	} else {
		nCurFee = nFee;
	}

	string strFee = strprintf("%d",nCurFee);

	string height =strprintf("%d", nHeight);

	string pmoney = strprintf("%ld", nMoney);

	const char *argv[] = { "rpctest", "createcontracttx",(char *) (addrs.c_str()), (char *) (scriptid.c_str()), (char *)pmoney.c_str(),
			(char *) (contract.c_str()), (char*)strFee.c_str(), (char*)height.c_str() };
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
		blockhash = find_value(value.get_obj(), "hash").get_str();
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
    int high= 0;
    GetBlockHeight(high);
	Value value;
	if (CommandLineRPC_GetValue(argc, argv, value)) {
		 int nHeight = 0;
		 int conter = 0;
		do{
			MilliSleep(1000);
	    	GetBlockHeight(nHeight);
	    	if(conter++ > 80)
	    	{
	    		break;
	    	}
		}while(high+1 > nHeight);
		BOOST_CHECK(conter < 80);
		return true;
	}
	return false;
}

bool SysTestBase::SetAddrGenerteBlock(const char *addr) {
	const char *argv[] = { "rpctest", "generateblock", addr };
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
	if(nFirstHeight <=0)
		return false;
	BOOST_CHECK(nNum>0 && nNum<=nFirstHeight);

	string strNum = strprintf("%d",nNum);
    const char *argv[3] = { "rpctest", "disconnectblock", strNum.c_str()};
	int argc = sizeof(argv) / sizeof(char*);

	Value value;
	if (CommandLineRPC_GetValue(argc, argv, value)) {
		int nHeightAfterDis =0;
		GetBlockHeight(nHeightAfterDis);
		BOOST_CHECK(nHeightAfterDis == nFirstHeight-nNum);
		return true;
	}
	return false;
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
	if (result.type() == str_type){
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

uint64_t SysTestBase::GetRandomBetfee() {
		srand(time(NULL));
		int r = (rand() % 1000000) + 100000000;
		return r;
	}

bool SysTestBase::GetKeyId(string const &addr,CKeyID &KeyId) {
	if (!CRegID::GetKeyID(addr, KeyId)) {
		KeyId=CKeyID(addr);
		if (KeyId.IsEmpty())
		return false;
	}
	return true;
};

bool SysTestBase::IsTxInMemorypool(const uint256& txHash) {
	for (const auto& entry : mempool.mapTx) {
		if (entry.first == txHash)
			return true;
	}

	return false;
}

bool SysTestBase::IsTxUnConfirmdInWallet(const uint256& txHash) {
		for (const auto &item : pwalletMain->UnConfirmTx) {
			if (txHash == item.first) {
				return true;
			}
		}
		return false;
	}

bool SysTestBase::GetRegID(string& strAddr,string& regID){
	Value value = GetAccountInfo(strAddr);

	regID = "RegID";
	return GetStrFromObj(value,regID);
}

bool SysTestBase::IsTxInTipBlock(const uint256& txHash) {
		CBlockIndex* pindex = chainActive.Tip();
		CBlock block;
		if (!ReadBlockFromDisk(block, pindex))
			return false;

		block.BuildMerkleTree();
		std::tuple<bool, int> ret = block.GetTxIndex(txHash);
		if (!std::get<0>(ret)) {
			return false;
		}

		return true;
	}

bool SysTestBase::GetRegID(string& strAddr,CRegID& regID) {
	CAccount account;
	CKeyID keyid;
	if (!GetKeyId(strAddr, keyid)) {
		return false;
	}

	CUserID userId = keyid;

	LOCK(cs_main);
	CAccountViewCache accView(*pAccountViewTip, true);
	if (!accView.GetAccount(userId, account)) {
		return false;
	}
	if((!account.IsRegister())||account.regID.IsEmpty())
	{
		return false;
	}

	regID = account.regID;
	return true;
}

bool SysTestBase::GetTxOperateLog(const uint256& txHash, vector<CAccountLog>& vLog) {
		if (!GetTxOperLog(txHash, vLog))
			return false;

		return true;
	}

bool SysTestBase::PrintLog(){
	const char *argv2[] = { "rpctest", "printblokdbinfo"};

		Value value;
		if (!CommandLineRPC_GetValue(sizeof(argv2) / sizeof(argv2[0]), argv2, value)) {
			return true;
		}
	return false;
}

bool SysTestBase::IsMemoryPoolEmpty() {
	//return mempool.mapTx.empty();
	const char *argv[] = { "rpctest", "getrawmempool"};
	int argc = sizeof(argv) / sizeof(char*);

	Value value;
	if (CommandLineRPC_GetValue(argc, argv, value)) {
		json_spirit::Value::Array narray = value.get_array();
		return narray.size() > 0 ? false : true;
	}
	return true;
}
