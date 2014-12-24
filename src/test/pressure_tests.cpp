
#include <string>
#include <map>
#include <math.h>
#include "tx.h"
#include "SysTestBase.h"
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
const int GetRandTxType() {
	unsigned char cType;
	RAND_bytes(&cType, sizeof(cType));
	//srand(time(NULL));
	int iIndex = cType % 5;
	return iIndex + 1;
}


class PressureTest: public SysTestBase {
public:
	bool GetContractData(string regId, vector<unsigned char> &vContract) {
		char money[64] = {0};
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
		BOOST_CHECK(CreateRegScriptTx(false, "mrjpqG4WsyjrCh8ssVs9Rp6JDini8suA7v"));
		BOOST_CHECK(SetBlockGenerte("mrjpqG4WsyjrCh8ssVs9Rp6JDini8suA7v"));
		BOOST_CHECK(GetOneScriptId(regScriptId));
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
		char *argv[] = { "rpctest", "sendtoaddresswithfee", const_cast<char *>(srcAddr.c_str()), const_cast<char *>(desAddr.c_str()), money, fee};
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
		char *argv[] = {"rpctest", "getnewaddress"};
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

		char fee[64] = {0};
		int nfee = GetRandomFee();
		sprintf(fee, "%d", nfee);

		//新产生地址注册账户
		char *argvReg[] = {"rpctest", "registeraccounttx", const_cast<char *>(newAddress.c_str()), fee, "false"};
		int argcReg = sizeof(argvReg) / sizeof(char *);
		if(!CommandLineRPC_GetValue(argcReg, argvReg, value))
		{
			return false;
		}
		const Value& result = find_value(value.get_obj(), "hash");
		if(result.type() == null_type) {
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
	 * 创建合约交易
	 */
	bool CreateContractTx() {
		map<string, string>::iterator iterSrcAddr = GetRandAddress();
		string srcAddr("[\""+iterSrcAddr->second+"\"]");
		vector<unsigned char> vContract;
		vContract.clear();
		if(!GetContractData(iterSrcAddr->first, vContract))
			return false;

		char fee[64] = {0};
		int nfee = GetRandomFee();
		sprintf(fee, "%d", nfee);
		char *argv[] = { "rpctest", "createcontracttx", const_cast<char *>(regScriptId.c_str()), const_cast<char *>(srcAddr.c_str()), const_cast<char *>(HexStr(vContract).c_str()), fee, "0"};
		int argc = sizeof(argv) / sizeof(char *);
		Value value;
		if (!CommandLineRPC_GetValue(argc, argv, value)) {
//			cout << "CreateContractTx failed!" << endl;
			return false;
		}
		const Value& result = find_value(value.get_obj(), "hash");
		if(result == null_type) {
//			cout << "CreateContractTx failed!" << endl;
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
	 * 创建主动冻结交易
	 * @return
	 */
	bool CreateFreezeTx() {
		map<string, string>::iterator iterAddr = GetRandAddress();
		string addr = iterAddr->second;
		char money[64] = {0};
		int frozenmoney = GetRandomMoney();
		sprintf(money, "%d00000000", frozenmoney);
		char fee[64] = {0};
		int nfee = GetRandomFee();
		sprintf(fee, "%d", nfee);
//		srand(time(NULL));
		int freeHeight = rand() % 90 + 10;
		char free[64] = {0};
		sprintf(free, "%d", freeHeight);
//		cout << "freeHeight:" << freeHeight << endl;
		char *argv[] = {"rpctest", "createfreezetx", const_cast<char *>(addr.c_str()), money, fee, "100", free};
		int argc = sizeof(argv) / sizeof(char *);
		Value value;
		if (!CommandLineRPC_GetValue(argc, argv, value)) {
//			cout << "CreateFreezeTx failed!" << endl;
			return false;
		}
		const Value& result = find_value(value.get_obj(), "hash");
		if(result.type() == null_type) {
//			cout << "CreateFreezeTx failed!" << endl;
			return false;
		}
		string txHash = result.get_str();
		vTransactionHash.push_back(txHash);
		if (mempool.mapTx.count(uint256(txHash)) > 0) {
			std::shared_ptr<CBaseTransaction> tx = mempool.mapTx[uint256(txHash)].GetTx();
			vTransactions.push_back(tx);
		}
		uint64_t uMoney = frozenmoney * COIN;
		vFreezeItem.push_back(make_tuple(freeHeight, uMoney, txHash));
		vSendFee.push_back(make_pair(txHash, nfee));
		return true;
	}

	bool CreateRegScriptTx(bool fFlag, string regAddress="") {
		if(regAddress=="") {
			map<string, string>::iterator iterSrcAddr = GetRandAddress();
			regAddress = iterSrcAddr->second;
		}
		uint64_t nFee = GetRandomFee();
		Value ret = RegisterScriptTx(regAddress, "test.bin", 100, nFee);
		const Value& result = find_value(ret.get_obj(), "hash");
		if(result.type() == null_type) {
//			cout << "CreateRegScriptTx failed!" << endl;
			return false;
		}
		if(fFlag) {
			string txHash = result.get_str();
			vTransactionHash.push_back(txHash);
			if (mempool.mapTx.count(uint256(txHash)) > 0) {
				std::shared_ptr<CBaseTransaction> tx = mempool.mapTx[uint256(txHash)].GetTx();
				vTransactions.push_back(tx);
			}
			vSendFee.push_back(make_pair(txHash, nFee));
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
				{
					BOOST_CHECK(CreateFreezeTx());
				}
				break;
			case 5:
				{
					BOOST_CHECK(CreateRegScriptTx(true));
				}
				break;
			default:
				assert(0);
			}
//			cout << "create tx order:" << i << "type:" <<nTxType << endl;
			//putchar('\b');	//将当前行全部清空，用以显示最新的进度条状态
			if(0 != i) {
				for(int j=0; j<100 ;++j)
					cout<<'\b';
				cout << "create tx progress: "<<  (int)(((i+1)/(float)txCount) * 100) << "%";
			}
		}
	}

	bool DetectionAccount(uint64_t llSendValue) {
		uint64_t rewardAmount(0);
		uint64_t SelfFreezeAmount(0);
		uint64_t freeValue(0);
		uint64_t totalValue(0);
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
				SelfFreezeAmount += account.GetSelfFreezeAmount(chainActive.Tip()->nHeight);
				freeValue += account.GetRawBalance(chainActive.Tip()->nHeight);
			}

		}
		totalValue += rewardAmount;
		totalValue += SelfFreezeAmount;
		totalValue += freeValue;
		//检查总账平衡
		BOOST_CHECK(totalValue  == 30000000*COIN);

		uint64_t selfFreeze(0);
		//检查主动冻结钱是否平衡
		for(auto & item : vFreezeItem) {
			if(mempool.mapTx.count(uint256(std::get<2>(item))) <= 0 ) //交易已被确认
			if(std::get<0>(item) >  chainActive.Tip()->nHeight) {  //主动冻结还未解冻
				selfFreeze += std::get<1>(item);
			}
		}
//		cout << "SelfFreezeAmount:" << SelfFreezeAmount << endl;
//		cout << "selfFreeze:" << selfFreeze << endl;

		BOOST_CHECK(SelfFreezeAmount == selfFreeze);

		return true;
	}

	bool SetBlockGenerte(const char *addr)
	{
		char *argv[] = { "rpctest", "generateblock", const_cast<char *>(addr) };
		int argc = sizeof(argv) / sizeof(char*);

		Value value;
		if (CommandLineRPC_GetValue(argc, argv, value)) {
			return true;
		}
		return false;
	}
};

BOOST_FIXTURE_TEST_SUITE(pressure_tests, PressureTest)
BOOST_FIXTURE_TEST_CASE(tests, PressureTest)
{
	//初始化环境,注册一个合约脚本，并且挖矿确认
	InitRegScript();
	BOOST_CHECK(DetectionAccount(0));
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
	int nSize = mempool.mapTx.size();
	int nConfirmTxCount(0);
	uint64_t llRegAcctFee(0);
	uint64_t llSendValue(0);
	while (nSize) {
		//挖矿
		map<string, string>::const_iterator iterAddr = GetRandAddress();
		BOOST_CHECK(SetBlockGenerte(iterAddr->second.c_str()));
		CBlock block;
		{
			LOCK(cs_main);
			CBlockIndex *pindex = chainActive.Tip();
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
		//检测block是否按照手续费比字节数大小值排序确认
		vector<std::shared_ptr<CBaseTransaction> >::iterator prePtx = block.vptx.begin()+1;
		vector<std::shared_ptr<CBaseTransaction> >::iterator iterPTx = ++prePtx;
		for(; iterPTx != block.vptx.end(); ++iterPTx){
			int preProriry = prePtx->get()->GetFee() / prePtx->get()->GetSerializeSize(SER_DISK, CLIENT_VERSION);
			int proriry = iterPTx->get()->GetFee() / iterPTx->get()->GetSerializeSize(SER_DISK, CLIENT_VERSION);
			prePtx = iterPTx;
//			cout << "preProriry:" << preProriry << endl;
//			cout << "proriry:" << proriry << endl;
			BOOST_CHECK(preProriry>= proriry);
		}
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
		BOOST_CHECK(DetectionAccount(llSendValue));
	}
	uint64_t totalFee(0);
	for(auto &item : vSendFee) {
		totalFee += item.second;
	}
	//校验总的手续费是否正确
	BOOST_CHECK(totalFee == llSendValue);


}
BOOST_AUTO_TEST_SUITE_END()
