
#include <string>
#include <map>
#include <math.h>
#include "tx.h"
#include "systestbase.h"
#include "miner.h"
#include "../json/json_spirit_value.h"
#include <boost/test/unit_test.hpp>
#include <boost/assign/list_of.hpp>
using namespace std;

const unsigned int iTxCount = 6000;
vector<std::shared_ptr<CBaseTransaction> > vTransactions;
vector<string> vTransactionHash;

map<string, string> mapAddress =
        boost::assign::map_list_of
        ("000000000100",	"mjSwCwMsvtKczMfta1tvr78z2FTsZA1JKw")
        ("000000000200",	"mw5wbV73gXbreYy8pX4FSb7DNYVKU3LENc")
        ("000000000300",	"mhVJJSAdPNDPvFWCmQN446GUBPzFm8aN4y");

vector<std::tuple<int, uint64_t, string> > vFreezeItem;
vector<pair<string, uint64_t> > vSendFee;

std::string regScriptId("");

/**
 * 获取随机账户地址
 */
map<string, string>::iterator GetRandAddress() {
//	srand(time(NULL));
	unsigned char cType;
	RAND_bytes(&cType, sizeof(cType));
	int iIndex = (cType % 3);
	map<string, string>::iterator iterAddress = mapAddress.begin();
	while(iIndex--) {
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
	//srand(time(NULL));
	int iIndex = cType % 4;
	return iIndex + 1;
}


class PressureTest: public SysTestBase {
public:
	bool GetContractData(string regId, vector<unsigned char> &vContract) {
		for(auto &addr : mapAddress) {
			if(addr.first == regId)
				continue;

			uint64_t llmoney = GetRandomMoney() * COIN;
			CRegID reg(addr.first);
			vContract.insert(vContract.end(), reg.GetVec6().begin(), reg.GetVec6().end());
			CDataStream ds(SER_DISK, CLIENT_VERSION);
			ds << llmoney;
			vector<unsigned char> temp(ds.begin(), ds.end());
			vContract.insert(vContract.end(), temp.begin(), temp.end());
		}
		return true;
	}

	bool InitRegScript() {
		ResetEnv();
		string hash = "";
		BOOST_CHECK(CreateRegScriptTx(false, hash,"mrjpqG4WsyjrCh8ssVs9Rp6JDini8suA7v"));
		BOOST_CHECK(SetBlockGenerte("mrjpqG4WsyjrCh8ssVs9Rp6JDini8suA7v"));
		BOOST_CHECK(GetTxConfirmedRegID(hash,regScriptId));
		return true;
	}

	/**
	 * 创建普通交易
	 * @return
	 */
	bool CreateCommonTx(string srcAddr, string desAddr) {
		char fee[64] = {0};
		int nfee = GetRandomFee();
		sprintf(fee, "%d", nfee);
//		char *argvFee[] = {"rpctest", "settxfee", fee};
//		int argcFee = sizeof(argvFee) / sizeof(char*);
//		Value value;
//		if (!CommandLineRPC_GetValue(argcFee, argvFee, value)) {
//			return false;
//		}
		char money[64] = {0};
		int nmoney = GetRandomMoney();
		sprintf(money, "%d00000000", nmoney);
		const char *argv[] = { "rpctest", "sendtoaddresswithfee", srcAddr.c_str(), desAddr.c_str(), money, fee};
		int argc = sizeof(argv) / sizeof(char*);
		Value value;
		if (!CommandLineRPC_GetValue(argc, argv, value)) {
//			cout << "CreateCommonTx failed!" << endl;
			return false;
		}
		const Value& result = find_value(value.get_obj(), "hash");
		if(result == null_type) {
//			cout << "CreateCommonTx failed!" << endl;
			return false;
		}
		string txHash = result.get_str();
		vTransactionHash.push_back(txHash);
		if (mempool.mapTx.count(uint256(txHash)) > 0) {
			std::shared_ptr<CBaseTransaction> tx = mempool.mapTx[uint256(txHash)].GetTx();
			vTransactions.push_back(tx);
		}
		vSendFee.push_back(make_pair(txHash, nfee));
		return true;
	}
	/**
	 * 创建注册账户交易
	 * @param addr
	 * @return
	 */
	bool CreateRegAcctTx() {
		//获取一个新的地址
		const char *argv[] = {"rpctest", "getnewaddress"};
		int argc = sizeof(argv) /sizeof(char *);
		Value value;
		if(!CommandLineRPC_GetValue(argc, argv, value))
		{
			return false;
		}
		const Value& retNewAddr = find_value(value.get_obj(), "addr");
		if(retNewAddr.type() == null_type) {
			return false;
		}
		string newAddress = retNewAddr.get_str();
		map<string, string>::iterator iterSrcAddr = GetRandAddress();
		//向新产生地址发送一笔钱
		if(!CreateCommonTx(iterSrcAddr->second, newAddress))
			return false;

		int nfee = GetRandomFee() + 100000000;
		Value result = registaccounttx(newAddress, nfee);
		string txHash = "";
		BOOST_CHECK(GetHashFromCreatedTx(value, txHash));

		vTransactionHash.push_back(txHash);
		if (mempool.mapTx.count(uint256(txHash)) > 0) {
			std::shared_ptr<CBaseTransaction> tx = mempool.mapTx[uint256(txHash)].GetTx();
			vTransactions.push_back(tx);
		}
		vSendFee.push_back(make_pair(txHash, nfee));
		return true;
	}

	/**
	 * 创建合约交易
	 */
	bool CreateContractTx() {
		map<string, string>::iterator iterSrcAddr = GetRandAddress();
		string srcAddr(iterSrcAddr->second);

		unsigned char cType;
		RAND_bytes(&cType, sizeof(cType));
		int iIndex = cType % 2;
		iIndex +=20;
		string contact =strprintf("%02x",iIndex);

		int nfee = GetRandomFee();
		uint64_t llmoney = GetRandomMoney() * COIN;
		Value value = SysTestBase::CreateContractTx(regScriptId,srcAddr, contact,0,nfee, llmoney);
		string txHash = "";
		BOOST_CHECK(GetHashFromCreatedTx(value,txHash));

		vTransactionHash.push_back(txHash);
		if (mempool.mapTx.count(uint256(txHash)) > 0) {
			std::shared_ptr<CBaseTransaction> tx = mempool.mapTx[uint256(txHash)].GetTx();
			vTransactions.push_back(tx);
		}
		vSendFee.push_back(make_pair(txHash, nfee));
		return true;
	}

	bool CreateRegScriptTx(bool fFlag, string &hash,string regAddress="") {
		if(regAddress=="") {
			map<string, string>::iterator iterSrcAddr = GetRandAddress();
			regAddress = iterSrcAddr->second;
		}
		uint64_t nFee = GetRandomFee() + 1 * COIN;
		Value ret = RegisterScriptTx(regAddress, "unit_test.bin", 100, nFee);
		BOOST_CHECK(GetHashFromCreatedTx(ret,hash));

		if(fFlag) {
			vTransactionHash.push_back(hash);
			if (mempool.mapTx.count(uint256(hash)) > 0) {
				std::shared_ptr<CBaseTransaction> tx = mempool.mapTx[uint256(hash)].GetTx();
				vTransactions.push_back(tx);
			}
			vSendFee.push_back(make_pair(hash, nFee));
		}

		return true;
	}

	//随机创建6000个交易
	void CreateRandTx(int txCount) {
		for (int i = 0; i < txCount; ++i) {
			int nTxType = GetRandTxType();
			switch(nTxType) {
			case 1:
//				{
//					BOOST_CHECK(CreateRegAcctTx());
//					++i;
//				}
				--i;
				break;
			case 2:
				{
					map<string, string>::iterator iterSrcAddr = GetRandAddress();
					map<string, string>::iterator iterDesAddr = GetRandAddress();
					while(iterDesAddr->first == iterSrcAddr->first) {
						iterDesAddr = GetRandAddress();
					}
					BOOST_CHECK(CreateCommonTx(iterSrcAddr->second, iterDesAddr->second));
				}
				break;
			case 3:
				{
					BOOST_CHECK(CreateContractTx());
				}
				break;
			case 4:
//				{
//					string hash ="";
//					BOOST_CHECK(CreateRegScriptTx(true,hash));
//				}
				--i;
				break;
			default:
				assert(0);
			}
//			cout << "create tx order:" << i << "type:" <<nTxType << endl;
			//putchar('\b');	//将当前行全部清空，用以显示最新的进度条状态
			if(0 != i) {
				ShowProgress("create tx progress: ",(int)(((i+1)/(float)txCount) * 100));
			}
		}
	}

	bool DetectionAccount(uint64_t llFuelValue) {
		uint64_t rewardAmount(0);
		uint64_t freeValue(0);
		uint64_t totalValue(0);
		uint64_t scriptaccValue(0);
		for(auto & item : mapAddress) {
			CRegID regId(item.first);
			CUserID userId = regId;
			{
				LOCK(cs_main);
				CAccount account;
				CAccountViewCache accView(*pAccountViewTip, true);
				if (!accView.GetAccount(userId, account)) {
					return false;
				}
				rewardAmount += account.GetRewardAmount(chainActive.Tip()->nHeight);
				freeValue += account.GetRawBalance(chainActive.Tip()->nHeight);
			}

		}

		if(regScriptId != ""){
			CRegID regId(regScriptId);
			CUserID userId = regId;
			{
				LOCK(cs_main);
				CAccount account;
				CAccountViewCache accView(*pAccountViewTip, true);
				if (!accView.GetAccount(userId, account)) {
					return false;
				}
				scriptaccValue += account.GetRewardAmount(chainActive.Tip()->nHeight);
				scriptaccValue += account.GetRawBalance(chainActive.Tip()->nHeight);
			}

		}
		totalValue += rewardAmount;
		totalValue += freeValue;
		totalValue += scriptaccValue;
		uint64_t uTotalRewardValue = 10 * COIN * (chainActive.Tip()->nHeight-1);
		//检查总账平衡
		BOOST_CHECK(totalValue + llFuelValue == (30000000 * COIN + uTotalRewardValue));

		return true;
	}

	bool SetBlockGenerte(const char *addr)
	{
		return SetAddrGenerteBlock(addr);

	}
};

BOOST_FIXTURE_TEST_SUITE(pressure_tests, PressureTest)
BOOST_FIXTURE_TEST_CASE(tests, PressureTest)
{
	//初始化环境,注册一个合约脚本，并且挖矿确认
	InitRegScript();
//	BOOST_CHECK(DetectionAccount(1*COIN));
	//随机创建6000个交易
	CreateRandTx(iTxCount);
	cout << endl;
//	cout << "vTransactions size:" << vTransactions.size() << endl;
	//检测mempool中是否有6000个交易
	BOOST_CHECK(vTransactions.size()==iTxCount);
	{
		//LOCK(pwalletMain->cs_wallet);
		//检测钱包未确认交易数量是否正确
		BOOST_CHECK(pwalletMain->UnConfirmTx.size() == iTxCount);
		//检测钱包未确认交易hash是否正确
		for(auto &item : vTransactionHash) {
			BOOST_CHECK(pwalletMain->UnConfirmTx.count(uint256(item)) > 0);
		}
	}
	unsigned int nSize = mempool.mapTx.size();
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
			LOCK(cs_main);
			CBlockIndex *pindex = chainActive.Tip();
			llFuelValue += pindex->nFuel;
			BOOST_CHECK(ReadBlockFromDisk(block, pindex));
		}

		for(auto &item : block.vptx) {
			{
				LOCK2(cs_main, pwalletMain->cs_wallet);
				//检测钱包未确认列表中没有block中交易
				BOOST_CHECK(!pwalletMain->UnConfirmTx.count(item->GetHash()) > 0);
				//检测block中交易是否都在钱包已确认列表中
				BOOST_CHECK(pwalletMain->mapInBlockTx[block.GetHash()].mapAccountTx.count(item->GetHash())>0);
				//检测mempool中没有了block已确认交易
				BOOST_CHECK(!mempool.mapTx.count(item->GetHash()) > 0);
			}

		}
		{
			LOCK2(cs_main, pwalletMain->cs_wallet);
			//检测block中交易总数和钱包已确认列表中总数相等
			BOOST_CHECK(pwalletMain->mapInBlockTx[block.GetHash()].mapAccountTx.size() == block.vptx.size());
			nConfirmTxCount += block.vptx.size() - 1;
			//检测剩余mempool中交易总数与已确认交易和等于总的产生的交易数
			nSize = mempool.mapTx.size();
			BOOST_CHECK((nSize + nConfirmTxCount) == vTransactions.size());
			//检测钱包中unconfirm交易和mempool中的相同
			BOOST_CHECK((nSize == pwalletMain->UnConfirmTx.size()));
		}
		//检测Block最大值
		BOOST_CHECK(block.GetSerializeSize(SER_DISK, CLIENT_VERSION) <= MAX_BLOCK_SIZE);

//		std::stable_sort(vptxSort.begin()+1, vptxSort.end(), [](const std::shared_ptr<CBaseTransaction> &pTx1, const std::shared_ptr<CBaseTransaction> &pTx2) {
//					if ((pTx1->GetFee()/pTx1->GetSerializeSize(SER_DISK, CLIENT_VERSION)) > (pTx2->GetFee()/pTx2->GetSerializeSize(SER_DISK, CLIENT_VERSION)))
//						return true;
//					return false;
//					});
//		BOOST_CHECK(vptxSort == block.vptx);
		for(auto & ptx : block.vptx) {
			if(ptx->IsCoinBase()) {
				continue;
			}
			if(REG_ACCT_TX == ptx->nTxType) {
				llRegAcctFee += ptx->GetFee();
			}
			if(COMMON_TX == ptx->nTxType) {
				std::shared_ptr<CTransaction> pTransaction(dynamic_pointer_cast<CTransaction>(ptx));
				if(typeid(pTransaction->desUserId) == typeid(CKeyID)) {
					llSendValue += pTransaction->llValues;				}
			}
		}
		llSendValue -= llRegAcctFee;
		llSendValue += block.GetFee();
//		cout <<"llSendValue:" << llSendValue << endl;
		//检测确认交易后总账是否平衡
		BOOST_CHECK(DetectionAccount(llFuelValue));
	}
	uint64_t totalFee(0);
	for(auto &item : vSendFee) {
		totalFee += item.second;
	}
	//校验总的手续费是否正确
	BOOST_CHECK(totalFee == llSendValue);


}
BOOST_AUTO_TEST_SUITE_END()
