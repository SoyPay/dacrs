/*
 * rpctx.cpp
 *
 *  Created on: Aug 26, 2014
 *      Author: leo
 */

#include "txdb.h"

#include "base58.h"
#include "rpcserver.h"
#include "init.h"
#include "net.h"
#include "netbase.h"
#include "util.h"
#include "wallet/wallet.h"
#include "wallet/walletdb.h"
#include "syncdatadb.h"
#include "checkpoints.h"
#include "miner.h"
#include "main.h"
#include "vm/script.h"
#include "vm/vmrunevn.h"
#include <stdint.h>
#include "vm/vmlua.h"

#include <boost/assign/list_of.hpp>
#include "json/json_spirit_utils.h"
#include "json/json_spirit_value.h"
#include "json/json_spirit_reader.h"

#include "boost/tuple/tuple.hpp"
#define revert(height) ((height<<24) | (height << 8 & 0xff0000) |  (height>>8 & 0xff00) | (height >> 24))

using namespace std;
using namespace boost;
using namespace boost::assign;
using namespace json_spirit;

extern CAccountViewDB *g_pAccountViewDB;
string RegIDToAddress(CUserID &cUserId) {
	CKeyID cKeid;
	if (g_pAccountViewTip->GetKeyId(cUserId, cKeid)) {
		return cKeid.ToAddress();
	}
	return "can not get address";
}

static bool GetKeyId(string const &strAddr, CKeyID &cKeyId) {
	if (!CRegID::GetKeyID(strAddr, cKeyId)) {
		cKeyId = CKeyID(strAddr);
		if (cKeyId.IsEmpty()) {
			return false;
		}
	}
	return true;
}

Object GetTxDetailJSON(const uint256& cTxHash) {
	Object obj;
	std::shared_ptr<CBaseTransaction> pBaseTx;
	{
		LOCK(g_cs_main);
		CBlock cGenesisBlock;
		CBlockIndex* pcGenesisBlockIndex = g_mapBlockIndex[SysCfg().HashGenesisBlock()];
		ReadBlockFromDisk(cGenesisBlock, pcGenesisBlockIndex);
		assert(cGenesisBlock.GetHashMerkleRoot() == cGenesisBlock.BuildMerkleTree());
		for (unsigned int i = 0; i < cGenesisBlock.vptx.size(); ++i) {
			if (cTxHash == cGenesisBlock.GetTxHash(i)) {
				obj = cGenesisBlock.vptx[i]->ToJSON(*g_pAccountViewTip);
				obj.push_back(Pair("blockhash", SysCfg().HashGenesisBlock().GetHex()));
				obj.push_back(Pair("confirmHeight", (int) 0));
				obj.push_back(Pair("confirmedtime", (int) cGenesisBlock.GetTime()));
				CDataStream cDs(SER_DISK, g_sClientVersion);
				cDs << cGenesisBlock.vptx[i];
				obj.push_back(Pair("rawtx", HexStr(cDs.begin(), cDs.end())));
				return obj;
			}
		}

		if (SysCfg().IsTxIndex()) {
			ST_DiskTxPos tPostx;
			if (g_pScriptDBTip->ReadTxIndex(cTxHash, tPostx)) {
				CAutoFile cAutoFile(OpenBlockFile(tPostx, true), SER_DISK, g_sClientVersion);
				CBlockHeader cHeader;
				try {
					cAutoFile >> cHeader;
					fseek(cAutoFile, tPostx.m_unTxOffset, SEEK_CUR);
					cAutoFile >> pBaseTx;
					obj = pBaseTx->ToJSON(*g_pAccountViewTip);
					obj.push_back(Pair("blockhash", cHeader.GetHash().GetHex()));
					obj.push_back(Pair("confirmHeight", (int) cHeader.GetHeight()));
					obj.push_back(Pair("confirmedtime", (int) cHeader.GetTime()));
					if (pBaseTx->m_chTxType == EM_CONTRACT_TX) {
						vector<CVmOperate> vcOutput;
						g_pScriptDBTip->ReadTxOutPut(pBaseTx->GetHash(), vcOutput);
						Array outputArray;
						for (auto & item : vcOutput) {
							outputArray.push_back(item.ToJson());
						}
						obj.push_back(Pair("listOutput", outputArray));
					}
					CDataStream cDs(SER_DISK, g_sClientVersion);
					cDs << pBaseTx;
					obj.push_back(Pair("rawtx", HexStr(cDs.begin(), cDs.end())));
				} catch (std::exception &e) {
					throw runtime_error(tfm::format("%s : Deserialize or I/O error - %s", __func__, e.what()).c_str());
				}
				return obj;
			}
		}
		{
			pBaseTx = g_cTxMemPool.lookup(cTxHash);
			if (pBaseTx.get()) {
				obj = pBaseTx->ToJSON(*g_pAccountViewTip);
				CDataStream cDs(SER_DISK, g_sClientVersion);
				cDs << pBaseTx;
				obj.push_back(Pair("rawtx", HexStr(cDs.begin(), cDs.end())));
				return obj;
			}
		}

	}
	return obj;
}

Value gettxdetail(const Array& params, bool bHelp) {
	if (bHelp || params.size() != 1) {
        throw runtime_error(
            "gettxdetail \"cTxHash\"\n"
			"\nget the transaction detail by given transaction hash.\n"
            "\nArguments:\n"
            "1.txhash   (string,required) The hast of transaction.\n"
        	"\nResult a object about the transaction detail\n"
            "\nResult:\n"
        	"\n\"txhash\"\n"
            "\nExamples:\n"
            + HelpExampleCli("gettxdetail","c5287324b89793fdf7fa97b6203dfd814b8358cfa31114078ea5981916d7a8ac\n")
            + "\nAs json rpc call\n"
            + HelpExampleRpc("gettxdetail","c5287324b89793fdf7fa97b6203dfd814b8358cfa31114078ea5981916d7a8ac\n"));
	}
	uint256 cTxHash(uint256S(params[0].get_str()));
	return GetTxDetailJSON(cTxHash);
}

//create a register cAccount tx
Value registaccounttx(const Array& params, bool bHelp) {
	if (bHelp || params.size() != 2) {
	       throw runtime_error(
	            "registaccounttx \"addr\" \"ullFee\"\n"
				"\nregister secure cAccount\n"
				"\nArguments:\n"
				"1.addr: (string, required)\n"
				"2.ullFee: (numeric, required) pay to miner\n"
	            "\nResult:\n"
	    		"\"txhash\": (string)\n"
	    		"\nExamples:\n"
	            + HelpExampleCli("registaccounttx", "n2dha9w3bz2HPVQzoGKda3Cgt5p5Tgv6oj 100000 ")
	            + "\nAs json rpc call\n"
	            + HelpExampleRpc("registaccounttx", "n2dha9w3bz2HPVQzoGKda3Cgt5p5Tgv6oj 100000 "));

	}

	string strAddr = params[0].get_str();
	uint64_t ullFee = params[1].get_uint64();

	//get cKeyId
	CKeyID cKeyid;
	if (!GetKeyId(strAddr, cKeyid)) {
		throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "in registaccounttx :address err");
	}
	CRegisterAccountTx cRtx;
	assert(g_pwalletMain != NULL);
	{
		//	LOCK2(g_cs_main, g_pwalletMain->cs_wallet);
		EnsureWalletIsUnlocked();

		//balance
		CAccountViewCache cView(*g_pAccountViewTip, true);
		CAccount cAccount;

		CUserID userId = cKeyid;
		if (!cView.GetAccount(userId, cAccount)) {
			throw JSONRPCError(RPC_WALLET_ERROR, "in registaccounttx Error: Account balance is insufficient.");
		}

		if (cAccount.IsRegister()) {
			throw JSONRPCError(RPC_WALLET_ERROR, "in registaccounttx Error: Account is already registered");
		}
		uint64_t ullBalance = cAccount.GetRawBalance();
		if (ullBalance < ullFee) {
			throw JSONRPCError(RPC_WALLET_ERROR, "in registaccounttx Error: Account balance is insufficient.");
		}

		CPubKey cPubkeyVar;
		if (!g_pwalletMain->GetPubKey(cKeyid, cPubkeyVar)) {
			throw JSONRPCError(RPC_WALLET_ERROR, "in registaccounttx Error: not find key.");
		}

		CPubKey cMinerPKey;
		if (g_pwalletMain->GetPubKey(cKeyid, cMinerPKey, true)) {
			cRtx.m_cMinerId = cMinerPKey;
		} else {
			CNullID cNullId;
			cRtx.m_cMinerId = cNullId;
		}
		cRtx.m_cUserId = cPubkeyVar;
		cRtx.m_llFees = ullFee;
		cRtx.m_nValidHeight = g_cChainActive.Tip()->m_nHeight;

		if (!g_pwalletMain->Sign(cKeyid, cRtx.SignatureHash(), cRtx.m_vchSignature)) {
			throw JSONRPCError(RPC_WALLET_ERROR, "in registaccounttx Error: Sign failed.");
		}

	}

	std::tuple<bool, string> ret;
	ret = g_pwalletMain->CommitTransaction((CBaseTransaction *) &cRtx);
	if (!std::get<0>(ret)) {
		throw JSONRPCError(RPC_WALLET_ERROR, "registaccounttx Error:" + std::get<1>(ret));
	}
	Object obj;
	obj.push_back(Pair("hash", std::get<1>(ret)));
	return obj;

}

//create a contract tx
Value createcontracttx(const Array& params, bool bHelp) {
	if (bHelp || params.size() < 5 || params.size() > 6) {
	   throw runtime_error(
			"createcontracttx \"userregid[\"addr\"]\" \"cAppId\" \"llAmount\" \"contract\" \"ullFee\" (\"height\")\n"
			"\ncreate contract transaction\n"
			"\nArguments:\n"
			"1.\"userregid\": (string, required)\n the address for send"
			"2.\"cAppId\":(string, required) the appID (for example: Ipo.bin)\n"
			"3.\"llAmount\":(numeric, required)\n"
			"4.\"contract\": (string, required)\n"
			"5.\"ullFee\": (numeric, required) pay to miner\n"
			"6.\"height\": (numeric, optional)create height,If not provide use the tip block hegiht in g_cChainActive\n"
			"\nResult:\n"
			"\"contract tx str\": (string)\n"
			"\nExamples:\n"
			+ HelpExampleCli("createcontracttx", "000000000100 [\"5zQPcC1YpFMtwxiH787pSXanUECoGsxUq3KZieJxVG\"] "
					"\"5yNhSL7746VV5qWHHDNLkSQ1RYeiheryk9uzQG6C5d\""
					"100000 "
					"\"5Vp1xpLT8D2FQg3kaaCcjqxfdFNRhxm4oy7GXyBga9\" "
					"01020304 "
					"1") + "\nAs json rpc call\n"
			+ HelpExampleRpc("createcontracttx", "000000000100 [\"5zQPcC1YpFMtwxiH787pSXanUECoGsxUq3KZieJxVG\"] "
					"\"5yNhSL7746VV5qWHHDNLkSQ1RYeiheryk9uzQG6C5d\""
					"100000 "
					"\"5Vp1xpLT8D2FQg3kaaCcjqxfdFNRhxm4oy7GXyBga9\" "
					"01020304 "
					"1"));
	}

	RPCTypeCheck(params, list_of(str_type)(str_type)(int_type)(str_type)(int_type)(int_type));

	CRegID cUserId(params[0].get_str());
	CKeyID cSrckeyId;
	if (cUserId.IsEmpty()) {
		if (!GetKeyId(params[0].get_str(), cSrckeyId)) {
			throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Source Invalid  address");
		}
		if (!g_pAccountViewTip->GetRegId(CUserID(cSrckeyId), cUserId)) {
			throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "address not regist");
		}

	}

	CRegID cAppId(params[1].get_str());
	uint64_t ullAmount = params[2].get_uint64();
	vector<unsigned char> vuchContract = ParseHex(params[3].get_str());
	uint64_t ullFee = params[4].get_uint64();
	uint32_t uHeight(0);
	if (params.size() > 5) {
		uHeight = params[5].get_int();
	}
	if (ullFee > 0 && ullFee < CTransaction::m_sMinTxFee) {
		throw runtime_error("in createcontracttx :ullFee is smaller than nMinTxFee\n");
	}

	if (cAppId.IsEmpty()) {
		throw runtime_error("in createcontracttx :addresss is error!\n");
	}
	EnsureWalletIsUnlocked();
	std::shared_ptr<CTransaction> tx = std::make_shared<CTransaction>();
	{
		//balance
		CAccountViewCache cView(*g_pAccountViewTip, true);
		CAccount cSecureAcc;

		if (!g_pScriptDBTip->HaveScript(cAppId)) {
			throw runtime_error(tinyformat::format("createcontracttx :script id %s is not exist\n", cAppId.ToString()));
		}
		tx.get()->m_chTxType = EM_CONTRACT_TX;
		tx.get()->m_cSrcRegId = cUserId;
		tx.get()->m_cDesUserId = cAppId;
		tx.get()->m_ullValues = ullAmount;
		tx.get()->m_ullFees = ullFee;
		tx.get()->m_vchContract = vuchContract;
		if (0 == uHeight) {
			uHeight = g_cChainActive.Tip()->m_nHeight;
		}
		tx.get()->m_nValidHeight = uHeight;

		//get cKeyId by accountid
		CKeyID cKeyid;
		if (!cView.GetKeyId(cUserId, cKeyid)) {
			CID id(cUserId);
			LogPrint("INFO", "vaccountid:%s have no key id\r\n", HexStr(id.GetID()).c_str());
			// assert(0);
			throw JSONRPCError(RPC_WALLET_ERROR, "createcontracttx Error: have no key id.");
		}

		vector<unsigned char> vuchSignature;
		if (!g_pwalletMain->Sign(cKeyid, tx.get()->SignatureHash(), tx.get()->m_vchSignature)) {
			throw JSONRPCError(RPC_WALLET_ERROR, "createcontracttx Error: Sign failed.");
		}
	}

	std::tuple<bool, string> ret;
	ret = g_pwalletMain->CommitTransaction((CBaseTransaction *) tx.get());
	if (!std::get<0>(ret)) {
		throw JSONRPCError(RPC_WALLET_ERROR, "Error:" + std::get<1>(ret));
	}
	Object obj;
	obj.push_back(Pair("hash", std::get<1>(ret)));
	return obj;
}

//create a register script tx
Value registerapptx(const Array& params, bool bHelp) {
	if (bHelp || params.size() < 3 || params.size() > 5) {
		throw runtime_error("registerapptx \"addr\" \"filepath\"\"ullFee\" (\"height\") (\"scriptdescription\")\n"
				"\ncreate a register script transaction\n"
				"\nArguments:\n"
				"1.\"addr\": (string required)\n"
				"2.\"filepath\": (string required),app's file path\n"
				"3.\"ullFee\": (numeric required) pay to miner\n"
				"4.\"height\": (numeric optional)valid height,If not provide, use the tip block hegiht in g_cChainActive\n"
				"5.\"scriptdescription\":(string optional) new script description\n"
				"\nResult:\n"
				"\"txhash\": (string)\n"
				"\nExamples:\n"
				+ HelpExampleCli("registerapptx",
						"\"5zQPcC1YpFMtwxiH787pSXanUECoGsxUq3KZieJxVG\" \"run.exe\" \"010203040506\" \"100000\" (\"scriptdescription\")") + "\nAs json rpc call\n"
				+ HelpExampleRpc("registerapptx",
						"5zQPcC1YpFMtwxiH787pSXanUECoGsxUq3KZieJxVG \"run.exe\" \"010203040506\" \"100000\" (\"scriptdescription\")"));
	}

	RPCTypeCheck(params, list_of(str_type)(str_type)(int_type)(int_type)(str_type));
	CVmScript cScript;
	vector<unsigned char> vuchScript;

	string strPath = params[1].get_str();
	FILE* pFile = fopen(strPath.c_str(), "rb+");
	if (!pFile) {
		throw runtime_error("create registerapptx open script file" + strPath + "error");
	}
	long lSize;
	fseek(pFile, 0, SEEK_END);
	lSize = ftell(pFile);
	rewind(pFile);

	// allocate memory to contain the whole file:
	char *pchBuffer = (char*) malloc(sizeof(char) * lSize);
	if (pchBuffer == NULL) {
		fclose(pFile); //及时关闭
		throw runtime_error("allocate memory failed");
	}
	if (fread(pchBuffer, 1, lSize, pFile) != (size_t) lSize) {
		free(pchBuffer);  //及时释放
		fclose(pFile);  //及时关闭
		throw runtime_error("read script file error");
	} else {
		fclose(pFile); //使用完关闭文件
	}

	cScript.m_vuchRom.insert(cScript.m_vuchRom.end(), pchBuffer, pchBuffer + lSize);
	if (pchBuffer) {
		free(pchBuffer);
	}
	if (params.size() > 4) {
		string strScriptDesc = params[4].get_str();
		cScript.m_vuchScriptExplain.insert(cScript.m_vuchScriptExplain.end(), strScriptDesc.begin(), strScriptDesc.end());
	}

	if (1 == cScript.getScriptType()) { //判断为lua脚本
		std::tuple<bool, string> result = CVmlua::syntaxcheck(true, strPath.c_str(), strPath.length());
		bool bOK = std::get<0>(result);
		if (!bOK) {
			throw JSONRPCError(RPC_INVALID_PARAMS, std::get<1>(result));
		}
	}

	CDataStream cDs(SER_DISK, g_sClientVersion);
	cDs << cScript;
	vuchScript.assign(cDs.begin(), cDs.end());

	uint64_t ullFee = params[2].get_uint64();
	int nHeight(0);
	if (params.size() > 3) {
		nHeight = params[3].get_int();
	}
	if (ullFee > 0 && ullFee < CTransaction::m_sMinTxFee) {
		throw runtime_error("in registerapptx :ullFee is smaller than nMinTxFee\n");
	}
	//get cKeyId
	CKeyID cKeyID;
	if (!GetKeyId(params[0].get_str(), cKeyID)) {
		throw runtime_error("in registerapptx :send address err\n");
	}

	assert(g_pwalletMain != NULL);
	CRegisterAppTx cTx;
	{
		//	LOCK2(g_cs_main, g_pwalletMain->cs_wallet);
		EnsureWalletIsUnlocked();
		//balance
		CAccountViewCache cView(*g_pAccountViewTip, true);
		CAccount cAccount;

		uint64_t llBalance = 0;
		CUserID userId = cKeyID;
		if (cView.GetAccount(userId, cAccount)) {
			llBalance = cAccount.GetRawBalance();
		}
		if (!cAccount.IsRegister()) {
			throw JSONRPCError(RPC_WALLET_ERROR, "in registerapptx Error: Account is not registered.");
		}
		if (!g_pwalletMain->HaveKey(cKeyID)) {
			throw JSONRPCError(RPC_WALLET_ERROR, "in registerapptx Error: WALLET file is not correct.");
		}
		if (llBalance < ullFee) {
			throw JSONRPCError(RPC_WALLET_ERROR, "in registerapptx Error: Account balance is insufficient.");
		}
		auto GetUserId = [&](CKeyID &mkeyId)
		{
			CAccount cAcct;
			if (cView.GetAccount(CUserID(mkeyId), cAcct)) {
				return cAcct.m_cRegID;
			}
			throw runtime_error(tinyformat::format("registerapptx :cAccount id %s is not exist\n", mkeyId.ToAddress()));
		};

		cTx.m_cRegAcctId = GetUserId(cKeyID);
		cTx.m_vchScript = vuchScript;
		cTx.m_ullFees = ullFee;
		cTx.m_ullRunStep = vuchScript.size();
		if (0 == nHeight) {
			nHeight = g_cChainActive.Tip()->m_nHeight;
		}
		cTx.m_nValidHeight = nHeight;

		if (!g_pwalletMain->Sign(cKeyID, cTx.SignatureHash(), cTx.m_vchSignature)) {
			throw JSONRPCError(RPC_WALLET_ERROR, "registerapptx Error: Sign failed.");
		}
	}

	std::tuple<bool, string> ret;
	ret = g_pwalletMain->CommitTransaction((CBaseTransaction *) &cTx);
	if (!std::get<0>(ret)) {
		throw JSONRPCError(RPC_WALLET_ERROR, "registerapptx Error:" + std::get<1>(ret));
	}
	Object obj;
	obj.push_back(Pair("hash", std::get<1>(ret)));
	return obj;

}

//估算合约的
Value contractreckon(const Array& params, bool bHelp) {
	if (bHelp || params.size() < 3 || params.size() > 3) {
		throw runtime_error("contractreckon \"contract\" \"userregid\"\"cAppId\" )\n"
				"\ncreate a register script transaction\n"
				"\nArguments:\n"
				"1.\"contract\": (string, required)\n"
				"2.\"userregid\": (string, required)\n the address for send"
				"3.\"cAppId\":(string, required) the appID (for example: Ipo.bin)\n"
				"\nResult:\n"
				"\"step\": \n"
				"\"fuelrate\": \n"
				"\"fuel\": \n"
				"\nExamples:\n"
				+ HelpExampleCli("contractreckon",
						"\"0405000000704548747658377956744634767745645a5941446a637043556433514c783978594600 \" "
						"\"pHoFvGYVtyLm5Zb3TwNPsjxqXy6ncs4A7m \" "
						"\"173883-1\" ") + "\nAs json rpc call\n"
				+ HelpExampleRpc("contractreckon",
						"\"0405000000704548747658377956744634767745645a5941446a637043556433514c783978594600 \" "
						"\"pHoFvGYVtyLm5Zb3TwNPsjxqXy6ncs4A7m \" "
						"\"173883-1\" "));
	}

	RPCTypeCheck(params, list_of(str_type)(str_type)(str_type));

	vector<unsigned char> vuchContract = ParseHex(params[0].get_str()); //合约内容

	CRegID cUserId(params[1].get_str());
	CKeyID cSrckeyId;
	if (cUserId.IsEmpty()) {
		if (!GetKeyId(params[1].get_str(), cSrckeyId)) {
			throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Source Invalid  address");
		}
		if (!g_pAccountViewTip->GetRegId(CUserID(cSrckeyId), cUserId)) {
			throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "address not regist");
		}
	}

	CRegID cAppId(params[2].get_str());
	if (cAppId.IsEmpty()) {
		throw runtime_error("in createcontracttx :addresss is error!\n");
	}
	std::shared_ptr<CTransaction> tx = std::make_shared<CTransaction>();
	{
		//balance
		CAccountViewCache cView(*g_pAccountViewTip, true);
		CAccount cSecureAcc;

		if (!g_pScriptDBTip->HaveScript(cAppId)) {
			throw runtime_error(tinyformat::format("createcontracttx :script id %s is not exist\n", cAppId.ToString()));
		}
		tx.get()->m_chTxType = EM_CONTRACT_TX;
		tx.get()->m_cSrcRegId = cUserId;
		tx.get()->m_cDesUserId = cAppId;
		tx.get()->m_ullValues = 500000;
		tx.get()->m_ullFees = SysCfg().GetMaxFee(); // 130000
		tx.get()->m_vchContract = vuchContract;
		tx.get()->m_nValidHeight = g_cChainActive.Tip()->m_nHeight;

		//get cKeyId by accountid
		CKeyID cKeyID;
		if (!cView.GetKeyId(cUserId, cKeyID)) {
			CID cID(cUserId);
			LogPrint("INFO", "vaccountid:%s have no key id\r\n", HexStr(cID.GetID()).c_str());
			throw JSONRPCError(RPC_WALLET_ERROR, "createcontracttx Error: have no key id.");
		}
	}

	CVmRunEvn cVmRunEvn;
	std::shared_ptr<CBaseTransaction> pTx = tx;
	uint64_t ullEl = GetElementForBurn(g_cChainActive.Tip());
	uint64_t ullRunStep =MAX_BLOCK_RUN_STEP;


	CAccountViewCache cpAccountViewTipTemp(*g_pAccountViewTip, true);
	CScriptDBViewCache cpScriptDBTipTemp(*g_pScriptDBTip, true);

	std::tuple<bool, uint64_t, string> ret = cVmRunEvn.run(pTx, cpAccountViewTipTemp, cpScriptDBTipTemp,g_cChainActive.Tip()->m_nHeight, ullEl, ullRunStep);
	if (!std::get<0>(ret)) {
		throw runtime_error("vmRunEvn.run() error\n");
	}

	int nFuelRate = GetElementForBurn(g_cChainActive.Tip());
	int64_t llFee = (ullRunStep / 100 + 1) * nFuelRate + SysCfg().GetTxFee();
	Object obj;
	obj.push_back(Pair("step", ullRunStep));
	obj.push_back(Pair("fuelrate", nFuelRate));
	obj.push_back(Pair("fuel", ValueFromAmount(llFee)));
	return obj;
}

Value listaddr(const Array& params, bool bHelp) {
	if (bHelp || params.size() != 0) {
		throw runtime_error(
				 "listaddr\n"
				 "\nreturn Array containing address,balance,haveminerkey,cRegId information.\n"
				 "\nArguments:\n"
				 "\nResult:\n"
				 "\nExamples:\n"
				 + HelpExampleCli("listaddr", "")
				 + "\nAs json rpc call\n"
                 + HelpExampleRpc("listaddr", ""));
	}
	Array retArry;
	assert(g_pwalletMain != NULL);
	{
		set<CKeyID> setKeyId;
		g_pwalletMain->GetKeys(setKeyId);
		if (setKeyId.size() == 0) {
			return retArry;
		}
		CAccountViewCache cAccView(*g_pAccountViewTip, true);

		for (const auto &keyId : setKeyId) {
			CUserID cUserId(keyId);
			CAccount cAcctInfo;
			cAccView.GetAccount(cUserId, cAcctInfo);
			CKeyCombi cKeyCombi;
			g_pwalletMain->GetKeyCombi(keyId, cKeyCombi);

			Object obj;
			obj.push_back(Pair("addr", keyId.ToAddress()));
			obj.push_back(Pair("balance", (double) cAcctInfo.GetRawBalance() / (double) COIN));
			obj.push_back(Pair("haveminerkey", cKeyCombi.IsContainMinerKey()));
			obj.push_back(Pair("RegId", cAcctInfo.m_cRegID.ToString()));
			retArry.push_back(obj);
		}
	}

	return retArry;
}

Value listtx(const Array& params, bool bHelp) {
	if (bHelp || params.size() != 0) {
		throw runtime_error(
				 "listtx\n"
				 "\nget all confirm transactions and all unconfirm transactions from wallet.\n"
				 "\nArguments:\n"
				 "\nResult:\n"
				 "\nExamples:\n"
				 + HelpExampleCli("listtx", "")
                 + HelpExampleRpc("listtx", ""));
	}

	Object retObj;
	assert(g_pwalletMain != NULL);
	{
		Object Inblockobj;
		for (auto const &wtx : g_pwalletMain->m_mapInBlockTx) {
			for (auto const & item : wtx.second.m_mapAccountTx) {
				Inblockobj.push_back(Pair("tx", item.first.GetHex()));
			}
		}
		retObj.push_back(Pair("ConfirmTx", Inblockobj));

		CAccountViewCache cView(*g_pAccountViewTip, true);
		Array UnConfirmTxArry;
		for (auto const &wtx : g_pwalletMain->m_mapUnConfirmTx) {
			UnConfirmTxArry.push_back(wtx.first.GetHex());
		}
		retObj.push_back(Pair("UnConfirmTx", UnConfirmTxArry));
	}
	return retObj;
}

Value getaccountinfo(const Array& params, bool bHelp) {
	if (bHelp || params.size() != 1) {
		  throw runtime_error(
				"getaccountinfo \"addr\"\n"
				"\nget cAccount information\n"
				"\nArguments:\n"
				"1.\"addr\": (string, required)"
				"Returns an object containing various cAccount info.\n"
				"\nResult:\n"
				"\nExamples:\n"
				+ HelpExampleCli("getaccountinfo", "000000000500\n")
				+ "\nAs json rpc call\n"
				+ HelpExampleRpc("getaccountinfo", "5Vp1xpLT8D2FQg3kaaCcjqxfdFNRhxm4oy7GXyBga9\n")
		   );
	}
	RPCTypeCheck(params, list_of(str_type));
	CKeyID cKeyID;
	CUserID cUserId;
	string strAddr = params[0].get_str();
	if (!GetKeyId(strAddr, cKeyID)) {
		throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid  address");
	}

	cUserId = cKeyID;
	Object obj;
	{
		CAccount cAccount;
		CAccountViewCache cAccView(*g_pAccountViewTip, true);
		if (cAccView.GetAccount(cUserId, cAccount)) {
			if (!cAccount.m_cPublicKey.IsValid()) {
				CPubKey cPk;
				CPubKey cMinerpk;
				if (g_pwalletMain->GetPubKey(cKeyID, cPk)) {
					g_pwalletMain->GetPubKey(cKeyID, cMinerpk, true);
					cAccount.m_cPublicKey = cPk;
					cAccount.m_cKeyID = std::move(cPk.GetKeyID());
					if (cPk != cMinerpk && !cAccount.m_cMinerPKey.IsValid()) {
						cAccount.m_cMinerPKey = cMinerpk;
					}
				}
			}
			obj = std::move(cAccount.ToJosnObj());
			obj.push_back(Pair("postion", "inblock"));
		} else {
			CPubKey cPk;
			CPubKey cMinerpk;
			if (g_pwalletMain->GetPubKey(cKeyID, cPk)) {
				g_pwalletMain->GetPubKey(cKeyID, cMinerpk, true);
				cAccount.m_cPublicKey = cPk;
				cAccount.m_cKeyID = cPk.GetKeyID();
				if (cMinerpk != cPk) {
					cAccount.m_cMinerPKey = cMinerpk;
				}
				obj = std::move(cAccount.ToJosnObj());
				obj.push_back(Pair("postion", "inwallet"));
			}
		}

	}
	return obj;
}

//list unconfirmed transaction of mine
Value listunconfirmedtx(const Array& params, bool bHelp) {
	if (bHelp || params.size() != 0) {
		 throw runtime_error(
		            "listunconfirmedtx \n"
					"\nget the list  of unconfirmedtx.\n"
		            "\nArguments:\n"
		        	"\nResult a object about the unconfirm transaction\n"
		            "\nResult:\n"
		            "\nExamples:\n"
		            + HelpExampleCli("listunconfirmedtx", "")
		            + "\nAs json rpc call\n"
		            + HelpExampleRpc("listunconfirmedtx", ""));
	}

	Object retObj;
	CAccountViewCache cView(*g_pAccountViewTip, true);
	Array UnConfirmTxArry;
	for (auto const &wtx : g_pwalletMain->m_mapUnConfirmTx) {
		UnConfirmTxArry.push_back(wtx.second.get()->ToString(cView));
	}
	retObj.push_back(Pair("m_mapUnConfirmTx", UnConfirmTxArry));

	return retObj;
}

//sign
Value sign(const Array& params, bool bHelp) {
	if (bHelp || params.size() != 2) {
		string msg = "sign nrequired \"str\"\n"
				"\nsign \"str\"\n"
				"\nArguments:\n"
				"1.\"str\": (string) \n"
				"\nResult:\n"
				"\"signhash\": (string)\n"
				"\nExamples:\n"
				+ HelpExampleCli("sign",
						"0001a87352387b5b4d6d01299c0dc178ff044f42e016970b0dc7ea9c72c08e2e494a01020304100000")
				+ "\nAs json rpc call\n"
				+ HelpExampleRpc("sign",
						"0001a87352387b5b4d6d01299c0dc178ff044f42e016970b0dc7ea9c72c08e2e494a01020304100000");
		throw runtime_error(msg);
	}

	vector<unsigned char> vuch(ParseHex(params[1].get_str()));

	//get cKeyId
	CKeyID cKeyID;
	if (!GetKeyId(params[0].get_str(), cKeyID)) {
		throw runtime_error("in sign :send address err\n");
	}
	vector<unsigned char> vuchSign;
	{
		LOCK(g_pwalletMain->m_cs_wallet);

		CKey cKey;
		if (!g_pwalletMain->GetKey(cKeyID, cKey)) {
			throw JSONRPCError(RPC_WALLET_ERROR, "sign Error: cannot find key.");
		}

		uint256 cHash = Hash(vuch.begin(), vuch.end());
		if (!cKey.Sign(cHash, vuchSign)) {
			throw JSONRPCError(RPC_WALLET_ERROR, "sign Error: Sign failed.");
		}
	}
	Object retObj;
	retObj.push_back(Pair("signeddata", HexStr(vuchSign)));
	return retObj;
}
//
//Value getaccountinfo(const Array& params, bool bHelp) {
//	if (bHelp || params.size() != 1) {
//		throw runtime_error(
//				"getaccountinfo \"address \" dspay address ( \"comment\" \"comment-to\" )\n"
//						"\nGet an cAccount info with dspay address\n" + HelpRequiringPassphrase() + "\nArguments:\n"
//						"1. \"address \"  (string, required) The Dacrs address.\n"
//						"\nResult:\n"
//						"\"cAccount info\"  (string) \n"
//						"\nExamples:\n" + HelpExampleCli("getaccountinfo", "\"1M72Sfpbz1BPpXFHz9m3CdqATR44Jvaydd\"")
//						+ HelpExampleCli("getaccountinfo", "\"000000010100\"")
//
//						);
//	}
//	CAccountViewCache cView(*g_pAccountViewTip, true);
//	string strParam = params[0].get_str();
//	CAccount aAccount;
//	if (strParam.length() != 12) {
//		CDacrsAddress address(params[0].get_str());
//		CKeyID cKeyId;
//		if (!address.GetKeyID(cKeyId))
//			throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid Dacrs address");
//
//		CUserID userId = cKeyId;
//		if (!cView.GetAccount(userId, aAccount)) {
//			return "can not get cAccount info by address:" + strParam;
//		}
//	} else {
//		CRegID regId(ParseHex(strParam));
//		if (!cView.GetAccount(regId, aAccount)) {
//			return "can not get cAccount info by cRegId:" + strParam;
//		}
//	}
//	string fundTypeArray[] = {"NULL_FUNDTYPE", "FREEDOM", "REWARD_FUND", "FREEDOM_FUND", "FREEZD_FUND", "SELF_FREEZD_FUND"};
//
//	Object obj;
//	obj.push_back(Pair("keyID:", aAccount.keyID.ToString()));
//	obj.push_back(Pair("publicKey:", aAccount.publicKey.ToString()));
//	obj.push_back(Pair("llValues:", tinyformat::format("%s", aAccount.llValues)));
//	Array array;
//	//string str = ("fundtype  txhash                                  value                        height");
//	string str = tinyformat::format("%-20.20s%-20.20s%-25.8lf%-6.6d", "fundtype", "scriptid", "value", "height");
//	array.push_back(str);
//	for (int i = 0; i < aAccount.vRewardFund.size(); ++i) {
//		CFund fund = aAccount.vRewardFund[i];
//		array.push_back(tinyformat::format("%-20.20s%-20.20s%-25.8lf%-6.6d",fundTypeArray[fund.nFundType], HexStr(fund.scriptID), fund.value, fund.nHeight));
//	}
//
//	for (int i = 0; i < aAccount.vFreedomFund.size(); ++i) {
//		CFund fund = aAccount.vFreedomFund[i];
//		array.push_back(tinyformat::format("%-20.20s%-20.20s%-25.8lf%-6.6d",fundTypeArray[fund.nFundType], HexStr(fund.scriptID), fund.value, fund.nHeight));
//	}
//	for (int i = 0; i < aAccount.vFreeze.size(); ++i) {
//		CFund fund = aAccount.vFreeze[i];
//		array.push_back(tinyformat::format("%-20.20s%-20.20s%-25.8lf%-6.6d",fundTypeArray[fund.nFundType], HexStr(fund.scriptID), fund.value, fund.nHeight));
//	}
//	for (int i = 0; i < aAccount.vSelfFreeze.size(); ++i) {
//		CFund fund = aAccount.vSelfFreeze[i];
//		array.push_back(tinyformat::format("%-20.20s%-20.20s%-25.8lf%-6.6d",fundTypeArray[fund.nFundType], HexStr(fund.scriptID), fund.value, fund.nHeight));
//	}
//	obj.push_back(Pair("detailinfo:", array));
//	return obj;
//}

static Value AccountLogToJson(const CAccountLog &accoutLog) {
	Object obj;
	obj.push_back(Pair("cKeyId", accoutLog.m_cKeyID.ToString()));
	obj.push_back(Pair("llValues", accoutLog.m_ullValues));
	obj.push_back(Pair("nHeight", accoutLog.m_nHeight));
	obj.push_back(Pair("nCoinDay", accoutLog.m_ullCoinDay));

	return obj;
}

Value gettxoperationlog(const Array& params, bool bHelp) {
	if (bHelp || params.size() != 1) {
		throw runtime_error("gettxoperationlog \"txhash\"\n"
					"\nget transaction operation log\n"
					"\nArguments:\n"
					"1.\"txhash\": (string required) \n"
					"\nResult:\n"
					"\"vOperFund\": (string)\n"
					"\"authorLog\": (string)\n"
					"\nExamples:\n"
					+ HelpExampleCli("gettxoperationlog",
							"\"0001a87352387b5b4d6d01299c0dc178ff044f42e016970b0dc7ea9c72c08e2e494a01020304100000\"")
					+ "\nAs json rpc call\n"
					+ HelpExampleRpc("gettxoperationlog",
							"\"0001a87352387b5b4d6d01299c0dc178ff044f42e016970b0dc7ea9c72c08e2e494a01020304100000\""));
	}
	RPCTypeCheck(params, list_of(str_type));
	uint256 cTxHash(uint256S(params[0].get_str()));
	vector<CAccountLog> vcLog;
	Object retobj;
	retobj.push_back(Pair("hash", cTxHash.GetHex()));
	if (!GetTxOperLog(cTxHash, vcLog)) {
		throw JSONRPCError(RPC_INVALID_PARAMS, "error hash");
	}
	{
		Array arrayvLog;
		for (auto const &te : vcLog) {
			Object obj;
			obj.push_back(Pair("addr", te.m_cKeyID.ToAddress()));
			Array array;
			array.push_back(AccountLogToJson(te));
			arrayvLog.push_back(obj);
		}
		retobj.push_back(Pair("AccountOperLog", arrayvLog));

	}
	return retobj;
}

static Value TestDisconnectBlock(int number) {
	CBlock cBlock;
	Object obj;

	CValidationState cState;
	if ((g_cChainActive.Tip()->m_nHeight - number) < 0) {
		throw JSONRPCError(RPC_INVALID_PARAMS, "restclient Error: number");
	}
	if (number > 0) {
		do {
			// check level 0: read from disk
			CBlockIndex * pTipIndex = g_cChainActive.Tip();
			LogPrint("vm", "current height:%d\n", pTipIndex->m_nHeight);
			if (!DisconnectBlockFromTip(cState)) {
				return false;
			}
			g_cChainMostWork.SetTip(pTipIndex->m_pPrevBlockIndex);
			if (!EraseBlockIndexFromSet(pTipIndex)) {
				return false;
			}
			if (!g_pblocktree->EraseBlockIndex(pTipIndex->GetBlockHash())) {
				return false;
			}
			g_mapBlockIndex.erase(pTipIndex->GetBlockHash());

		} while (--number);
	}

	obj.push_back(Pair("tip", strprintf("hash:%s hight:%s",g_cChainActive.Tip()->GetBlockHash().ToString(),g_cChainActive.Tip()->m_nHeight)));
	return obj;
}

Value disconnectblock(const Array& params, bool bHelp) {
	if (bHelp || params.size() != 1) {
		throw runtime_error("disconnectblock \"numbers\" \n"
				"\ndisconnect block\n"
				"\nArguments:\n"
				"1. \"numbers \"  (numeric, required) the block numbers.\n"
				"\nResult:\n"
				"\"disconnect result\"  (bool) \n"
				"\nExamples:\n"
				+ HelpExampleCli("disconnectblock", "\"1\"")
				+ HelpExampleRpc("gettxoperationlog","\"1\""));
	}
	int nNumber = params[0].get_int();

	Value te = TestDisconnectBlock(nNumber);
	return te;
}

Value resetclient(const Array& params, bool bHelp) {
	if (bHelp || params.size() != 0) {
		throw runtime_error("resetclient \n"
						"\nreset client\n"
						"\nArguments:\n"
						"\nResult:\n"
						"\nExamples:\n"
						+ HelpExampleCli("resetclient", "")
						+ HelpExampleRpc("resetclient",""));
		}
	Value te = TestDisconnectBlock(g_cChainActive.Tip()->m_nHeight);

	if (g_cChainActive.Tip()->m_nHeight == 0) {
		g_pwalletMain->CleanAll();
		CBlockIndex* pBlockIndex = g_cChainActive.Tip();
		uint256 cHash = pBlockIndex->GetBlockHash();
		//		auto ret = remove_if( g_mapBlockIndex.begin(), g_mapBlockIndex.end(),[&](std::map<uint256, CBlockIndex*>::reference a) {
		//			return (a.first == hash);
		//		});
		//		g_mapBlockIndex.erase(ret,g_mapBlockIndex.end());
		for (auto it = g_mapBlockIndex.begin(), ite = g_mapBlockIndex.end(); it != ite;) {
			if (it->first != cHash) {
				it = g_mapBlockIndex.erase(it);
			} else {
				++it;
			}
		}
		g_pAccountViewTip->Flush();
		g_pScriptDBTip->Flush();
		g_pTxCacheTip->Flush();

		assert(g_pAccountViewDB->GetDbCount() == 43);
		assert(g_pScriptDB->GetDbCount() == 0 || g_pScriptDB->GetDbCount() == 1);
		assert(g_pTxCacheTip->GetSize() == 0);

		CBlock cFirs = SysCfg().GenesisBlock();
		g_pwalletMain->SyncTransaction(uint256(), NULL, &cFirs);
		g_cTxMemPool.clear();
	} else {
		throw JSONRPCError(RPC_WALLET_ERROR, "restclient Error: Sign failed.");
	}
	return te;
}

Value listapp(const Array& params, bool bHelp) {
	if (bHelp || params.size() != 1) {
		throw runtime_error("listapp \"showDetail\" \n"
				"\nget the list register script\n"
				"\nArguments:\n"
	            "1. showDetail  (boolean, required)true to show scriptContent,otherwise to not show it.\n"
				"\nResult an object contain many script data\n"
				"\nResult:\n"
				"\nExamples:\n" + HelpExampleCli("listapp", "true") + HelpExampleRpc("listapp", "true"));
	}
	bool bShowDetail = false;
	bShowDetail = params[0].get_bool();
	Object obj;
	Array arrayScript;

	//	CAccountViewCache cView(*g_pAccountViewTip, true);
	if (g_pScriptDBTip != NULL) {
		int nCount(0);
		if (!g_pScriptDBTip->GetScriptCount(nCount)) {
			throw JSONRPCError(RPC_DATABASE_ERROR, "get script error: cannot get registered count.");
		}
		CRegID cRegID;
		vector<unsigned char> vuchScript;
		Object script;
		if (!g_pScriptDBTip->GetScript(0, cRegID, vuchScript)) {
			throw JSONRPCError(RPC_DATABASE_ERROR, "get script error: cannot get registered script.");
		}
		script.push_back(Pair("scriptId", cRegID.ToString()));
		script.push_back(Pair("scriptId2", HexStr(cRegID.GetVec6())));
		CDataStream cDs(vuchScript, SER_DISK, g_sClientVersion);
		CVmScript cVmScript;
		cDs >> cVmScript;
		string strDes(cVmScript.m_vuchScriptExplain.begin(), cVmScript.m_vuchScriptExplain.end());
		script.push_back(Pair("description", HexStr(cVmScript.m_vuchScriptExplain)));

		if (bShowDetail) {
			script.push_back(Pair("scriptContent", HexStr(cVmScript.m_vuchRom.begin(), cVmScript.m_vuchRom.end())));
		}
		arrayScript.push_back(script);
		while (g_pScriptDBTip->GetScript(1, cRegID, vuchScript)) {
			Object obj;
			obj.push_back(Pair("scriptId", cRegID.ToString()));
			obj.push_back(Pair("scriptId2", HexStr(cRegID.GetVec6())));
			CDataStream cDs(vuchScript, SER_DISK, g_sClientVersion);
			CVmScript cVmScript;
			cDs >> cVmScript;
			string strDes(cVmScript.m_vuchScriptExplain.begin(), cVmScript.m_vuchScriptExplain.end());
			obj.push_back(Pair("description", HexStr(cVmScript.m_vuchScriptExplain)));
			if (bShowDetail) {
				obj.push_back(Pair("scriptContent", HexStr(cVmScript.m_vuchRom.begin(), cVmScript.m_vuchRom.end())));
			}
			arrayScript.push_back(obj);
		}
	}

	obj.push_back(Pair("listregedscript", arrayScript));
	return obj;
}

Value getappinfo(const Array& params, bool bHelp) {
	if (bHelp || params.size() != 1)
	        throw runtime_error(
	            "getappinfo ( \"scriptid\" )\n"
	            "\nget app information.\n"
	            "\nArguments:\n"
	            "1. \"scriptid\"    (string). The script ID. \n"
	            "\nget app information in the systems\n"
				"\nExamples:\n" + HelpExampleCli("getappinfo", "123-1") + HelpExampleRpc("getappinfo", "123-1"));

	string strRegId = params[0].get_str();
	CRegID cRegId(strRegId);
	if (cRegId.IsEmpty() == true) {
		throw runtime_error("in getappinfo :scriptid size is error!\n");
	}

	if (!g_pScriptDBTip->HaveScript(cRegId)) {
		throw runtime_error("in getappinfo :scriptid  is not exist!\n");
	}

	vector<unsigned char> vuchScript;
	if (!g_pScriptDBTip->GetScript(cRegId, vuchScript)) {
		throw JSONRPCError(RPC_DATABASE_ERROR, "get script error: cannot get registered script.");
	}

	Object obj;
	obj.push_back(Pair("scriptId", cRegId.ToString()));
	obj.push_back(Pair("scriptId2", HexStr(cRegId.GetVec6())));
	CDataStream cDs(vuchScript, SER_DISK, g_sClientVersion);
	CVmScript cVmScript;
	cDs >> cVmScript;
	obj.push_back(Pair("description", HexStr(cVmScript.m_vuchScriptExplain)));
	obj.push_back(Pair("scriptContent", HexStr(cVmScript.m_vuchRom.begin(), cVmScript.m_vuchRom.end())));
	return obj;
}

Value getaddrbalance(const Array& params, bool bHelp) {
	if (bHelp || params.size() != 1) {
		string msg = "getaddrbalance nrequired [\"key\",...] ( \"cAccount\" )\n"
				"\nAdd a nrequired-to-sign multisignature address to the wallet.\n"
				"Each key is a  address or hex-encoded public key.\n" + HelpExampleCli("getaddrbalance", "")
				+ "\nAs json rpc call\n" + HelpExampleRpc("getaddrbalance", "");
		throw runtime_error(msg);
	}

	assert(g_pwalletMain != NULL);

	CKeyID cKeyId;
	if (!GetKeyId(params[0].get_str(), cKeyId)) {
		throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid  address");
	}
	double dbalance = 0.0;
	{
		LOCK(g_cs_main);
		CAccountViewCache cAccView(*g_pAccountViewTip, true);
		CAccount cSecureAcc;
		CUserID cUserId = cKeyId;
		if (cAccView.GetAccount(cUserId, cSecureAcc)) {
			dbalance = (double) cSecureAcc.GetRawBalance() / (double) COIN;
		}
	}
	return dbalance;
}

Value generateblock(const Array& params, bool bHelp) {
	if (bHelp || params.size() != 1) {
		throw runtime_error("generateblock \"addr\"\n"
				"\ncteate a block with the appointed address\n"
				"\nArguments:\n"
				"1.\"addr\": (string, required)\n"
				"\nResult:\n"
				"\nblockhash\n"
				"\nExamples:\n" +
				HelpExampleCli("generateblock", "\"5Vp1xpLT8D2FQg3kaaCcjqxfdFNRhxm4oy7GXyBga9\"")
				+ "\nAs json rpc call\n"
				+ HelpExampleRpc("generateblock", "\"5Vp1xpLT8D2FQg3kaaCcjqxfdFNRhxm4oy7GXyBga9\""));
	}
	//get cKeyId
	CKeyID cKeyId;

	if (!GetKeyId(params[0].get_str(), cKeyId)) {
		throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "in generateblock :address err");
	}

	uint256 cHash = CreateBlockWithAppointedAddr(cKeyId);
	if (cHash.IsNull()) {
		throw runtime_error("in generateblock :cannot generate block\n");
	}
	Object obj;
	obj.push_back(Pair("blockhash", cHash.GetHex()));
	return obj;
}

Value listtxcache(const Array& params, bool bHelp) {
	if (bHelp || params.size() != 0) {
		throw runtime_error("listtxcache\n"
				"\nget all transactions in cahce\n"
				"\nArguments:\n"
				"\nResult:\n"
				"\"txcache\"  (string) \n"
				"\nExamples:\n" + HelpExampleCli("listtxcache", "")+ HelpExampleRpc("listtxcache", ""));
	}
	const map<uint256, vector<uint256> > &mapTxHashByBlockHash = g_pTxCacheTip->GetTxHashCache();

	Array retTxHashArray;
	for (auto &item : mapTxHashByBlockHash) {
		Object blockObj;
		Array txHashArray;
		blockObj.push_back(Pair("blockhash", item.first.GetHex()));
		for (auto &txHash : item.second) {
			txHashArray.push_back(txHash.GetHex());
		}
		blockObj.push_back(Pair("txcache", txHashArray));
		retTxHashArray.push_back(blockObj);
	}

	return retTxHashArray;
}

Value reloadtxcache(const Array& params, bool bHelp) {
	if (bHelp || params.size() != 0) {
		throw runtime_error("reloadtxcache \n"
				"\nreload transactions catch\n"
				"\nArguments:\n"
				"\nResult:\n"
				"\nExamples:\n"
				+ HelpExampleCli("reloadtxcache", "")
				+ HelpExampleRpc("reloadtxcache", ""));
	}
	g_pTxCacheTip->Clear();
	CBlockIndex *pIndex = g_cChainActive.Tip();
	if ((g_cChainActive.Tip()->m_nHeight - SysCfg().GetTxCacheHeight()) >= 0) {
		pIndex = g_cChainActive[(g_cChainActive.Tip()->m_nHeight - SysCfg().GetTxCacheHeight())];
	} else {
		pIndex = g_cChainActive.Genesis();
	}
	CBlock cBlock;
	do {
		if (!ReadBlockFromDisk(cBlock, pIndex))
			return ERRORMSG("reloadtxcache() : *** ReadBlockFromDisk failed at %d, hash=%s", pIndex->m_nHeight,
					pIndex->GetBlockHash().ToString());
		g_pTxCacheTip->AddBlockToCache(cBlock);
		pIndex = g_cChainActive.Next(pIndex);
	} while (NULL != pIndex);

	Object obj;
	obj.push_back(Pair("info", "reload tx cache succeed"));
	return obj;
}

static int getDataFromSriptData(CScriptDBViewCache &cache, const CRegID &cRegId, int pagesize, int index,
		vector<std::tuple<vector<unsigned char>, vector<unsigned char> > >&ret) {
	int nDbsize;
	int nHeight = g_cChainActive.Height();
	cache.GetScriptDataCount(cRegId, nDbsize);
	if (0 == nDbsize) {
		throw runtime_error("in getscriptdata :the scirptid database not data!\n");
	}
	vector<unsigned char> vuchValue;
	vector<unsigned char> vuchScriptKey;

	if (!cache.GetScriptData(nHeight, cRegId, 0, vuchScriptKey, vuchValue)) {
		throw runtime_error("in getscriptdata :the scirptid get data failed!\n");
	}
	if (index == 1) {
		ret.push_back(std::make_tuple(vuchScriptKey, vuchValue));
	}
	int nReadCount(1);
	while (--nDbsize) {
		if (cache.GetScriptData(nHeight, cRegId, 1, vuchScriptKey, vuchValue)) {
			++nReadCount;
			if (nReadCount > pagesize * (index - 1)) {
				ret.push_back(std::make_tuple(vuchScriptKey, vuchValue));
			}
		}
		if (nReadCount >= pagesize * index) {
			return ret.size();
		}
	}
	return ret.size();
}

Value getscriptdata(const Array& params, bool bHelp) {
	if (bHelp || params.size() < 2 || params.size() > 3) {
		throw runtime_error("getscriptdata \"scriptid\" \"[pagesize or key]\" (\"index\")\n"
				"\nget the script data by given scriptID\n"
				"\nArguments:\n"
				"1.\"scriptid\": (string, required)\n"
				"2.[pagesize or key]: (pagesize int, required),if only two param,it is key,otherwise it is pagesize\n"
				"3.\"index\": (int optional)\n"
				"\nResult:\n"
				"\nExamples:\n"
				+ HelpExampleCli("getscriptdata", "\"123456789012\"")
				+ HelpExampleRpc("getscriptdata", "\"123456789012\""));
	}
	int nHeight = g_cChainActive.Height();
	//	//RPCTypeCheck(params, list_of(str_type)(int_type)(int_type));
	//	vector<unsigned char> vscriptid = ParseHex(params[0].get_str());
	CRegID cRegId(params[0].get_str());
	if (cRegId.IsEmpty() == true) {
		throw runtime_error("in getscriptdata :vscriptid size is error!\n");
	}

	if (!g_pScriptDBTip->HaveScript(cRegId)) {
		throw runtime_error("in getscriptdata :vscriptid id is exist!\n");
	}
	Object script;

	CScriptDBViewCache cContractScriptTemp(*g_pScriptDBTip, true);
	if (params.size() == 2) {
		vector<unsigned char> vchKey = ParseHex(params[1].get_str());
		vector<unsigned char> vchValue;
		if (!cContractScriptTemp.GetScriptData(nHeight, cRegId, vchKey, vchValue)) {
			throw runtime_error("in getscriptdata :the key not exist!\n");
		}
		script.push_back(Pair("scritpid", params[0].get_str()));
		script.push_back(Pair("key", HexStr(vchKey)));
		script.push_back(Pair("value", HexStr(vchValue)));
		return script;

	} else {
		int nDbsize;
		cContractScriptTemp.GetScriptDataCount(cRegId, nDbsize);
		if (0 == nDbsize) {
			throw runtime_error("in getscriptdata :the scirptid database not data!\n");
		}
		int nPagesize = params[1].get_int();
		int nIndex = params[2].get_int();

		vector<std::tuple<vector<unsigned char>, vector<unsigned char> > > ret;
		getDataFromSriptData(cContractScriptTemp, cRegId, nPagesize, nIndex, ret);
		Array retArray;
		for (auto te : ret) {
			vector<unsigned char> key = std::get<0>(te);
			vector<unsigned char> valvue = std::get<1>(te);
			Object firt;
			firt.push_back(Pair("key", HexStr(key)));
			firt.push_back(Pair("value", HexStr(valvue)));
			retArray.push_back(firt);
		}
		return retArray;
	}

	return script;
}

Value getscriptvalidedata(const Array& params, bool bHelp) {
	if (bHelp || (params.size() != 3 && params.size() !=4)) {
		throw runtime_error("getscriptvalidedata \"scriptid\" \"pagesize\" \"index\"\n"
					"\nget script valide data\n"
					"\nArguments:\n"
					"1.\"scriptid\": (string, required)\n"
					"2.\"pagesize\": (int, required)\n"
					"3.\"index\": (int, required )\n"
					"4.\"minconf\":  (numeric, optional, default=1) Only include contract transactions confirmed \n"
				    "\nResult:\n"
				    "\nExamples:\n"
				    + HelpExampleCli("getscriptvalidedata", "\"123456789012\" \"1\"  \"1\"")
				    + HelpExampleRpc("getscriptvalidedata", "\"123456789012\" \"1\"  \"1\""));
	}
	std::shared_ptr<CScriptDBViewCache> pAccountViewCache;
	if (4 == params.size() && 0 == params[3].get_int()) {
		pAccountViewCache.reset(new CScriptDBViewCache(*g_cTxMemPool.m_pScriptDBViewCache, true));
	} else {
		pAccountViewCache.reset(new CScriptDBViewCache(*g_pScriptDBTip, true));
	}
	int nHeight = g_cChainActive.Height();
	RPCTypeCheck(params, list_of(str_type)(int_type)(int_type));
	CRegID cRegId(params[0].get_str());
	if (cRegId.IsEmpty() == true) {
		throw runtime_error("in getscriptdata :vscriptid size is error!\n");
	}
	if (!pAccountViewCache->HaveScript(cRegId)) {
		throw runtime_error("in getscriptdata :vscriptid id is exist!\n");
	}
	Object obj;
	int nPagesize = params[1].get_int();
	int nIndex = params[2].get_int();

	int nKey = revert(nHeight);
	CDataStream cDs(SER_NETWORK, g_sProtocolVersion);
	cDs << nKey;
	std::vector<unsigned char> vuchScriptKey(cDs.begin(), cDs.end());
	std::vector<unsigned char> vuchValue;
	Array retArray;
	int nReadCount = 0;
	while (pAccountViewCache->GetScriptData(nHeight, cRegId, 1, vuchScriptKey, vuchValue)) {
		Object item;
		++nReadCount;
		if (nReadCount > nPagesize * (nIndex - 1)) {
			item.push_back(Pair("key", HexStr(vuchScriptKey)));
			item.push_back(Pair("value", HexStr(vuchValue)));
			retArray.push_back(item);
		}
		if (nReadCount >= nPagesize * nIndex) {
			break;
		}
	}
	return retArray;
}

Value saveblocktofile(const Array& params, bool bHelp) {
	if (bHelp || params.size() != 2) {
		throw runtime_error("saveblocktofile \"blockhash\" \"filepath\"\n"
				"\n save the given block info to the given file\n"
				"\nArguments:\n"
				"1.\"blockhash\": (string, required)\n"
				"2.\"filepath\": (string, required)\n"
				"\nResult:\n"
		        "\nExamples:\n"
				+ HelpExampleCli("saveblocktofile", "\"12345678901211111\" \"block.log\"")
				+ HelpExampleRpc("saveblocktofile", "\"12345678901211111\" \"block.log\""));
	}
	string strBlockHash = params[0].get_str();
	uint256 cBlockHash(uint256S(params[0].get_str()));
	if (0 == g_mapBlockIndex.count(cBlockHash)) {
		throw JSONRPCError(RPC_MISC_ERROR, "block hash is not exist!");
	}
	CBlockIndex *pIndex = g_mapBlockIndex[cBlockHash];
	CBlock cBlockInfo;
	if (!pIndex || !ReadBlockFromDisk(cBlockInfo, pIndex)) {
		throw runtime_error(_("Failed to read block"));
	}
	assert(strBlockHash == cBlockInfo.GetHash().ToString());
	//	CDataStream cDs(SER_NETWORK, g_sProtocolVersion);
	//	cDs << blockInfo.GetBlockHeader();
	//	cout << "block header:" << HexStr(cDs) << endl;
	string strFile = params[1].get_str();
	try {
		FILE* pFile = fopen(strFile.c_str(), "wb+");
		CAutoFile cFileout = CAutoFile(pFile, SER_DISK, g_sClientVersion);
		if (!cFileout) {
			throw JSONRPCError(RPC_MISC_ERROR, "open file:" + strBlockHash + "failed!");
		}
		if (g_cChainActive.Contains(pIndex)) {
			cFileout << pIndex->m_nHeight;
		}

		cFileout << cBlockInfo;
		fflush(cFileout);
		// fileout object auto free fp point, don't need double free fp point.
		// fclose(fp);
	} catch (std::exception &e) {
		throw JSONRPCError(RPC_MISC_ERROR, "save block to file error");
	}
	return "save succeed";
}

Value getscriptdbsize(const Array& params, bool bHelp) {
	if (bHelp || params.size() != 1) {
		throw runtime_error("getscriptdbsize \"scriptid\"\n"
							"\nget script data count\n"
							"\nArguments:\n"
							"1.\"scriptid\": (string, required)\n"
							"\nResult:\n"
							"\nExamples:\n"
							+ HelpExampleCli("getscriptdbsize", "\"123456789012\"")
							+ HelpExampleRpc("getscriptdbsize","\"123456789012\""));
	}
	CRegID cRegId(params[0].get_str());
	if (cRegId.IsEmpty() == true) {
		throw runtime_error("in getscriptdata :vscriptid is error!\n");
	}

	if (!g_pScriptDBTip->HaveScript(cRegId)) {
		throw runtime_error("in getscriptdata :vscriptid id is not exist!\n");
	}
	int nDataCount = 0;
	if (!g_pScriptDBTip->GetScriptDataCount(cRegId, nDataCount)) {
		throw runtime_error("GetScriptDataCount error!");
	}
	return nDataCount;
}

Value registaccounttxraw(const Array& params, bool bHelp) {
	if (bHelp || (params.size() < 2  || params.size() > 4)) {
		throw runtime_error("registaccounttxraw \"ullFee\" \"publickey\" (\"minerpublickey\") (\"height\")\n"
				"\ncreate a register cAccount transaction\n"
				"\nArguments:\n"
				"1.ullFee: (numeric, required) pay to miner\n"
				"2.publickey: (string, required)\n"
				"3.minerpublickey: (string,optional)\n"
				"4.height: (numeric, optional) pay to miner\n"
				"\nResult:\n"
				"\"txhash\": (string)\n"
				"\nExamples:\n"
				+ HelpExampleCli("registaccounttxraw",  "10000 \"038f679e8b63d6f9935e8ca6b7ce1de5257373ac5461874fc794004a8a00a370ae\" \"026bc0668c767ab38a937cb33151bcf76eeb4034bcb75e1632fd1249d1d0b32aa9\" 10 ")
				+ "\nAs json rpc call\n"
				+ HelpExampleRpc("registaccounttxraw", " 10000 \"038f679e8b63d6f9935e8ca6b7ce1de5257373ac5461874fc794004a8a00a370ae\" \"026bc0668c767ab38a937cb33151bcf76eeb4034bcb75e1632fd1249d1d0b32aa9\" 10"));
	}
	CUserID cUserkey;
	CUserID cUserMinerkey = CNullID();

	int64_t ullFee = AmountToRawValue(params[0]);

	CKeyID cDummy;
	CPubKey cPubk = CPubKey(ParseHex(params[1].get_str()));
	if (!cPubk.IsCompressed() || !cPubk.IsFullyValid()) {
		throw JSONRPCError(RPC_INVALID_PARAMS, "CPubKey err");
	}
	cUserkey = cPubk;
	cDummy = cPubk.GetKeyID();

	if (params.size() > 2) {
		CPubKey pubk = CPubKey(ParseHex(params[2].get_str()));
		if (!pubk.IsCompressed() || !pubk.IsFullyValid()) {
			throw JSONRPCError(RPC_INVALID_PARAMS, "CPubKey err");
		}
		cUserMinerkey = pubk;
	}

	int nHight = g_cChainActive.Tip()->m_nHeight;
	if (params.size() > 3) {
		nHight = params[3].get_int();
	}

	std::shared_ptr<CRegisterAccountTx> tx = std::make_shared<CRegisterAccountTx>(cUserkey, cUserMinerkey, ullFee, nHight);
	CDataStream cDs(SER_DISK, g_sClientVersion);
	std::shared_ptr<CBaseTransaction> pBaseTx = tx->GetNewInstance();
	cDs << pBaseTx;
	Object obj;
	obj.push_back(Pair("rawtx", HexStr(cDs.begin(), cDs.end())));
	obj.push_back(Pair("signhash", pBaseTx->SignatureHash().GetHex()));
	return obj;
}

Value submittx(const Array& params, bool bHelp) {
	if (bHelp || params.size() != 1) {
		throw runtime_error("submittx \"transaction\" \n"
				"\nsubmit transaction\n"
				"\nArguments:\n"
				"1.\"transaction\": (string, required)\n"
				"\nExamples:\n"
				+ HelpExampleCli("submittx", "\"n2dha9w3bz2HPVQzoGKda3Cgt5p5Tgv6oj\"")
				+ "\nAs json rpc call\n"
				+ HelpExampleRpc("submittx", "\"n2dha9w3bz2HPVQzoGKda3Cgt5p5Tgv6oj\""));
	}
	EnsureWalletIsUnlocked();
	vector<unsigned char> vuch(ParseHex(params[0].get_str()));
	CDataStream cDataStream(vuch, SER_DISK, g_sClientVersion);

	std::shared_ptr<CBaseTransaction> tx;
	cDataStream >> tx;
	std::tuple<bool, string> ret;

	std::shared_ptr<CRegisterAccountTx> pRegAcctTx = std::make_shared<CRegisterAccountTx>(tx.get());
	LogPrint("INFO", "pubkey:%s keyId:%s\n", boost::get<CPubKey>(pRegAcctTx->m_cUserId).ToString(),
			boost::get<CPubKey>(pRegAcctTx->m_cUserId).GetKeyID().ToString());
	ret = g_pwalletMain->CommitTransaction((CBaseTransaction *) tx.get());
	if (!std::get<0>(ret)) {
		throw JSONRPCError(RPC_WALLET_ERROR, "submittx Error:" + std::get<1>(ret));
	}
	Object obj;
	obj.push_back(Pair("hash", std::get<1>(ret)));
	return obj;
}

Value createcontracttxraw(const Array& params, bool bHelp) {
	if (bHelp || params.size() < 5 || params.size() > 6) {
		throw runtime_error("createcontracttxraw \"height\" \"ullFee\" \"llAmount\" \"addr\" \"contract\" \n"
				"\ncreate contract\n"
				"\nArguments:\n"
				"1.\"ullFee\": (numeric, required) pay to miner\n"
				"2.\"llAmount\": (numeric, required)\n"
				"3.\"addr\": (string, required)\n"
				"4.\"cAppId\": (string required)"
				"5.\"contract\": (string, required)\n"
				"6.\"height\": (int, optional)create height\n"
				"\nResult:\n"
				"\"contract tx str\": (string)\n"
				"\nExamples:\n"
				+ HelpExampleCli("createcontracttxraw",
						"1000 01020304 000000000100 [\"5zQPcC1YpFMtwxiH787pSXanUECoGsxUq3KZieJxVG\"] "
								"[\"5yNhSL7746VV5qWHHDNLkSQ1RYeiheryk9uzQG6C5d\","
								"\"5Vp1xpLT8D2FQg3kaaCcjqxfdFNRhxm4oy7GXyBga9\"] 10") + "\nAs json rpc call\n"
				+ HelpExampleRpc("createcontracttxraw",
						"1000 01020304 000000000100 000000000100 [\"5zQPcC1YpFMtwxiH787pSXanUECoGsxUq3KZieJxVG\"] "
								"[\"5yNhSL7746VV5qWHHDNLkSQ1RYeiheryk9uzQG6C5d\","
								"\"5Vp1xpLT8D2FQg3kaaCcjqxfdFNRhxm4oy7GXyBga9\"] 10"));
	}

	RPCTypeCheck(params, list_of(int_type)(real_type)(real_type)(str_type)(str_type)(str_type));


	uint64_t ullFee = AmountToRawValue(params[0]);
	uint64_t ullAmount = AmountToRawValue(params[1]);
	CRegID cUserId(params[2].get_str());
	CRegID cAppId(params[3].get_str());

	vector<unsigned char> vuchContract = ParseHex(params[4].get_str());

	if (ullFee > 0 && ullFee < CTransaction::m_sMinTxFee) {
		throw runtime_error("in createcontracttxraw :ullFee is smaller than nMinTxFee\n");
	}
	if (cAppId.IsEmpty()) {
		throw runtime_error("in createcontracttxraw :addresss is error!\n");
	}

	CAccountViewCache cView(*g_pAccountViewTip, true);
	CAccount cSecureAcc;

	if (!g_pScriptDBTip->HaveScript(cAppId)) {
		throw runtime_error(tinyformat::format("createcontracttx :app id %s is not exist\n", cAppId.ToString()));
	}

	CKeyID cKeyId;
	if (!cView.GetKeyId(cUserId, cKeyId)) {
		CID id(cUserId);
		LogPrint("INFO", "vaccountid:%s have no key id\r\n", HexStr(id.GetID()).c_str());
		// assert(0);
		throw runtime_error(
				tinyformat::format("createcontracttx :vaccountid:%s have no key id\r\n", HexStr(id.GetID()).c_str()));
	}

	int nHeight = g_cChainActive.Tip()->m_nHeight;
	if (params.size() > 5) {
		nHeight = params[5].get_int();
	}

	std::shared_ptr<CTransaction> tx = std::make_shared<CTransaction>(cUserId, cAppId, ullFee, ullAmount, nHeight,
			vuchContract);

	CDataStream cDs(SER_DISK, g_sClientVersion);
	std::shared_ptr<CBaseTransaction> pBaseTx = tx->GetNewInstance();
	cDs << pBaseTx;
	Object obj;
	obj.push_back(Pair("rawtx", HexStr(cDs.begin(), cDs.end())));
	return obj;
}

Value registerscripttxraw(const Array& params, bool bHelp) {
	if (bHelp || params.size() < 4) {
		throw runtime_error("registerscripttxraw \"height\" \"ullFee\" \"addr\" \"flag\" \"script or scriptid\" (\"script description\")\n"
						"\nregister script\n"
						"\nArguments:\n"
						"1.\"ullFee\": (numeric required) pay to miner\n"
						"2.\"addr\": (string required)\nfor send"
						"3.\"flag\": (bool, required) 0-1\n"
						"4.\"script or scriptid\": (string required), if flag=0 is script's file path, else if flag=1 scriptid\n"
						"5.\"height\": (int required)valid height\n"
						"6.\"script description\":(string optional) new script description\n"
						"\nResult:\n"
						"\"txhash\": (string)\n"
						"\nExamples:\n"
						+ HelpExampleCli("registerscripttxraw",
								"\"10\" \"10000\" \"5zQPcC1YpFMtwxiH787pSXanUECoGsxUq3KZieJxVG\" \"1\" \"010203040506\" ")
						+ "\nAs json rpc call\n"
						+ HelpExampleRpc("registerscripttxraw",
								"\"10\" \"10000\" \"5zQPcC1YpFMtwxiH787pSXanUECoGsxUq3KZieJxVG\" \"1\" \"010203040506\" "));
	}

	RPCTypeCheck(params, list_of(real_type)(str_type)(int_type)(str_type)(int_type)(str_type));

	uint64_t ullFee = AmountToRawValue(params[0]);

	CVmScript cVmScript;
	vector<unsigned char> vuchScript;
	int bFlag = params[2].get_bool();
	if (0 == bFlag) {
		string strPath = params[3].get_str();
		FILE* pFile = fopen(strPath.c_str(), "rb+");
		if (!pFile) {
			throw runtime_error("create registerapptx open script file" + strPath + "error");
		}
		long lSize;
		fseek(pFile, 0, SEEK_END);
		lSize = ftell(pFile);
		rewind(pFile);

		// allocate memory to contain the whole file:
		char *pchBuffer = (char*) malloc(sizeof(char) * lSize);
		if (pchBuffer == NULL) {
			fclose(pFile); //及时关闭
			throw runtime_error("allocate memory failed");
		}

		if (fread(pchBuffer, 1, lSize, pFile) != (size_t) lSize) {
			free(pchBuffer);
			fclose(pFile); //及时关闭
			throw runtime_error("read script file error");
		} else {
			fclose(pFile);
		}
		cVmScript.m_vuchRom.insert(cVmScript.m_vuchRom.end(), pchBuffer, pchBuffer + lSize);
		if (pchBuffer) {
			free(pchBuffer);
		}
		CDataStream cDs(SER_DISK, g_sClientVersion);
		cDs << cVmScript;

		vuchScript.assign(cDs.begin(), cDs.end());

	} else if (1 == bFlag) {
		vuchScript = ParseHex(params[3].get_str());
	}

	if (params.size() > 5) {
		string strScriptDesc = params[5].get_str();
		cVmScript.m_vuchScriptExplain.insert(cVmScript.m_vuchScriptExplain.end(), strScriptDesc.begin(), strScriptDesc.end());
	}
	if (ullFee > 0 && ullFee < CTransaction::m_sMinTxFee) {
		throw runtime_error("in registerapptx :ullFee is smaller than nMinTxFee\n");
	}
	//get cKeyId
	CKeyID cKeyId;
	if (!GetKeyId(params[2].get_str(), cKeyId)) {
		throw runtime_error("in registerapptx :send address err\n");
	}

	//balance
	CAccountViewCache cView(*g_pAccountViewTip, true);
	CAccount cAccount;

	CUserID cUserId = cKeyId;
	if (!cView.GetAccount(cUserId, cAccount)) {
		throw JSONRPCError(RPC_WALLET_ERROR, "in registerscripttxraw Error: Account is not exist.");
	}
	if (!cAccount.IsRegister()) {
		throw JSONRPCError(RPC_WALLET_ERROR, "in registerscripttxraw Error: Account is not registered.");
	}
	if (bFlag) {
		vector<unsigned char> vscriptcontent;
		CRegID cRegId(params[2].get_str());
		if (!g_pScriptDBTip->GetScript(CRegID(vuchScript), vscriptcontent)) {
			throw JSONRPCError(RPC_WALLET_ERROR, "script id find failed");
		}
	}
	auto GetUserId = [&](CKeyID &mkeyId)
	{
		CAccount cAcct;
		if (cView.GetAccount(CUserID(mkeyId), cAcct)) {
			return cAcct.m_cRegID;
		}
		throw runtime_error(
				tinyformat::format("registerscripttxraw :cAccount id %s is not exist\n", mkeyId.ToAddress()));
	};
	std::shared_ptr<CRegisterAppTx> tx = std::make_shared<CRegisterAppTx>();
	tx.get()->m_cRegAcctId = GetUserId(cKeyId);
	tx.get()->m_vchScript = vuchScript;
	tx.get()->m_ullFees = ullFee;

	uint32_t nHeight = g_cChainActive.Tip()->m_nHeight;
	if (params.size() > 4) {
		nHeight = params[4].get_int();
	}
	tx.get()->m_nValidHeight = nHeight;

	CDataStream cDs(SER_DISK, g_sClientVersion);
	std::shared_ptr<CBaseTransaction> pBaseTx = tx->GetNewInstance();
	cDs << pBaseTx;
	Object obj;
	obj.push_back(Pair("rawtx", HexStr(cDs.begin(), cDs.end())));
	return obj;
}

Value sigstr(const Array& params, bool bHelp) {
	if (bHelp || params.size() != 2) {
		throw runtime_error("sigstr \"str\" \"addr\"\n"
				"\nsignature transaction\n"
				"\nArguments:\n"
				"1.\"str\": (string, required) sig str\n"
				"2.\"addr\": (string, required)\n"
				"\nExamples:\n"
				+ HelpExampleCli("sigstr", "\"1010000010203040506\" \"5zQPcC1YpFMtwxiH787pSXanUECoGsxUq3KZieJxVG\" ")
				+ "\nAs json rpc call\n"
				+ HelpExampleRpc("sigstr", "\"1010000010203040506\" \"5zQPcC1YpFMtwxiH787pSXanUECoGsxUq3KZieJxVG 010203040506\" "));
	}
	vector<unsigned char> vuch(ParseHex(params[0].get_str()));
	string strAddr = params[1].get_str();
	CKeyID cKeyId;
	if (!GetKeyId(params[1].get_str(), cKeyId)) {
		throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "to address Invalid  ");
	}
	CDataStream cDataStream(vuch, SER_DISK, g_sClientVersion);
	std::shared_ptr<CBaseTransaction> pBaseTx;
	cDataStream >> pBaseTx;
	CAccountViewCache cView(*g_pAccountViewTip, true);
	Object obj;
	switch (pBaseTx.get()->m_chTxType) {
	case EM_COMMON_TX: {
		std::shared_ptr<CTransaction> tx = std::make_shared<CTransaction>(pBaseTx.get());
		CKeyID cKeyId;
		if (!cView.GetKeyId(tx.get()->m_cSrcRegId, cKeyId)) {
			throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "vaccountid have no key id");
		}
		if (!g_pwalletMain->Sign(cKeyId, tx.get()->SignatureHash(), tx.get()->m_vchSignature)) {
			throw JSONRPCError(RPC_INVALID_PARAMETER, "Sign failed");
		}
		CDataStream cDs(SER_DISK, g_sClientVersion);
		std::shared_ptr<CBaseTransaction> pBaseTx = tx->GetNewInstance();
		cDs << pBaseTx;
		obj.push_back(Pair("rawtx", HexStr(cDs.begin(), cDs.end())));
	}
		break;
	case EM_REG_ACCT_TX: {
		std::shared_ptr<CRegisterAccountTx> tx = std::make_shared<CRegisterAccountTx>(pBaseTx.get());
		if (!g_pwalletMain->Sign(cKeyId, tx.get()->SignatureHash(), tx.get()->m_vchSignature)) {
			throw JSONRPCError(RPC_INVALID_PARAMETER, "Sign failed");
		}
		CDataStream cDs(SER_DISK, g_sClientVersion);
		std::shared_ptr<CBaseTransaction> pBaseTx = tx->GetNewInstance();
		cDs << pBaseTx;
		obj.push_back(Pair("rawtx", HexStr(cDs.begin(), cDs.end())));
	}
		break;
	case EM_CONTRACT_TX: {
		std::shared_ptr<CTransaction> tx = std::make_shared<CTransaction>(pBaseTx.get());
		if (!g_pwalletMain->Sign(cKeyId, tx.get()->SignatureHash(), tx.get()->m_vchSignature)) {
			throw JSONRPCError(RPC_INVALID_PARAMETER, "Sign failed");
		}
		CDataStream cDs(SER_DISK, g_sClientVersion);
		std::shared_ptr<CBaseTransaction> pBaseTx = tx->GetNewInstance();
		cDs << pBaseTx;
		obj.push_back(Pair("rawtx", HexStr(cDs.begin(), cDs.end())));
	}
		break;
	case EM_REWARD_TX: {
		break;
	}
	case EM_REG_APP_TX: {
		std::shared_ptr<CRegisterAppTx> tx = std::make_shared<CRegisterAppTx>(pBaseTx.get());
		if (!g_pwalletMain->Sign(cKeyId, tx.get()->SignatureHash(), tx.get()->m_vchSignature)) {
			throw JSONRPCError(RPC_INVALID_PARAMETER, "Sign failed");
		}
		CDataStream cDs(SER_DISK, g_sClientVersion);
		std::shared_ptr<CBaseTransaction> pBaseTx = tx->GetNewInstance();
		cDs << pBaseTx;
		obj.push_back(Pair("rawtx", HexStr(cDs.begin(), cDs.end())));
	}
		break;
	default: {
		break;
	}
	}
	return obj;
}

Value getalltxinfo(const Array& params, bool bHelp) {
	if (bHelp || (params.size() != 0 && params.size() != 1)) {
		throw runtime_error("getalltxinfo \n"
				"\nget all transaction info\n"
				"\nArguments:\n"
				"1.\"nlimitCount\": (numeric, optional, default=0) 0 return all tx, else return number of nlimitCount txs \n"
				"\nResult:\n"
				"\nExamples:\n" + HelpExampleCli("getalltxinfo", "") + "\nAs json rpc call\n"
				+ HelpExampleRpc("getalltxinfo", ""));
	}

	Object retObj;
	int nLimitCount(0);
	if (params.size() == 1) {
		nLimitCount = params[0].get_int();
	}
	assert(g_pwalletMain != NULL);
	if (nLimitCount <= 0) {
		Array ComfirmTx;
		for (auto const &wtx : g_pwalletMain->m_mapInBlockTx) {
			for (auto const & item : wtx.second.m_mapAccountTx) {
				Object objtx = GetTxDetailJSON(item.first);
				ComfirmTx.push_back(objtx);
			}
		}
		retObj.push_back(Pair("Confirmed", ComfirmTx));

		Array UnComfirmTx;
		CAccountViewCache cView(*g_pAccountViewTip, true);
		for (auto const &wtx : g_pwalletMain->m_mapUnConfirmTx) {
			Object objtx = GetTxDetailJSON(wtx.first);
			UnComfirmTx.push_back(objtx);
		}
		retObj.push_back(Pair("UnConfirmed", UnComfirmTx));
	} else {
		Array ComfirmTx;
		multimap<int, Object, std::greater<int> > mapTx;
		for (auto const &wtx : g_pwalletMain->m_mapInBlockTx) {
			for (auto const & item : wtx.second.m_mapAccountTx) {
				Object objtx = GetTxDetailJSON(item.first);
				int nConfHeight = find_value(objtx, "confirmHeight").get_int();
				mapTx.insert(pair<int, Object>(nConfHeight, objtx));
			}
		}
		int nSize(0);
		for (auto & txItem : mapTx) {
			if (++nSize > nLimitCount)
				break;
			ComfirmTx.push_back(txItem.second);
		}
		retObj.push_back(Pair("Confirmed", ComfirmTx));
	}

	return retObj;
}

Value printblokdbinfo(const Array& params, bool bHelp) {
	if (bHelp || params.size() != 0) {
		throw runtime_error("printblokdbinfo \n"
				"\nprint block log\n"
				"\nArguments:\n"
				"\nExamples:\n" + HelpExampleCli("printblokdbinfo", "")
				+ HelpExampleRpc("printblokdbinfo", ""));
	}

	if (!g_pAccountViewTip->Flush()) {
		throw runtime_error("Failed to write to cAccount database\n");
	}
	if (!g_pScriptDBTip->Flush()) {
		throw runtime_error("Failed to write to cAccount database\n");
	}
	WriteBlockLog(false, "");
	return Value::null;
}

Value getappaccinfo(const Array& params, bool bHelp) {
	if (bHelp || (params.size() != 2 && params.size() != 3)) {
		throw runtime_error("getappaccinfo  \"scriptid\" \"address\""
				"\nget appaccount info\n"
				"\nArguments:\n"
				"1.\"scriptid\":(string, required) \n"
				"2.\"address\": (string, required) \n"
				"3.\"minconf\"  (numeric, optional, default=1) Only include contract transactions confirmed \n"
				"\nExamples:\n"
				+ HelpExampleCli("getappaccinfo", "\"000000100000\" \"5zQPcC1YpFMtwxiH787pSXanUECoGsxUq3KZieJxVG\"")
				+ "\nAs json rpc call\n"
				+ HelpExampleRpc("getappaccinfo", "\"000000100000\" \"5zQPcC1YpFMtwxiH787pSXanUECoGsxUq3KZieJxVG\""));
	}


	CRegID cScript(params[0].get_str());
	vector<unsigned char> vuchkey;

	if (CRegID::IsSimpleRegIdStr(params[1].get_str())) {
		CRegID reg(params[1].get_str());
		vuchkey.insert(vuchkey.begin(), reg.GetVec6().begin(), reg.GetVec6().end());
	} else {
		string strAddr = params[1].get_str();
		vuchkey.assign(strAddr.c_str(), strAddr.c_str() + strAddr.length());
	}
	std::shared_ptr<CAppUserAccout> tem = std::make_shared<CAppUserAccout>();
	if (params.size() == 3 && 0 == params[2].get_int()) {
		CScriptDBViewCache cContractScriptTemp(*g_cTxMemPool.m_pScriptDBViewCache, true);
		if (!cContractScriptTemp.GetScriptAcc(cScript, vuchkey, *tem.get())) {
			tem = std::make_shared<CAppUserAccout>(vuchkey);
		}
	} else {
		CScriptDBViewCache cContractScriptTemp(*g_pScriptDBTip, true);
		if (!cContractScriptTemp.GetScriptAcc(cScript, vuchkey, *tem.get())) {
			tem = std::make_shared<CAppUserAccout>(vuchkey);
		}
	}

	tem.get()->AutoMergeFreezeToFree(cScript.getHight(), g_cChainActive.Tip()->m_nHeight);
	return Value(tem.get()->toJSON());
}

Value gethash(const Array& params, bool bHelp) {
	if (bHelp || params.size() != 1) {
		throw runtime_error("gethash  \"str\"\n"
					"\nget the hash of given str\n"
					"\nArguments:\n"
					"1.\"str\": (string, required) \n"
				    "\nresult an object \n"
					"\nExamples:\n"
					+ HelpExampleCli("gethash", "\"0000001000005zQPcC1YpFMtwxiH787pSXanUECoGsxUq3KZieJxVG\"")
					+ "\nAs json rpc call\n"
					+ HelpExampleRpc("gethash", "\"0000001000005zQPcC1YpFMtwxiH787pSXanUECoGsxUq3KZieJxVG\""));
	}

	string str = params[0].get_str();
	vector<unsigned char> vuchTemp;
	vuchTemp.assign(str.c_str(), str.c_str() + str.length());
	uint256 cHash = Hash(vuchTemp.begin(), vuchTemp.end());
	Object obj;
	obj.push_back(Pair("hash", cHash.ToString()));
	return obj;
}

Value getappkeyvalue(const Array& params, bool bHelp) {
	if (bHelp || params.size() != 2) {
		throw runtime_error("getappkeyvalue  \"scriptid\" \"array\""
						"\nget application key value\n"
						"\nArguments:\n"
						"1.\"scriptid\": (string, required) \n"
						"2.\"array\": (string, required) \n"
						"\nExamples:\n"
						+ HelpExampleCli("getappkeyvalue", "\"000000100000\" \"5zQPcC1YpFMtwxiH787pSXanUECoGsxUq3KZieJxVG\"")
						+ "\nAs json rpc call\n"
						+ HelpExampleRpc("getappkeyvalue", "\"000000100000\" \"5zQPcC1YpFMtwxiH787pSXanUECoGsxUq3KZieJxVG\""));
	}

	CRegID cScriptid(params[0].get_str());
	Array array = params[1].get_array();

	int nHeight = g_cChainActive.Height();

	if (cScriptid.IsEmpty() == true) {
		throw runtime_error("in getappkeyvalue :vscriptid size is error!\n");
	}
	if (!g_pScriptDBTip->HaveScript(cScriptid)) {
		throw runtime_error("in getappkeyvalue :vscriptid id is exist!\n");
	}
	Array retArry;
	CScriptDBViewCache cContractScriptTemp(*g_pScriptDBTip, true);

	for (size_t i = 0; i < array.size(); i++) {
		uint256 cTxHash(uint256S(array[i].get_str()));
		vector<unsigned char> vuchKey;	// = ParseHex(array[i].get_str());
		vuchKey.insert(vuchKey.begin(), cTxHash.begin(), cTxHash.end());
		vector<unsigned char> vuchValue;
		Object obj;
		if (!cContractScriptTemp.GetScriptData(nHeight, cScriptid, vuchKey, vuchValue)) {
			obj.push_back(Pair("key", array[i].get_str()));
			obj.push_back(Pair("value", HexStr(vuchValue)));
		} else {
			obj.push_back(Pair("key", array[i].get_str()));
			obj.push_back(Pair("value", HexStr(vuchValue)));
		}

		std::shared_ptr<CBaseTransaction> pBaseTx;
		int nTime = 0;
		int nHeight = 0;
		if (SysCfg().IsTxIndex()) {
			ST_DiskTxPos tPosTx;
			if (g_pScriptDBTip->ReadTxIndex(cTxHash, tPosTx)) {
				CAutoFile cFile(OpenBlockFile(tPosTx, true), SER_DISK, g_sClientVersion);
				CBlockHeader cBlockHeader;
				try {
					cFile >> cBlockHeader;
					fseek(cFile, tPosTx.m_unTxOffset, SEEK_CUR);
					cFile >> pBaseTx;
					nHeight = cBlockHeader.GetHeight();
					nTime = cBlockHeader.GetTime();
				} catch (std::exception &e) {
					throw runtime_error(tfm::format("%s : Deserialize or I/O error - %s", __func__, e.what()).c_str());
				}
			}
		}

		obj.push_back(Pair("confirmHeight", (int) nHeight));
		obj.push_back(Pair("confirmedtime", (int) nTime));
		retArry.push_back(obj);
	}

	return retArry;
}

Value gencheckpoint(const Array& params, bool bHelp) {
	if (bHelp || params.size() != 2) {
		throw runtime_error(
				 "gencheckpoint \"privatekey\" \"filepath\"\n"
				 "\ngenerate checkpoint by Private key signature block.\n"
				 "\nArguments:\n"
				 "1. \"privatekey\"  (string, required) the private key\n"
				 "2. \"filepath\"  (string, required) check point block path\n"
				"\nResult:\n"
				"\nExamples:\n"
				 + HelpExampleCli("gencheckpoint", "\"privatekey\" \"filepath\"")
                 + HelpExampleRpc("gencheckpoint", "\"privatekey\" \"filepath\""));
	}
	std::string strSecret = params[0].get_str();
	CDacrsSecret cSecret;
	bool bFGood = cSecret.SetString(strSecret);

	if (!bFGood) {
		throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid private key encoding");
	}

	CKey cKey = cSecret.GetKey();
	if (!cKey.IsValid()) {
		throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Private key outside allowed range");
	}

	string strFile = params[1].get_str();
	int nHeight(0);
	CBlock cBlock;
	try {
		FILE* pFile = fopen(strFile.c_str(), "rb+");
		CAutoFile cFileout = CAutoFile(pFile, SER_DISK, g_sClientVersion);
		if (!cFileout) {
			throw JSONRPCError(RPC_MISC_ERROR, "open file:" + strFile + "failed!");
		}

		cFileout >> nHeight;
		cFileout >> cBlock;
	} catch (std::exception &e) {
		throw JSONRPCError(RPC_MISC_ERROR, strprintf("read block to file error:%s", e.what()).c_str());
	}

	SyncData::CSyncData data;
	SyncData::CSyncCheckPoint point;
	CDataStream cDataStream(SER_NETWORK, g_sProtocolVersion);
	point.m_height = nHeight;
	point.m_hashCheckpoint = cBlock.GetHash();	//g_cChainActive[intTemp]->GetBlockHash();
	LogPrint("CHECKPOINT", "send hash = %s\n", cBlock.GetHash().ToString());
	cDataStream << point;
	Object obj;
	if (data.Sign(cKey, std::vector<unsigned char>(cDataStream.begin(), cDataStream.end()))
			&& data.CheckSignature(SysCfg().GetPublicKey())) {
		obj.push_back(Pair("chenkpoint", data.ToJsonObj()));
		return obj;
	}
	return obj;
}

Value setcheckpoint(const Array& params, bool bHelp) {
	if (bHelp || params.size() != 1) {
		throw runtime_error(
				 "setcheckpoint \"filepath\"\n"
				 "\nadd new checkpoint and send it out.\n"
				 "\nArguments:\n"
				 "1. \"filepath\"  (string, required) check point block path\n"
				 "\nResult:\n"
				 "\nExamples:\n"
		         + HelpExampleCli("setcheckpoint", "\"filepath\"")
		         + HelpExampleRpc("setcheckpoint", "\"filepath\""));
	}
	SyncData::CSyncData cData;
	ifstream file;
	file.open(params[0].get_str().c_str(), ios::in | ios::ate);
	if (!file.is_open()) {
		throw JSONRPCError(RPC_INVALID_PARAMETER, "Cannot open check point dump file");
	}
	file.seekg(0, file.beg);
    if (file.good()){
    	Value reply;
    	json_spirit::read(file,reply);
    	const Value & checkpoint = find_value(reply.get_obj(),"chenkpoint");
    	if(checkpoint.type() ==  json_spirit::null_type) {
    		throw JSONRPCError(RPC_INVALID_PARAMETER, "read check point failed");
    	}
    	const Value & msg = find_value(checkpoint.get_obj(), "msg");
    	const Value & sig = find_value(checkpoint.get_obj(), "sig");
    	if(msg.type() == json_spirit::null_type || sig.type() == json_spirit::null_type) {
    		throw JSONRPCError(RPC_INVALID_PARAMETER, "read msg or sig failed");
    	}
    	cData.m_vchMsg = ParseHex(msg.get_str());
    	cData.m_vchSig = ParseHex(sig.get_str());
    }
    file.close();
    if(!cData.CheckSignature(SysCfg().GetPublicKey())) {
    	throw JSONRPCError(RPC_INVALID_PARAMETER, "check signature failed");
    }
	SyncData::CSyncDataDb cSyncDataDb;
	std::vector<SyncData::CSyncData> vcData;
	SyncData::CSyncCheckPoint cSyncCheckPoint;
	CDataStream cDStream(cData.m_vchMsg, SER_NETWORK, g_sProtocolVersion);
	cDStream >> cSyncCheckPoint;
	cSyncDataDb.WriteCheckpoint(cSyncCheckPoint.m_height, cData);
	Checkpoints::AddCheckpoint(cSyncCheckPoint.m_height, cSyncCheckPoint.m_hashCheckpoint);
	CheckActiveChain(cSyncCheckPoint.m_height, cSyncCheckPoint.m_hashCheckpoint);
	vcData.push_back(cData);
	LOCK(g_cs_vNodes);
	BOOST_FOREACH(CNode* pNode, g_vNodes)
	{
		if (pNode->m_setcheckPointKnown.count(cSyncCheckPoint.m_height) == 0) {
			pNode->m_setcheckPointKnown.insert(cSyncCheckPoint.m_height);
			pNode->PushMessage("checkpoint", vcData);
		}
	}
	return tfm::format("sendcheckpoint :%d\n", cSyncCheckPoint.m_height);
}

Value validateaddress(const Array& params, bool bHelp) {
	if (bHelp || params.size() != 1) {
		throw runtime_error(
				"isvalideaddess \"dacrs address\"\n"
						"\ncheck address is valide\n"
						"\nArguments:\n"
						"1. \"dacrs address\"  (string, required) dacrs address\n"
						"\nResult:\n"
						"\nExamples:\n" + HelpExampleCli("isvalideaddress", "\"De5nZAbhMikMPGHzxvSGqHTgEuf3eNUiZ7\"")
						+ HelpExampleRpc("isvalideaddress", "\"De5nZAbhMikMPGHzxvSGqHTgEuf3eNUiZ7\""));
	}
	{
		Object obj;
		CKeyID cKeyId;
		string strAddr = params[0].get_str();
		if (!GetKeyId(strAddr, cKeyId)) {
			obj.push_back(Pair("ret", false));
		} else {
			obj.push_back(Pair("ret", true));
		}
		return obj;
	}
}

Value gettotalcoin(const Array& params, bool bHelp) {
	if (bHelp || params.size() != 0) {
		throw runtime_error("gettotalcoin \n"
				"\nget all coin llAmount\n"
				"\nArguments:\n"
				"\nResult:\n"
				"\nExamples:\n" + HelpExampleCli("gettotalcoin", "") + HelpExampleRpc("gettotalcoin", ""));
	}
	Object obj;
	{
		CAccountViewCache cView(*g_pAccountViewTip, true);
		uint64_t ullTotalcoin = cView.TraverseAccount();
		obj.push_back(Pair("TotalCoin", ValueFromAmount(ullTotalcoin)));
	}
	return obj;
}

Value gettotalassets(const Array& params, bool bHelp) {
	if (bHelp || params.size() != 1) {
		throw runtime_error("gettotalassets \n"
				"\nget all assets\n"
				"\nArguments:\n"
				"1.\"scriptid\": (string, required)\n"
				"\nResult:\n"
				"\nExamples:\n" + HelpExampleCli("gettotalassets", "11-1") + HelpExampleRpc("gettotalassets", "11-1"));
	}
	CRegID cRegId(params[0].get_str());
	if (cRegId.IsEmpty() == true) {
		throw runtime_error("in gettotalassets :scriptid size is error!\n");
	}

	if (!g_pScriptDBTip->HaveScript(cRegId)) {
		throw runtime_error("in gettotalassets :scriptid  is not exist!\n");
	}

	CScriptDBViewCache cContractScriptTemp(*g_pScriptDBTip, true);
	Object obj;
	{
		map<vector<unsigned char>, vector<unsigned char> > mapAcc;
		bool bRet = cContractScriptTemp.GetAllScriptAcc(cRegId, mapAcc);
		if (bRet) {
			uint64_t ullTotalassets = 0;
			map<vector<unsigned char>, vector<unsigned char>>::iterator it;
			for (it = mapAcc.begin(); it != mapAcc.end(); ++it) {
				CAppUserAccout cAppAccOut;
				vector<unsigned char> vuchKey = it->first;
				vector<unsigned char> vuchValue = it->second;

				CDataStream cDs(vuchValue, SER_DISK, g_sClientVersion);
				cDs >> cAppAccOut;

				ullTotalassets += cAppAccOut.getllValues();
				ullTotalassets += cAppAccOut.GetAllFreezedValues();
			}

			obj.push_back(Pair("TotalAssets", ValueFromAmount(ullTotalassets)));
		} else {
			throw runtime_error("in gettotalassets :find script cAccount failed!\n");
		}

	}
	return obj;
}

Value gettxhashbyaddress(const Array& params, bool bHelp) {
	if (bHelp || params.size() != 2) {
		throw runtime_error(
				"gettxbyaddress \n"
						"\nget all tx hash by addresss\n"
						"\nArguments:\n"
						"\nArguments:\n"
						"1.\"address\": (string, required) \n"
						"2.\"height\": (numeric, required) \n"
						"\nResult: tx relate tx hash as array\n"
						"\nExamples:\n"
						+ HelpExampleCli("gettxhashbyaddress",
								"\"5zQPcC1YpFMtwxiH787pSXanUECoGsxUq3KZieJxVG\" \"10023\"") + "\nAs json rpc call\n"
						+ HelpExampleRpc("gettxhashbyaddress",
								"\"5zQPcC1YpFMtwxiH787pSXanUECoGsxUq3KZieJxVG\" \"10023\""));
	}
	string strAddress = params[0].get_str();
	int nHeight = params[1].get_int();

	Object obj;
	{
		CScriptDBViewCache cScriptDbView(*g_pScriptDBTip, true);
		map<vector<unsigned char>, vector<unsigned char> > mapTxHash;
		vector<string> vstrTxArray;
		CKeyID cKeyID;
		if (!GetKeyId(strAddress, cKeyID)) {
			throw runtime_error("gettxhashbyaddress : input params address is invalide!\n");
		}
		if (!cScriptDbView.GetTxHashByAddress(cKeyID, nHeight, mapTxHash)) {
			throw runtime_error("call GetTxHashByAddress failed!\n");
			;
		}
		obj.push_back(Pair("address", strAddress));
		obj.push_back(Pair("nHeight", nHeight));
		Array arrayObj;
		for (auto item : mapTxHash) {
			arrayObj.push_back(string(item.second.begin(), item.second.end()));
		}
		obj.push_back(Pair("txarray", arrayObj));
	}
	return obj;
}

Value getrawtx(const Array& params, bool bHelp) {
	if (bHelp || params.size() != 1) {
		throw runtime_error("submittx \"transaction\" \n"
				"\nsubmit transaction\n"
				"\nArguments:\n"
				"1.\"transaction\": (string, required)\n"
				"\nExamples:\n"
				+ HelpExampleCli("submittx", "\"n2dha9w3bz2HPVQzoGKda3Cgt5p5Tgv6oj\"")
				+ "\nAs json rpc call\n"
				+ HelpExampleRpc("submittx", "\"n2dha9w3bz2HPVQzoGKda3Cgt5p5Tgv6oj\""));
	}
	EnsureWalletIsUnlocked();
	vector<unsigned char> vuch(ParseHex(params[0].get_str()));
	CDataStream cDataStream(vuch, SER_DISK, g_sClientVersion);
	CDataStream cDataStreamRawTx(SER_DISK, g_sClientVersion);

	std::shared_ptr<CBaseTransaction> pa;
	cDataStream >> pa;
	cDataStreamRawTx << pa->m_chTxType;
	if (pa->m_chTxType == EM_REG_ACCT_TX) {
		CRegisterAccountTx *pRegAcctTx = (CRegisterAccountTx *) pa.get();
		cDataStreamRawTx << pRegAcctTx->m_nVersion;
		cDataStreamRawTx << pRegAcctTx->m_nValidHeight;
		CID id(pRegAcctTx->m_cUserId);
		cDataStreamRawTx << id;
		CID mMinerid(pRegAcctTx->m_cMinerId);
		cDataStreamRawTx << mMinerid;
		cDataStreamRawTx << pRegAcctTx->m_llFees;
		cDataStreamRawTx << pRegAcctTx->m_vchSignature;
	} else if (pa->m_chTxType == EM_COMMON_TX || pa->m_chTxType == EM_CONTRACT_TX) {
		CTransaction * pTx = (CTransaction *) pa.get();
		cDataStreamRawTx << pTx->m_nVersion;
		cDataStreamRawTx << pTx->m_nValidHeight;
		CID srcId(pTx->m_cSrcRegId);
		cDataStreamRawTx << srcId;
		CID desId(pTx->m_cDesUserId);
		cDataStreamRawTx << desId;
		cDataStreamRawTx << pTx->m_ullFees;
		cDataStreamRawTx << pTx->m_ullValues;
		cDataStreamRawTx << pTx->m_vchContract;
		cDataStreamRawTx << pTx->m_vchSignature;
	}  else if (pa->m_chTxType == EM_REWARD_TX) {
		CRewardTransaction * pRewardTx = (CRewardTransaction *) pa.get();
		cDataStreamRawTx << pRewardTx->m_nVersion;
		CID acctId(pRewardTx->m_cAccount);
		cDataStreamRawTx << acctId;
		cDataStreamRawTx << pRewardTx->m_ullRewardValue;
		cDataStreamRawTx << pRewardTx->m_nHeight;
	} else if (pa->m_chTxType == EM_REG_APP_TX) {
		CRegisterAppTx * pRegAppTx = (CRegisterAppTx *) pa.get();
		cDataStreamRawTx << pRegAppTx->m_nVersion;
		cDataStreamRawTx << pRegAppTx->m_nValidHeight;
		CID regId(pRegAppTx->m_cRegAcctId);
		cDataStreamRawTx	<< regId;
		cDataStreamRawTx << pRegAppTx->m_vchScript;
		cDataStreamRawTx << pRegAppTx->m_ullFees;
		cDataStreamRawTx << pRegAppTx->m_vchSignature;

	} else {
		 throw runtime_error("seiralize tx type value error, must be ranger(1...5)\n");
	}
	vector<unsigned char> vuchRetCh(cDataStreamRawTx.begin(), cDataStreamRawTx.end());
	Object obj;
	obj.push_back(Pair("txraw", HexStr(vuchRetCh)));
	return obj;
}

