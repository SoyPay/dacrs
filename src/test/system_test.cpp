#include <stdlib.h>
#include <time.h>
#include "main.h"
#include "init.h"
#include "miner.h"
#include "uint256.h"
#include "util.h"
#include <boost/test/unit_test.hpp>
#include "rpcclient.h"
#include "tx.h"
#include "wallet.h"
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include "./VmScript/VmScript.h"
#include "rpcserver.h"
#include "noui.h"
#include "ui_interface.h"
#include "SysTestBase.h"
#include <boost/algorithm/string/predicate.hpp>

using namespace std;
using namespace boost;

class CSystemTest:public SysTestBase
{
public:
	CSystemTest() {
		StartServer();
	}

	~CSystemTest(){
		StopServer();
	}
private:
	void StartServer() {
//		int argc = 2;
//		char* argv[] = {"D:\\cppwork\\soypay\\src\\soypayd.exe","-datadir=d:\\bitcoin" };
//		SysTestBase::StartServer(argc,argv);
	}

	void StopServer() {
//		SysTestBase::StopServer();
	}

public:
	int GetBlockHeight()
	{
		return  (int)chainActive.Height();
	}

	bool GetBlockHash(int nHeight,uint256& blockHash)
	{
		if (nHeight < 0 || nHeight > chainActive.Height())
			return false;

		CBlockIndex* pblockindex = chainActive[nHeight];
		blockHash = pblockindex->GetBlockHash();
		return true;
	}

	bool IsTxConfirmdInWallet(const uint256& blockHash,const uint256& txHash)
	{
		auto itAccountTx = pwalletMain->mapInBlockTx.find(blockHash);
		if (pwalletMain->mapInBlockTx.end() == itAccountTx)
			return false;

		for (const auto &item :itAccountTx->second.mapAccountTx) {
			if (txHash == item.first) {
				return true;
			}
		}
		return false;
	}

	bool GetTxIndexInBlock(const uint256& txHash, int& nIndex) {
		CBlockIndex* pindex = chainActive.Tip();
		CBlock block;
		if (!ReadBlockFromDisk(block, pindex))
			return false;

		block.BuildMerkleTree();
		std::tuple<bool,int> ret = block.GetTxIndex(txHash);
		if (!std::get<0>(ret)) {
			return false;
		}

		nIndex = std::get<1>(ret);
		return true;
	}

	bool GetRegScript(map<string, string>& mapRegScript) {
		CRegID regId;
		vector<unsigned char> vScript;

		if (NULL == pScriptDBTip || pScriptDBTip && !pScriptDBTip->GetScript(0, regId, vScript))
			return false;

		string strRegID = HexStr(regId.GetVec6());
		string strScript = HexStr(vScript.begin(), vScript.end());
		mapRegScript.insert(make_pair(strRegID, strScript));

		while (pScriptDBTip->GetScript(1, regId, vScript)) {
			strRegID = HexStr(regId.GetVec6());
			strScript = HexStr(vScript.begin(), vScript.end());
			mapRegScript.insert(make_pair(strRegID, strScript));
		}

		return true;
	}

	bool CheckRegScript(const string& strRegID,const string& strPath) {
		map<string, string> mapRegScript;
		if (!GetRegScript(mapRegScript)) {
			return false;
		}

		string strFileData;
		if (!GetFileData(strPath,strFileData)) {
			return false;
		}

		for (const auto& item:mapRegScript) {
			if (strRegID == item.first) {
				if (strFileData == item.second) {
					return true;
				}
			}
		}

		return false;
	}

	bool GetFileData(const string& strFilePath, string& strFileData) {
		FILE* file = fopen(strFilePath.c_str(), "rb+");
		if (!file) {
			return false;
		}

		long lSize;
		size_t nSize = 1;
		fseek(file, 0, SEEK_END);
		lSize = ftell(file);
		rewind(file);

		// allocate memory to contain the whole file:
		char *buffer = (char*) malloc(sizeof(char) * lSize);
		if (buffer == NULL) {
			return false;
		}

		if (fread(buffer, 1, lSize, file) != lSize) {
			if (buffer)
				free(buffer);
			throw runtime_error("read script file error");
		}

		CVmScript vmScript;
		vmScript.Rom.insert(vmScript.Rom.end(), buffer, buffer + lSize);
		CDataStream ds(SER_DISK, CLIENT_VERSION);
		ds << vmScript;

		vector<unsigned char> vscript;
		vscript.assign(ds.begin(), ds.end());

		if (file)
			fclose(file);
		if (buffer)
			free(buffer);

		strFileData = HexStr(vscript);
		return true;
	}

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
private:
	boost::thread* pThreadShutdown;
};

BOOST_FIXTURE_TEST_SUITE(system_test,CSystemTest)
BOOST_FIXTURE_TEST_CASE(reg_test,CSystemTest)
{
	int nOldBlockHeight = GetBlockHeight();
	string strAddr("mvVp2PDRuG4JJh6UjkJFzXUC8K5JVbMFFA");

	int nFee = 10000;
	string strTxHash;
	string strFileName("RegScriptTest.bin");
	Value valueRes = RegisterScriptTx(strAddr,strFileName , 100, nFee);
	BOOST_CHECK(GetHashFromCreatedTx(valueRes,strTxHash));

	//挖矿
	uint64_t nOldMoney = GetFreeMoney(strAddr);
	BOOST_CHECK(GenerateOneBlock());
	int nNewBlockHeight = GetBlockHeight();

	//确认钱已经扣除
	BOOST_CHECK(nNewBlockHeight = nOldBlockHeight +1 );
	cout<<"new: "<<GetFreeMoney(strAddr)<<" old: "<<nOldMoney<<" nfee: "<<nFee<<endl;
	BOOST_CHECK(GetFreeMoney(strAddr) == nOldMoney - nFee);

	//确认脚本账号已经生成
	int nIndex = 0;
	BOOST_CHECK(GetTxIndexInBlock(uint256(strTxHash),nIndex));
	CRegID regID(nNewBlockHeight,nIndex);
	BOOST_CHECK(IsScriptAccCreated(HexStr(regID.GetVec6())));
	BOOST_CHECK(!IsScriptAccCreated("000000000000"));

	//检查钱包里的已确认交易里是否有此笔交易
	uint256 blockHash;
	BOOST_CHECK(GetBlockHash(nNewBlockHeight,blockHash));
	BOOST_CHECK(IsTxConfirmdInWallet(blockHash,uint256(strTxHash)));


	//通过listregscript 获取相关信息，一一核对，看是否和输入的一致
	string strPath = SysCfg().GetDefaultTestDataPath() + strFileName;
	BOOST_CHECK(CheckRegScript(HexStr(regID.GetVec6()),strPath));
}
BOOST_AUTO_TEST_SUITE_END()

