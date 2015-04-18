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


typedef struct  {
	unsigned char dnType;					//!<类型
	unsigned char seller[6];						//!<卖家ID（采用6字节的账户ID）
	IMPLEMENT_SERIALIZE
	(
			READWRITE(dnType);
			for(int i = 0;i < 6;i++)
			READWRITE(seller[i]);
	)
} FIRST_CONTRACT;

typedef struct {
	unsigned char dnType;				//!<交易类型
	unsigned char hash[32];		//!<上一个交易包的哈希
	IMPLEMENT_SERIALIZE
	(
		READWRITE(dnType);
		for(int i = 0;i < 32;i++)
		READWRITE(hash[i]);
	)
} NEXT_CONTRACT;

enum TXTYPE{
	TX_BUYTRADE = 0x01,
	TX_SELLERTRADE = 0x02,
	TX_BUYERCONFIM = 0x03,
	TX_BUYERCANCEL = 0x04,
};
#define BUYER_A    "mnnS4upLeY7RZpNnvoGMZzs9ELscQjtvqy"
#define SELLER_B    "mjSwCwMsvtKczMfta1tvr78z2FTsZA1JKw"


class CDarkAndAnony: public CycleTestBase {
  int step;
	string sritpthash;
	string buyerhash;
	string sellerhash;
	string buyerconfiredhash;
	string buyercancelhash;
	string scriptid ;
	uint64_t sendmonye;
public:
	CDarkAndAnony();
	virtual ~CDarkAndAnony();
	int GetRandomFee() {
		srand(time(NULL));
		int r = (rand() % 1000000) + 100000000;
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
};

#endif /* CDARKANDANONY_H_ */
