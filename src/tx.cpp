#include "serialize.h"
#include <boost/foreach.hpp>
#include "hash.h"
#include "util.h"
#include "account.h"
#include "main.h"
#include <algorithm>
#include "txdb.h"
#include "vm/vmrunevn.h"
#include "core.h"
#include "miner.h"
#include "json/json_spirit_utils.h"
#include "json/json_spirit_value.h"
#include "json/json_spirit_writer_template.h"
using namespace json_spirit;

string txTypeArray[] = { "NULL_TXTYPE", "REWARD_TX", "REG_ACCT_TX", "COMMON_TX", "CONTRACT_TX", "REG_APP_TX"};


bool CID::Set(const CRegID &id) {
	CDataStream ds(SER_DISK, CLIENT_VERSION);
	ds << id;
	vchData.clear();
	vchData.insert(vchData.end(), ds.begin(), ds.end());
	return true;
}
bool CID::Set(const CKeyID &id) {
	vchData.resize(20);
	memcpy(&vchData[0], &id, 20);
	return true;
}
bool CID::Set(const CPubKey &id) {
	vchData.resize(id.size());
	memcpy(&vchData[0], &id, id.size());
	return true;
}
bool CID::Set(const CNullID &id) {
	return true;
}
bool CID::Set(const CUserID &userid) {
	return boost::apply_visitor(CIDVisitor(this), userid);
}
CUserID CID::GetUserId() {
	if (1< vchData.size() && vchData.size() <= 10) {
		CRegID regId;
		regId.SetRegIDByCompact(vchData);
		return CUserID(regId);
	} else if (vchData.size() == 33) {
		CPubKey pubKey(vchData);
		return CUserID(pubKey);
	} else if (vchData.size() == 20) {
		uint160 data = uint160(vchData);
		CKeyID keyId(data);
		return CUserID(keyId);
	} else if(vchData.empty()) {
		return CNullID();
	}
	else {
		LogPrint("ERROR", "vchData:%s, len:%d\n", HexStr(vchData).c_str(), vchData.size());
		assert(0);
	}
	return CNullID();
}


bool CRegID::clean()  {
	nHeight = 0 ;
	nIndex = 0 ;
	vRegID.clear();
	return true;
}
CRegID::CRegID(const vector<unsigned char>& vIn) {
	assert(vIn.size() == 6);
	vRegID = vIn;
	nHeight = 0;
	nIndex = 0;
	CDataStream ds(vIn, SER_DISK, CLIENT_VERSION);
	ds >> nHeight;
	ds >> nIndex;
}
bool CRegID::IsSimpleRegIdStr(const string & str)
{
	int len = str.length();
	if (len >= 3) {
		int pos = str.find('-');

		if (pos > len - 1) {
			return false;
		}
		string firtstr = str.substr(0, pos);

		if (firtstr.length() > 10 || firtstr.length() == 0) //int max is 4294967295 can not over 10
			return false;

		for (auto te : firtstr) {
			if (!isdigit(te))
				return false;
		}
		string endstr = str.substr(pos + 1);
		if (endstr.length() > 10 || endstr.length() == 0) //int max is 4294967295 can not over 10
			return false;
		for (auto te : endstr) {
			if (!isdigit(te))
				return false;
		}
	}
	return true;
}
bool CRegID::GetKeyID(const string & str,CKeyID &keyId)
{
	CRegID te(str);
	if(te.IsEmpty())
		return false;
	keyId = te.getKeyID(*pAccountViewTip);
	return !keyId.IsEmpty();
}
bool CRegID::IsRegIdStr(const string & str)
 {
	if(IsSimpleRegIdStr(str)){
		return true;
	}
	else if(str.length()==12){
		return true;
	}
	return false;
}
void CRegID::SetRegID(string strRegID){
	nHeight = 0;
	nIndex = 0;
	vRegID.clear();

	if(IsSimpleRegIdStr(strRegID))
	{
		int pos = strRegID.find('-');
		nHeight = atoi(strRegID.substr(0, pos).c_str());
		nIndex = atoi(strRegID.substr(pos+1).c_str());
		vRegID.insert(vRegID.end(), BEGIN(nHeight), END(nHeight));
		vRegID.insert(vRegID.end(), BEGIN(nIndex), END(nIndex));
//		memcpy(&vRegID.at(0),&nHeight,sizeof(nHeight));
//		memcpy(&vRegID[sizeof(nHeight)],&nIndex,sizeof(nIndex));
	}
	else if(strRegID.length()==12)
	{
	vRegID = ::ParseHex(strRegID);
	memcpy(&nHeight,&vRegID[0],sizeof(nHeight));
	memcpy(&nIndex,&vRegID[sizeof(nHeight)],sizeof(nIndex));
	}

}
void CRegID::SetRegID(const vector<unsigned char>& vIn) {
	assert(vIn.size() == 6);
	vRegID = vIn;
	CDataStream ds(vIn, SER_DISK, CLIENT_VERSION);
	ds >> nHeight;
	ds >> nIndex;
}
CRegID::CRegID(string strRegID) {
	SetRegID(strRegID);
}
CRegID::CRegID(uint32_t nHeightIn, uint16_t nIndexIn) {
	nHeight = nHeightIn;
	nIndex = nIndexIn;
	vRegID.clear();
	vRegID.insert(vRegID.end(), BEGIN(nHeightIn), END(nHeightIn));
	vRegID.insert(vRegID.end(), BEGIN(nIndexIn), END(nIndexIn));
}
string CRegID::ToString() const {
//	if(!IsEmpty())
//	return ::HexStr(vRegID);
	if(!IsEmpty())
	  return  strprintf("%d-%d",nHeight,nIndex);
	return string(" ");
}
CKeyID CRegID::getKeyID(const CAccountViewCache &view)const
{
	CKeyID ret;
	CAccountViewCache(view).GetKeyId(*this,ret);
	return ret;
}
void CRegID::SetRegIDByCompact(const vector<unsigned char> &vIn) {
	if(vIn.size()>0)
	{
		CDataStream ds(vIn, SER_DISK, CLIENT_VERSION);
		ds >> *this;
	}
	else
	{
		clean();
	}
}


bool CBaseTransaction::IsValidHeight(int nCurHeight, int nTxCacheHeight) const
{
	if(REWARD_TX == nTxType)
		return true;
	if (nValidHeight > nCurHeight + nTxCacheHeight / 2)
			return false;
	if (nValidHeight < nCurHeight - nTxCacheHeight / 2)
			return false;
	return true;
}
bool CBaseTransaction::UndoExecuteTx(int nIndex, CAccountViewCache &view, CValidationState &state, CTxUndo &txundo,
		int nHeight, CTransactionDBCache &txCache, CScriptDBViewCache &scriptCache) {
	vector<CAccountOperLog>::reverse_iterator rIterAccountLog = txundo.vAccountOperLog.rbegin();
	for (; rIterAccountLog != txundo.vAccountOperLog.rend(); ++rIterAccountLog) {
		CAccount account;
		CUserID userId = rIterAccountLog->keyID;
		if (!view.GetAccount(userId, account)) {
			return state.DoS(100, ERRORMSG("UndoExecuteTx() : undo ExecuteTx read accountId= %s account info error"),
					UPDATE_ACCOUNT_FAIL, "bad-read-accountdb");
		}
		if (!account.UndoOperateAccount(*rIterAccountLog)) {
			return state.DoS(100, ERRORMSG("UndoExecuteTx() : undo UndoOperateAccount failed"), UPDATE_ACCOUNT_FAIL,
					"undo-operate-account-failed");
		}
		if (COMMON_TX == nTxType
				&& (account.IsEmptyValue()
						&& (!account.PublicKey.IsFullyValid() || account.PublicKey.GetKeyID() != account.keyID))) {
			view.EraseAccount(userId);
		} else {
			if (!view.SetAccount(userId, account)) {
				return state.DoS(100,
						ERRORMSG("UndoExecuteTx() : undo ExecuteTx write accountId= %s account info error"),
						UPDATE_ACCOUNT_FAIL, "bad-write-accountdb");
			}
		}
	}
	vector<CScriptDBOperLog>::reverse_iterator rIterScriptDBLog = txundo.vScriptOperLog.rbegin();
	for (; rIterScriptDBLog != txundo.vScriptOperLog.rend(); ++rIterScriptDBLog) {
		if (!scriptCache.UndoScriptData(rIterScriptDBLog->vKey, rIterScriptDBLog->vValue))
			return state.DoS(100, ERRORMSG("UndoExecuteTx() : undo scriptdb data error"), UPDATE_ACCOUNT_FAIL,
					"bad-save-scriptdb");
	}
	if(CONTRACT_TX == nTxType) {
		if (!scriptCache.EraseTxRelAccout(GetHash()))
			return state.DoS(100, ERRORMSG("UndoExecuteTx() : erase tx rel account error"), UPDATE_ACCOUNT_FAIL,
							"bad-save-scriptdb");
	}
	return true;
}


bool CRegisterAccountTx::ExecuteTx(int nIndex, CAccountViewCache &view, CValidationState &state, CTxUndo &txundo,
		int nHeight, CTransactionDBCache &txCache, CScriptDBViewCache &scriptCache) {
	CAccount account;
	CRegID regId(nHeight, nIndex);
	CKeyID keyId = boost::get<CPubKey>(userId).GetKeyID();
	if (!view.GetAccount(userId, account))
		return state.DoS(100, ERRORMSG("ExecuteTx() : read source keyId %s account info error", keyId.ToString()),
				UPDATE_ACCOUNT_FAIL, "bad-read-accountdb");

	if(account.PublicKey.IsFullyValid() && account.PublicKey.GetKeyID() == keyId) {
		return state.DoS(100, ERRORMSG("ExecuteTx() : read source keyId %s duplicate register", keyId.ToString()),
					UPDATE_ACCOUNT_FAIL, "duplicate-register-account");
	}
	account.PublicKey = boost::get<CPubKey>(userId);
	if (llFees > 0) {
		account.CompactAccount(nHeight);
		CFund fund(llFees);
		if(!account.OperateAccount(MINUS_FREE, fund))
			return state.DoS(100, ERRORMSG("ExecuteTx() : not sufficient funds in account, keyid=%s", keyId.ToString()),
					UPDATE_ACCOUNT_FAIL, "not-sufficiect-funds");
	}

	account.regID = regId;
	if (typeid(CPubKey) == minerId.type()) {
		account.MinerPKey = boost::get<CPubKey>(minerId);

		if (account.MinerPKey.IsValid() && !account.MinerPKey.IsFullyValid()) {
			return state.DoS(100, ERRORMSG("ExecuteTx() : MinerPKey:%s Is Invalid", account.MinerPKey.ToString()),
					UPDATE_ACCOUNT_FAIL, "MinerPKey Is Invalid");
		}
	}

	if (!view.SaveAccountInfo(regId, keyId, account)) {
		return state.DoS(100, ERRORMSG("ExecuteTx() : write source addr %s account info error", regId.ToString()),
				UPDATE_ACCOUNT_FAIL, "bad-read-accountdb");
	}
	txundo.vAccountOperLog.push_back(account.accountOperLog);
	txundo.txHash = GetHash();
	return true;
}
bool CRegisterAccountTx::UndoExecuteTx(int nIndex, CAccountViewCache &view, CValidationState &state,
		CTxUndo &txundo, int nHeight, CTransactionDBCache &txCache, CScriptDBViewCache &scriptCache) {
	//drop account
	CRegID accountId(nHeight, nIndex);
	CAccount oldAccount;
	if (!view.GetAccount(accountId, oldAccount))
		return state.DoS(100,
				ERRORMSG("ExecuteTx() : read secure account=%s info error", accountId.ToString()),
				UPDATE_ACCOUNT_FAIL, "bad-read-accountdb");
	CKeyID keyId;
	view.GetKeyId(accountId, keyId);

	if (llFees > 0) {
		CAccountOperLog accountOperLog;
		if (!txundo.GetAccountOperLog(keyId, accountOperLog))
			return state.DoS(100, ERRORMSG("ExecuteTx() : read keyId=%s tx undo info error", keyId.GetHex()),
					UPDATE_ACCOUNT_FAIL, "bad-read-txundoinfo");
		oldAccount.UndoOperateAccount(accountOperLog);
	}

	if (!oldAccount.IsEmptyValue()) {
		CPubKey empPubKey;
		oldAccount.PublicKey = empPubKey;
		oldAccount.MinerPKey = empPubKey;
		CUserID userId(keyId);
		view.SetAccount(userId, oldAccount);
	} else {
		view.EraseAccount(userId);
	}
	view.EraseId(accountId);
	return true;
}
bool CRegisterAccountTx::GetAddress(set<CKeyID> &vAddr, CAccountViewCache &view) {
	if (!boost::get<CPubKey>(userId).IsFullyValid()) {
		return false;
	}
	vAddr.insert(boost::get<CPubKey>(userId).GetKeyID());
	return true;
}
string CRegisterAccountTx::ToString(CAccountViewCache &view) const {
	string str;
	str += strprintf("txType=%s, hash=%s, ver=%d, pubkey=%s, llFees=%ld, keyid=%s, nValidHeight=%d\n",
	txTypeArray[nTxType],GetHash().ToString().c_str(), nVersion, boost::get<CPubKey>(userId).ToString(), llFees, boost::get<CPubKey>(userId).GetKeyID().ToAddress(), nValidHeight);
	return str;
}
bool CRegisterAccountTx::CheckTransction(CValidationState &state, CAccountViewCache &view) {
	//check pubKey valid
	if (!boost::get<CPubKey>(userId).IsFullyValid()) {
		return state.DoS(100, ERRORMSG("CheckTransaction() : register tx public key is invalid"), REJECT_INVALID,
				"bad-regtx-publickey");
	}

	//check signature script
	uint256 sighash = SignatureHash();
	if(!CheckSignScript(sighash, signature, boost::get<CPubKey>(userId))) {
		return state.DoS(100, ERRORMSG("CheckTransaction() : register tx signature error "), REJECT_INVALID,
				"bad-regtx-signature");
	}

	if (!MoneyRange(llFees))
		return state.DoS(100, ERRORMSG("CheckTransaction() : register tx fee out of range"), REJECT_INVALID,
				"bad-regtx-fee-toolarge");
	return true;
}


//bool CTransaction::ExecuteTx(int nIndex, CAccountViewCache &view, CValidationState &state, CTxUndo &txundo,
//		int nHeight, CTransactionDBCache &txCache, CScriptDBViewCache &scriptCache) {
//	CAccount srcAcct;
//	CAccount desAcct;
//	if (!view.GetAccount(srcUserId, srcAcct))
//		return state.DoS(100,
//				ERRORMSG("ExecuteTx() : read source addr %s account info error", (boost::get<CRegID>(srcUserId).ToString())),
//				UPDATE_ACCOUNT_FAIL, "bad-read-accountdb");
//
//	if(!view.GetAccount(desUserId, desAcct))
//		desAcct.keyID = boost::get<CKeyID>(desUserId);
//
//	uint64_t minusValue = llFees + llValues;
//	CFund minusFund(minusValue);
//	srcAcct.CompactAccount(nHeight);
//	if (!srcAcct.OperateAccount(MINUS_FREE, minusFund))
//		return state.DoS(100, ERRORMSG("ExecuteTx() : accounts insufficient funds"), UPDATE_ACCOUNT_FAIL,
//				"not-sufficient-funds");
//	if(!view.SetAccount(srcUserId, srcAcct)) {
//		return state.DoS(100, ERRORMSG("ExecuteTx() : save account error, kyeId=%s", srcAcct.keyID.ToString()), UPDATE_ACCOUNT_FAIL,
//						"bad-save-account");
//	}
//	uint64_t addValue = llValues;
//	CFund addFund(FREEDOM_FUND,addValue, nHeight);
//	desAcct.CompactAccount(nHeight);
//	if (!desAcct.OperateAccount(ADD_FREE, addFund)) {
//		return state.DoS(100, ERRORMSG("ExecuteTx() : operate accounts error"), UPDATE_ACCOUNT_FAIL,
//				"bad-operate-account");
//	}
//	if (!view.SetAccount(desUserId, desAcct)) {
//		return state.DoS(100, ERRORMSG("ExecuteTx() : save account error, kyeId=%s", desAcct.keyID.ToString()),
//				UPDATE_ACCOUNT_FAIL, "bad-save-account");
//	}
//	txundo.vAccountOperLog.push_back(srcAcct.accountOperLog);
//	txundo.vAccountOperLog.push_back(desAcct.accountOperLog);
//	txundo.txHash = GetHash();
//	return true;
//}
//bool CTransaction::GetAddress(set<CKeyID> &vAddr, CAccountViewCache &view) {
//	CKeyID srcKeyId;
//	if (!view.GetKeyId(srcUserId, srcKeyId))
//		return false;
//
//	CKeyID desKeyId;
//	if(desUserId.type() == typeid(CKeyID)) {
//		desKeyId = boost::get<CKeyID>(desUserId);
//	} else if(desUserId.type() == typeid(CRegID)){
//		if (!view.GetKeyId(desUserId, desKeyId))
//			return false;
//	} else
//		return false;
//
//	vAddr.insert(srcKeyId);
//	vAddr.insert(desKeyId);
//	return true;
//}
//
//string CTransaction::ToString(CAccountViewCache &view) const {
//	string str;
//	CKeyID srcKeyId, desKeyId;
//	view.GetKeyId(srcUserId, srcKeyId);
//	if (desUserId.type() == typeid(CKeyID)) {
//		str += strprintf("txType=%s, hash=	%s, nVersion=%d, srcAccountId=%s, llFees=%ld, llValues=%ld, desKeyId=%s, nValidHeight=%d\n",
//		txTypeArray[nTxType], GetHash().ToString().c_str(), nVersion, (boost::get<CRegID>(srcUserId).ToString()), llFees, llValues, boost::get<CKeyID>(desUserId).GetHex(), nValidHeight);
//	} else if(desUserId.type() == typeid(CRegID)) {
//		view.GetKeyId(desUserId, desKeyId);
//		str += strprintf("txType=%s, hash=%s, nVersion=%d, srcAccountId=%s, srcKeyId=%s, llFees=%ld, llValues=%ld, desAccountId=%s, desKeyId=%s, nValidHeight=%d\n",
//		txTypeArray[nTxType], GetHash().ToString().c_str(), nVersion, boost::get<CRegID>(srcUserId).ToString(), srcKeyId.GetHex(), llFees, llValues, boost::get<CRegID>(desUserId).ToString(), desKeyId.GetHex(), nValidHeight);
//	}
//
//	return str;
//}
//bool CTransaction::CheckTransction(CValidationState &state, CAccountViewCache &view) {
//	//check source addr, destination addr
//	if (srcUserId.type() != typeid(CRegID)) {
//		return state.DoS(100, ERRORMSG("CheckTransaction() : normal tx source address or des address is invalid"),
//				REJECT_INVALID, "bad-normaltx-sourceaddr");
//	}
//	if (!MoneyRange(llFees)) {
//		return state.DoS(100, ERRORMSG("CheckTransaction() : normal tx fee out of range"), REJECT_INVALID,
//				"bad-normaltx-fee-toolarge");
//	}
//	if (!MoneyRange(llValues)) {
//		return state.DoS(100, ERRORMSG("CheckTransaction(): normal tx value our of range"), REJECT_INVALID,
//				"bad-normaltx-value-toolarge");
//	}
//	CAccount acctInfo;
//	if (!view.GetAccount(boost::get<CRegID>(srcUserId), acctInfo)) {
//		return state.DoS(100, ERRORMSG("CheckTransaction() :tx GetAccount falied"), REJECT_INVALID, "bad-getaccount");
//	}
//	if (!acctInfo.IsRegister()) {
//		return state.DoS(100, ERRORMSG("CheckTransaction(): account have not registed public key"), REJECT_INVALID,
//				"bad-no-pubkey");
//	}
//
//	//check signature script
//	uint256 sighash = SignatureHash();
//	if (!CheckSignScript(sighash, signature, acctInfo.PublicKey)) {
//		return state.DoS(100, ERRORMSG("CheckTransaction() :CheckSignScript failed"), REJECT_INVALID,
//				"bad-signscript-check");
//	}
//
//	CDiskTxPos postx;
//	if (!pblocktree->ReadTxIndex(GetHash(), postx)) {
//			CAccount acctDesInfo;
//			if (desUserId.type() == typeid(CKeyID)) {
//				if (view.GetAccount(desUserId, acctDesInfo) && acctDesInfo.IsRegister()) {
//					return state.DoS(100,
//							ERRORMSG(
//									"CheckTransaction() : normal tx des account have regested, destination addr must be account id"),
//							REJECT_INVALID, "bad-normal-desaddr error");
//				}
//			}
//
//	}
//
//	return true;
//}
bool CTransaction::ExecuteTx(int nIndex, CAccountViewCache &view, CValidationState &state, CTxUndo &txundo,
		int nHeight, CTransactionDBCache &txCache, CScriptDBViewCache &scriptCache) {

	CAccount srcAcct;
	CAccount desAcct;
	uint64_t minusValue = llFees+llValues;
	CFund minusFund(minusValue);
	if (!view.GetAccount(srcRegId, srcAcct))
		return state.DoS(100,
				ERRORMSG("ExecuteTx() : read source addr %s account info error", boost::get<CRegID>(srcRegId).ToString()),
				UPDATE_ACCOUNT_FAIL, "bad-read-accountdb");
	srcAcct.CompactAccount(nHeight);
	if (!srcAcct.OperateAccount(MINUS_FREE, minusFund))
		return state.DoS(100, ERRORMSG("ExecuteTx() : secure accounts insufficient funds"), UPDATE_ACCOUNT_FAIL,
				"bad-read-accountdb");
	CUserID userId = srcAcct.keyID;
	if(!view.SetAccount(userId, srcAcct)){
		return state.DoS(100, ERRORMSG("UpdataAccounts() :save account%s info error",  boost::get<CRegID>(srcRegId).ToString()),
				UPDATE_ACCOUNT_FAIL, "bad-write-accountdb");
	}

	uint64_t addValue = llValues;
	CFund addFund(FREEDOM_FUND, addValue, nHeight);
	if(!view.GetAccount(desUserId, desAcct)) {
		if(COMMON_TX == nTxType) {
			desAcct.keyID = boost::get<CKeyID>(desUserId);
		}
		else {
			return state.DoS(100, ERRORMSG("ExecuteTx() : get account info failed by regid"), UPDATE_ACCOUNT_FAIL, "bad-read-accountdb");
		}
	}
	else
		desAcct.CompactAccount(nHeight);
	if (!desAcct.OperateAccount(ADD_FREE, addFund)) {
		return state.DoS(100, ERRORMSG("ExecuteTx() : operate accounts error"), UPDATE_ACCOUNT_FAIL,
				"bad-operate-account");
	}
	if (!view.SetAccount(desUserId, desAcct)) {
		return state.DoS(100, ERRORMSG("ExecuteTx() : save account error, kyeId=%s", desAcct.keyID.ToString()),
				UPDATE_ACCOUNT_FAIL, "bad-save-account");
	}
	txundo.vAccountOperLog.push_back(srcAcct.accountOperLog);
	txundo.vAccountOperLog.push_back(desAcct.accountOperLog);

	if (CONTRACT_TX == nTxType) {
		vector<unsigned char> vScript;
		if(!scriptCache.GetScript(boost::get<CRegID>(desUserId), vScript)) {
			return state.DoS(100, ERRORMSG("ExecuteTx() : save account error, kyeId=%s", desAcct.keyID.ToString()),
						UPDATE_ACCOUNT_FAIL, "bad-save-account");
		}
		CVmRunEvn vmRunEvn;
		std::shared_ptr<CBaseTransaction> pTx = GetNewInstance();
		uint64_t el = GetElementForBurn(chainActive.Tip());
		int64_t llTime = GetTimeMillis();
		tuple<bool, uint64_t, string> ret = vmRunEvn.run(pTx, view, scriptCache, nHeight, el, nRunStep);
		if (!std::get<0>(ret))
			return state.DoS(100,
					ERRORMSG("ExecuteTx() : ContractTransaction ExecuteTx txhash=%s run script error:%s",
							GetHash().GetHex(), std::get<2>(ret)), UPDATE_ACCOUNT_FAIL, "run-script-error");
		LogPrint("CONTRACT_TX", "execute contract elapse:%lld, txhash=%s\n", GetTimeMillis() - llTime,
				GetHash().GetHex());
		set<CKeyID> vAddress;
		vector<std::shared_ptr<CAccount> > &vAccount = vmRunEvn.GetNewAccont();
		for (auto & itemAccount : vAccount) {
			vAddress.insert(itemAccount->keyID);
			userId = itemAccount->keyID;
			if (!view.SetAccount(userId, *itemAccount))
				return state.DoS(100,
						ERRORMSG("ExecuteTx() : ContractTransaction ExecuteTx write account info error"),
						UPDATE_ACCOUNT_FAIL, "bad-write-accountdb");
			txundo.vAccountOperLog.push_back((itemAccount->accountOperLog));
		}
		txundo.vScriptOperLog.insert(txundo.vScriptOperLog.end(), vmRunEvn.GetDbLog()->begin(), vmRunEvn.GetDbLog()->end());
		if(!scriptCache.SetTxRelAccout(GetHash(), vAddress))
				return ERRORMSG("ExecuteTx() : ContractTransaction ExecuteTx save tx relate account info to script db error");

	}
	txundo.txHash = GetHash();
	return true;
}
bool CTransaction::GetAddress(set<CKeyID> &vAddr, CAccountViewCache &view) {
	CKeyID keyId;
	if(!view.GetKeyId(srcRegId, keyId))
		return false;
	vAddr.insert(keyId);
	CKeyID desKeyId;
	if(desUserId.type() == typeid(CKeyID)) {
		desKeyId = boost::get<CKeyID>(desUserId);
	} else if(desUserId.type() == typeid(CRegID)){
		if (!view.GetKeyId(desUserId, desKeyId))
			return false;
	} else
		return false;

	if (CONTRACT_TX == nTxType) {
		CVmRunEvn vmRunEvn;
		std::shared_ptr<CBaseTransaction> pTx = GetNewInstance();
		uint64_t el = GetElementForBurn(chainActive.Tip());
		CScriptDBViewCache scriptDBView(*pScriptDBTip, true);
		if (uint256(0) == pTxCacheTip->IsContainTx(GetHash())) {
			CAccountViewCache accountView(view, true);
			tuple<bool, uint64_t, string> ret = vmRunEvn.run(pTx, accountView, scriptDBView, chainActive.Height() + 1, el,
					nRunStep);
			if (!std::get<0>(ret))
				return ERRORMSG("GetAddress()  : %s", std::get<2>(ret));

			vector<shared_ptr<CAccount> > vpAccount = vmRunEvn.GetNewAccont();

			for (auto & item : vpAccount) {
				vAddr.insert(item->keyID);
			}
		} else {
			set<CKeyID> vTxRelAccount;
			if (!scriptDBView.GetTxRelAccount(GetHash(), vTxRelAccount))
				return false;
			vAddr.insert(vTxRelAccount.begin(), vTxRelAccount.end());
		}
	}
	return true;
}
string CTransaction::ToString(CAccountViewCache &view) const {
	string str;
	string desId;
	if (desUserId.type() == typeid(CKeyID)) {
		desId = boost::get<CKeyID>(desUserId).ToString();
	} else if (desUserId.type() == typeid(CRegID)) {
		desId = boost::get<CRegID>(desUserId).ToString();
	}
	str += strprintf("txType=%s, hash=%s, ver=%d, srcId=%s desId=%s, llFees=%ld, vContract=%s\n",
	txTypeArray[nTxType], GetHash().ToString().c_str(), nVersion, boost::get<CRegID>(srcRegId).ToString(), desId.c_str(), llFees, HexStr(vContract).c_str());
	return str;
}
bool CTransaction::CheckTransction(CValidationState &state, CAccountViewCache &view) {
	if (!MoneyRange(llFees)) {
		return state.DoS(100, ERRORMSG("CheckTransaction() : appeal tx fee out of range"), REJECT_INVALID,
				"bad-appeal-fee-toolarge");
	}

	CAccount acctInfo;
	if (!view.GetAccount(boost::get<CRegID>(srcRegId), acctInfo)) {
		return state.DoS(100, ERRORMSG("CheckTransaction() :tx GetAccount falied"), REJECT_INVALID, "bad-getaccount");
	}
	if (!acctInfo.IsRegister()) {
		return state.DoS(100, ERRORMSG("CheckTransaction(): account have not registed public key"), REJECT_INVALID,
				"bad-no-pubkey");
	}

	//check signature script
	uint256 sighash = SignatureHash();
	if (!CheckSignScript(sighash, signature, acctInfo.PublicKey)) {
		return state.DoS(100, ERRORMSG("CheckTransaction() :CheckSignScript failed"), REJECT_INVALID,
				"bad-signscript-check");
	}

	return true;
}
uint256 CTransaction::SignatureHash() const  {
	CHashWriter ss(SER_GETHASH, 0);
	CID srcId(srcRegId);
	CID desId(desUserId);
	ss <<VARINT(nVersion) << nTxType << VARINT(nValidHeight);
	ss << srcId << desId << VARINT(llFees) << vContract;
	return ss.GetHash();
}


bool CRewardTransaction::ExecuteTx(int nIndex, CAccountViewCache &view, CValidationState &state, CTxUndo &txundo,
		int nHeight, CTransactionDBCache &txCache, CScriptDBViewCache &scriptCache) {
	CID id(account);
	if (account.type() != typeid(CRegID) && account.type() != typeid(CPubKey)) {
		return state.DoS(100,
				ERRORMSG("ExecuteTx() : account  %s error, either accountId, or pubkey", HexStr(id.GetID())),
				UPDATE_ACCOUNT_FAIL, "bad-account");
	}
	CAccount acctInfo;
	if (!view.GetAccount(account, acctInfo)) {
		return state.DoS(100, ERRORMSG("ExecuteTx() : read source addr %s account info error", HexStr(id.GetID())),
				UPDATE_ACCOUNT_FAIL, "bad-read-accountdb");
	}
	acctInfo.CompactAccount(nHeight);
	acctInfo.ClearAccPos(GetHash(), nHeight - 1, SysCfg().GetIntervalPos());
	CFund fund(REWARD_FUND,rewardValue, nHeight);
	if(!acctInfo.OperateAccount(ADD_FREE, fund))
	{
		CKeyID keyId;
		view.GetKeyId(account, keyId);
		return state.DoS(100, ERRORMSG("ExecuteTx() : OperateAccount account keyId=%s error", keyId.ToString()),
							UPDATE_ACCOUNT_FAIL, "operate-account-failed");;
	}
	CUserID userId = acctInfo.keyID;
	if (!view.SetAccount(userId, acctInfo))
		return state.DoS(100, ERRORMSG("ExecuteTx() : write secure account info error"), UPDATE_ACCOUNT_FAIL,
				"bad-save-accountdb");
	txundo.vAccountOperLog.push_back(acctInfo.accountOperLog);
	txundo.txHash = GetHash();
	return true;
}
bool CRewardTransaction::GetAddress(set<CKeyID> &vAddr, CAccountViewCache &view) {
	CKeyID keyId;
	if (account.type() == typeid(CRegID)) {
		if (!view.GetKeyId(account, keyId))
			return false;
		vAddr.insert(keyId);
	} else if (account.type() == typeid(CPubKey)) {
		CPubKey pubKey = boost::get<CPubKey>(account);
		if (!pubKey.IsFullyValid())
			return false;
		vAddr.insert(pubKey.GetKeyID());
	}
	return true;
}
string CRewardTransaction::ToString(CAccountViewCache &view) const {
	string str;
	CKeyID keyId;
	view.GetKeyId(account, keyId);
	CRegID regId;
	view.GetRegId(account, regId);
	str += strprintf("txType=%s, hash=%s, ver=%d, account=%s, keyid=%s, rewardValue=%ld\n", txTypeArray[nTxType], GetHash().ToString().c_str(), nVersion, regId.ToString(), keyId.GetHex(), rewardValue);
	return str;
}
bool CRewardTransaction::CheckTransction(CValidationState &state, CAccountViewCache &view) {
	return true;
}


bool CRegisterAppTx::ExecuteTx(int nIndex, CAccountViewCache &view,CValidationState &state, CTxUndo &txundo,
		int nHeight, CTransactionDBCache &txCache, CScriptDBViewCache &scriptCache) {
	CID id(regAcctId);
	CAccount acctInfo;
	CScriptDBOperLog operLog;
	if (!view.GetAccount(regAcctId, acctInfo)) {
		return state.DoS(100, ERRORMSG("ExecuteTx() : read regist addr %s account info error", HexStr(id.GetID())),
				UPDATE_ACCOUNT_FAIL, "bad-read-accountdb");
	}

	uint64_t minusValue = llFees;
	if (minusValue > 0) {
		acctInfo.CompactAccount(nHeight);
		CFund fund(minusValue);
		if(!acctInfo.OperateAccount(MINUS_FREE, fund))
			return state.DoS(100, ERRORMSG("ExecuteTx() : OperateAccount account regId=%s error", boost::get<CRegID>(regAcctId).ToString()),
					UPDATE_ACCOUNT_FAIL, "operate-account-failed");
		txundo.vAccountOperLog.push_back(acctInfo.accountOperLog);
	}
	txundo.txHash = GetHash();


	CVmScript vmScript;
	CDataStream stream(script, SER_DISK, CLIENT_VERSION);
	try {
		stream >> vmScript;
	} catch (exception& e) {
		return state.DoS(100, ERRORMSG(("ExecuteTx() :intial() Unserialize to vmScript error:" + string(e.what())).c_str()),
				UPDATE_ACCOUNT_FAIL, "bad-query-scriptdb");
	}
	if(!vmScript.IsValid())
		return state.DoS(100, ERRORMSG("ExecuteTx() : vmScript invalid"), UPDATE_ACCOUNT_FAIL, "bad-query-scriptdb");

	CRegID regId(nHeight, nIndex);
	//create script account
	CKeyID keyId = Hash160(regId.GetVec6());
	CAccount account;
	account.keyID = keyId;
	account.regID = regId;
	//save new script content
	if(!scriptCache.SetScript(regId, script)){
		return state.DoS(100,
				ERRORMSG("ExecuteTx() : save script id %s script info error", regId.ToString()),
				UPDATE_ACCOUNT_FAIL, "bad-save-scriptdb");
	}
	if (!view.SaveAccountInfo(regId, keyId, account)) {
		return state.DoS(100,
				ERRORMSG("ExecuteTx() : create new account script id %s script info error",
						regId.ToString()), UPDATE_ACCOUNT_FAIL, "bad-save-scriptdb");
	}

	if(!operLog.vKey.empty()) {
		txundo.vScriptOperLog.push_back(operLog);
	}
	CUserID userId = acctInfo.keyID;
	if (!view.SetAccount(userId, acctInfo))
		return state.DoS(100, ERRORMSG("ExecuteTx() : write secure account info error"), UPDATE_ACCOUNT_FAIL,
				"bad-save-accountdb");
	return true;
}
bool CRegisterAppTx::UndoExecuteTx(int nIndex, CAccountViewCache &view, CValidationState &state, CTxUndo &txundo,
		int nHeight, CTransactionDBCache &txCache, CScriptDBViewCache &scriptCache) {
	CID id(regAcctId);
	CAccount account;
	CUserID userId;
	if (!view.GetAccount(regAcctId, account)) {
		return state.DoS(100, ERRORMSG("UndoUpdateAccount() : read regist addr %s account info error", HexStr(id.GetID())),
				UPDATE_ACCOUNT_FAIL, "bad-read-accountdb");
	}

	if(script.size() != 6) {

		CRegID scriptId(nHeight, nIndex);
		//delete script content
		if (!scriptCache.EraseScript(scriptId)) {
			return state.DoS(100, ERRORMSG("UndoUpdateAccount() : erase script id %s error", scriptId.ToString()),
					UPDATE_ACCOUNT_FAIL, "bad-save-scriptdb");
		}
		//delete account
		if(!view.EraseId(scriptId)){
			return state.DoS(100, ERRORMSG("UndoUpdateAccount() : erase script account %s error", scriptId.ToString()),
								UPDATE_ACCOUNT_FAIL, "bad-save-scriptdb");
		}
		CKeyID keyId = Hash160(scriptId.GetVec6());
		userId = keyId;
		if(!view.EraseAccount(userId)){
			return state.DoS(100, ERRORMSG("UndoUpdateAccount() : erase script account %s error", scriptId.ToString()),
								UPDATE_ACCOUNT_FAIL, "bad-save-scriptdb");
		}
	}

	for(auto &itemLog : txundo.vAccountOperLog){
		if(itemLog.keyID == account.keyID) {
			if(!account.UndoOperateAccount(itemLog))
				return state.DoS(100, ERRORMSG("UndoUpdateAccount: UndoOperateAccount error, keyId=%s", account.keyID.ToString()),
						UPDATE_ACCOUNT_FAIL, "undo-account-failed");
		}
	}

	vector<CScriptDBOperLog>::reverse_iterator rIterScriptDBLog = txundo.vScriptOperLog.rbegin();
	for(; rIterScriptDBLog != txundo.vScriptOperLog.rend(); ++rIterScriptDBLog) {
		if(!scriptCache.UndoScriptData(rIterScriptDBLog->vKey, rIterScriptDBLog->vValue))
			return state.DoS(100,
					ERRORMSG("ExecuteTx() : undo scriptdb data error"), UPDATE_ACCOUNT_FAIL, "bad-save-scriptdb");
	}
	userId = account.keyID;
	if (!view.SetAccount(userId, account))
		return state.DoS(100, ERRORMSG("ExecuteTx() : write secure account info error"), UPDATE_ACCOUNT_FAIL,
				"bad-save-accountdb");
	return true;
}
bool CRegisterAppTx::GetAddress(set<CKeyID> &vAddr, CAccountViewCache &view) {
	CKeyID keyId;
	if (!view.GetKeyId(regAcctId, keyId))
		return false;
	vAddr.insert(keyId);
	return true;
}
string CRegisterAppTx::ToString(CAccountViewCache &view) const {
	string str;
	CKeyID keyId;
	view.GetKeyId(regAcctId, keyId);
	str += strprintf("txType=%s, hash=%s, ver=%d, accountId=%s, keyid=%s, llFees=%ld, nValidHeight=%d\n",
	txTypeArray[nTxType], GetHash().ToString().c_str(), nVersion,boost::get<CRegID>(regAcctId).ToString(), keyId.GetHex(), llFees, nValidHeight);
	return str;
}
bool CRegisterAppTx::CheckTransction(CValidationState &state, CAccountViewCache &view) {
	CAccount  account;
	if(!view.GetAccount(regAcctId, account)) {
		return state.DoS(100, ERRORMSG("CheckTransaction() : register script tx get registe account info error"), REJECT_INVALID,
				"bad-read-account-info");
	}

	if (!MoneyRange(llFees)) {
			return state.DoS(100, ERRORMSG("CheckTransaction() : register script tx fee out of range"), REJECT_INVALID,
					"bad-register-script-fee-toolarge");
	}

	CAccount acctInfo;
	if (!view.GetAccount(boost::get<CRegID>(regAcctId), acctInfo)) {
		return state.DoS(100, ERRORMSG("CheckTransaction() :tx GetAccount falied"), REJECT_INVALID, "bad-getaccount");
	}
	if (!acctInfo.IsRegister()) {
		return state.DoS(100, ERRORMSG("CheckTransaction(): account have not registed public key"), REJECT_INVALID,
				"bad-no-pubkey");
	}
	uint256 signhash = SignatureHash();
	if (!CheckSignScript(signhash, signature, acctInfo.PublicKey)) {
		return state.DoS(100, ERRORMSG("CheckTransaction() :CheckSignScript failed"), REJECT_INVALID,
				"bad-signscript-check");
	}

	return true;
}


bool CFund::IsMergeFund(const int & nCurHeight, int &nMergeType) const {
	if (nCurHeight - nHeight > SysCfg().GetMaxCoinDay() / SysCfg().GetTargetSpacing()) {
		nMergeType = FREEDOM;
		return true;
	}

	switch (nFundType) {
	case REWARD_FUND:
		if (nCurHeight - nHeight > COINBASE_MATURITY) {
			nMergeType = FREEDOM_FUND;  // Merget to Freedom;
			return true;
		}
		break;
	case FREEDOM_FUND:
		return false;
	default:
		assert(0);
	}
	return false;
}
Object CFund::ToJosnObj() const
{
	Object obj;
	static const string fundTypeArray[] = { "NULL_FUNDTYPE", "FREEDOM", "REWARD_FUND", "FREEDOM_FUND", "IN_FREEZD_FUND",
			"OUT_FREEZD_FUND", "SELF_FREEZD_FUND" };
	obj.push_back(Pair("nType",     fundTypeArray[nFundType]));
	obj.push_back(Pair("value",     value));
	obj.push_back(Pair("confirmed hight",     nHeight));
	return obj;
}
string CFund::ToString() const {
	string str;
	static const string fundTypeArray[] = { "NULL_FUNDTYPE", "FREEDOM", "REWARD_FUND", "FREEDOM_FUND", "FREEZD_FUND",
			"SELF_FREEZD_FUND" };
	str += strprintf("            nType=%s, appId=%s, value=%ld, nHeight=%d\n",
	fundTypeArray[nFundType], HexStr(appId).c_str(), value, nHeight);
	return str;
//	return write_string(Value(ToJosnObj()),true);
}

string COperFund::ToString() const {
	string str("");
	string strOperType[] = { "NULL_OPER_TYPE", "ADD_FUND", "MINUS_FUND" };
	str += strprintf("        list funds: operType=%s\n", strOperType[operType]);
	vector<CFund>::const_iterator iterFund = vFund.begin();
	for (; iterFund != vFund.end(); ++iterFund)
		str += iterFund->ToString();
	return str;
}

string CAccountOperLog::ToString() const {
	string str("");
	str += strprintf("    list oper funds: keyId=%d\n",keyID.GetHex());
	vector<COperFund>::const_iterator iterOperFund = vOperFund.begin();
	for (; iterOperFund != vOperFund.end(); ++iterOperFund)
		str += iterOperFund->ToString();
	return str;
}

string CTxUndo::ToString() const {
	vector<CAccountOperLog>::const_iterator iterLog = vAccountOperLog.begin();
	string strTxHash("txHash:");
	strTxHash += txHash.GetHex();
	strTxHash += "\n";
	string str("  list account oper Log:\n");
	for (; iterLog != vAccountOperLog.end(); ++iterLog) {
		str += iterLog->ToString();
	}
	strTxHash += str;
	vector<CScriptDBOperLog>::const_iterator iterDbLog = vScriptOperLog.begin();
	string strDbLog(" list script db oper Log:\n");
	for	(; iterDbLog !=  vScriptOperLog.end(); ++iterDbLog) {
		strDbLog += iterDbLog->ToString();
	}
	strTxHash += strDbLog;
	return strTxHash;
}

bool CTxUndo::GetAccountOperLog(const CKeyID &keyId, CAccountOperLog &accountOperLog) {
	vector<CAccountOperLog>::iterator iterLog = vAccountOperLog.begin();
	for (; iterLog != vAccountOperLog.end(); ++iterLog) {
		if (iterLog->keyID == keyId) {
			accountOperLog = *iterLog;
			return true;
		}
	}
	return false;
}


bool CAccount::CompactAccount(int nCurHeight) {
	if (nCurHeight <= 0) {
		return false;
	}
	bool bMergeRewardRes = MergerFund(vRewardFund, nCurHeight);
	bool bMergeFreeRes = MergerFund(vFreedomFund, nCurHeight);
	return bMergeRewardRes||bMergeFreeRes;
}
bool CAccount::MergerFund(vector<CFund> &vFund, int nCurHeight) {
	stable_sort(vFund.begin(), vFund.end(), greater<CFund>());
	uint64_t value = 0;
	vector<CFund> vMinusFunds;
	vector<CFund> vAddFunds;
	vector<CFund>::reverse_iterator iterFund = vFund.rbegin();
	bool bMergeFund = false;
	bool bHasMergd = false;
	for (; iterFund != vFund.rend();) {
		int nMergerType(0);
		if (iterFund->IsMergeFund(nCurHeight, nMergerType)) {
			bMergeFund = true;
			if (FREEDOM == nMergerType) {
				value += iterFund->value;
			} else if (FREEDOM_FUND == nMergerType) {
				CFund fund(*iterFund);
				fund.nFundType = FREEDOM_FUND;
				AddToFreedom(fund);
			} else {
				assert(0);
			}
			vMinusFunds.push_back(*iterFund);
			vFund.erase((++iterFund).base());
		}
		if (!bMergeFund) {
			break;
		} else {
			bHasMergd = true;
			bMergeFund = false;
		}
	}

	if (value) {
		llValues += value;
		CFund addFund;
		addFund.nFundType = FREEDOM;
		addFund.value = value;
		vAddFunds.push_back(addFund);
	}
	if (!vMinusFunds.empty()) {
		COperFund log(MINUS_FUND, vMinusFunds);
		WriteOperLog(log);
	}
	if (!vAddFunds.empty()) {
		COperFund OperLog(ADD_FUND, vAddFunds);
		WriteOperLog(OperLog);
	}
	return bHasMergd;
}
void CAccount::WriteOperLog(const COperFund &operLog) {
	accountOperLog.InsertOperateLog(operLog);
}
void CAccount::AddToFreedom(const CFund &fund, bool bWriteLog) {
	int nTenDayBlocks = 10 * ((24 * 60 * 60) / SysCfg().GetTargetSpacing());
	int nHeightPoint = fund.nHeight - fund.nHeight % nTenDayBlocks;
	vector<CFund>::iterator it = find_if(vFreedomFund.begin(), vFreedomFund.end(), [&](const CFund& fundInVector)
	{	return fundInVector.nHeight == nHeightPoint;});
	if (vFreedomFund.end() == it) {
		CFund addFund(fund);
		addFund.nHeight = nHeightPoint;
		vFreedomFund.push_back(addFund);
		if (bWriteLog)
			WriteOperLog(ADD_FUND, addFund);
	} else {
		CFund addFund(*it);
		it->value += fund.value;
		addFund.value = fund.value;
		if (bWriteLog)
			WriteOperLog(ADD_FUND, addFund);
	}
}
bool CAccount::MinusFree(const CFund &fund) {
	vector<CFund> vOperFund;
	uint64_t nCandidateValue = 0;
	vector<CFund>::iterator iterFound = vFreedomFund.begin();
	if (!vFreedomFund.empty()) {
		for (; iterFound != vFreedomFund.end(); ++iterFound) {
			nCandidateValue += iterFound->value;
			if (nCandidateValue >= fund.value) {
				break;
			}
		}
	}
	if (iterFound != vFreedomFund.end()) {
		uint64_t remainValue = nCandidateValue - fund.value;
		if (remainValue > 0) {
			vOperFund.insert(vOperFund.end(), vFreedomFund.begin(), iterFound);
			CFund fundMinus(*iterFound);
			fundMinus.value = iterFound->value - remainValue;
			iterFound->value = remainValue;
			vOperFund.push_back(fundMinus);
			vFreedomFund.erase(vFreedomFund.begin(), iterFound);
		}else{
			vOperFund.insert(vOperFund.end(), vFreedomFund.begin(), iterFound + 1);
			vFreedomFund.erase(vFreedomFund.begin(), iterFound + 1);
		}

		COperFund operLog(MINUS_FUND, vOperFund);
		WriteOperLog(operLog);
		return true;
	} else {
		if (llValues < fund.value - nCandidateValue)
			return false;

		CFund freedom;
		freedom.nFundType = FREEDOM;
		freedom.value = fund.value - nCandidateValue;
		llValues -= fund.value - nCandidateValue;

		vOperFund.insert(vOperFund.end(), vFreedomFund.begin(), vFreedomFund.end());
		vFreedomFund.clear();
		vOperFund.push_back(freedom);
		COperFund operLog(MINUS_FUND, vOperFund);
		WriteOperLog(operLog);

		return true;
	}

}
bool CAccount::UndoOperateAccount(const CAccountOperLog & accountOperLog) {
//	LogPrint("undo_account", "after operate:%s\n", ToString());
	vector<COperFund>::const_reverse_iterator iterOperFundLog = accountOperLog.vOperFund.rbegin();
	for (; iterOperFundLog != accountOperLog.vOperFund.rend(); ++iterOperFundLog) {
		vector<CFund>::const_iterator iterFund = iterOperFundLog->vFund.begin();
		for (; iterFund != iterOperFundLog->vFund.end(); ++iterFund) {
			switch (iterFund->nFundType) {
			case FREEDOM:
				if (ADD_FUND == iterOperFundLog->operType) {
					assert(llValues >= iterFund->value);
					llValues -= iterFund->value;
				} else if (MINUS_FUND == iterOperFundLog->operType) {
					llValues += iterFund->value;
				}
				break;
			case REWARD_FUND:
				if (ADD_FUND == iterOperFundLog->operType)
					vRewardFund.erase(remove(vRewardFund.begin(), vRewardFund.end(), *iterFund), vRewardFund.end());
				else if (MINUS_FUND == iterOperFundLog->operType)
					vRewardFund.push_back(*iterFund);
				break;
			case FREEDOM_FUND:
				if (ADD_FUND == iterOperFundLog->operType) {
					auto it = find_if(vFreedomFund.begin(), vFreedomFund.end(), [&](const CFund& fundInVector) {
						if (fundInVector.nFundType== iterFund->nFundType &&
								fundInVector.nHeight == iterFund->nHeight&&
								fundInVector.value>=iterFund->value) {
							return true;
						}
						return false;
					});

					assert(it != vFreedomFund.end());

					it->value -= iterFund->value;
					if (!it->value)
						vFreedomFund.erase(it);
				} else if (MINUS_FUND == iterOperFundLog->operType) {
					AddToFreedom(*iterFund, false);
				}

				break;
			default:
				assert(0);
				return false;
			}
		}
	}
//	LogPrint("undo_account", "before operate:%s\n", ToString().c_str());
	return true;
}
void CAccount::ClearAccPos(uint256 hash, int prevBlockHeight, int nIntervalPos) {
	/**
	 * @todo change the  uint256 hash to uint256 &hash
	 */

	int days = 0;
	uint64_t money = 0;
	money = llValues;
	{
		COperFund acclog;
		acclog.operType = MINUS_FUND;
		{
			if(llValues > 0) {
				CFund fund(FREEDOM, llValues, 0);
				acclog.vFund.push_back(fund);
				llValues = 0;
			}
		}
		vector<CFund>::iterator iterFund = vFreedomFund.begin();
		for (; iterFund != vFreedomFund.end();) {
			days = (prevBlockHeight - iterFund->nHeight) / nIntervalPos;
			days = min(days, 30);
			days = max(days, 0);
			if (days != 0) {
				money += iterFund->value;
				acclog.vFund.push_back(*iterFund);
				iterFund = vFreedomFund.erase(iterFund);
			} else
				++iterFund;
		}
		if (money > 0) {
			WriteOperLog(acclog);
		}
	}
	{
		if (money > 0) {
			CFund fund(FREEDOM_FUND, money, prevBlockHeight + 1);
			AddToFreedom(fund);
		}
	}
}
uint64_t CAccount::GetAccountPos(int prevBlockHeight) const {
	uint64_t accpos = 0;
	int days = 0;

	accpos = llValues * 30;
	for (const auto &freeFund :vFreedomFund) {

		int nIntervalPos = SysCfg().GetIntervalPos();
		assert(nIntervalPos);
		days = (prevBlockHeight - freeFund.nHeight) / nIntervalPos;
		days = min(days, 30);
		days = max(days, 0);
		if (days != 0) {
			accpos += freeFund.value * days;
		}
	}
	return accpos;
}
uint64_t CAccount::GetRewardAmount(int nCurHeight) {
	CompactAccount(nCurHeight);
	uint64_t balance = 0;

	for(auto &fund:vRewardFund) {
		balance += fund.value;
	}
	return balance;
}
uint64_t CAccount::GetRawBalance(int nCurHeight) {
	CompactAccount(nCurHeight);
	uint64_t balance = llValues;

	for (auto &fund : vFreedomFund) {
		balance += fund.value;
	}
	return balance;
}
uint256 CAccount::BuildMerkleTree(int prevBlockHeight) const {
	vector<uint256> vMerkleTree;
	vMerkleTree.clear();

	for (const auto &freeFund : vFreedomFund) {
		//at least larger than 100 height
		//if (prevBlockHeight < freeFund.confirmHeight + 100) {
		vMerkleTree.push_back(freeFund.GetHash());
		//}
	}

	int j = 0;
	int nSize = vMerkleTree.size();
	for (; nSize > 1; nSize = (nSize + 1) / 2) {
		for (int i = 0; i < nSize; i += 2) {
			int i2 = min(i + 1, nSize - 1);
			vMerkleTree.push_back(
					Hash(BEGIN(vMerkleTree[j + i]), END(vMerkleTree[j + i]), BEGIN(vMerkleTree[j + i2]),
							END(vMerkleTree[j + i2])));
		}
		j += nSize;
	}
	return (vMerkleTree.empty() ? 0 : vMerkleTree.back());
}
Object CAccount::ToJosnObj() const
{
	using namespace json_spirit;
	Object obj;
	static const string fundTypeArray[] = { "NULL_FUNDTYPE", "FREEDOM", "REWARD_FUND", "FREEDOM_FUND"};
//	obj.push_back(Pair("height", chainActive.Height()));
	obj.push_back(Pair("Address",     keyID.ToAddress()));
	obj.push_back(Pair("KeyID",     keyID.ToString()));
	obj.push_back(Pair("RegID",     regID.ToString()));
	obj.push_back(Pair("PublicKey",  PublicKey.ToString()));
	obj.push_back(Pair("MinerPKey",  MinerPKey.ToString()));
	obj.push_back(Pair("FreeValues",     llValues));



	Array RewardFund;

	vector<CFund> te=vRewardFund;
	stable_sort(te.begin(), te.end(), greater<CFund>());
	for (auto const & rew:te) {
		RewardFund.push_back(rew.ToJosnObj());
	}
	obj.push_back(Pair("RewardFund",     RewardFund));


	Array FreedomFund;
	te.clear();
	te=vFreedomFund;
	stable_sort(te.begin(), te.end(), greater<CFund>());
	for (auto& rew:te) {
		FreedomFund.push_back(rew.ToJosnObj());
	}
	obj.push_back(Pair("FreedomFund",     FreedomFund));
	return obj;
}
string CAccount::ToString() const {
	string str;
	str += strprintf("regID=%s, keyID=%s, publicKey=%s, minerpubkey=%s, values=%ld\n",
	regID.ToString(), keyID.GetHex().c_str(), PublicKey.ToString().c_str(), MinerPKey.ToString().c_str(), llValues);
	for (unsigned int i = 0; i < vRewardFund.size(); ++i) {
		str += "    " + vRewardFund[i].ToString() + "\n";
	}
	for (unsigned int i = 0; i < vFreedomFund.size(); ++i) {
		str += "    " + vFreedomFund[i].ToString() + "\n";
	}
	return str;
	//return  write_string(Value(ToJosnObj()),true);
}
void CAccount::WriteOperLog(AccountOper emOperType, const CFund &fund) {
	vector<CFund> vFund;
	vFund.push_back(fund);
	COperFund operLog(emOperType, vFund);
	WriteOperLog(operLog);
}
bool CAccount::IsMoneyOverflow(uint64_t nAddMoney) {
	if (!MoneyRange(nAddMoney))
		return false;

	uint64_t nTotalMoney = 0;
	nTotalMoney = GetVecMoney(vFreedomFund)+GetVecMoney(vRewardFund)+llValues+nAddMoney;
	return MoneyRange(static_cast<int64_t>(nTotalMoney) );
}
uint64_t CAccount::GetVecMoney(const vector<CFund>& vFund){
	uint64_t nTotal = 0;
	for(vector<CFund>::const_iterator it = vFund.begin();it != vFund.end();it++){
		nTotal += it->value;
	}

	return nTotal;
}
bool CAccount::FindFund(const vector<CFund>& vFund, const vector_unsigned_char &scriptID,CFund&fund) {
	for (vector<CFund>::const_iterator it = vFund.begin(); it != vFund.end(); it++) {
		if (it->appId == scriptID) {
			fund = *it;
			return true;
		}
	}
	return false;
}
bool CAccount::IsFundValid(OperType type, const CFund &fund) {

	if(ADD_FREE == type)
	{
		if (REWARD_FUND != fund.nFundType && FREEDOM_FUND != fund.nFundType)
			return false;
		if (!IsMoneyOverflow(fund.value))
			return false;
	}

	return true;
}
bool CAccount::OperateAccount(OperType type, const CFund &fund) {
//	LogPrint("op_account", "before operate:%s\n", ToString());
	assert(keyID != uint160(0));
	if (keyID != accountOperLog.keyID)
		accountOperLog.keyID = keyID;

	if (!IsFundValid(type, fund))
		return false;

	if (!fund.value)
		return true;

	bool bRet = true;
	switch (type) {
	case ADD_FREE: {
		if (REWARD_FUND == fund.nFundType) {
			vRewardFund.push_back(fund);
			WriteOperLog(ADD_FUND, fund);
		} else
			AddToFreedom(fund);
		break;
	}
	case MINUS_FREE: {
		bRet = MinusFree(fund);
		break;
	}
	default:
		assert(0);
	}
//	LogPrint("op_account", "after operate:%s\n", ToString());
//	LogPrint("account", "oper log list:%s\n", accountOperLog.ToString());
	return bRet;
}
