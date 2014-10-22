// Copyright (c) 2011-2014 The Bitcoin Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#include <stdlib.h>
#include <time.h>
#include "main.h"
#include "miner.h"
#include "uint256.h"
#include "util.h"
#include <boost/foreach.hpp>
#include <boost/test/unit_test.hpp>

#include "json/json_spirit_writer_template.h"
#include "rpcclient.h"
using namespace std;
using namespace boost;
using namespace json_spirit;

extern Object CallRPC(const string& strMethod, const Array& params);
extern void SHA256Transform(void* pstate, void* pinput, const void* pinit);

#define LogPrint(a,...) printf(__VA_ARGS__)

int CommandLineRPC_GetValue(int argc, char *argv[],Value &value)
{
    string strPrint;
    int nRet = 0;
    try
    {
        // Skip switches
        while (argc > 1 && IsSwitchChar(argv[1][0]))
        {
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
        const Value& error  = find_value(reply, "error");

        if (error.type() != null_type)
        {
            // Error
            strPrint = "error: " + write_string(error, false);
            int code = find_value(error.get_obj(), "code").get_int();
            nRet = abs(code);
        }
        else
        {
        	value = result;
            // Result
            if (result.type() == null_type)
                strPrint = "";
            else if (result.type() == str_type)
                strPrint = result.get_str();
            else
                strPrint = write_string(result, true);
        }
    }
    catch (boost::thread_interrupted) {
        throw;
    }
    catch (std::exception& e) {
        strPrint = string("error: ") + e.what();
        nRet = abs(-1);
    }
    catch (...) {
        PrintExceptionContinue(NULL, "CommandLineRPC()");
        throw;
    }

    if (strPrint != "")
    {
       // fprintf((nRet == 0 ? stdout : stderr), "%s\n", strPrint.c_str());
    }

    return nRet;
}
const static int nFrozenHeight = 100;
const static int nMatureHeight = 100;
struct AccState{
	int64_t dUnmatureMoney;
	int64_t dFreeMoney;
	int64_t dFrozenMoney;
	AccState()
	{
		dUnmatureMoney = 0;
		dFreeMoney = 0;
		dFrozenMoney = 0;
	}
	AccState(int64_t a,int64_t b,int64_t c)
	{
		dUnmatureMoney = a;
		dFreeMoney = b;
		dFrozenMoney = c;
	}

	bool operator==(AccState &a)
	{
		if(dUnmatureMoney == a.dUnmatureMoney &&
		   dFreeMoney == a.dFreeMoney &&
		   dFrozenMoney == a.dFrozenMoney)
		{
			return true;
		}
		return false;
	}

	bool SumEqual(AccState &a)
	{
		int64_t l = dUnmatureMoney+dFreeMoney+dFrozenMoney;
		int64_t r = a.dUnmatureMoney+a.dFreeMoney+a.dFrozenMoney;

		return l==r;
	}

	AccState& operator+=(AccState &a)
	{
		dUnmatureMoney += a.dUnmatureMoney;
		dFreeMoney += a.dFreeMoney;
		dFrozenMoney += a.dFrozenMoney;
		return *this;
	}
};

struct AccOperLog{
	std::map<int,AccState> mapAccState;
	AccOperLog()
	{
		mapAccState.clear();
	}
	bool Add(int &nHeight,AccState &accstate)
	{
		mapAccState[nHeight] = accstate;
	}

	void MergeAcc(int nHeight)
	{
		for(auto &item:mapAccState)
		{
			if(nHeight > item.first+nFrozenHeight)
			{
				item.second.dFreeMoney += item.second.dFrozenMoney;
				item.second.dFrozenMoney = 0.0;
			}

			if(nHeight > item.first+nMatureHeight)
			{
				item.second.dFreeMoney += item.second.dUnmatureMoney;
				item.second.dUnmatureMoney = 0.0;
			}
		}
	}
};

class CMinerTest{
public:
	std::map<string,AccState> mapAccState;
	std::map<string,AccOperLog> mapAccOperLog;
	int nCurHeight;
	int64_t nCurMoney;
	int64_t nCurFee;
private:
	int GetRandomFee()
	{
		srand(time(NULL));
		int r =(rand()%1000000)+1000000;
		return r;
	}
	int GetRandomMoney()
	{
		srand(time(NULL));
		int r = (rand() % 1000) + 1000;
		return r;
	}
public:

	bool GetOneAddr(std::string &addr,char *pStrMinMoney,char *bpBoolReg)
	{
		//CommanRpc
		char *argv[] = {"rpctest", "getoneaddr",pStrMinMoney,bpBoolReg};
		int argc = sizeof(argv)/sizeof(char*);

		Value value;
		int ret = CommandLineRPC_GetValue(argc,argv,value);
		if(!ret)
		{
			addr = value.get_str();
			LogPrint("test_miners","GetOneAddr:%s\r\n",addr.c_str());
			return true;
		}
		return false;
	}

	bool GetNewAddr(std::string &addr)
	{
		//CommanRpc
		char *argv[] = {"rpctest", "getnewaddress"};
		int argc = sizeof(argv)/sizeof(char*);

		Value value;
		int ret = CommandLineRPC_GetValue(argc, argv, value);
		if (!ret)
		{
			addr = value.get_str();
			LogPrint("test_miners","GetNewAddr:%s\r\n",addr.c_str());
			return true;
		}
		return false;
	}

	bool GetAccState(const std::string &addr,AccState &accstate)
	{
		//CommanRpc
		char temp[64] = {0};
		strncpy(temp,addr.c_str(),sizeof(temp)-1);

		char *argv[] = {"rpctest", "getaddramount",temp};
		int argc = sizeof(argv)/sizeof(char*);

		Value value;
		int ret = CommandLineRPC_GetValue(argc, argv, value);
		if (!ret)
		{
			Object obj = value.get_obj();
			double dfree = find_value(obj,"free amount").get_real();
			double dmature = find_value(obj,"mature amount").get_real();
			double dfrozen = find_value(obj,"frozen amount").get_real();
			accstate.dFreeMoney = roundint64(dfree*COIN);
			accstate.dUnmatureMoney = roundint64(dmature*COIN);
			accstate.dFrozenMoney = roundint64(dfrozen*COIN);
			LogPrint("test_miners","GetAccState FreeMoney:%0.8lf matureMoney:%0.8lf FrozenMoney:%0.8lf\r\n",
					dfree,dmature,dfrozen);
			return true;
		}
		return false;
	}

	bool GetBlockHeight(int &nHeight)
	{
		nHeight = 0;
		char *argv[] = {"rpctest", "getinfo",};
		int argc = sizeof(argv) / sizeof(char*);

		Value value;
		int ret = CommandLineRPC_GetValue(argc, argv, value);
		if (!ret)
		{
			Object obj = value.get_obj();

			nHeight = find_value(obj,"blocks").get_int();
			LogPrint("test_miners","GetBlockHeight:%d\r\n",nHeight);
			return true;
		}
		return false;
	}

	bool CreateNormalTx(const std::string &srcAddr,const std::string &desAddr,const int nHeight)
	{
		//CommanRpc
		char src[64] = { 0 };
		strncpy(src, srcAddr.c_str(), sizeof(src)-1);

		char dest[64] = { 0 };
		strncpy(dest, desAddr.c_str(), sizeof(dest)-1);

		char money[64] = {0};
		int nmoney = GetRandomMoney();
		sprintf(money,"%d00000000",nmoney);
		nCurMoney = nmoney*COIN;

		char fee[64] = {0};
		int nfee = GetRandomFee();
		sprintf(fee,"%d",nfee);
		nCurFee = nfee;

		char height[16] = {0};
		sprintf(height,"%d",nHeight);

		char *argv[] = { "rpctest", "createnormaltx", src,dest,money,fee,height};
		int argc = sizeof(argv)/sizeof(char*);

		Value value;
		int ret = CommandLineRPC_GetValue(argc, argv, value);
		if (!ret)
		{
			LogPrint("test_miners","CreateNormalTx:%s\r\n",value.get_str().c_str());
			return true;
		}
		return false;
	}

	bool CreateFreezeTx(const std::string &addr,const int nHeight)
	{
		//CommanRpc
		char caddr[64] = { 0 };
		strncpy(caddr, addr.c_str(), sizeof(caddr)-1);

		char money[64] = {0};
		int nmoney = GetRandomMoney();
		sprintf(money, "%d00000000", nmoney);
		nCurMoney = nmoney * COIN;

		char fee[64] = { 0 };
		int nfee = GetRandomFee();
		sprintf(fee, "%d", nfee);
		nCurFee = nfee;

		char height[16] = {0};
		sprintf(height,"%d",nHeight);

		char freeheight[16] = {0};
		sprintf(freeheight,"%d",nHeight+100);

		char *argv[] = { "rpctest", "createfreezetx", caddr,money,fee,height,freeheight};
		int argc = sizeof(argv)/sizeof(char*);

		Value value;
		int ret = CommandLineRPC_GetValue(argc, argv, value);
		if (!ret)
		{
			LogPrint("test_miners","CreateFreezeTx:%s\r\n",value.get_str().c_str());
			return true;
		}
		return false;
	}

	bool RegisterSecureTx(const std::string &addr,const int nHeight)
	{
		//CommanRpc
		char caddr[64] = { 0 };
		strncpy(caddr, addr.c_str(), sizeof(caddr)-1);

		char fee[64] = { 0 };
		int nfee = GetRandomFee();
		sprintf(fee, "%d", nfee);
		nCurFee = nfee;

		char height[16] = {0};
		sprintf(height,"%d",nHeight);


		char *argv[] = { "rpctest", "registersecuretx", caddr,fee,height};
		int argc = sizeof(argv)/sizeof(char*);

		Value value;
		int ret = CommandLineRPC_GetValue(argc, argv, value);
		if (!ret)
		{
			LogPrint("test_miners","RegisterSecureTx:%s\r\n",value.get_str().c_str());
			return true;
		}
		return false;
	}

	bool RegisterScriptTx(const std::string &addr,const std::string &script,const int nHeight)
	{
		//CommanRpc
		char caddr[64] = { 0 };
		strncpy(caddr, addr.c_str(), sizeof(caddr)-1);

		char csript[128*1024] = {0};
		strncpy(csript, script.c_str(), sizeof(csript)-1);

		char fee[64] = { 0 };
		int nfee = GetRandomFee();
		sprintf(fee, "%d", nfee);
		nCurFee = nfee;

		char height[16] = {0};
		sprintf(height,"%d",nHeight);


		char *argv[] = { "rpctest", "registerscripttx", caddr,csript,fee,height};
		int argc = sizeof(argv)/sizeof(char*);

		Value value;
		int ret = CommandLineRPC_GetValue(argc, argv, value);
		if (!ret)
		{
			LogPrint("test_miners","RegisterSecureTx:%s\r\n",value.get_str().c_str());
			return true;
		}
		return false;
	}

	bool CreateSecureTx(const string &scriptid,const vector<string> &obaddrs,
			const vector<string> &addrs,const string&contract,const int nHeight)
	{
		//CommanRpc
		char cscriptid[64] = { 0 };
		strncpy(cscriptid, scriptid.c_str(), sizeof(cscriptid)-1);

		char cobstr[512] = {0};
		{
			Array array;
			array.clear();
			for (const auto &str : obaddrs)
			{
				array.push_back(str);
			}
			string arraystr = write_string(Value(array),false);
			strncpy(cobstr, arraystr.c_str(), sizeof(cobstr)-1);
		}
		char addrstr[512] = {0};
		{
			Array array;
			array.clear();
			for (const auto &str : addrs) {
				array.push_back(str);
			}
			string arraystr = write_string(Value(array), false);
			strncpy(addrstr, arraystr.c_str(), sizeof(addrstr) - 1);
		}

		char ccontract[10*1024] = { 0 };
		strncpy(ccontract, contract.c_str(), sizeof(ccontract)-1);

		char height[16] = {0};
		sprintf(height,"%d",nHeight);


		char *argv[] = { "rpctest", "createsecuretx", cscriptid,cobstr,addrstr,ccontract,"1000000",height};
		int argc = sizeof(argv)/sizeof(char*);

		Value value;
		int ret = CommandLineRPC_GetValue(argc, argv, value);
		if (!ret)
		{
			LogPrint("test_miners","CreateSecureTx:%s\r\n",value.get_str().c_str());
			return true;
		}
		return false;
	}

	bool SignSecureTx(const string &securetx)
	{
		//CommanRpc
		char csecuretx[10*1024] = { 0 };
		strncpy(csecuretx, securetx.c_str(), sizeof(csecuretx)-1);


		char *argv[] = { "rpctest", "signsecuretx", csecuretx};
		int argc = sizeof(argv)/sizeof(char*);

		Value value;
		int ret = CommandLineRPC_GetValue(argc, argv, value);
		if (!ret)
		{
			LogPrint("test_miners","SignSecureTx:%s\r\n",value.get_str().c_str());
			return true;
		}
		return false;
	}

	bool IsAllTxInBlock()
	{
		char *argv[] = { "rpctest", "listunconfirmedtx" };
		int argc = sizeof(argv) / sizeof(char*);

		Value value;
		int ret = CommandLineRPC_GetValue(argc, argv, value);
		if (!ret) {
			Array array = value.get_array();
			if(array.size() == 0)
			return true;
		}
		return false;
	}

	bool GetBlockHash(const int nHeight,std::string &blockhash)
	{
		char height[16] = {0};
		sprintf(height,"%d",nHeight);

		char *argv[] = {"rpctest", "getblockhash",height};
		int argc = sizeof(argv) / sizeof(char*);

		Value value;
		int ret = CommandLineRPC_GetValue(argc, argv, value);
		if (!ret)
		{
			blockhash = value.get_str();
			LogPrint("test_miners","GetBlockHash:%s\r\n",blockhash.c_str());
			return true;
		}
		return false;
	}

	bool GetBlockMinerAddr(const std::string &blockhash,std::string &addr)
	{
		char cblockhash[80] = {0};
		strncpy(cblockhash,blockhash.c_str(),sizeof(cblockhash)-1);

		char *argv[] = {"rpctest", "getblock",cblockhash};
		int argc = sizeof(argv) / sizeof(char*);

		Value value;
		int ret = CommandLineRPC_GetValue(argc, argv, value);
		if (!ret)
		{
			Array txs = find_value(value.get_obj(),"tx").get_array();
			addr = find_value(txs[0].get_obj(),"addr").get_str();
			LogPrint("test_miners","GetBlockMinerAddr:%s\r\n",addr.c_str());
			return true;
		}
		return false;
	}

	bool GenerateOneBlock()
	{
		char *argv[] = {"rpctest", "setgenerate","true"};
		int argc = sizeof(argv) / sizeof(char*);

		Value value;
		int ret = CommandLineRPC_GetValue(argc, argv, value);
		if (!ret) {
			return true;
		}
		return false;
	}

public:
	CMinerTest()
	{
		mapAccState.clear();
		mapAccOperLog.clear();
		nCurHeight = 0;
	}
	bool CheckAccState(const string &addr,AccState &lastState,bool bAccurate = false)
	{
		AccState initState = mapAccState[addr];
		AccOperLog operlog = mapAccOperLog[addr];
		operlog.MergeAcc(nCurHeight);

		for(auto & item:operlog.mapAccState)
		{
			initState += item.second;
		}


		bool b = false;
		if(bAccurate)
		{
			b = (initState==lastState);
		}
		else
		{
			b = lastState.SumEqual(initState);
		}
		return b;
	}
};

BOOST_FIXTURE_TEST_SUITE(miner_tests,CMinerTest)

BOOST_FIXTURE_TEST_CASE(block_normaltx,CMinerTest)
{
	//printf("\r\block_normaltx test start:\r\n");
	string srcaddr;
	BOOST_REQUIRE(GetOneAddr(srcaddr,"1100000000000","true"));

	AccState initState;
	BOOST_REQUIRE(GetAccState(srcaddr,initState));
	mapAccState[srcaddr] = initState;//insert

	int height = 0;
	BOOST_REQUIRE(GetBlockHeight(height));
	nCurHeight = height;
	uint64_t totalfee = 0;
	{
		string destaddr;
		BOOST_REQUIRE(GetNewAddr(destaddr));
		BOOST_REQUIRE(CreateNormalTx(srcaddr,destaddr,height));
		totalfee += nCurFee;
		AccOperLog &operlog1 = mapAccOperLog[srcaddr];
		AccOperLog &operlog2 = mapAccOperLog[destaddr];
		AccState acc1(0,-(nCurMoney+nCurFee),0);
		AccState acc2(0,nCurMoney,0);
		operlog1.Add(height,acc1);
		operlog2.Add(height,acc2);
	}
	BOOST_REQUIRE(GenerateOneBlock());
	height++;

	BOOST_REQUIRE(IsAllTxInBlock());
	string mineraddr;
	string blockhash;
	BOOST_REQUIRE(GetBlockHash(height,blockhash));
	BOOST_REQUIRE(GetBlockMinerAddr(blockhash,mineraddr));

	if(mineraddr == srcaddr)
	{
		AccState acc(totalfee,0,0);
		mapAccOperLog[mineraddr].Add(height,acc);
	}
	else
	{
		BOOST_REQUIRE(GetAccState(mineraddr,initState));
		mapAccState[mineraddr] = initState;//insert
	}

	for(auto & item:mapAccOperLog)
	{
		AccState lastState;
		BOOST_REQUIRE(GetAccState(item.first,lastState));
		BOOST_REQUIRE(CheckAccState(item.first,lastState));
	}
}

BOOST_FIXTURE_TEST_CASE(block_frozentx,CMinerTest)
{
	//printf("\r\nblock_frozentx test start:\r\n");
	string srcaddr;
	BOOST_REQUIRE(GetOneAddr(srcaddr,"1100000000000","true"));

	AccState initState;
	BOOST_REQUIRE(GetAccState(srcaddr,initState));
	mapAccState[srcaddr] = initState;//insert

	int height = 0;
	BOOST_REQUIRE(GetBlockHeight(height));
	nCurHeight = height;
	uint64_t totalfee = 0;
	{
		BOOST_REQUIRE(CreateFreezeTx(srcaddr,height));
		totalfee += nCurFee;
		AccOperLog &operlog1 = mapAccOperLog[srcaddr];
		AccState acc(0,-(nCurMoney+nCurFee),nCurMoney);
		operlog1.Add(height,acc);
	}
	BOOST_REQUIRE(GenerateOneBlock());
	height++;
	BOOST_REQUIRE(IsAllTxInBlock());
	string mineraddr;
	string blockhash;
	BOOST_REQUIRE(GetBlockHash(height,blockhash));
	BOOST_REQUIRE(GetBlockMinerAddr(blockhash,mineraddr));

	if(mineraddr == srcaddr)
	{
		AccState acc(totalfee,0,0);
		mapAccOperLog[mineraddr].Add(height,acc);
	}
	else
	{
		BOOST_REQUIRE(GetAccState(mineraddr,initState));
		mapAccState[mineraddr] = initState;//insert
	}

	for(auto & item:mapAccOperLog)
	{
		AccState lastState;
		BOOST_REQUIRE(GetAccState(item.first,lastState));
		BOOST_REQUIRE(CheckAccState(item.first,lastState));
	}
}

BOOST_AUTO_TEST_SUITE_END()

