#include <boost/test/unit_test.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include "../test/SysTestBase.h"
#include "CycleSesureTrade_tests.h"
using namespace std;
using namespace boost;

class CSesureTrade:public CSesureTradeHelp
{
public:
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

	void CreateFirstTx() {
		string strTxHash;
		Value valueRes = RegisterScriptTx(BUYER_ADDR, "SecuredTrade.bin", 0, 100000);
		BOOST_CHECK(GetHashFromCreatedTx(valueRes, strTxHash));
		BOOST_CHECK(GenerateOneBlock());
		VerifyTxInBlock(strTxHash,true);
		BOOST_CHECK(GetTxConfirmedRegID(strTxHash, strRegScriptID));

		BOOST_CHECK(ModifyAuthor(1, BUYER_ADDR,strTxHash));
		BOOST_CHECK(GenerateOneBlock());
		VerifyTxInBlock(strTxHash,true);

		FIRST_TRADE_CONTRACT firstConstract;
		PacketFirstContract(BUYER_ID, SELLER_ID, ARBIT_ID, 200, 100000, 100000, 100000, 100000, &firstConstract);
		string strData = PutDataIntoString((char*) &firstConstract,sizeof(firstConstract));
		valueRes = CreateContractTxEx(strRegScriptID, VADDR_BUYER, strData, 0, 100000);
		BOOST_CHECK(GetHashFromCreatedTx(valueRes, strTxHash));
		BOOST_CHECK(GenerateOneBlock());
		VerifyTxInBlock(strTxHash,true);
		strFirstTxHash = strTxHash;
		//cout << "first tx hash:" << strTxHash << endl;
	}

	void CreateSecondTx() {
		string strTxHash;
		BOOST_CHECK(ModifyAuthor(2, SELLER_ADDR,strTxHash));
		BOOST_CHECK(GenerateOneBlock());
		VerifyTxInBlock(strTxHash,true);

		string strReversFirstTxHash = GetReverseHash(strFirstTxHash);
		NEXT_TRADE_CONTRACT secondContract;
		PacketNextContract(2, (unsigned char*) strReversFirstTxHash.c_str(), &secondContract);
		string strData = PutDataIntoString((char*) &secondContract, sizeof(secondContract));

		Value valueRes = CreateContractTxEx(strRegScriptID, VADDR_SELLER, strData, 0, 100000);
		BOOST_CHECK(GetHashFromCreatedTx(valueRes, strTxHash));
		BOOST_CHECK(GenerateOneBlock());
		VerifyTxInBlock(strTxHash,true);
		strSecondTxHash = strTxHash;
		//cout << "second tx hash:" << strTxHash << endl;
	}

	void CreateThirdTx() {
		string strTxHash;
		BOOST_CHECK(ModifyAuthor(1, ARBIT_ADDR,strTxHash));
		BOOST_CHECK(GenerateOneBlock());
		VerifyTxInBlock(strTxHash,true);

		string strReversFirstTxHash = GetReverseHash(strSecondTxHash);
		NEXT_TRADE_CONTRACT thirdContract;
		PacketNextContract(3, (unsigned char*) strReversFirstTxHash.c_str(), &thirdContract);
		string strData = PutDataIntoString((char*) &thirdContract, sizeof(thirdContract));

		Value valueRes = CreateContractTxEx(strRegScriptID, VADDR_ARBIT, strData, 0, 100000);
		BOOST_CHECK(GetHashFromCreatedTx(valueRes, strTxHash));
		BOOST_CHECK(GenerateOneBlock());
		VerifyTxInBlock(strTxHash,true);
		strThirdTxHash = strTxHash;
		//cout << "Third tx hash:" << strTxHash << endl;
	}

	void CreateLastTx() {
		string strTxHash;
		BOOST_CHECK(ModifyAuthor(3, ARBIT_ADDR,strTxHash));
		BOOST_CHECK(GenerateOneBlock());
		VerifyTxInBlock(strTxHash,true);

		string strReversFirstTxHash = GetReverseHash(strThirdTxHash);
		ARBIT_RES_CONTRACT 	arContract;
		PacketLastContract((unsigned char*) strReversFirstTxHash.c_str(), 100000,&arContract);
		string strData = PutDataIntoString((char*) &arContract, sizeof(arContract));

		Value valueRes = CreateContractTxEx(strRegScriptID, VADDR_ARBIT, strData, 0, 100000);
		BOOST_CHECK(GetHashFromCreatedTx(valueRes, strTxHash));
		BOOST_CHECK(GenerateOneBlock());
		VerifyTxInBlock(strTxHash,true);
		//cout << "last tx hash:" << strTxHash << endl;
	}

protected:
	string 				strFirstTxHash;
	string 				strSecondTxHash;
	string 				strThirdTxHash;
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












