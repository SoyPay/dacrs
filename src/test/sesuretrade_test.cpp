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

#define VADDR_BUYER   	"[\"mw5wbV73gXbreYy8pX4FSb7DNYVKU3LENc\"]"
#define VADDR_SELLER   	"[\"mhVJJSAdPNDPvFWCmQN446GUBPzFm8aN4y\"]"
#define VADDR_ARBIT   	"[\"moZJZgsGFC4qvwRdjzS7Bj3fHrtpUfEVEE\"]"

#define MAX_ACCOUNT_LEN 20
#define ACCOUNT_ID_SIZE 6
#define MAX_ARBITRATOR 3
#define HASH_SIZE		32

#pragma pack(1)
typedef struct tag_INT64 {
	unsigned char data[8];
} Int64;

typedef struct tagACCOUNT_ID
{
	char accounid[MAX_ACCOUNT_LEN];
}ACCOUNT_ID;

typedef struct  {
	unsigned char nType;
	unsigned char nArbitratorCount;
	ACCOUNT_ID 	buyer;
	ACCOUNT_ID seller;
	ACCOUNT_ID arbitrator[MAX_ARBITRATOR];
	long nHeight;
	Int64 nFineMoney;
	Int64 nPayMoney;
	Int64 nFee;
	Int64 ndeposit;
}FIRST_CONTRACT;

typedef struct {
	unsigned char nType;
	unsigned char hash[HASH_SIZE];
} NEXT_CONTRACT;

typedef struct {
	unsigned char nType;
	unsigned char hash[HASH_SIZE];
	Int64 nMinus;
}ARBIT_RES_CONTRACT;
#pragma popup()

class CSesureTrade:public SysTestBase
{
public:
	bool GetHashFromCreatedTx(const Value& valueRes,string& strHash)
	{
		if (valueRes.type() == null_type) {
			cout<<write_string(valueRes, true)<<endl;
			return false;
		}

		const Value& result = find_value(valueRes.get_obj(), "hash");
		if (result.type() == null_type){
			cout<<write_string(valueRes, true)<<endl;
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

	bool ModifyAuthor(unsigned char nUserData,const string& strSignAddr,string& strTxHash) {
		vector<unsigned char> vUserDefine;
		string strHash1, strHash2;
		vUserDefine.push_back(nUserData);
		CNetAuthorizate author(10000, vUserDefine, 100000, 1000000, 1000000);
		Value valueRes = SysTestBase::ModifyAuthor(strSignAddr, strRegScriptID, 0, 10000, author);

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

	string GetReverseHash(const string& strTxHash) {
		vector<unsigned char> vHash = ParseHex(strTxHash);
		reverse(vHash.begin(), vHash.end());
		string strHash;
		strHash.assign(vHash.begin(), vHash.end());
		return strHash;
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

	void PacketNextContract(unsigned char nStep, unsigned char* pHash, NEXT_CONTRACT* pNextContract) {
		memset(pNextContract, 0, sizeof(NEXT_CONTRACT));
		pNextContract->nType = nStep;
		memcpy(pNextContract->hash, pHash, HASH_SIZE);
	}

	void PacketLastContract(unsigned char* pHash,int nFine,ARBIT_RES_CONTRACT* pLastContract)
	{
		memset(pLastContract,0,sizeof(ARBIT_RES_CONTRACT));
		pLastContract->nType = 4;
		memcpy(pLastContract->hash,pHash,HASH_SIZE);
		memcpy(&pLastContract->nMinus,(const char*)&nFine,sizeof(int));
	}

	string PutDataIntoString(char* pData, int nDateLen) {
		string strData;
		strData.assign(pData, pData + nDateLen);
		return strData;
	}

	void CreateFirstTx() {
		string strTxHash;
		Value valueRes = RegisterScriptTx(BUYER_ADDR, "SecuredTrade.bin", 0, 100000);
		BOOST_CHECK(GetHashFromCreatedTx(valueRes, strTxHash));
		BOOST_CHECK(GenerateOneBlock());
		VerifyTxInBlock(strTxHash);
		BOOST_CHECK(GetScriptID(strTxHash, strRegScriptID));

		BOOST_CHECK(ModifyAuthor(1, BUYER_ADDR,strTxHash));
		BOOST_CHECK(GenerateOneBlock());
		VerifyTxInBlock(strTxHash);

		PacketFirstContract(BUYER_ID, SELLER_ID, ARBIT_ID, 200, 100000, 100000, 100000, 100000, &firstConstract);
		string strData = PutDataIntoString((char*) &firstConstract,sizeof(firstConstract));
		valueRes = CreateContractTxEx(strRegScriptID, VADDR_BUYER, strData, 0, 100000);
		BOOST_CHECK(GetHashFromCreatedTx(valueRes, strTxHash));
		BOOST_CHECK(GenerateOneBlock());
		VerifyTxInBlock(strTxHash);
		strFirstTxHash = strTxHash;
		//cout << "first tx hash:" << strTxHash << endl;
	}

	void CreateSecondTx() {
		string strTxHash;
		BOOST_CHECK(ModifyAuthor(2, SELLER_ADDR,strTxHash));
		BOOST_CHECK(GenerateOneBlock());
		VerifyTxInBlock(strTxHash);

		string strReversFirstTxHash = GetReverseHash(strFirstTxHash);
		PacketNextContract(2, (unsigned char*) strReversFirstTxHash.c_str(), &secondContract);
		string strData = PutDataIntoString((char*) &secondContract, sizeof(secondContract));

		Value valueRes = CreateContractTxEx(strRegScriptID, VADDR_SELLER, strData, 0, 100000);
		BOOST_CHECK(GetHashFromCreatedTx(valueRes, strTxHash));
		BOOST_CHECK(GenerateOneBlock());
		VerifyTxInBlock(strTxHash);
		strSecondTxHash = strTxHash;
		//cout << "second tx hash:" << strTxHash << endl;
	}

	void CreateThirdTx() {
		string strTxHash;
		BOOST_CHECK(ModifyAuthor(1, ARBIT_ADDR,strTxHash));
		BOOST_CHECK(GenerateOneBlock());
		VerifyTxInBlock(strTxHash);

		string strReversFirstTxHash = GetReverseHash(strSecondTxHash);
		PacketNextContract(3, (unsigned char*) strReversFirstTxHash.c_str(), &thirdContract);
		string strData = PutDataIntoString((char*) &thirdContract, sizeof(thirdContract));

		Value valueRes = CreateContractTxEx(strRegScriptID, VADDR_ARBIT, strData, 0, 100000);
		BOOST_CHECK(GetHashFromCreatedTx(valueRes, strTxHash));
		BOOST_CHECK(GenerateOneBlock());
		VerifyTxInBlock(strTxHash);
		strThirdTxHash = strTxHash;
		//cout << "Third tx hash:" << strTxHash << endl;
	}

	void CreateLastTx() {
		string strTxHash;
		BOOST_CHECK(ModifyAuthor(3, ARBIT_ADDR,strTxHash));
		BOOST_CHECK(GenerateOneBlock());
		VerifyTxInBlock(strTxHash);

		string strReversFirstTxHash = GetReverseHash(strThirdTxHash);
		PacketLastContract((unsigned char*) strReversFirstTxHash.c_str(), 100000,&arContract);
		string strData = PutDataIntoString((char*) &arContract, sizeof(arContract));

		Value valueRes = CreateContractTxEx(strRegScriptID, VADDR_ARBIT, strData, 0, 100000);
		BOOST_CHECK(GetHashFromCreatedTx(valueRes, strTxHash));
		BOOST_CHECK(GenerateOneBlock());
		VerifyTxInBlock(strTxHash);
		//cout << "last tx hash:" << strTxHash << endl;
	}

protected:
	string 				strRegScriptID;
	string 				strFirstTxHash;
	string 				strSecondTxHash;
	string 				strThirdTxHash;
	FIRST_CONTRACT		firstConstract;
	NEXT_CONTRACT 		secondContract;
	NEXT_CONTRACT 		thirdContract;
	ARBIT_RES_CONTRACT 	arContract;
};

BOOST_FIXTURE_TEST_SUITE(sesuretrade,CSesureTrade)
BOOST_AUTO_TEST_CASE(test)
{
	while (1)
	{
		CreateFirstTx();

		CreateSecondTx();

		CreateThirdTx();

		CreateLastTx();
		break;
	}

}
BOOST_AUTO_TEST_SUITE_END()












