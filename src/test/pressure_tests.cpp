
#include <string>
#include <map>
#include <deque>
#include <math.h>
#include "tx.h"
#include "systestbase.h"
#include "miner.h"
#include "../json/json_spirit_value.h"
#include <boost/test/unit_test.hpp>
#include <boost/assign/list_of.hpp>
using namespace std;

const unsigned int g_kunTxCount = 6000;
vector<std::shared_ptr<CBaseTransaction> > g_vcTransactions;
vector<string> g_vstrTransactionHash;
deque<uint64_t> g_dFee;
deque<uint64_t> g_dFuel;
uint64_t llTotalFee(0);

map<string, string> g_mapAddress =
        boost::assign::map_list_of
        ("000000000100",	"dggsWmQ7jH46dgtA5dEZ9bhFSAK1LASALw")
        ("000000000200",	"dtKsuK9HUvLLHtBQL8Psk5fUnTLTFC83GS")
        ("000000000300",	"dejEcGCsBkwsZaiUH1hgMapjbJqPdPNV9U");

vector<std::tuple<int, uint64_t, string> > g_vFreezeItem;
vector<pair<string, uint64_t> > g_vSendFee;

std::string g_strRegScriptId("");

/**
 * 获取随机账户地址
 */
map<string, string>::iterator GetRandAddress() {
	// srand(time(NULL));
	unsigned char uchType;
	RAND_bytes(&uchType, sizeof(uchType));
	int nIndex = (uchType % 3);
	map<string, string>::iterator iterAddress = g_mapAddress.begin();
	while (nIndex--) {
		++iterAddress;
	}
	return iterAddress;
}

/**
 * 获取随机的交易类型
 */
int GetRandTxType() {
	unsigned char cType;
	RAND_bytes(&cType, sizeof(cType));
	// srand(time(NULL));
	int nIndex = cType % 4;
	return nIndex + 1;
}

class PressureTest : public SysTestBase {
 public:
	bool GetContractData(string strRegId, vector<unsigned char> &vuchContract) {
		for(auto &addr : g_mapAddress) {
			if (addr.first == strRegId) {
				continue;
			}
			uint64_t ullmoney = GetRandomMoney() * COIN;
			CRegID cReg(addr.first);
			vuchContract.insert(vuchContract.end(), cReg.GetVec6().begin(), cReg.GetVec6().end());
			CDataStream cDs(SER_DISK, g_sClientVersion);
			cDs << ullmoney;
			vector<unsigned char> vuchTemp(cDs.begin(), cDs.end());
			vuchContract.insert(vuchContract.end(), vuchTemp.begin(), vuchTemp.end());
		}
		return true;
	}

	bool InitRegScript() {
		ResetEnv();
		string strHash = "";
		BOOST_CHECK(CreateRegScriptTx(false, strHash,"doym966kgNUKr2M9P7CmjJeZdddqvoU5RZ"));
		BOOST_CHECK(SetBlockGenerte("doym966kgNUKr2M9P7CmjJeZdddqvoU5RZ"));
		BOOST_CHECK(GetTxConfirmedRegID(strHash,g_strRegScriptId));
		return true;
	}

	/**
	 * 创建普通交易
	 * @return
	 */
	bool CreateCommonTx(string srcAddr, string desAddr) {
		char arrchFee[64] = { 0 };
		int nFee = GetRandomFee();
		sprintf(arrchFee, "%d", nFee);
		char arrchMoney[64] = { 0 };
		int nMoney = GetRandomMoney();
		sprintf(arrchMoney, "%d00000000", nMoney);
		const char *argv[] =
				{ "rpctest", "sendtoaddresswithfee", srcAddr.c_str(), desAddr.c_str(), arrchMoney, arrchFee };
		int argc = sizeof(argv) / sizeof(char*);
		Value value;
		if (!CommandLineRPC_GetValue(argc, argv, value)) {
			return false;
		}
		const Value& result = find_value(value.get_obj(), "hash");
		if (result == null_type) {
			return false;
		}
		string strTxHash = result.get_str();
		g_vstrTransactionHash.push_back(strTxHash);
		if (g_cTxMemPool.m_mapTx.count(uint256(uint256S(strTxHash))) > 0) {
			std::shared_ptr<CBaseTransaction> cTx = g_cTxMemPool.m_mapTx[uint256(uint256S(strTxHash))].GetTx();
			g_vcTransactions.push_back(cTx);
		}
		g_vSendFee.push_back(make_pair(strTxHash, nFee));
		return true;
	}

	/**
	 * 创建注册账户交易
	 * @param addr
	 * @return
	 */
	bool CreateRegAcctTx() {
		//获取一个新的地址
		const char *argv[] = { "rpctest", "getnewaddress" };
		int argc = sizeof(argv) / sizeof(char *);
		Value value;
		if (!CommandLineRPC_GetValue(argc, argv, value)) {
			return false;
		}
		const Value& retNewAddr = find_value(value.get_obj(), "addr");
		if (retNewAddr.type() == null_type) {
			return false;
		}
		string strNewAddress = retNewAddr.get_str();
		map<string, string>::iterator mapIterSrcAddr = GetRandAddress();
		//向新产生地址发送一笔钱
		if (!CreateCommonTx(mapIterSrcAddr->second, strNewAddress)) {
			return false;
		}
		int nFee = GetRandomFee() + 100000000;
		Value result = RegistAccountTx(strNewAddress, nFee);
		string strTxHash = "";
		BOOST_CHECK(GetHashFromCreatedTx(value, strTxHash));

		g_vstrTransactionHash.push_back(strTxHash);
		if (g_cTxMemPool.m_mapTx.count(uint256(uint256S(strTxHash))) > 0) {
			std::shared_ptr<CBaseTransaction> tx = g_cTxMemPool.m_mapTx[uint256(uint256S(strTxHash))].GetTx();
			g_vcTransactions.push_back(tx);
		}
		g_vSendFee.push_back(make_pair(strTxHash, nFee));
		return true;
	}

	/**
	 * 创建合约交易
	 */
	bool CreateContractTx() {
		map<string, string>::iterator mapIterSrcAddr = GetRandAddress();
		string strSrcAddr(mapIterSrcAddr->second);

		unsigned char uchType;
		RAND_bytes(&uchType, sizeof(uchType));
		int nIndex = uchType % 2;
		nIndex += 20;
		string contact = strprintf("%02x",nIndex);

		int nFee = GetRandomFee() + 100000;
		uint64_t ullMoney = GetRandomMoney() * COIN;
		Value value = SysTestBase::CreateContractTx(g_strRegScriptId, strSrcAddr, contact, 0, nFee, ullMoney);
		string strTxHash = "";
		BOOST_CHECK(GetHashFromCreatedTx(value, strTxHash));

		g_vstrTransactionHash.push_back(strTxHash);
		if (g_cTxMemPool.m_mapTx.count(uint256(uint256S(strTxHash))) > 0) {
			std::shared_ptr<CBaseTransaction> tx = g_cTxMemPool.m_mapTx[uint256(uint256S(strTxHash))].GetTx();
			g_vcTransactions.push_back(tx);
		}
		g_vSendFee.push_back(make_pair(strTxHash, nFee));
		return true;
	}

	bool CreateRegScriptTx(bool bFlag, string &strHash, string strRegAddress = "") {
		if (strRegAddress == "") {
			map<string, string>::iterator mapIterSrcAddr = GetRandAddress();
			strRegAddress = mapIterSrcAddr->second;
		}
		uint64_t ullFee = GetRandomFee() + 1 * COIN;
		Value ret = RegisterAppTx(strRegAddress, "unit_test.bin", 100, ullFee);
		BOOST_CHECK(GetHashFromCreatedTx(ret, strHash));

		if (bFlag) {
			g_vstrTransactionHash.push_back(strHash);
			if (g_cTxMemPool.m_mapTx.count(uint256(uint256S(strHash))) > 0) {
				std::shared_ptr<CBaseTransaction> tx = g_cTxMemPool.m_mapTx[uint256(uint256S(strHash))].GetTx();
				g_vcTransactions.push_back(tx);
			}
			g_vSendFee.push_back(make_pair(strHash, ullFee));
		}

		return true;
	}

	//随机创建6000个交易
	void CreateRandTx(int nTxCount) {
		for (int i = 0; i < nTxCount; ++i) {
			int nTxType = GetRandTxType();
			switch (nTxType) {
			case 1: {
				--i;
				break;
			}
			case 2: {
				map<string, string>::iterator mapIterSrcAddr = GetRandAddress();
				map<string, string>::iterator iterDesAddr = GetRandAddress();
				while (iterDesAddr->first == mapIterSrcAddr->first) {
					iterDesAddr = GetRandAddress();
				}
				BOOST_CHECK(CreateCommonTx(mapIterSrcAddr->second, iterDesAddr->second));
			}
				break;
			case 3: {
				BOOST_CHECK(CreateContractTx());
			}
				break;
			case 4: {
				--i;
				break;
			}
			default:
				assert(0);
			}
			if (0 != i) {
				ShowProgress("create tx progress: ", (int) (((i + 1) / (float) nTxCount) * 100));
			}
		}
	}

	bool DetectionAccount(uint64_t ullFuelValue, uint64_t ullFees) {
		uint64_t ullFreeValue(0);
		uint64_t ullTotalValue(0);
		uint64_t ullScriptaccValue(0);
		for (auto & item : g_mapAddress) {
			CRegID cRegId(item.first);
			CUserID userId = cRegId;
			{
				LOCK(g_cs_main);
				CAccount cAccount;
				CAccountViewCache cAccView(*g_pAccountViewTip, true);
				if (!cAccView.GetAccount(userId, cAccount)) {
					return false;
				}
				ullFreeValue += cAccount.GetRawBalance();
			}
		}

		if (g_strRegScriptId != "") {
			CRegID cRegId(g_strRegScriptId);
			CUserID cUserId = cRegId;
			{
				LOCK(g_cs_main);
				CAccount cAccount;
				CAccountViewCache cAccView(*g_pAccountViewTip, true);
				if (!cAccView.GetAccount(cUserId, cAccount)) {
					return false;
				}
				ullScriptaccValue += cAccount.GetRawBalance();
			}
		}
		ullTotalValue += ullFreeValue;
		ullTotalValue += ullScriptaccValue;

		uint64_t ullTotalRewardValue(0);
		if (g_cChainActive.Tip()->m_nHeight - 1 > COINBASE_MATURITY) {
			ullTotalRewardValue = 10 * COIN * (g_cChainActive.Tip()->m_nHeight - 101);
		}
		g_dFee.push_back(ullFees);
		g_dFuel.push_back(ullFuelValue);
		llTotalFee += ullFees;

		if (g_dFee.size() > 100) {
			uint64_t ullFeeTemp = g_dFee.front();
			uint64_t ullFuelTemp = g_dFuel.front();
			llTotalFee -= ullFeeTemp;
			llTotalFee += ullFuelTemp;
			g_dFee.pop_front();
			g_dFuel.pop_front();
		}
		//检查总账平衡
		BOOST_CHECK(ullTotalValue + llTotalFee == (3000000000 * COIN + ullTotalRewardValue));
		return true;
	}

	bool SetBlockGenerte(const char *addr) {
		return SetAddrGenerteBlock(addr);

	}
};

BOOST_FIXTURE_TEST_SUITE(pressure_tests, PressureTest)
BOOST_FIXTURE_TEST_CASE(tests, PressureTest)
{
#if 0
	//初始化环境,注册一个合约脚本，并且挖矿确认
	InitRegScript();
//	BOOST_CHECK(DetectionAccount(1*COIN));
	for(int i=0; i<1; ++i) {
		//随机创建6000个交易

		CreateRandTx(g_kunTxCount);
		//检测mempool中是否有6000个交易
		BOOST_CHECK(g_vcTransactions.size()==g_kunTxCount);
		{
			//LOCK(pwalletMain->cs_wallet);
			//检测钱包未确认交易数量是否正确
			BOOST_CHECK(pwalletMain->UnConfirmTx.size() == g_kunTxCount);
			//检测钱包未确认交易hash是否正确
			for(auto &item : g_vstrTransactionHash) {
				BOOST_CHECK(pwalletMain->UnConfirmTx.count(uint256(uint256S(item))) > 0);
			}

		}

		unsigned int nSize = g_cTxMemPool.m_mapTx.size();
		int nConfirmTxCount(0);
		uint64_t llRegAcctFee(0);
		uint64_t llSendValue(0);
		uint64_t llFuelValue(0);
		while (nSize) {
			//挖矿
			map<string, string>::const_iterator iterAddr = GetRandAddress();
			BOOST_CHECK(SetBlockGenerte(iterAddr->second.c_str()));
			CBlock block;
			{
				LOCK(g_cs_main);
				CBlockIndex *pindex = g_cChainActive.Tip();
				llFuelValue += pindex->nFuel;
				BOOST_CHECK(ReadBlockFromDisk(block, pindex));
			}

			for(auto &item : block.vptx) {
				{
					LOCK2(g_cs_main, pwalletMain->cs_wallet);
					//检测钱包未确认列表中没有block中交易
					BOOST_CHECK(!pwalletMain->UnConfirmTx.count(item->GetHash()) > 0);
					//检测block中交易是否都在钱包已确认列表中
					BOOST_CHECK(pwalletMain->mapInBlockTx[block.GetHash()].mapAccountTx.count(item->GetHash())>0);
					//检测mempool中没有了block已确认交易
					BOOST_CHECK(!g_cTxMemPool.m_mapTx.count(item->GetHash()) > 0);
				}

			}
			{
				LOCK2(g_cs_main, pwalletMain->cs_wallet);
				//检测block中交易总数和钱包已确认列表中总数相等
				BOOST_CHECK(pwalletMain->mapInBlockTx[block.GetHash()].mapAccountTx.size() == block.vptx.size());
				nConfirmTxCount += block.vptx.size() - 1;
				//检测剩余mempool中交易总数与已确认交易和等于总的产生的交易数
				nSize = g_cTxMemPool.m_mapTx.size();
				BOOST_CHECK((nSize + nConfirmTxCount) == g_vcTransactions.size());
				//检测钱包中unconfirm交易和mempool中的相同
				BOOST_CHECK((nSize == pwalletMain->UnConfirmTx.size()));
			}
			//检测Block最大值
			BOOST_CHECK(block.GetSerializeSize(SER_DISK, g_sClientVersion) <= MAX_BLOCK_SIZE);
			for(auto & ptx : block.vptx) {
				if(ptx->IsCoinBase()) {
					continue;
				}
				if(REG_ACCT_TX == ptx->nTxType) {
					llRegAcctFee += ptx->GetFee();
				}
				if(EM_COMMON_TX == ptx->nTxType) {
					std::shared_ptr<CTransaction> pTransaction(dynamic_pointer_cast<CTransaction>(ptx));
					if(typeid(pTransaction->desUserId) == typeid(CKeyID)) {
						llSendValue += pTransaction->llValues;				}
				}
			}
			llSendValue -= llRegAcctFee;
			llSendValue += block.GetFee();
			//检测确认交易后总账是否平衡
			BOOST_CHECK(DetectionAccount(llFuelValue, block.GetFee()));
		}
		uint64_t totalFee(0);
		for(auto &item : g_vSendFee) {
			totalFee += item.second;
		}
		//校验总的手续费是否正确
		BOOST_CHECK(totalFee == llSendValue);

		g_vcTransactions.clear();
		g_vstrTransactionHash.clear();
		g_vSendFee.clear();
	}
#endif //0
}
BOOST_AUTO_TEST_SUITE_END()
