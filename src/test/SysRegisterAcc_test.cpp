#include <boost/test/unit_test.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include "SysTestBase.h"
using namespace std;
using namespace boost;

class CSysRegisterAccTest:public SysTestBase {
public:
	bool GetKeyId(string const &addr,CKeyID &KeyId) {
		if (!CRegID::GetKeyID(addr, KeyId)) {
			KeyId=CKeyID(addr);
			if (KeyId.IsEmpty())
			return false;
		}
		return true;
	};

	bool RegisterAccount(const string& strAddr, uint64_t nFee,string& strTxHash) {
		CKeyID keyid;
		if (!GetKeyId(strAddr, keyid))
			return false;

		CPubKey pubkey;
		if (!pwalletMain->GetPubKey(keyid, pubkey)) {
			return false;
		}

		CRegisterAccountTx rtx;
		CNullID nullId;
		rtx.minerId = nullId;

		rtx.userId = pubkey;
		rtx.llFees = nFee;
		rtx.nValidHeight = chainActive.Tip()->nHeight;

		CKey key;
		pwalletMain->GetKey(keyid, key);
		if (!key.Sign(rtx.SignatureHash(), rtx.signature)) {
			return false;
		}

		std::tuple<bool,string> ret = pwalletMain->CommitTransaction((CBaseTransaction *) &rtx);
		if (std::get<0>(ret)) {
			strTxHash = std::get<1>(ret);
		} else {
			strTxHash = rtx.GetHash().ToString();
		}

		return std::get<0>(ret);
	}

	bool SendMoney(const string& strRegAddr, const string& strDestAddr, uint64_t nMoney) {
		CKeyID keyid;
		if (!GetKeyId(strDestAddr, keyid))
			return false;
		std::tuple<bool, string> ret = pwalletMain->SendMoney(strRegAddr, CUserID(keyid), nMoney);
		return std::get<0>(ret);
	}

	bool GetAccountInfo(const string& strAddr,CAccount& account) {
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

	bool GetRegID(string& strAddr,CRegID& regID) {
		CAccount account;
		if (!GetAccountInfo(strAddr,account)) {
			return false;
		}

		regID = account.regID;
		return true;
	}

	bool GetNewAddress(bool IsForMiner,string& strNewKeyID)
	{
	    CKey  mCkey;
	    mCkey.MakeNewKey();

	    CKey  Minter;
	    Minter.MakeNewKey();

	    CPubKey newKey = mCkey.GetPubKey();
	    CKeyID keyID = newKey.GetKeyID();

		if (IsForMiner) {
			if (!pwalletMain->AddKey(mCkey, Minter))
				return false;
		}
		else if (!pwalletMain->AddKey(mCkey)) {
			return false;
		}

		strNewKeyID = keyID.ToAddress();
		return true;
	}

	bool IsTxInMemorypool(const uint256& txHash) {
		for (const auto& entry : mempool.mapTx) {
			if (entry.first == txHash)
				return true;
		}

		return false;
	}

	bool GenerateBlock(const string& strKeyID) {
		int nOldBlockHeight = GetBlockHeight();

		CKeyID keyID(strKeyID);
		uint256 hash = CreateBlockWithAppointedAddr(keyID);

		int nNewBlockHeight = GetBlockHeight();
		if (nNewBlockHeight != nOldBlockHeight + 1) {
			return false;
		}
		return 0 != hash;
	}

};

BOOST_FIXTURE_TEST_SUITE(sysregisteracc_test,CSysRegisterAccTest)
BOOST_FIXTURE_TEST_CASE(rpc_test,CSysRegisterAccTest)
{
	//转账
	string strRegAddr = "mo51PMpnadiFx5JcZaeUdWBa4ngLBVgoGz";
	string strSrcRegID = "000000000400";
	uint64_t nMoney = 100000;
	BOOST_CHECK(SendMoney(strSrcRegID,strRegAddr,nMoney));
	BOOST_CHECK(GenerateOneBlock());

	//确认转账成功
	uint64_t nFreeMoney = GetFreeMoney(strRegAddr);
	BOOST_CHECK(nFreeMoney == nMoney);

	//使用一个没有注册过的keyID地址测试，看是能够注册，用getaccountinfo 检查regid是否存在
	string strTxHash;
	CRegID regID;
	BOOST_CHECK(RegisterAccount(strRegAddr,10000,strTxHash));
	BOOST_CHECK(GenerateOneBlock());
	BOOST_CHECK(GetRegID(strRegAddr,regID));
	BOOST_CHECK(false == regID.IsEmpty());

	//使用一个注册过的keyID地址测试，看是否能够注册成功
	string strResigterd("msdDQ1SXNmknrLuTDivmJiavu5J9VyX9fV");
	BOOST_CHECK(!RegisterAccount(strRegAddr,10000,strTxHash));

	//用getnewaddress获取一个没有设置挖坑的公钥地址，注册账户最后一个参数是true，看此地址是否能够注册成功
	string strNewKeyID;
	BOOST_CHECK(GetNewAddress(true,strNewKeyID));

	//用一个注册的地址和一个未注册的地址来发一个交易，看是否能够发送成功
	string strUnRegister("mydRNvqewpZt9tyNtBSmBCrKr1NTiii5JH");
	vector<unsigned char> vRegID = regID.GetVec6();
	nMoney = nMoney/10;
	BOOST_CHECK(SendMoney(HexStr(vRegID),strUnRegister,nMoney));
	BOOST_CHECK(GenerateOneBlock());

	//确认转账成功
	nFreeMoney = GetFreeMoney(strUnRegister);
	BOOST_CHECK(nFreeMoney == nMoney);
}

BOOST_FIXTURE_TEST_CASE(sysonly_test,CSysRegisterAccTest)
 {
	string strTxHash;
	CRegID regID;
	string strRegAddr1("mo51PMpnadiFx5JcZaeUdWBa4ngLBVgoGz");
	string strRegAddr2("mydRNvqewpZt9tyNtBSmBCrKr1NTiii5JH");
	int nInValidHeight = 1000;
	BOOST_CHECK(RegisterAccount(strRegAddr1, nInValidHeight, strTxHash));

	BOOST_CHECK(IsTxInMemorypool(uint256(strTxHash)));





//	string strKeyID("mvVp2PDRuG4JJh6UjkJFzXUC8K5JVbMFFA");
//	CKeyID keyID(strKeyID);
//	CRegID regID(0,9);
//	CUserID userId = keyID;
//
//	CAccount account;
//	CAccountViewCache accView(*pAccountViewTip, true);
//
//	BOOST_CHECK(accView.GetAccount(userId, account));
//	string strScript("000000000400");
//	vector_unsigned_char vScript = ParseHex(strScript);
//	BOOST_CHECK(account.OperateAccount(ADD_FREEZD,CFund(FREEZD_FUND,10000,2,vScript)));
//	BOOST_CHECK(accView.SaveAccountInfo(regID,strKeyID,account));
//	BOOST_CHECK(accView.Flush());
//	BOOST_CHECK(pAccountViewTip->Flush());
}

BOOST_FIXTURE_TEST_CASE(sysonly_test1,CSysRegisterAccTest)
{
	string strKeyID("mvVp2PDRuG4JJh6UjkJFzXUC8K5JVbMFFA");
	BOOST_CHECK(GenerateBlock(strKeyID));
}
BOOST_AUTO_TEST_SUITE_END()






