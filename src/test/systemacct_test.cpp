#include <boost/test/unit_test.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include "systestbase.h"
using namespace std;
using namespace boost;

class CSysAccountTest : public SysTestBase {
 public:
	bool RegisterAccount(const string& strAddr, uint64_t ullFee, string& strTxHash, bool bSign = false) {
		Value value = RegistAccountTx(strAddr, ullFee);
		return GetHashFromCreatedTx(value, strTxHash);
	}

	bool SendMoney(const string& strRegAddr, const string& strDestAddr, uint64_t ullMoney, uint64_t ullFee = 0) {
		Value value = CreateNormalTx(strRegAddr, strDestAddr, ullMoney);
		string strHash = "";
		return GetHashFromCreatedTx(value, strHash);
	}
};

BOOST_FIXTURE_TEST_SUITE(sysacct_test, CSysAccountTest)
BOOST_FIXTURE_TEST_CASE(transfer_test, CSysAccountTest) {
	ResetEnv();
	BOOST_CHECK(0 == g_cChainActive.Height());//如果有挖矿就是1
	//转账
	string strRegAddr = "dkJwhBs2P2SjbQWt5Bz6vzjqUhXTymvsGr";
	string strSrcRegID = "000000000400";
	uint64_t ullMoney = 10000000;
	BOOST_CHECK(SendMoney(strSrcRegID, strRegAddr, ullMoney));
	BOOST_CHECK(GenerateOneBlock());

	//确认转账成功
	uint64_t ullFreeMoney = GetBalance(strRegAddr);
	BOOST_CHECK(ullFreeMoney == ullMoney);

	//使用一个没有注册过的keyID地址测试，看是能够注册，用getaccountinfo 检查regid是否存在
	string strTxHash;
	CRegID cRegID;
	BOOST_CHECK(RegisterAccount(strRegAddr, 10000, strTxHash));
	BOOST_CHECK(GenerateOneBlock());
	BOOST_CHECK(GetRegID(strRegAddr, cRegID));
	BOOST_CHECK(false == cRegID.IsEmpty());

	//使用一个注册过的keyID地址测试，看是否能够注册成功
	string strResigterd("dps9hqUmBAVGVg7ijLGPcD9CJz9HHiTw6H");
	BOOST_CHECK(!RegisterAccount(strRegAddr, 10000, strTxHash));

	//用getnewaddress获取一个没有设置挖坑的公钥地址，注册账户最后一个参数是true，看此地址是否能够注册成功
	string strNewKeyID;
	BOOST_CHECK(GetNewAddr(strNewKeyID, true));

	//用一个注册的地址和一个未注册的地址来发一个交易，看是否能够发送成功
	string strUnRegister("dgZjR2S98gmdvXDzwKASxKiaGr9Dw1GD8F");
	vector<unsigned char> vuchRegID = cRegID.GetVec6();
	ullMoney = ullMoney / 10;
	BOOST_CHECK(SendMoney(HexStr(vuchRegID), strUnRegister, ullMoney, 10000));
	BOOST_CHECK(GenerateOneBlock());

	//确认转账成功
	ullFreeMoney = GetBalance(strUnRegister);
	BOOST_CHECK(ullFreeMoney == ullMoney);
}

BOOST_FIXTURE_TEST_CASE(register_test,CSysAccountTest) {
	ResetEnv();
	BOOST_CHECK(0 == g_cChainActive.Height());
	string strTxHash;
	CRegID cRegID;
	string strRegAddr1("dkJwhBs2P2SjbQWt5Bz6vzjqUhXTymvsGr");
	string strRegAddr2 = "";
	BOOST_CHECK(GetNewAddr(strRegAddr2, false));
	uint64_t ullFee = 10000;
	vector<string> vstrFailedTxHash;

	//余额不够
	BOOST_CHECK(!RegisterAccount(strRegAddr1, ullFee, strTxHash));
	vstrFailedTxHash.push_back(strTxHash);

	//无法读取的账号地址
	string strInvalidAddr("fjsofeoifdsfdsfdsafafafafafafafa");
	BOOST_CHECK(!RegisterAccount(strInvalidAddr, ullFee, strTxHash));
	vstrFailedTxHash.push_back(strTxHash);

//	//不签名
//	BOOST_CHECK(!RegisterAccount(strRegAddr1, nFee, strTxHash,nInValidHeight,false));
//	vFailedTxHash.push_back(strTxHash);

//手续费超过最大值
	BOOST_CHECK(!RegisterAccount(strRegAddr1, ullFee + GetMaxMoney(), strTxHash));
	vstrFailedTxHash.push_back(strTxHash);

	//重复注册的地址
	string strReRegisrerAddr("dtKsuK9HUvLLHtBQL8Psk5fUnTLTFC83GS");
	BOOST_CHECK(!RegisterAccount(strReRegisrerAddr, ullFee, strTxHash));
	vstrFailedTxHash.push_back(strTxHash);

	//账户上没有余额的未注册地址
	BOOST_CHECK(!RegisterAccount(strRegAddr2, ullFee, strTxHash));
	vstrFailedTxHash.push_back(strTxHash);

//	//检查失败的交易是否在memorypool中
//	for (const auto& item : vFailedTxHash) {
//		uint256 txHash(item);
//		BOOST_CHECK(!IsTxInMemorypool(txHash));
//		BOOST_CHECK(!IsTxUnConfirmdInWallet(txHash));
//	}

//给没有余额的账户转账
	string strSrcRegID = "000000000400";
	uint64_t ullMoney = 10000000;
	BOOST_CHECK(SendMoney(strSrcRegID, strRegAddr2, ullMoney));
	BOOST_CHECK(GenerateOneBlock());

	//确认转账成功
	uint64_t ullFreeMoney = GetBalance(strRegAddr2);
	BOOST_CHECK(ullFreeMoney == ullMoney);

	//再次检查失败的交易是否在memorypool中
	for (const auto& item : vstrFailedTxHash) {
		uint256 txHash(uint256S(item));
		BOOST_CHECK(!IsTxInMemorypool(txHash));
		BOOST_CHECK(!IsTxUnConfirmdInWallet(txHash));
	}

	//给地址为：dkJwhBs2P2SjbQWt5Bz6vzjqUhXTymvsGr 的账户转账
	BOOST_CHECK(SendMoney(strSrcRegID, strRegAddr1, ullMoney));
	BOOST_CHECK(GenerateOneBlock());

	//确认转账成功
	ullFreeMoney = GetBalance(strRegAddr1);
	BOOST_CHECK(ullFreeMoney == ullMoney);

	string strSpecial;
	BOOST_CHECK(RegisterAccount(strRegAddr1, ullFee, strSpecial, false));
	BOOST_CHECK(IsTxInMemorypool(uint256(uint256S(strSpecial))));
	BOOST_CHECK(IsTxUnConfirmdInWallet(uint256(uint256S(strSpecial))));

	//交易已经在memorypool中
	BOOST_CHECK(!RegisterAccount(strRegAddr1, ullFee, strTxHash, false));
	BOOST_CHECK(!IsTxInMemorypool(uint256(uint256S(strTxHash))));
	BOOST_CHECK(!IsTxUnConfirmdInWallet(uint256(uint256S(strTxHash))));

	BOOST_CHECK(GenerateOneBlock());

	//确认注册成功的交易在tip中
	BOOST_CHECK(IsTxInTipBlock(uint256(uint256S(strSpecial))));

	vector<CAccountLog> vcLog;
	BOOST_CHECK(GetTxOperateLog(uint256(uint256S(strSpecial)), vcLog));

	//检查日志记录是否正确
//	BOOST_CHECK(1 == vLog.size() && 1 == vLog[0].vOperFund.size() && 1 == vLog[0].vOperFund[0].vFund.size());
	BOOST_CHECK(strRegAddr1 == vcLog[0].m_cKeyID.ToAddress());
//	BOOST_CHECK(vLog[0].vOperFund[0].operType == MINUS_FREE && vLog[0].vOperFund[0].vFund[0].value == nFee);
}

BOOST_AUTO_TEST_SUITE_END()






