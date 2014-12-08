#include "serialize.h"
#include <boost/foreach.hpp>
#include "hash.h"
#include "util.h"
#include "account.h"
#include "main.h"
#include <algorithm>
#include "txdb.h"
#include "VmScript/VmScriptRun.h"
#include "core.h"
#include "miner.h"
#include "json/json_spirit_utils.h"
#include "json/json_spirit_value.h"
#include "json/json_spirit_writer_template.h"
using namespace json_spirit;
static string txTypeArray[] = { "NULL_TXTYPE", "REG_ACCT_TX", "NORMAL_TX", "CONTRACT_TX", "FREEZE_TX",
		"REWARD_TX", "REG_SCRIPT_TX" };


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
		assert(0);
	}
	return CNullID();
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

bool CRegisterAccountTx::UpdateAccount(int nIndex, CAccountViewCache &view, CValidationState &state, CTxUndo &txundo,
		int nHeight, CTransactionDBCache &txCache, CScriptDBViewCache &scriptCache) {
	CAccount account;
	CRegID regId(nHeight, nIndex);
	CKeyID keyId = boost::get<CPubKey>(userId).GetKeyID();
	if (!view.GetAccount(userId, account))
		return state.DoS(100, ERROR("UpdateAccounts() : read source keyId %s account info error", keyId.ToString()),
				UPDATE_ACCOUNT_FAIL, "bad-read-accountdb");

	if(account.PublicKey.IsFullyValid() && account.PublicKey.GetKeyID() == keyId) {
		return state.DoS(100, ERROR("UpdateAccounts() : read source keyId %s duplicate register", keyId.ToString()),
					UPDATE_ACCOUNT_FAIL, "duplicate-register-account");
	}
	account.PublicKey = boost::get<CPubKey>(userId);
	if (llFees > 0) {
		CFund fund(llFees);
		account.OperateAccount(MINUS_FREE, fund);
	}

	account.PublicKey = boost::get<CPubKey>(userId);
	account.regID = regId;
	if (typeid(CPubKey) == typeid(minerId)) {
		account.MinerPKey = boost::get<CPubKey>(minerId);

		if (account.MinerPKey.IsValid() && !account.MinerPKey.IsFullyValid()) {
			return state.DoS(100, ERROR("UpdateAccounts() : MinerPKey:%s Is Invalid", account.MinerPKey.ToString()),
					UPDATE_ACCOUNT_FAIL, "MinerPKey Is Invalid");
		}
	}

	if (!view.SaveAccountInfo(regId, keyId, account)) {
		return state.DoS(100, ERROR("UpdateAccounts() : write source addr %s account info error", regId.ToString()),
				UPDATE_ACCOUNT_FAIL, "bad-read-accountdb");
	}
	txundo.vAccountOperLog.push_back(account.accountOperLog);
	txundo.txHash = GetHash();
	return true;
}
bool CRegisterAccountTx::UndoUpdateAccount(int nIndex, CAccountViewCache &view, CValidationState &state,
		CTxUndo &txundo, int nHeight, CTransactionDBCache &txCache, CScriptDBViewCache &scriptCache) {
	//drop account
	CRegID accountId(nHeight, nIndex);
	CAccount oldAccount;
	if (!view.GetAccount(accountId, oldAccount))
		return state.DoS(100,
				ERROR("UpdateAccounts() : read secure account=%s info error", accountId.ToString()),
				UPDATE_ACCOUNT_FAIL, "bad-read-accountdb");
	CKeyID keyId;
	view.GetKeyId(accountId, keyId);
	if (!oldAccount.IsEmptyValue()) {
		CPubKey empPubKey;
		oldAccount.PublicKey = empPubKey;
		if (llFees > 0) {
			CAccountOperLog accountOperLog;
			if (!txundo.GetAccountOperLog(keyId, accountOperLog))
				return state.DoS(100, ERROR("UpdateAccounts() : read keyId=%s tx undo info error", keyId.GetHex()),
						UPDATE_ACCOUNT_FAIL, "bad-read-txundoinfo");
			oldAccount.UndoOperateAccount(accountOperLog);
		}
		CUserID userId(keyId);
		view.SetAccount(userId, oldAccount);
	} else {
		view.EraseAccount(userId);
	}
	view.EraseId(accountId);
	return true;
}
bool CRegisterAccountTx::IsValidHeight(int nCurHeight, int nTxCacheHeight) const {
	if (nValidHeight > nCurHeight + nTxCacheHeight / 2)
		return false;
	if (nValidHeight < nCurHeight - nTxCacheHeight / 2)
		return false;
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
		return state.DoS(100, ERROR("CheckTransaction() : register tx public key is invalid"), REJECT_INVALID,
				"bad-regtx-publickey");
	}

	//check signature script
	uint256 sighash = SignatureHash();
	if (!boost::get<CPubKey>(userId).Verify(sighash, signature))
		return state.DoS(100, ERROR("CheckTransaction() : register tx signature error "), REJECT_INVALID,
				"bad-regtx-signature");

	if (!MoneyRange(llFees))
		return state.DoS(100, ERROR("CheckTransaction() : register tx fee out of range"), REJECT_INVALID,
				"bad-regtx-fee-toolarge");
	return true;
}

bool CTransaction::UpdateAccount(int nIndex, CAccountViewCache &view, CValidationState &state, CTxUndo &txundo,
		int nHeight, CTransactionDBCache &txCache, CScriptDBViewCache &scriptCache) {
	CAccount sourceAccount;
	CAccount desAccount;
	if (!view.GetAccount(srcUserId, sourceAccount))
		return state.DoS(100,
				ERROR("UpdateAccounts() : read source addr %s account info error", (boost::get<CRegID>(srcUserId).ToString())),
				UPDATE_ACCOUNT_FAIL, "bad-read-accountdb");


	CID destId(desUserId);
	if(!view.GetAccount(desUserId, desAccount))
		desAccount.keyID = boost::get<CKeyID>(desUserId);

	uint64_t minusValue = llFees + llValues;
	CFund minusFund(minusValue);
	sourceAccount.CompactAccount(nHeight - 1);
	if (!sourceAccount.OperateAccount(MINUS_FREE, minusFund))
		return state.DoS(100, ERROR("UpdateAccounts() : secure accounts insufficient funds"), UPDATE_ACCOUNT_FAIL,
				"bad-read-accountdb");
	uint64_t addValue = llValues;
	CFund addFund(FREEDOM_FUND,addValue, nHeight);
	desAccount.CompactAccount(nHeight - 1);
	desAccount.OperateAccount(ADD_FREE, addFund);
	vector<CAccount> vSecureAccounts;
	vSecureAccounts.push_back(sourceAccount);
	vSecureAccounts.push_back(desAccount);
	if (!view.BatchWrite(vSecureAccounts))
		return state.DoS(100, ERROR("UpdateAccounts() : batch write secure accounts info error"), UPDATE_ACCOUNT_FAIL,
				"bad-read-accountdb");
	txundo.vAccountOperLog.push_back(sourceAccount.accountOperLog);
	txundo.vAccountOperLog.push_back(desAccount.accountOperLog);
	txundo.txHash = GetHash();
	return true;
}
bool CTransaction::UndoUpdateAccount(int nIndex, CAccountViewCache &view, CValidationState &state, CTxUndo &txundo,
		int nHeight, CTransactionDBCache &txCache, CScriptDBViewCache &scriptCache) {
	CAccount sourceAccount;
	CAccount desAccount;
	CID srcId(srcUserId);
	if (!view.GetAccount(srcUserId, sourceAccount))
		return state.DoS(100,
				ERROR("UpdateAccounts() : read source addr %s account info error", HexStr(srcId.GetID())),
				UPDATE_ACCOUNT_FAIL, "bad-read-accountdb");

	CID destId(desUserId);
	if (!view.GetAccount(desUserId, desAccount)) {
		return state.DoS(100,
				ERROR("UpdateAccounts() : read destination addr %s account info error", HexStr(destId.GetID())),
				UPDATE_ACCOUNT_FAIL, "bad-read-accountdb");
	}

	for(auto &itemLog : txundo.vAccountOperLog){
		if(itemLog.keyID == sourceAccount.keyID) {
			sourceAccount.UndoOperateAccount(itemLog);
		}else if(itemLog.keyID == desAccount.keyID) {
			desAccount.UndoOperateAccount(itemLog);
		}
	}
	vector<CAccount> vAccounts;
	vAccounts.push_back(sourceAccount);
	vAccounts.push_back(desAccount);

	if (!view.BatchWrite(vAccounts))
		return state.DoS(100, ERROR("UpdateAccounts() : batch save accounts info error"), UPDATE_ACCOUNT_FAIL,
				"bad-read-accountdb");
	return true;
}
bool CTransaction::GetAddress(set<CKeyID> &vAddr, CAccountViewCache &view) {
	CKeyID srcKeyId;
	if (!view.GetKeyId(srcUserId, srcKeyId))
		return false;

	CKeyID desKeyId;
	if(desUserId.type() == typeid(CKeyID)) {
		desKeyId = boost::get<CKeyID>(desUserId);
	} else if(desUserId.type() == typeid(CRegID)){
		if (!view.GetKeyId(desUserId, desKeyId))
			return false;
	} else
		return false;

	vAddr.insert(srcKeyId);
	vAddr.insert(desKeyId);
	return true;
}
bool CTransaction::IsValidHeight(int nCurHeight, int nTxCacheHeight) const {
	if (nValidHeight > nCurHeight + nTxCacheHeight / 2)
		return false;
	if (nValidHeight < nCurHeight - nTxCacheHeight / 2)
		return false;
	return true;
}
string CTransaction::ToString(CAccountViewCache &view) const {
	string str;
	CKeyID srcKeyId, desKeyId;
	view.GetKeyId(srcUserId, srcKeyId);
	if (desUserId.type() == typeid(CKeyID)) {
		str += strprintf("txType=%s, hash=	%s, nVersion=%d, srcAccountId=%s, llFees=%ld, llValues=%ld, desKeyId=%s, nValidHeight=%d\n",
		txTypeArray[nTxType], GetHash().ToString().c_str(), nVersion, (boost::get<CRegID>(srcUserId).ToString()), llFees, llValues, boost::get<CKeyID>(desUserId).GetHex(), nValidHeight);
	} else if(desUserId.type() == typeid(CRegID)) {
		view.GetKeyId(desUserId, desKeyId);
		str += strprintf("txType=%s, hash=%s, nVersion=%d, srcAccountId=%s, srcKeyId=%s, llFees=%ld, llValues=%ld, desAccountId=%s, desKeyId=%s, nValidHeight=%d\n",
		txTypeArray[nTxType], GetHash().ToString().c_str(), nVersion, boost::get<CRegID>(srcUserId).ToString(), srcKeyId.GetHex(), llFees, llValues, boost::get<CRegID>(desUserId).ToString(), desKeyId.GetHex(), nValidHeight);
	}

	return str;
}
bool CTransaction::CheckTransction(CValidationState &state, CAccountViewCache &view) {
	//check source addr, destination addr
	if (srcUserId.type() != typeid(CRegID)) {
		return state.DoS(100, ERROR("CheckTransaction() : normal tx source address or des address is invalid"),
				REJECT_INVALID, "bad-normaltx-sourceaddr");
	}
	if (!MoneyRange(llFees)) {
		return state.DoS(100, ERROR("CheckTransaction() : normal tx fee out of range"), REJECT_INVALID,
				"bad-normaltx-fee-toolarge");
	}
	if (!MoneyRange(llValues)) {
		return state.DoS(100, ERROR("CheckTransaction(): normal tx value our of range"), REJECT_INVALID,
				"bad-normaltx-value-toolarge");
	}

	//check signature script
	uint256 sighash = SignatureHash();
	if (!CheckSignScript(boost::get<CRegID>(srcUserId), sighash, signature, state, view)) {
		return state.DoS(100, ERROR("CheckTransaction() :CheckSignScript failed"), REJECT_INVALID,
				"bad-signscript-check");
	}

	//鑻ュ湪浜ゆ槗绱㈠紩鏁版嵁搴撲腑瀛樺湪浜ゆ槗hash锛屾浜ゆ槗宸茬粡琚‘璁よ繃锛屾棤椤绘鏌�
	CDiskTxPos postx;
	if (!pblocktree->ReadTxIndex(GetHash(), postx)) {
		//濡傛灉鏄氦鏄撹纭杩涘叆block涓椂锛岃嫢鐩殑鍦板潃涓簁eyId鏃跺繀椤绘槸鏈敞鍐岃处鎴�
			CAccount acctDesInfo;
			if (desUserId.type() == typeid(CKeyID)) {
				if (view.GetAccount(desUserId, acctDesInfo) && acctDesInfo.IsRegister()) {
					return state.DoS(100,
							ERROR(
									"CheckTransaction() : normal tx des account have regested, destination addr must be account id"),
							REJECT_INVALID, "bad-normal-desaddr error");
				}
			}

	}

	return true;
}


bool CContractTransaction::UpdateAccount(int nIndex, CAccountViewCache &view, CValidationState &state, CTxUndo &txundo,
		int nHeight, CTransactionDBCache &txCache, CScriptDBViewCache &scriptCache) {

	CAccount sourceAccount;
	uint64_t minusValue = llFees;
	CFund minusFund(minusValue);
	CID id(*(vAccountRegId.rbegin()));
	if (!view.GetAccount(*(vAccountRegId.rbegin()), sourceAccount))
		return state.DoS(100,
				ERROR("UpdateAccounts() : read source addr %s account info error", HexStr(id.GetID())),
				UPDATE_ACCOUNT_FAIL, "bad-read-accountdb");
	sourceAccount.CompactAccount(nHeight - 1);
	if (!sourceAccount.OperateAccount(MINUS_FREE, minusFund))
		return state.DoS(100, ERROR("UpdateAccounts() : secure accounts insufficient funds"), UPDATE_ACCOUNT_FAIL,
				"bad-read-accountdb");
	CUserID userId = sourceAccount.keyID;
	if(!view.SetAccount(userId, sourceAccount)){
		return state.DoS(100, ERROR("UpdataAccounts() :save account%s info error", HexStr(id.GetID())),
				UPDATE_ACCOUNT_FAIL, "bad-write-accountdb");

	}
	//鎵ｅ噺灏忚垂鏃ュ織
	txundo.vAccountOperLog.push_back(sourceAccount.accountOperLog);

	CVmScriptRun vmRun;
	std::shared_ptr<CBaseTransaction> pTx = GetNewInstance();
	uint64_t el = GetElementForBurn(chainActive.Tip());
	tuple<bool, uint64_t, string> ret = vmRun.run(pTx, view, scriptCache, nHeight, el);
	if (!std::get<0>(ret))
		return state.DoS(100,
				ERROR("UpdateAccounts() : ContractTransaction UpdateAccount txhash=%s run script error:%s",
						GetHash().GetHex(), std::get<2>(ret)), UPDATE_ACCOUNT_FAIL, "run-script-error");

	set<CKeyID> vAddress;
	vector<std::shared_ptr<CAccount> > &vAccount = vmRun.GetNewAccont();
	for (auto & itemAccount : vAccount) {
		vAddress.insert(itemAccount->keyID);
		userId = itemAccount->keyID;
		if (!view.SetAccount(userId, *itemAccount))
			return state.DoS(100,
					ERROR("UpdateAccounts() : ContractTransaction Updateaccount write account info error"),
					UPDATE_ACCOUNT_FAIL, "bad-write-accountdb");
		txundo.vAccountOperLog.push_back((itemAccount->accountOperLog));
	}
	txundo.vScriptOperLog = *vmRun.GetDbLog();
	txundo.txHash = GetHash();
	if(!scriptCache.SetTxRelAccout(GetHash(), vAddress))
		return ERROR("UpdateAccounts() : ContractTransaction Updateaccount save tx relate account info to script db error");
	return true;
}
bool CContractTransaction::UndoUpdateAccount(int nIndex, CAccountViewCache &view, CValidationState &state,
		CTxUndo &txundo, int nHeight, CTransactionDBCache &txCache, CScriptDBViewCache &scriptCache) {

	for(auto & operacctlog : txundo.vAccountOperLog) {
		CAccount account;
		CUserID userId = operacctlog.keyID;
		if(!view.GetAccount(userId, account))  {
			return state.DoS(100,
							ERROR("UpdateAccounts() : ContractTransaction undo updateaccount read accountId= %s account info error"),
							UPDATE_ACCOUNT_FAIL, "bad-read-accountdb");
		}
		account.UndoOperateAccount(operacctlog);
		if(!view.SetAccount(userId, account)) {
			return state.DoS(100,
					ERROR("UpdateAccounts() : ContractTransaction undo updateaccount write accountId= %s account info error"),
					UPDATE_ACCOUNT_FAIL, "bad-write-accountdb");
		}
	}
	for(auto &operlog : txundo.vScriptOperLog)
		if(!scriptCache.SetData(operlog.vKey, operlog.vValue))
			return state.DoS(100,
					ERROR("UpdateAccounts() : ContractTransaction undo scriptdb data error"), UPDATE_ACCOUNT_FAIL, "bad-save-scriptdb");
	return true;
}

bool CContractTransaction::GetAddress(set<CKeyID> &vAddr, CAccountViewCache &view) {
	CKeyID keyId;
	for(auto & accountId : vAccountRegId) {
		if(!view.GetKeyId(accountId, keyId))
			return false;
		vAddr.insert(keyId);
	}
	CVmScriptRun vmRun;
	std::shared_ptr<CBaseTransaction> pTx = GetNewInstance();
	uint64_t el = GetElementForBurn(chainActive.Tip());
	CScriptDBViewCache scriptDBView(*pScriptDBTip, true);
	if(!pTxCacheTip->IsContainTx(GetHash())) {
		CAccountViewCache accountView(view, true);
		tuple<bool, uint64_t, string> ret = vmRun.run(pTx, accountView, scriptDBView, chainActive.Height() +1, el);
		if (!std::get<0>(ret))
			return ERROR("GetAddress()  : %s", std::get<2>(ret));

		vector<shared_ptr<CAccount> > vpAccount = vmRun.GetNewAccont();

		for(auto & item : vpAccount) {
			vAddr.insert(item->keyID);
		}
	}
	else {
		set<CKeyID> vTxRelAccount;
		if(!scriptDBView.GetTxRelAccount(GetHash(), vTxRelAccount))
			return false;
		vAddr.insert(vTxRelAccount.begin(), vTxRelAccount.end());
	}
	return true;
}

bool CContractTransaction::IsValidHeight(int nCurHeight, int nTxCacheHeight) const {
	if (nValidHeight > nCurHeight + nTxCacheHeight / 2)
		return false;
	if (nValidHeight < nCurHeight - nTxCacheHeight / 2)
		return false;
	return true;
}

string CContractTransaction::ToString(CAccountViewCache &view) const {
	string str;
	string strAccountId("");
	for(auto accountId : vAccountRegId) {
		strAccountId += boost::get<CRegID>(accountId).ToString();
		strAccountId += "|";
	}
	strAccountId = strAccountId.substr(0, strAccountId.length()-1);
	str += strprintf("txType=%s, hash=%s, ver=%d, vAccountRegId=%s, llFees=%ld, vContract=%s\n",
	txTypeArray[nTxType], GetHash().ToString().c_str(), nVersion, strAccountId, llFees, HexStr(vContract).c_str());
	return str;
}
bool CContractTransaction::CheckTransction(CValidationState &state, CAccountViewCache &view) {
	if (!MoneyRange(llFees)) {
		return state.DoS(100, ERROR("CheckTransaction() : appeal tx fee out of range"), REJECT_INVALID,
				"bad-appeal-fee-toolarge");
	}

	if ((vAccountRegId.size()) != (vSignature.size())) {
		return state.DoS(100, ERROR("CheckTransaction() :account size not equal to sign size"), REJECT_INVALID,
				"bad-vpre-size ");
	}

	for (int i = 0; i < vAccountRegId.size(); i++) {
		if (!CheckSignScript(boost::get<CRegID>(vAccountRegId[i]), SignatureHash(), vSignature[i], state, view)) {
			return state.DoS(100, ERROR("CheckTransaction() :CheckSignScript failed"), REJECT_INVALID,
					"bad-signscript-check");
		}
	}

	//for VerifyDB checkblock return true
	if (pTxCacheTip->IsContainTx(GetHash())) {
		return true;
	}

	CVmScriptRun vmRun;
	std::shared_ptr<CBaseTransaction> pTx = GetNewInstance();

	CScriptDBViewCache scriptDBCache(*pScriptDBTip, true);
	uint64_t el = GetElementForBurn(chainActive.Tip());
	tuple<bool, uint64_t, string> ret = vmRun.run(pTx, view, scriptDBCache, chainActive.Height() +1, el);

	if (!std::get<0>(ret))
		return state.DoS(100,
				ERROR("CheckTransaction() : ContractTransaction txhash=%s run script error,%s",
						GetHash().GetHex(), std::get<2>(ret)), UPDATE_ACCOUNT_FAIL, "run-script-error");
	return true;
}

bool CFreezeTransaction::UpdateAccount(int nIndex, CAccountViewCache &view, CValidationState &state, CTxUndo &txundo,
		int nHeight, CTransactionDBCache &txCache, CScriptDBViewCache &scriptCache) {
	uint64_t minusValue = llFees + llFreezeFunds;
	uint64_t freezeValue = llFreezeFunds;
	CID id(regAccountId);
	CAccount secureAccount;
	if (!view.GetAccount(regAccountId, secureAccount)) {
		return state.DoS(100, ERROR("UpdateAccounts() : read source addr %s account info error", HexStr(id.GetID())),
				UPDATE_ACCOUNT_FAIL, "bad-read-accountdb");
	}
	secureAccount.CompactAccount(nHeight - 1);
	CFund minusFund(minusValue);
	if (!secureAccount.OperateAccount(MINUS_FREE, minusFund))
		return state.DoS(100, ERROR("UpdateAccounts() : secure accounts insufficient funds"), UPDATE_ACCOUNT_FAIL,
				"bad-read-accountdb");
	CFund selfFund(SELF_FREEZD_FUND,freezeValue, nUnfreezeHeight);
	if (!secureAccount.OperateAccount(ADD_SELF_FREEZD, selfFund))
		return state.DoS(100, ERROR("UpdateAccounts() : secure accounts insufficient funds"), UPDATE_ACCOUNT_FAIL,
				"bad-read-accountdb");
	CUserID userid = secureAccount.keyID;
	if (!view.SetAccount(userid, secureAccount))
		return state.DoS(100, ERROR("UpdateAccounts() : batch write secure account info error"), UPDATE_ACCOUNT_FAIL,
				"bad-read-accountdb");
	txundo.vAccountOperLog.push_back(secureAccount.accountOperLog);
	txundo.txHash = GetHash();
	return true;
}
bool CFreezeTransaction::UndoUpdateAccount(int nIndex, CAccountViewCache &view, CValidationState &state,
		CTxUndo &txundo, int nHeight, CTransactionDBCache &txCache, CScriptDBViewCache &scriptCache) {
	CID id(regAccountId);
	CAccount account;
	if (!view.GetAccount(regAccountId, account))
		return state.DoS(100, ERROR("UpdateAccounts() : read source addr %s account info error", HexStr(id.GetID())),
				UPDATE_ACCOUNT_FAIL, "bad-read-accountdb");
	CAccountOperLog accountOperLog;
	if (!txundo.GetAccountOperLog(account.keyID, accountOperLog))
		return state.DoS(100, ERROR("UpdateAccounts() : read keyid=%s undo info error", account.keyID.GetHex()),
				UPDATE_ACCOUNT_FAIL, "bad-read-txundoinfo");
	account.UndoOperateAccount(accountOperLog);
	CUserID userId = account.keyID;
	if (!view.SetAccount(userId, account))
		return state.DoS(100, ERROR("UpdateAccounts() : write secure account info error"), UPDATE_ACCOUNT_FAIL,
				"bad-read-accountdb");
	return true;
}
bool CFreezeTransaction::GetAddress(set<CKeyID> &vAddr, CAccountViewCache &view) {
	CKeyID keyId;
	if (!view.GetKeyId(regAccountId, keyId))
		return false;
	vAddr.insert(keyId);
	return true;
}
bool CFreezeTransaction::IsValidHeight(int nCurHeight, int nTxCacheHeight) const {
	if (nValidHeight > nCurHeight + nTxCacheHeight / 2)
		return false;
	if (nValidHeight < nCurHeight - nTxCacheHeight / 2)
		return false;
	return true;
}
string CFreezeTransaction::ToString(CAccountViewCache &view) const {
	string str;
	CKeyID keyId;
	view.GetKeyId(regAccountId, keyId);
	str += strprintf("txType=%s, hash=%s, ver=%d, accountId=%s, llFees=%ld, keyid=%s, llFreezeFunds=%ld, nValidHeight=%ld, nUnfreezeHeight=%d\n",
	txTypeArray[nTxType], GetHash().ToString().c_str(), nVersion, boost::get<CRegID>(regAccountId).ToString(), llFees, keyId.GetHex(), llFreezeFunds, nValidHeight, nUnfreezeHeight);
	return str;
}
bool CFreezeTransaction::CheckTransction(CValidationState &state, CAccountViewCache &view) {
	if (!MoneyRange(llFees)) {
		return state.DoS(100, ERROR("CheckTransaction() : freeze tx fee out of range"), REJECT_INVALID,
				"bad-freezetx-fee-toolarge");
	}

	if (!MoneyRange(llFreezeFunds)) {
		return state.DoS(100, ERROR("CheckTransaction(): freeze tx value our of range"), REJECT_INVALID,
				"bad-freezetx-value-toolarge");
	}

	if (!CheckSignScript(boost::get<CRegID>(regAccountId), SignatureHash(), signature, state, view)) {
		return state.DoS(100, ERROR("CheckTransaction() :CheckSignScript failed"), REJECT_INVALID,
				"bad-signscript-check");
	}
	return true;
}

bool CRewardTransaction::UpdateAccount(int nIndex, CAccountViewCache &view, CValidationState &state, CTxUndo &txundo,
		int nHeight, CTransactionDBCache &txCache, CScriptDBViewCache &scriptCache) {
	CID id(account);
	CAccount acctInfo;
	if (!view.GetAccount(account, acctInfo)) {
		return state.DoS(100, ERROR("UpdateAccounts() : read source addr %s account info error", HexStr(id.GetID())),
				UPDATE_ACCOUNT_FAIL, "bad-read-accountdb");
	}
//	LogPrint("INFO", "before rewardtx confirm account:%s\n", acctInfo.ToString());
	acctInfo.ClearAccPos(GetHash(), nHeight - 1, SysCfg().GetIntervalPos());
	CFund fund(REWARD_FUND,rewardValue, nHeight);
	acctInfo.OperateAccount(ADD_FREE, fund);
//	LogPrint("INFO", "after rewardtx confirm account:%s\n", acctInfo.ToString());
	CUserID userId = acctInfo.keyID;
	if (!view.SetAccount(userId, acctInfo))
		return state.DoS(100, ERROR("UpdateAccounts() : write secure account info error"), UPDATE_ACCOUNT_FAIL,
				"bad-save-accountdb");
	txundo.vAccountOperLog.push_back(acctInfo.accountOperLog);
	txundo.txHash = GetHash();
	return true;
}
bool CRewardTransaction::UndoUpdateAccount(int nIndex, CAccountViewCache &view, CValidationState &state,
		CTxUndo &txundo, int nHeight, CTransactionDBCache &txCache, CScriptDBViewCache &scriptCache) {
	CID id(account);
	if (account.type() != typeid(CRegID) && account.type() != typeid(CPubKey)) {
		return state.DoS(100,
				ERROR("UpdateAccounts() : account  %s error, either accountId, or pubkey",
						HexStr(id.GetID())), UPDATE_ACCOUNT_FAIL, "bad-read-accountdb");
	}
	CAccount acctInfo;
	if (!view.GetAccount(account, acctInfo)) {
		return state.DoS(100, ERROR("UpdateAccounts() : read source addr %s account info error", HexStr(id.GetID())),
				UPDATE_ACCOUNT_FAIL, "bad-read-accountdb");
	}
	CAccountOperLog accountOperLog;
	if (!txundo.GetAccountOperLog(acctInfo.keyID, accountOperLog))
		return state.DoS(100, ERROR("UpdateAccounts() : read keyid=%s undo info error", acctInfo.keyID.GetHex()),
				UPDATE_ACCOUNT_FAIL, "bad-read-txundoinfo");
	acctInfo.UndoOperateAccount(accountOperLog);
	CUserID userId = acctInfo.keyID;
	if (!view.SetAccount(userId, acctInfo))
		return state.DoS(100, ERROR("UpdateAccounts() : write secure account info error"), UPDATE_ACCOUNT_FAIL,
				"bad-read-accountdb");
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
	CID id(account);
	str += strprintf("txType=%s, hash=%s, ver=%d, account=%s, keyid=%s, rewardValue=%ld\n", txTypeArray[nTxType], GetHash().ToString().c_str(), nVersion, HexStr(id.GetID()).c_str(), keyId.GetHex(), rewardValue);
	return str;
}
bool CRewardTransaction::CheckTransction(CValidationState &state, CAccountViewCache &view) {
	return true;
}

bool CRegistScriptTx::UpdateAccount(int nIndex, CAccountViewCache &view, CValidationState &state, CTxUndo &txundo,
		int nHeight, CTransactionDBCache &txCache, CScriptDBViewCache &scriptCache) {
	LogPrint("INFO" ,"registscript UpdateAccount\n");
	CID id(regAccountId);
	CAccount acctInfo;
	if (!view.GetAccount(regAccountId, acctInfo)) {
		return state.DoS(100, ERROR("UpdateAccounts() : read regist addr %s account info error", HexStr(id.GetID())),
				UPDATE_ACCOUNT_FAIL, "bad-read-accountdb");
	}

	uint64_t minusValue = llFees;
	if (minusValue > 0) {
		CFund fund(minusValue);
		acctInfo.OperateAccount(MINUS_FREE, fund);
		txundo.vAccountOperLog.push_back(acctInfo.accountOperLog);
	}
	txundo.txHash = GetHash();

	if(script.size() == SCRIPT_ID_SIZE) {
		vector<unsigned char> vScript;
		CRegID regId(script);
		if (!scriptCache.GetScript(regId, vScript)) {
			return state.DoS(100,
					ERROR("UpdateAccounts() : Get script id=%s error", HexStr(script.begin(), script.end())),
					UPDATE_ACCOUNT_FAIL, "bad-query-scriptdb");
		}
		if (!aAuthorizate.IsNull()) {
			acctInfo.mapAuthorizate[regId.GetVec6()] = aAuthorizate;
		}
	}
	else {
		CVmScript vmScript;
		CDataStream stream(script, SER_DISK, CLIENT_VERSION);
		try {
			stream >> vmScript;
		} catch (exception& e) {
			return state.DoS(100, ERROR(("UpdateAccounts() :intial() Unserialize to vmScript error:" + string(e.what())).c_str()),
					UPDATE_ACCOUNT_FAIL, "bad-query-scriptdb");
		}
		if(!vmScript.IsValid())
			return state.DoS(100, ERROR("UpdateAccounts() : vmScript invalid"), UPDATE_ACCOUNT_FAIL, "bad-query-scriptdb");
		if (0 == nIndex)
		return true;

		CRegID regId(nHeight, nIndex);
		//create script account
		CKeyID keyId = Hash160(regId.GetVec6());
		CAccount account;
		account.keyID = keyId;
		account.regID = regId;
		//save new script content
		if(!scriptCache.SetScript(regId, script)){
			return state.DoS(100,
					ERROR("UpdateAccounts() : save script id %s script info error", regId.ToString()),
					UPDATE_ACCOUNT_FAIL, "bad-save-scriptdb");
		}
		if (!view.SaveAccountInfo(regId, keyId, account)) {
			return state.DoS(100,
					ERROR("UpdateAccounts() : create new account script id %s script info error",
							regId.ToString()), UPDATE_ACCOUNT_FAIL, "bad-save-scriptdb");
		}
		if (!aAuthorizate.IsNull()) {
			acctInfo.mapAuthorizate[regId.GetVec6()] = aAuthorizate;
		}
	}

	CUserID userId = acctInfo.keyID;
	if (!view.SetAccount(userId, acctInfo))
		return state.DoS(100, ERROR("UpdateAccounts() : write secure account info error"), UPDATE_ACCOUNT_FAIL,
				"bad-save-accountdb");
	return true;
}
bool CRegistScriptTx::UndoUpdateAccount(int nIndex, CAccountViewCache &view, CValidationState &state, CTxUndo &txundo,
		int nHeight, CTransactionDBCache &txCache, CScriptDBViewCache &scriptCache) {
	CID id(regAccountId);
	CAccount account;
	CUserID userId;
	if (!view.GetAccount(regAccountId, account)) {
		return state.DoS(100, ERROR("UpdateAccounts() : read regist addr %s account info error", HexStr(id.GetID())),
				UPDATE_ACCOUNT_FAIL, "bad-read-accountdb");
	}

	if(script.size() != 6) {

		CRegID scriptId(nHeight, nIndex);
		//delete script content
		if (!scriptCache.EraseScript(scriptId)) {
			return state.DoS(100, ERROR("UpdateAccounts() : erase script id %s error", scriptId.ToString()),
					UPDATE_ACCOUNT_FAIL, "bad-save-scriptdb");
		}
		//delete account
		if(!view.EraseId(scriptId)){
			return state.DoS(100, ERROR("UpdateAccounts() : erase script account %s error", scriptId.ToString()),
								UPDATE_ACCOUNT_FAIL, "bad-save-scriptdb");
		}
		CKeyID keyId = Hash160(scriptId.GetVec6());
		userId = keyId;
		if(!view.EraseAccount(userId)){
			return state.DoS(100, ERROR("UpdateAccounts() : erase script account %s error", scriptId.ToString()),
								UPDATE_ACCOUNT_FAIL, "bad-save-scriptdb");
		}
	}
	CAccountOperLog accountOperLog;
	if (!txundo.GetAccountOperLog(account.keyID, accountOperLog))
		return state.DoS(100, ERROR("UpdateAccounts() : read keyid=%s undo info error", account.keyID.GetHex()),
				UPDATE_ACCOUNT_FAIL, "bad-read-txundoinfo");
	account.UndoOperateAccount(accountOperLog);
	userId = account.keyID;
	if (!view.SetAccount(userId, account))
		return state.DoS(100, ERROR("UpdateAccounts() : write secure account info error"), UPDATE_ACCOUNT_FAIL,
				"bad-save-accountdb");
	return true;
}
bool CRegistScriptTx::GetAddress(set<CKeyID> &vAddr, CAccountViewCache &view) {
	CKeyID keyId;
	if (!view.GetKeyId(regAccountId, keyId))
		return false;
	vAddr.insert(keyId);
	return true;
}
bool CRegistScriptTx::IsValidHeight(int nCurHeight, int nTxCacheHeight) const {
	if (nValidHeight > nCurHeight + nTxCacheHeight / 2)
		return false;
	if (nValidHeight < nCurHeight - nTxCacheHeight / 2)
		return false;
	return true;
}
string CRegistScriptTx::ToString(CAccountViewCache &view) const {
	string str;
	CKeyID keyId;
	view.GetKeyId(regAccountId, keyId);
	str += strprintf("txType=%s, hash=%s, ver=%d, accountId=%s, keyid=%s, llFees=%ld, nValidHeight=%d\n",
	txTypeArray[nTxType], GetHash().ToString().c_str(), nVersion,boost::get<CRegID>(regAccountId).ToString(), keyId.GetHex(), llFees, nValidHeight);
	return str;
}
bool CRegistScriptTx::CheckTransction(CValidationState &state, CAccountViewCache &view) {
	CAccount  account;
	if(!view.GetAccount(regAccountId, account)) {
		return state.DoS(100, ERROR("CheckTransaction() : register script tx get registe account info error"), REJECT_INVALID,
				"bad-read-account-info");
	}
	if(script.size() == SCRIPT_ID_SIZE) {
		vector<unsigned char> vScriptContent;
		CRegID retId(script);
		if(!pScriptDBTip->GetScript(retId, vScriptContent)) {
			return state.DoS(100,
					ERROR("CheckTransaction() : register script tx get exit script content by script reg id:%s error",
							HexStr(script.begin(), script.end())), REJECT_INVALID, "bad-read-script-info");
		}
	}

	if (!MoneyRange(llFees)) {
			return state.DoS(100, ERROR("CheckTransaction() : register script tx fee out of range"), REJECT_INVALID,
					"bad-register-script-fee-toolarge");
	}

	uint256 signhash = SignatureHash();
	if (!CheckSignScript(boost::get<CRegID>(regAccountId), signhash, signature, state, view)) {
		return state.DoS(100, ERROR("CheckTransaction() :CheckSignScript failed"), REJECT_INVALID,
				"bad-signscript-check");
	}

	if (!aAuthorizate.IsValid())
		return state.DoS(100, ERROR("CheckTransaction() : Authorizate data invalid"), REJECT_INVALID, "bad-signcript-check");

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
	case FREEZD_FUND:
	case SELF_FREEZD_FUND:
		if (nCurHeight >= nHeight) {
			nMergeType = FREEDOM_FUND;  // Merget to Freedom;
			return true;
		}
		break;
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
	obj.push_back(Pair("scriptID",  (scriptID.size()==6 ? CRegID(scriptID).ToString() : string(" "))));
	obj.push_back(Pair("value",     value));
	obj.push_back(Pair("timeout hight",     nHeight));
	return obj;
}

string CFund::ToString() const {

	return  write_string(Value(ToJosnObj()),true);
}

string COperFund::ToString() const {
	string str("");
	string strOperType[] = { "NULL_OPER_TYPE", "ADD_FUND", "MINUS_FUND" };
	str += strprintf("        list funds: operType=%s\n", strOperType[operType]);
	vector<CFund>::const_iterator iterFund = vFund.begin();
//	LogPrint("INFO", "        list funds: operType=%s\n", strOperType[operType]);
	for (; iterFund != vFund.end(); ++iterFund)
		str += iterFund->ToString();
	return str;
}

string CAuthorizateLog::ToString() const {
	string str("");
	str += strprintf("bvalid is %d,LastOperHeight is %d,lastCurMoney is %d,lastMaxTotalMoney is %d,scriptID is %s \n"
	, bValid,nLastOperHeight,nLastCurMaxMoneyPerDay,nLastMaxMoneyTotal,HexStr(scriptID));

	return str;
}
string CAccountOperLog::ToString() const {
	string str("");
	str += strprintf("    list oper funds: keyId=%d\n",keyID.GetHex());
	vector<COperFund>::const_iterator iterOperFund = vOperFund.begin();
//	LogPrint("INFO", "    list oper funds: keyId=%d\n", keyID.GetHex());
	for (; iterOperFund != vOperFund.end(); ++iterOperFund)
		str += iterOperFund->ToString();
	return str;
}

string CTxUndo::ToString() const {
	vector<CAccountOperLog>::const_iterator iterLog = vAccountOperLog.begin();
	string str("  list account oper Log:\n");
	for (; iterLog != vAccountOperLog.end(); ++iterLog) {
		str += iterLog->ToString();
	}
	return str;
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

void CAccount::CompactAccount(int nCurHeight) {
	MergerFund(vRewardFund, nCurHeight);
	MergerFund(vFreeze, nCurHeight);
	MergerFund(vSelfFreeze, nCurHeight);
	MergerFund(vFreedomFund, nCurHeight);
}

void CAccount::MergerFund(vector<CFund> &vFund, int nCurHeight) {
	stable_sort(vFund.begin(), vFund.end(), greater<CFund>());
	uint64_t value = 0;
	vector<CFund> vMinusFunds;
	vector<CFund> vAddFunds;
	vector<CFund>::reverse_iterator iterFund = vFund.rbegin();
	bool bMergeFund = false;
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
}

void CAccount::WriteOperLog(const COperFund &operLog) {
	for(auto item:operLog.vFund)
	{
		LogPrint("key","keyid:%s\n",HexStr(keyID).c_str());
		LogPrint("key"," type is:%d,fund:%s\n",static_cast<int>(operLog.operType),item.ToString().c_str());
//		string s("9f2faa80029ee70f87819c283f5e96aa0e83d421");
//		if (keyID == uint160(s) )
//		cout<<"height is "<<item.nHeight<<
//				" type is: "<<static_cast<int>(operLog.operType)<<" value is: "<<item.value<<
//				" fund type is: "<<static_cast<int>(item.nFundType) <<" scriptID is: "<<HexStr(item.scriptID).c_str()<<endl;
	}

	accountOperLog.InsertOperateLog(operLog);
}

void CAccount::AddToSelfFreeze(const CFund &fund, bool bWriteLog) {
	bool bMerge = false;
	for (auto& item : vSelfFreeze) {
		if (item.nHeight == fund.nHeight) {
			item.value += fund.value;
			bMerge = true;
			break;
		}
	}

	if (!bMerge)
		vSelfFreeze.push_back(fund);

	if (bWriteLog)
		WriteOperLog(ADD_FUND, fund);
}

void CAccount::AddToFreeze(const CFund &fund, bool bWriteLog) {
	bool bMerge = false;
	for (auto& item : vFreeze) {
		if (item.scriptID == fund.scriptID && item.nHeight == fund.nHeight) {
			item.value += fund.value;
			bMerge = true;
			break;
		}
	}

	if (!bMerge)
		vFreeze.push_back(fund);

	if (bWriteLog)
		WriteOperLog(ADD_FUND, fund);
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
		//MergerFund(vFreedomFund, nCurHeight);
	} else {
		CFund addFund(*it);
		it->value += fund.value;
		addFund.value = fund.value;
		if (bWriteLog)
			WriteOperLog(ADD_FUND, addFund);
	}
}

bool CAccount::MinusFree(const CFund &fund,bool bAuthorizated) {
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

		COperFund operLog(MINUS_FUND, vOperFund,bAuthorizated);
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
		COperFund operLog(MINUS_FUND, vOperFund,bAuthorizated);
		WriteOperLog(operLog);

		return true;
	}

}

bool CAccount::UndoOperateAccount(const CAccountOperLog & accountOperLog) {
	bool bOverDay = false;
	if (accountOperLog.authorLog.IsLogValid())
		bOverDay = true;
//	LogPrint("vm","befor undo %s\n",ToString().c_str());
	vector<COperFund>::const_reverse_iterator iterOperFundLog = accountOperLog.vOperFund.rbegin();
	for (; iterOperFundLog != accountOperLog.vOperFund.rend(); ++iterOperFundLog) {
//		LogPrint("INFO", "undo account:%s\n" ,iterOperFundLog->ToString());
		vector<CFund>::const_iterator iterFund = iterOperFundLog->vFund.begin();
		for (; iterFund != iterOperFundLog->vFund.end(); ++iterFund) {
		//	LogPrint("vm","fund_type is %d,oper_type is %d,fund info:%s",iterFund->nFundType,iterOperFundLog->operType,iterFund->ToString().c_str());
			switch (iterFund->nFundType) {
			case FREEDOM:
				if (ADD_FUND == iterOperFundLog->operType) {
					assert(llValues >= iterFund->value);
					llValues -= iterFund->value;
				} else if (MINUS_FUND == iterOperFundLog->operType) {
					llValues += iterFund->value;
					if (iterOperFundLog->bAuthorizated && !bOverDay)
						UndoAuthorityOnDay(iterFund->value, accountOperLog.authorLog);
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
								fundInVector.value>=iterFund->value)
						return true;
					});
					assert(it != vFreedomFund.end());
					it->value -= iterFund->value;
					if (!it->value)
						vFreedomFund.erase(it);
				} else if (MINUS_FUND == iterOperFundLog->operType) {
					AddToFreedom(*iterFund, false);
					if (iterOperFundLog->bAuthorizated && !bOverDay)
						UndoAuthorityOnDay(iterFund->value, accountOperLog.authorLog);
				}

				break;
			case FREEZD_FUND:
				if (ADD_FUND == iterOperFundLog->operType) {
					auto it = find_if(vFreeze.begin(), vFreeze.end(), [&](const CFund& fundInVector) {
						if (fundInVector.nFundType== iterFund->nFundType &&
								fundInVector.nHeight == iterFund->nHeight&&
								fundInVector.scriptID == iterFund->scriptID &&
								fundInVector.value>=iterFund->value)
						return true;
					});

					assert(it != vFreeze.end());
					it->value -= iterFund->value;
					if (!it->value)
						vFreeze.erase(it);
				} else if (MINUS_FUND == iterOperFundLog->operType) {
					AddToFreeze(*iterFund, false);
				}

				break;
			case SELF_FREEZD_FUND:
				if (ADD_FUND == iterOperFundLog->operType) {
					auto it = find_if(vSelfFreeze.begin(), vSelfFreeze.end(), [&](const CFund& fundInVector) {
						if (fundInVector.nFundType== iterFund->nFundType &&
								fundInVector.nHeight == iterFund->nHeight&&
								fundInVector.value>=iterFund->value)
						return true;
					});

					assert(it != vSelfFreeze.end());
					it->value -= iterFund->value;
					if (!it->value)
						vSelfFreeze.erase(it);
				} else if (MINUS_FUND == iterOperFundLog->operType) {
					AddToSelfFreeze(*iterFund, false);
					if (iterOperFundLog->bAuthorizated && !bOverDay)
						UndoAuthorityOnDay(iterFund->value, accountOperLog.authorLog);
				}

				break;
			default:
				assert(0);
				return false;
			}
		}
	}

	if (bOverDay) {
		UndoAuthorityOverDay(accountOperLog.authorLog);
	}

//	LogPrint("vm","after undo %s\n",ToString().c_str());
	return true;
}

//caculate pos
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
			CFund fund(FREEDOM, llValues, 0);
			acclog.vFund.push_back(fund);
			llValues = 0;
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

//caculate pos
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

uint64_t CAccount::GetSripteFreezeAmount(int nCurHeight) {
	CompactAccount(nCurHeight);
	uint64_t balance = 0;

	for (auto &fund : vFreeze) {
		balance += fund.value;
	}

	return balance;
}
uint64_t CAccount::GetSelfFreezeAmount(int nCurHeight) {
	CompactAccount(nCurHeight);
	uint64_t balance = 0;


	for (auto &fund : vSelfFreeze) {
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
		vMerkleTree.push_back(uint256(freeFund.scriptID));
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
	static const string fundTypeArray[] = { "NULL_FUNDTYPE", "FREEDOM", "REWARD_FUND", "FREEDOM_FUND", "IN_FREEZD_FUND",
			"OUT_FREEZD_FUND", "SELF_FREEZD_FUND" };
	obj.push_back(Pair("Address",     keyID.ToAddress()));
	obj.push_back(Pair("KeyID",     keyID.ToString()));
	obj.push_back(Pair("RegID",     regID.ToString()));
	obj.push_back(Pair("PublicKey",  PublicKey.ToString()));
	obj.push_back(Pair("MinerPKey",  MinerPKey.ToString()));
	obj.push_back(Pair("FreeValues",     llValues));

	Array RewardFund;
	for (auto& rew:vRewardFund) {
		RewardFund.push_back(rew.ToJosnObj());
	}
	obj.push_back(Pair("RewardFund",     RewardFund));


	Array FreedomFund;
	for (auto& rew:vFreedomFund) {
		FreedomFund.push_back(rew.ToJosnObj());
	}
	obj.push_back(Pair("FreedomFund",     FreedomFund));

	Array Freeze;
	for (auto& rew:vFreeze) {
		Freeze.push_back(rew.ToJosnObj());
	}
	obj.push_back(Pair("Freeze",     Freeze));

	Array SelfFreeze;
	for (auto& rew:vSelfFreeze) {
		SelfFreeze.push_back(rew.ToJosnObj());
	}
	obj.push_back(Pair("SelfFreeze",     SelfFreeze));
	Array listAuthorize;
	Object authorizateObj;
	for(auto & item : mapAuthorizate) {
		authorizateObj.push_back(Pair(HexStr(item.first), item.second.ToJosnObj()));
		listAuthorize.push_back(authorizateObj);
	}
	obj.push_back(Pair("Authorizates", listAuthorize));
	return obj;
}

string CAccount::ToString() const {
	return  write_string(Value(ToJosnObj()),true);
}

void CAccount::WriteOperLog(AccountOper emOperType, const CFund &fund, bool bAuthorizated) {
	vector<CFund> vFund;
	vFund.push_back(fund);
	COperFund operLog(emOperType, vFund, bAuthorizated);
	WriteOperLog(operLog);
}

bool CAccount::MinusFreezed(const CFund& fund) {
	vector<CFund>::iterator it = vFreeze.begin();
	for (; it != vFreeze.end(); it++) {
		if (it->scriptID == fund.scriptID && it->nHeight == fund.nHeight) {
			break;
		}
	}

	if (it == vFreeze.end()) {
		return false;
	}

	if (fund.value > it->value) {
		return false;
	} else {

		if (it->value > fund.value) {
			CFund logfund(*it);
			logfund.value = fund.value;
			it->value -= fund.value;
			WriteOperLog(MINUS_FUND, logfund);
		} else {
			WriteOperLog(MINUS_FUND, *it);
			vFreeze.erase(it);
		}
		return true;
	}
}

bool CAccount::MinusSelf(const CFund &fund,bool bAuthorizated) {
	vector<CFund> vOperFund;
	uint64_t nCandidateValue = 0;
	vector<CFund>::iterator iterFound = vSelfFreeze.begin();
	if (!vSelfFreeze.empty()) {
		for (; iterFound != vSelfFreeze.end(); ++iterFound) {
			nCandidateValue += iterFound->value;
			if (nCandidateValue >= fund.value) {
				break;
			}
		}
	}

	if (iterFound != vSelfFreeze.end()) {
		uint64_t remainValue = nCandidateValue - fund.value;
		if (remainValue > 0) {
			vOperFund.insert(vOperFund.end(), vSelfFreeze.begin(), iterFound);
			CFund fundMinus(*iterFound);
			fundMinus.value = iterFound->value - remainValue;
			iterFound->value = remainValue;
			vOperFund.push_back(fundMinus);
			vSelfFreeze.erase(vSelfFreeze.begin(), iterFound);
		} else {
			vOperFund.insert(vOperFund.end(), vSelfFreeze.begin(), iterFound + 1);
			vSelfFreeze.erase(vSelfFreeze.begin(), iterFound + 1);
		}

		COperFund operLog(MINUS_FUND, vOperFund,bAuthorizated);
		WriteOperLog(operLog);
		return true;
	} else {
		return false;
	}
}

bool CAccount::IsMoneyOverflow(uint64_t nAddMoney) {
	if (!MoneyRange(nAddMoney))
		return false;

	uint64_t nTotalMoney = 0;
	nTotalMoney = GetVecMoney(vFreedomFund)+GetVecMoney(vRewardFund)+GetVecMoney(vFreeze)\
			+GetVecMoney(vSelfFreeze)+llValues+nAddMoney;
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
		if (it->scriptID == scriptID) {
			fund = *it;
			return true;
		}
	}
	return false;
}

bool CAccount::IsAuthorized(uint64_t nMoney, int nHeight, const vector_unsigned_char& scriptID) {
	vector<unsigned char> vscript;
	CRegID regId(scriptID);
	if (NULL == pScriptDBTip || !pScriptDBTip->GetScript(regId, vscript))
		return false;

	auto it = mapAuthorizate.find(scriptID);
	if (it == mapAuthorizate.end())
		return false;

	CAuthorizate& authorizate = it->second;
	if (authorizate.GetAuthorizeTime() < nHeight || authorizate.GetLastOperHeight() >nHeight)
		return false;

	//amount of blocks that connected into chain per day
	const uint64_t nBlocksPerDay = 24 * 60 * 60 / SysCfg().GetTargetSpacing();
	if (authorizate.GetLastOperHeight() / nBlocksPerDay == nHeight / nBlocksPerDay) {
		if (authorizate.GetCurMaxMoneyPerDay() < nMoney)
			return false;
	} else {
		if (authorizate.GetMaxMoneyPerDay() < nMoney)
			return false;
	}

	if (authorizate.GetMaxMoneyPerTime() < nMoney || authorizate.GetMaxMoneyTotal() < nMoney)
		return false;

	return true;
}

bool CAccount::IsFundValid(OperType type, const CFund &fund, int nHeight, const vector_unsigned_char* pscriptID,
		bool bCheckAuthorized) {
	switch (type) {
	case ADD_FREE: {
		if (REWARD_FUND != fund.nFundType && FREEDOM_FUND != fund.nFundType)
			return false;
		if (!IsMoneyOverflow(fund.value))
			return false;
		break;
	}

	case ADD_SELF_FREEZD: {
		if (SELF_FREEZD_FUND != fund.nFundType)
			return false;
		if (!IsMoneyOverflow(fund.value))
			return false;
		break;
	}

	case ADD_FREEZD: {
		if (FREEZD_FUND != fund.nFundType)
			return false;
		if (!IsMoneyOverflow(fund.value))
			return false;
		break;
	}

	case MINUS_FREEZD: {
		assert(pScriptDBTip);
		vector<unsigned char> vscript;
		CRegID regId(fund.scriptID);
		if (!pScriptDBTip->GetScript(regId, vscript))
			return false;
		break;
	}

	case MINUS_FREE:
	case MINUS_SELF_FREEZD: {
		if (bCheckAuthorized && pscriptID) {
			if (!IsAuthorized(fund.value, nHeight, *pscriptID))
				return false;

			if (accountOperLog.authorLog.GetScriptID() != *pscriptID)
				accountOperLog.authorLog.SetScriptID(*pscriptID);

			if (0 == accountOperLog.authorLog.GetLastOperHeight()) {

			}
		}
		break;
	}

	default:
		assert(0);
		return false;
	}

	return true;
}

bool CAccount::OperateAccount(OperType type, const CFund &fund, int nHeight,
		const vector_unsigned_char* pscriptID,
		bool bCheckAuthorized) {
	assert(keyID != uint160(0));
	if (keyID != accountOperLog.keyID)
		accountOperLog.keyID = keyID;

	if (!IsFundValid(type, fund, nHeight, pscriptID, bCheckAuthorized))
		return false;

	if (!fund.value)
		return true;

	bool bRet = true;
	uint64_t nOperateValue = 0;
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
		bRet = MinusFree(fund, bCheckAuthorized);
		break;
	}

	case ADD_SELF_FREEZD: {
		AddToSelfFreeze(fund);
		break;
	}

	case MINUS_SELF_FREEZD: {
		bRet = MinusSelf(fund, bCheckAuthorized);
		break;
	}

	case ADD_FREEZD: {
		AddToFreeze(fund);
		break;
	}

	case MINUS_FREEZD: {
		bRet = MinusFreezed(fund);
		break;
	}

	default:
		assert(0);
	}

	if ((MINUS_FREE == type || MINUS_SELF_FREEZD == type) && bCheckAuthorized && bRet)
		UpdateAuthority(nHeight, fund.value, *pscriptID);

	return bRet;
}

void CAccount::UpdateAuthority(int nHeight, uint64_t nMoney, const vector_unsigned_char& scriptID) {
	map<vector_unsigned_char, CAuthorizate>::iterator it = mapAuthorizate.find(scriptID);
	if (it == mapAuthorizate.end()) {
		assert(it != mapAuthorizate.end());
		return;
	}

	//save last operating height and scriptID
	CAuthorizate& authorizate = it->second;
	if (accountOperLog.authorLog.GetScriptID() != scriptID)
		accountOperLog.authorLog.SetScriptID(scriptID);

	if (0 == accountOperLog.authorLog.GetLastOperHeight()) {
		accountOperLog.authorLog.SetLastOperHeight(authorizate.GetLastOperHeight());
	}

	//update authority after current operate
	const uint64_t nBlocksPerDay = 24 * 60 * 60 / SysCfg().GetTargetSpacing();
	if (authorizate.GetLastOperHeight() / nBlocksPerDay < nHeight / nBlocksPerDay) {
		CAuthorizateLog log(authorizate.GetLastOperHeight(), authorizate.GetCurMaxMoneyPerDay(),
				authorizate.GetMaxMoneyTotal(), true, scriptID);
		accountOperLog.InsertAuthorLog(log);
		authorizate.SetCurMaxMoneyPerDay(authorizate.GetMaxMoneyPerDay());
	}

	uint64_t nCurMaxMoneyPerDay = authorizate.GetCurMaxMoneyPerDay();
	uint64_t nMaxMoneyTotal = authorizate.GetMaxMoneyTotal();
	assert(nCurMaxMoneyPerDay >= nMoney && nMaxMoneyTotal >= nMoney);
	authorizate.SetCurMaxMoneyPerDay(nCurMaxMoneyPerDay - nMoney);
	authorizate.SetMaxMoneyTotal(nMaxMoneyTotal - nMoney);
	authorizate.SetLastOperHeight(static_cast<uint32_t>(nHeight));
}

void CAccount::UndoAuthorityOverDay(const CAuthorizateLog& log) {
	auto it = mapAuthorizate.find(log.GetScriptID());
	if (it == mapAuthorizate.end()) {
		assert(it != mapAuthorizate.end());
		return;
	}

	CAuthorizate& authorizate = it->second;
	authorizate.SetMaxMoneyTotal(log.GetLastMaxMoneyTotal());
	authorizate.SetCurMaxMoneyPerDay(log.GetLastCurMaxMoneyPerDay());
	authorizate.SetLastOperHeight(log.GetLastOperHeight());
}

void CAccount::UndoAuthorityOnDay(uint64_t nUndoMoney, const CAuthorizateLog& log) {
	auto it = mapAuthorizate.find(log.GetScriptID());
	if (it == mapAuthorizate.end()) {
		assert(it != mapAuthorizate.end());
		return;
	}

	CAuthorizate& authorizate = it->second;
	uint64_t nCurMaxMoneyPerDay = authorizate.GetCurMaxMoneyPerDay();
	uint64_t nNewMaxMoneyPerDay = nCurMaxMoneyPerDay + nUndoMoney;
	uint64_t nMaxMoneyTotal = authorizate.GetMaxMoneyTotal();
	uint64_t nNewMaxMoneyTotal = nMaxMoneyTotal + nUndoMoney;

	authorizate.SetCurMaxMoneyPerDay(nNewMaxMoneyPerDay);
	authorizate.SetMaxMoneyTotal(nNewMaxMoneyTotal);
	authorizate.SetLastOperHeight(accountOperLog.authorLog.GetLastOperHeight());
}

bool CAccount::GetUserData(const vector_unsigned_char& scriptID, vector<unsigned char> & vData) {
	vector<unsigned char> vscript;
	CRegID regId(scriptID);
	if (NULL == pScriptDBTip || !pScriptDBTip->GetScript(regId, vscript))
		return false;

	auto it = mapAuthorizate.find(scriptID);
	if (it == mapAuthorizate.end())
		return false;

	vData = mapAuthorizate[scriptID].GetUserData();
	return true;
}


void CRegID::SetRegID(const vector<unsigned char>& vIn) {
	assert(vIn.size() == 6);
	vRegID = vIn;
	CDataStream ds(vIn, SER_DISK, CLIENT_VERSION);
	ds >> nHeight;
	ds >> nIndex;
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

bool CRegID::clean()  {
	nHeight = 0 ;
	nIndex = 0 ;
	vRegID.clear();
	return true;
}

CNetAuthorizate::CNetAuthorizate(uint32_t nauthorizetime, vector<unsigned char> nuserdefine, uint64_t nmaxmoneypertime,
		uint64_t nmaxmoneytotal, uint64_t nmaxmoneyperday)  {
	nAuthorizeTime = nauthorizetime;
	nUserDefine = nuserdefine;
	nMaxMoneyPerTime = nmaxmoneypertime;
	nMaxMoneyTotal = nmaxmoneytotal;
	nMaxMoneyPerDay = nmaxmoneyperday;
}

CAuthorizate::CAuthorizate(CNetAuthorizate te) {
	nAuthorizeTime = te.GetAuthorizeTime();
	nUserDefine = te.GetUserData();
	nMaxMoneyPerTime = te.GetMaxMoneyPerTime();
	nMaxMoneyTotal = te.GetMaxMoneyTotal();
	nMaxMoneyPerDay = te.GetMaxMoneyPerDay();
	nLastOperHeight = 0;
	nCurMaxMoneyPerDay = 0;
}

unsigned int CAuthorizate::GetSerializeSize(int nType, int nVersion) const {
	CSerActionGetSerializeSize ser_action;
	unsigned int nSerSize = 0;
	ser_streamplaceholder s;
	s.nType = nType;
	vector<unsigned char> vData;
	vData.clear();
	if (nAuthorizeTime > 0) {
		CDataStream ds(SER_DISK, CLIENT_VERSION);
		ds << VARINT(nAuthorizeTime);
		ds << nUserDefine;
		ds << VARINT(nMaxMoneyPerTime);
		ds << VARINT(nMaxMoneyTotal);
		ds << VARINT(nMaxMoneyPerDay);
		ds << VARINT(nLastOperHeight);
		ds << VARINT(nCurMaxMoneyPerDay);
		vData.insert(vData.end(), ds.begin(), ds.end());
	}
	s.nVersion = nVersion;
	{
		(nSerSize += ::SerReadWrite(s, (vData), nType, nVersion, ser_action));
	}
	return nSerSize;
}

Object CNetAuthorizate::ToJosnObj() const {
	Object obj;
	obj.push_back(Pair("AuthorizeTime",(uint64_t)nAuthorizeTime));
	obj.push_back(Pair("MaxMoneyPerTime",nMaxMoneyPerTime));
	obj.push_back(Pair("MaxMoneyTotal",nMaxMoneyTotal));
	obj.push_back(Pair("MaxMoneyPerDay",nMaxMoneyPerDay));
	obj.push_back(Pair("UserDefine",HexStr(nUserDefine)));
	return obj;
}

string CNetAuthorizate::ToString(bool bFlag) const {
	return write_string(Value(ToJosnObj()),bFlag);
}
Object CAuthorizate::ToJosnObj() const {
	Object obj = CNetAuthorizate::ToJosnObj();
	obj.push_back(Pair("LastOperHeight",(uint64_t)nLastOperHeight));
	obj.push_back(Pair("CurMaxMoneyPerDay",nCurMaxMoneyPerDay));
	return obj;
}

string CAuthorizate::ToString(bool bFlag) const {
	return write_string(Value(ToJosnObj()),bFlag);
}

uint256 CContractTransaction::SignatureHash() const  {
	CHashWriter ss(SER_GETHASH, 0);
	CID scriptId(scriptRegId);
	ss <<VARINT(nVersion) << nTxType << scriptId;
	for(auto & acctRegId : vAccountRegId) {
		CID acctId(acctRegId);
		ss << acctId;
	}
	ss << VARINT(llFees) << vContract << VARINT(nValidHeight) ;
	return ss.GetHash();
}
