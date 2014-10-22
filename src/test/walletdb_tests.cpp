#include "walletdb.h"
#include "wallet.h"
#include <boost/foreach.hpp>
#include <boost/test/unit_test.hpp>

#define TEST_SIZE 10000
void GenerateRandomHash(vector<uint256>& vHashKey, vector<uint256>& vHashValue) {
	vHashKey.reserve(TEST_SIZE);
	vHashValue.reserve(TEST_SIZE);
	vector<unsigned char> vRandom;
	vRandom.reserve(32);

	//generate key hash
	for (int i = 0; i < TEST_SIZE; i++) {
		CKey key;
		key.MakeNewKey(false);
		string str(key.begin(), key.end());
		vRandom.clear();
		vRandom.insert(vRandom.begin(), key.begin(), key.end());
		uint256 hash(vRandom);
		vHashValue.push_back(hash);
		//cout << hash.GetHex() << endl;
	}

	//generate value hash
	for (int j = 0; j < TEST_SIZE; j++) {
		CKey key;
		key.MakeNewKey(false);
		vRandom.clear();
		vRandom.insert(vRandom.begin(), key.begin(), key.end());
		uint256 hash(vRandom);
		vHashKey.push_back(hash);
	}
}

bool WriteAccountTx(vector<uint256>& vHashKey, vector<uint256>& vHashValue, CWalletDB& wdb) {
	for (int i = 0; i < TEST_SIZE; i++) {
		CAccountTx tx(NULL, vHashValue.at(i));
		if (!wdb.WriteAccountTx(vHashKey.at(i), tx)) {
			return false;
		}
	}

	return true;
}

bool CheckAccountTx(vector<uint256>& vHashValue, CWalletDB& wdb, bool bCheckAfterWrite) {
	//read from db
	CWallet wallet;
	DBErrors nDBError = wdb.LoadWallet(&wallet);
	if (DB_LOAD_OK != nDBError) {
		//cout << "LoadWallet failed!" << endl;
		return false;
	}

	//check
	const CAccountTx* pTx = NULL;
	for (int i = 0; i < TEST_SIZE; i++) {
		pTx = wallet.GetAccountTx(vHashValue.at(i));
		if (bCheckAfterWrite) {
			if (NULL == pTx) {
				return false;
			}
		} else {
			if (NULL != pTx) {
				return false;
			}
		}
	}

	return true;
}

bool EraseAccountTx(vector<uint256>& vHashKey, CWalletDB& wdb) {
	for (int i = 0; i < TEST_SIZE; i++) {
		if (!wdb.EraseAccountTx(vHashKey.at(i))) {
			return false;
		}
	}

	return true;
}

BOOST_AUTO_TEST_SUITE(walletdb_tests)

#if 0

BOOST_AUTO_TEST_CASE(walletdb_write_tests)
{
	vector<uint256> vHashKey;
	vector<uint256> vHashValue;
	GenerateRandomHash(vHashKey,vHashValue);

	//write AccountTx
	CWalletDB wdb("walletdb.dat","cr+");
	BOOST_CHECK(WriteAccountTx(vHashKey,vHashValue,wdb) );

	//check
	BOOST_CHECK(CheckAccountTx(vHashValue,wdb,true) );

	//erase AccountTx
	BOOST_CHECK(EraseAccountTx(vHashKey,wdb) );

	//check
	BOOST_CHECK(CheckAccountTx(vHashValue, wdb, false) );

	//cout<<"WalletDB Check Success!"<<endl;

	//clean
	bitdb.CloseDb("walletdb.dat");
	bitdb.Close();
}

#else
BOOST_AUTO_TEST_CASE(xxxx) {
	BOOST_ERROR("ERROR:THE SUITE NEED TO MODIFY!");
}
#endif

BOOST_AUTO_TEST_SUITE_END()
