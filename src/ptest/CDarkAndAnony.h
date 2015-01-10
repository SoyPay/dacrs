/*
 * CDarkAndAnony.h
 *
 *  Created on: 2014年12月30日
 *      Author: ranger.shi
 */

#ifndef CDARKANDANONY_H_
#define CDARKANDANONY_H_

#include "CycleTestBase.h"
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
#include "json/json_spirit_reader_template.h"
#include "json/json_spirit_reader.h"
#include "json/json_spirit_writer.h"
#include "json/json_spirit_value.h"
#include "json/json_spirit_stream_reader.h"
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
using namespace std;
using namespace boost;
using namespace json_spirit;
extern Object CallRPC(const string& strMethod, const Array& params);
extern time_t sleepTime;
extern int64_t llTime;
	#pragma pack(1)
	typedef struct  {
		unsigned char dnType;					//!<类型
		char buyer[6];						//!<买家ID（采用6字节的账户ID）
		char seller[6];						//!<卖家ID（采用6字节的账户ID）
		int nHeight;							//!<超时绝对高度
		uint64_t nPayMoney;						//!<买家向卖家支付的金额
		IMPLEMENT_SERIALIZE
		(
				READWRITE(dnType);
				for(int i = 0;i < 6;i++)
				READWRITE(buyer[i]);
				for(int i = 0;i < 6;i++)
				READWRITE(seller[i]);
				READWRITE(nHeight);
				READWRITE(nPayMoney);
		)
	} FIRST_CONTRACT;

	typedef struct {
		unsigned char dnType;				//!<交易类型
		char hash[32];		//!<上一个交易包的哈希
		IMPLEMENT_SERIALIZE
		(
				READWRITE(nType);
		for(int i = 0;i < 32;i++)
				READWRITE(hash[i]);
		)
	} NEXT_CONTRACT;

	typedef struct  {
		char 	Sender[6];						//!<转账人ID（采用6字节的账户ID）
		int nHeight;							//!<超时绝对高度
		uint64_t nPayMoney;						//!<转账的人支付的金额
		unsigned short len;                     //!<接受钱账户信息长度
		IMPLEMENT_SERIALIZE
		(
				for(int i = 0;i < 6;i++)
				READWRITE(Sender[i]);
				READWRITE(nHeight);
				READWRITE(nPayMoney);
				READWRITE(len);
		)
	}CONTRACT_ANONY;
	typedef struct  {
		char 	account[6];						//!<接受钱的ID（采用6字节的账户ID）
		uint64_t nReciMoney;						    //!<	收到钱的金额
		IMPLEMENT_SERIALIZE
		(
				for(int i = 0;i < 6;i++)
				READWRITE(account[i]);
				READWRITE(nReciMoney);

		)
	} ACCOUNT_INFO;

class CDarkAndAnony: public CycleTestBase {
  int step;
	string darkhash;
	string anonyhahs;
	string darkRegId;
	string anonyRegId;
	char* dest1[3];
public:
	CDarkAndAnony();
	virtual ~CDarkAndAnony();
	int GetRandomFee() {
		srand(time(NULL));
		int r = (rand() % 1000000) + 1000000;
		return r;
	}
	virtual TEST_STATE run() ;
	uint64_t GetPayMoney() {
		uint64_t r = 0;
		while(true)
		{
			srand(time(NULL));
			r = (rand() % 1000002) + 100000000;
			if(r%2 == 0 && r != 0)
				break;
		}

		return r;
	}
	int TestCallRPC(std::string strMethod, const std::vector<std::string> &vParams, std::string &strRet);

	bool step0RegistScript()
	{
		vector<string> param;
		bool flag = true;
		string buyaddr,selleraddr;
		GetAddress(buyaddr,selleraddr);
		string nfee;
		nfee = strprintf("%d",GetRandomFee());

		darkhash = CreateScript("D:\\bitcoin\\data\\darksecure.bin",buyaddr,nfee);
		BOOST_CHECK(Parsejson(darkhash) != "");

		anonyhahs = CreateScript("D:\\bitcoin\\data\\anony.bin",buyaddr,nfee);
		BOOST_CHECK(Parsejson(anonyhahs) != "");

		darkhash = Parsejson(darkhash);
	    anonyhahs = Parsejson(anonyhahs);
	    step++;
	//    BOOST_CHECK(basetest.GenerateOneBlock());
	    return true;
	}
	bool step1RegistScript() {
		if (basetest.GetTxConfirmedRegID(darkhash, darkRegId)) {
			if (basetest.GetTxConfirmedRegID(anonyhahs, anonyRegId)) {
				step++;
				return true;
			}
		}
		return false;

	}
	bool step2RegistScript() {
		SendDarkTx(darkRegId);
	//	SendanonyTx(anonyRegId);
		return false;
	}

	string CreateScript(char * vmpath,string addr,string nfee);
	string Parsejson(string str);
	string CreateDarkTx(string scriptid,string buyeraddr,string selleraddr,string nfee,uint64_t paymoney);
	string CreateSecondDarkTx(string scriptid,string hash,string buyeraddr,string nfee);
	string Createanony(string scriptid,string addr,string toaddress1,string toaddress2,string nfee,uint64_t paymoney);
	string GetScript(string hash);

	void GetAddress(string& buyaddr,string& selleraddr)
	{
		srand(time(NULL));
		int i = rand() % 3;
		int k = 0;
		while(true)
		{
			k = rand() % 3;
			if(k != i)
				break;
		}
		buyaddr = dest1[i];
		selleraddr = dest1[k];
	}

	void GetAddress(string& addr1,string& addr2,string& addr3)
	{
		srand(time(NULL));
		int i = rand() % 3;
		int k = 0;
		int d = 0;
		while(true)
		{
			k = rand() % 3;
			d = rand() % 3;
			if(k != i && d != k && d != i)
				break;
		}
		addr1 = dest1[i];
		addr2 = dest1[k];
		addr3 = dest1[d];
	}

	void SendDarkTx(string scriptid);

	void SendanonyTx(string scriptid);
	bool ImportWalletKey();
};

#endif /* CDARKANDANONY_H_ */
