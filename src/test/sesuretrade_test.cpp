#include <boost/test/unit_test.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include "SysTestBase.h"
using namespace std;
using namespace boost;

#define BUYER	"01"
#define SELLER	"02"
#define ARBIT	"03"

#define BUYER_ADDR 		"mw5wbV73gXbreYy8pX4FSb7DNYVKU3LENc"
#define SELLER_ADDR 	"mhVJJSAdPNDPvFWCmQN446GUBPzFm8aN4y"
#define ARBIT_ADDR		"moZJZgsGFC4qvwRdjzS7Bj3fHrtpUfEVEE"

#define BUYER_ID 		"000000000200"
#define SELLER_ID 		"000000000300"
#define ARBIT_ID		"000000000400"

#define ACCOUNT_ID_SIZE 6
#define MAX_ARBITRATOR 3
#define HASH_SIZE		32

#pragma pack(1)
typedef struct tag_INT64 {
	unsigned char data[8];
} Int64;

typedef struct tagACCOUNT_ID
{
	char accounid[ACCOUNT_ID_SIZE];
}ACCOUNT_ID;

typedef struct  {
	unsigned char nType;					//!<ÀàÐÍ
	unsigned char nArbitratorCount;			//!<ÖÙ²ÃÕß¸öÊý
	ACCOUNT_ID 	buyer;						//!<Âò¼ÒID£¨²ÉÓÃ6×Ö½ÚµÄÕË»§ID£©
	ACCOUNT_ID seller;						//!<Âô¼ÒID£¨²ÉÓÃ6×Ö½ÚµÄÕË»§ID£©
	ACCOUNT_ID arbitrator[MAX_ARBITRATOR];	//!<ÖÙ²ÃÕßID£¨²ÉÓÃ6×Ö½ÚµÄÕË»§ID£©
	long nHeight;							//!<³¬Ê±¾ø¶Ô¸ß¶È
	Int64 nFineMoney;						//!<Âô¼ÒÎ¥Ô¼ºó×î´ó·£¿î½ð¶î
	Int64 nPayMoney;						//!<Âò¼ÒÏòÂô¼ÒÖ§¸¶µÄ½ð¶î
	Int64 nFee;								//!<ÖÙ²ÃÊÖÐø·Ñ
	Int64 ndeposit;							//!<ÖÙ²ÃÑº½ð,ÉêËßÊ±´ÓÖÙ²ÃÕß¿Û³ýµÄÑº½ð(Èç¹ûÖÙ²Ã²»ÏìÓ¦Ç®¹éÂò¼Ò·ñÔòÂò¼ÒÍË»¹¸øÖÙ²ÃÕß)
}FIRST_CONTRACT;

typedef struct {
	unsigned char nType;					//!<½»Ò×ÀàÐÍ
	unsigned char hash[HASH_SIZE];			//!<ÉÏÒ»¸ö½»Ò×°üµÄ¹þÏ£
} NEXT_CONTRACT;

typedef struct {
	unsigned char nType;					//!<½»Ò×ÀàÐÍ
	unsigned char hash[HASH_SIZE];			//!<ÉÏÒ»¸ö½»Ò×µÄ¹þÏ£
	Int64 nMinus;
}ARBIT_RES_CONTRACT;
#pragma popup()

class CSesureTrade:public SysTestBase
{
public:
	bool GetHashFromCreatedTx(const Value& valueRes,string& strHash)
	{
		if (valueRes.type() == null_type) {
			return false;
		}

		const Value& result = find_value(valueRes.get_obj(), "hash");
		if (result.type() == null_type){
			return false;
		}

		strHash = result.get_str();
		return true;
	}

	bool GetScriptID(const string& strTxHash, string& strScriptID) {
		uint256 txhash(strTxHash);

		int nIndex = 0;
		int BlockHeight = GetTxComfirmHigh(txhash);
		if (BlockHeight > chainActive.Height() || BlockHeight == -1) {
			return false;
		}
		CBlockIndex* pindex = chainActive[BlockHeight];
		CBlock block;
		if (!ReadBlockFromDisk(block, pindex))
			return false;

		block.BuildMerkleTree();
		std::tuple<bool, int> ret = block.GetTxIndex(txhash);
		if (!std::get<0>(ret)) {
			return false;
		}

		nIndex = std::get<1>(ret);

		CRegID striptID(BlockHeight, nIndex);

		strScriptID = HexStr(striptID.GetVec6());
		return true;
	}

	void VerifyTxInBlock(const string& strTxHash) {
		string strScriptID;
		while (true) {
			if (GetScriptID(strTxHash, strScriptID)) {
				if (!strScriptID.empty())
					break;
			}
		}
	}

	bool ModifyAuthor(unsigned char nUserData,string& strTxHash) {
		vector<unsigned char> vUserDefine;
		string strHash1, strHash2;
		vUserDefine.push_back(nUserData);
		CNetAuthorizate author(100, vUserDefine, 100000, 1000000, 1000000);
		Value valueRes = SysTestBase::ModifyAuthor(BUYER_ADDR, strRegScriptID, 200, 10000, author);

		if (!GetHashFromCreatedTx(valueRes,strTxHash)) {
			return false;
		}

		return true;
	}

	int GetBlockHeight() {
		return (int) chainActive.Height();
	}

	bool GenerateOneBlock() {
		int nOldBlockHeight = GetBlockHeight();
		if (!SysTestBase::GenerateOneBlock()) {
			return false;
		}

		int nNewBlockHeight = GetBlockHeight();
		if (nNewBlockHeight != nOldBlockHeight + 1) {
			return false;
		}

		return true;
	}

	void PacketFirstContract(const char*pBuyID,const char* pSellID,const char* pArID,
		int nHeight,int nFine,int nPay,int nFee,int ndeposit,FIRST_CONTRACT* pContract)
	{
		BOOST_CHECK(pContract);
		memset(pContract,0,sizeof(FIRST_CONTRACT));
		pContract->nType = 1;
		pContract->nArbitratorCount = 1;
		pContract->nHeight = nHeight;

		unsigned char nSize = sizeof(int);
		vector<unsigned char> v = ParseHex(pBuyID);
		memcpy(pContract->buyer.accounid,&v[0],ACCOUNT_ID_SIZE);

		v = ParseHex(pSellID);
		memcpy(pContract->seller.accounid,&v[0],ACCOUNT_ID_SIZE);

		v = ParseHex(pArID);
		memcpy(pContract->arbitrator[0].accounid,&v[0],ACCOUNT_ID_SIZE);

		memcpy(&pContract->nFineMoney,(const char*)&nFine,nSize);//100
		memcpy(&pContract->nPayMoney,(const char*)&nPay,nSize);//80
		memcpy(&pContract->ndeposit,(const char*)&ndeposit,nSize);//20
		memcpy(&pContract->nFee,(const char*)&nFee,nSize);//10
	}
protected:
	string 				strRegScriptID;
	FIRST_CONTRACT		firstConstract;
};

BOOST_FIXTURE_TEST_SUITE(sesuretrade,CSesureTrade)
BOOST_AUTO_TEST_CASE(test)
{
//	string strTxHash;
//	Value valueRes = RegisterScriptTx(BUYER_ADDR,"SecuredTrade.bin",100,100000);
//	BOOST_CHECK(GetHashFromCreatedTx(valueRes,strTxHash));
//	BOOST_CHECK(GenerateOneBlock());
//	VerifyTxInBlock(strTxHash);
//	BOOST_CHECK(GetScriptID(strTxHash,strRegScriptID));
//
//	BOOST_CHECK(ModifyAuthor(1,strTxHash));
//	BOOST_CHECK(GenerateOneBlock());
//	VerifyTxInBlock(strTxHash);
//
//	PacketFirstContract(BUYER_ID,SELLER_ID,ARBIT_ID,100,10000,10000,10000,10000,&firstConstract);
//	string strContract;
//	BOOST_CHECK(CreateContractTx(strRegScriptID,BUYER_ADDR,strContract,100,10000));
}
BOOST_AUTO_TEST_SUITE_END()












