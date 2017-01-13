/*
 * Black_Halo_tests.h
 *
 *  Created on: 2014年12月30日
 *      Author: ranger.shi
 */

#ifndef DACRS_PTEST_BLACK_HALO_TEST_H_
#define DACRS_PTEST_BLACK_HALO_TEST_H_

#include "cycle_test_base.h"
#include <stdlib.h>
#include <time.h>
#include "main.h"
#include "miner.h"
#include "uint256.h"
#include "util.h"
#include <boost/foreach.hpp>
#include <boost/test/unit_test.hpp>

#include "json/json_spirit_writer_template.h"
#include "./rpc/rpcclient.h"
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

typedef struct {
	unsigned char uchDnType;					//!<类型
	unsigned char arruchSeller[6];			    //!<卖家ID（采用6字节的账户ID）
	IMPLEMENT_SERIALIZE
	(
			READWRITE(uchDnType);
			for (int i = 0;i < 6;i++) {
				READWRITE(arruchSeller[i]);
			}
	)
} ST_FIRST_CONTRACT;

typedef struct {
	unsigned char uchDnType;				//!<交易类型
	unsigned char arruchHash[32];		        //!<上一个交易包的哈希
	IMPLEMENT_SERIALIZE
	(
			READWRITE(uchDnType);
			for(int i = 0;i < 32;i++) {
				READWRITE(arruchHash[i]);
			}
	)
} ST_NEXT_CONTRACT;

//enum emTX_TYPE{
//	TX_BUYTRADE = 0x01,
//	TX_SELLERTRADE = 0x02,
//	TX_BUYERCONFIM = 0x03,
//	TX_BUYERCANCEL = 0x04,
//};
#define BUYER_A    	"dk2NNjraSvquD9b4SQbysVRQeFikA55HLi"
#define SELLER_B    "dggsWmQ7jH46dgtA5dEZ9bhFSAK1LASALw"

class CBlackHalo: public CycleTestBase {
 public:
	CBlackHalo();
	virtual ~CBlackHalo();
	int GetRandomFee() {
		srand(time(NULL));
		int nR = (rand() % 1000000) + 100000000;
		return nR;
	}
	virtual emTEST_STATE Run();

	uint64_t GetPayMoney() {
		uint64_t ullR = 0;
		while (true) {
			srand(time(NULL));
			ullR = (rand() % 1000002) + 100000000;
			if (ullR % 2 == 0 && ullR != 0) {
				break;
			}
		}
		return ullR;
	}

	bool RegistScript();
	bool SendBuyerPackage();
	bool SendSellerPackage();
	bool SendBuyerConfirmedPackage();
	bool SendBuyerCancelPackage();
	bool WaitRegistScript();
	bool WaitSendBuyerPackage();
	bool WaitSendSellerPackage();
	bool WaitSendBuyerConfirmedPackage();
	bool WaitSendBuyerCancelPackage();
 private:
	int m_nStep;
	string m_strSritpthash;
	string m_strBuyerhash;
	string m_strSellerhash;
	string m_strBuyerconfiredhash;
	string m_strBuyercancelhash;
	string m_strScriptid;
	uint64_t m_ullSendMonye;
};

#endif /* DACRS_PTEST_BLACK_HALO_TEST_H_ */
