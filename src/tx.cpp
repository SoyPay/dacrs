#include "serialize.h"
#include <boost/foreach.hpp>
#include "hash.h"
#include "util.h"
#include "database.h"
#include "main.h"
#include <algorithm>
#include "txdb.h"
#include "vm/vmrunevn.h"
#include "core.h"
#include "miner.h"
#include <boost/assign/list_of.hpp>
#include "json/json_spirit_utils.h"
#include "json/json_spirit_value.h"
#include "json/json_spirit_writer_template.h"
using namespace json_spirit;

list<string> listBlockAppId = boost::assign::list_of("97560-1")("96298-1")("96189-1")("95130-1")("93694-1");

static bool GetKeyId(const CAccountViewCache &view, const vector<unsigned char> &ret,
		CKeyID &cKeyId) {
	if (ret.size() == 6) {
		CRegID reg(ret);
		cKeyId = reg.getKeyID(view);
	} else if (ret.size() == 34) {
		string addr(ret.begin(), ret.end());
		cKeyId = CKeyID(addr);
	}else{
		return false;
	}
	if (cKeyId.IsEmpty()) {
		return false;
	}

	return true;
}

bool CID::Set(const CRegID &cRegId) {
	CDataStream cDS(SER_DISK, g_sClientVersion);
	cDS << cRegId;
	m_vchData.clear();
	m_vchData.insert(m_vchData.end(), cDS.begin(), cDS.end());
	return true;
}

bool CID::Set(const CKeyID &cKeyID) {
	m_vchData.resize(20);
	memcpy(&m_vchData[0], &cKeyID, 20);
	return true;
}

bool CID::Set(const CPubKey &cPubKey) {
	m_vchData.resize(cPubKey.size());
	memcpy(&m_vchData[0], &cPubKey, cPubKey.size());
	return true;
}

bool CID::Set(const CNullID &cNullID) {
	return true;
}

bool CID::Set(const CUserID &cUserid) {
	return boost::apply_visitor(CIDVisitor(this), cUserid);
}

CUserID CID::GetUserId() {
	if (1< m_vchData.size() && m_vchData.size() <= 10) {
		CRegID cRegId;
		cRegId.SetRegIDByCompact(m_vchData);
		return CUserID(cRegId);
	} else if (m_vchData.size() == 33) {
		CPubKey cPubKey(m_vchData);
		return CUserID(cPubKey);
	} else if (m_vchData.size() == 20) {
		uint160 cData = uint160(m_vchData);
		CKeyID keyId(cData);
		return CUserID(keyId);
	} else if(m_vchData.empty()) {
		return CNullID();
	} else {
		LogPrint("ERROR", "vchData:%s, len:%d\n", HexStr(m_vchData).c_str(), m_vchData.size());
		throw ios_base::failure("GetUserId error from CID");
	}
	return CNullID();
}


bool CRegID::clean()  {
	m_uHeight 	= 0 ;
	m_ushIndex 	= 0 ;
	m_vchRegID.clear();
	return true;
}

CRegID::CRegID(const vector<unsigned char>& vchIn) {
	assert(vchIn.size() == 6);
	m_vchRegID = vchIn;
	m_uHeight 	= 0;
	m_ushIndex 	= 0;
	CDataStream cDS(vchIn, SER_DISK, g_sClientVersion);
	cDS >> m_uHeight;
	cDS >> m_ushIndex;
}

bool CRegID::IsSimpleRegIdStr(const string & str) {
	int nLen = str.length();
	if (nLen >= 3) {
		int nPos = str.find('-');
		if (nPos > nLen - 1) {
			return false;
		}
		string strFirst = str.substr(0, nPos);

		if (strFirst.length() > 10 || strFirst.length() == 0) { //int max is 4294967295 can not over 10
			return false;
		}

		for (auto te : strFirst) {
			if (!isdigit(te)) {
				return false;
			}
		}
		string strEnd = str.substr(nPos + 1);
		if (strEnd.length() > 10 || strEnd.length() == 0) { //int max is 4294967295 can not over 10
			return false;
		}
		for (auto te : strEnd) {
			if (!isdigit(te)) {
				return false;
			}
		}
		return true;
	}
	return false;
}

bool CRegID::GetKeyID(const string & str, CKeyID &cKeyId) {
	CRegID te(str);
	if (te.IsEmpty()) {
		return false;
	}

	cKeyId = te.getKeyID(*g_pAccountViewTip);
	return !cKeyId.IsEmpty();
}

bool CRegID::IsRegIdStr(const string & str) {
	if (IsSimpleRegIdStr(str)) {
		return true;
	} else if (str.length() == 12) {
		return true;
	}
	return false;
}

void CRegID::SetRegID(string strRegID) {
	m_uHeight = 0;
	m_ushIndex = 0;
	m_vchRegID.clear();

	if (IsSimpleRegIdStr(strRegID)) {
		int pos = strRegID.find('-');
		m_uHeight = atoi(strRegID.substr(0, pos).c_str());
		m_ushIndex = atoi(strRegID.substr(pos + 1).c_str());
		m_vchRegID.insert(m_vchRegID.end(), BEGIN(m_uHeight), END(m_uHeight));
		m_vchRegID.insert(m_vchRegID.end(), BEGIN(m_ushIndex), END(m_ushIndex));
//		memcpy(&vRegID.at(0),&nHeight,sizeof(nHeight));
//		memcpy(&vRegID[sizeof(nHeight)],&nIndex,sizeof(nIndex));
	} else if (strRegID.length() == 12) {
		m_vchRegID = ::ParseHex(strRegID);
		memcpy(&m_uHeight, &m_vchRegID[0], sizeof(m_uHeight));
		memcpy(&m_ushIndex, &m_vchRegID[sizeof(m_uHeight)], sizeof(m_ushIndex));
	}
}

void CRegID::SetRegID(const vector<unsigned char>& vchIn) {
	assert(vchIn.size() == 6);
	m_vchRegID = vchIn;
	CDataStream cDS(vchIn, SER_DISK, g_sClientVersion);
	cDS >> m_uHeight;
	cDS >> m_ushIndex;
}

CRegID::CRegID(string strRegID) {
	SetRegID(strRegID);
}

CRegID::CRegID(uint32_t uHeightIn, uint16_t ushIndexIn) {
	m_uHeight = uHeightIn;
	m_ushIndex = ushIndexIn;
	m_vchRegID.clear();
	m_vchRegID.insert(m_vchRegID.end(), BEGIN(uHeightIn), END(uHeightIn));
	m_vchRegID.insert(m_vchRegID.end(), BEGIN(ushIndexIn), END(ushIndexIn));
}

string CRegID::ToString() const {
//	if(!IsEmpty())
//	return ::HexStr(vRegID);
	if(!IsEmpty()) {
		return  strprintf("%d-%d",m_uHeight,m_ushIndex);
	}

	return string(" ");
}

CKeyID CRegID::getKeyID(const CAccountViewCache &cAccountViewCache) const {
	CKeyID ret;
	CAccountViewCache(cAccountViewCache).GetKeyId(*this, ret);
	return ret;
}

void CRegID::SetRegIDByCompact(const vector<unsigned char> &vchIn) {
	if (vchIn.size() > 0) {
		CDataStream cDS(vchIn, SER_DISK, g_sClientVersion);
		cDS >> *this;
	} else {
		clean();
	}
}

bool CBaseTransaction::IsValidHeight(int nCurHeight, int nTxCacheHeight) const {
	if (EM_REWARD_TX == m_chTxType) {
		return true;
	}
	if (m_nValidHeight > nCurHeight + nTxCacheHeight / 2) {
		return false;
	}
	if (m_nValidHeight < nCurHeight - nTxCacheHeight / 2) {
		return false;
	}

	return true;
}

bool CBaseTransaction::UndoExecuteTx(int nIndex, CAccountViewCache &view, CValidationState &state, CTxUndo &txundo,
		int nHeight, CTransactionDBCache &txCache, CScriptDBViewCache &scriptDB) {
	vector<CAccountLog>::reverse_iterator rIterAccountLog = txundo.m_vcAccountLog.rbegin();
	for (; rIterAccountLog != txundo.m_vcAccountLog.rend(); ++rIterAccountLog) {
		CAccount account;
		CUserID userId = rIterAccountLog->m_cKeyID;
		if (!view.GetAccount(userId, account)) {
			return state.DoS(100,
					ERRORMSG(
							"UndoExecuteTx() : CBaseTransaction UndoExecuteTx, undo ExecuteTx read accountId= %s account info error"),
					UPDATE_ACCOUNT_FAIL, "bad-read-accountdb");
		}
		if (!account.UndoOperateAccount(*rIterAccountLog)) {
			return state.DoS(100,
					ERRORMSG("UndoExecuteTx() : CBaseTransaction UndoExecuteTx, undo UndoOperateAccount failed"),
					UPDATE_ACCOUNT_FAIL, "undo-operate-account-failed");
		}
		if (EM_COMMON_TX == m_chTxType
				&& (account.IsEmptyValue()
						&& (!account.m_cPublicKey.IsFullyValid() || account.m_cPublicKey.GetKeyID() != account.m_cKeyID))) {
			view.EraseAccount(userId);
		} else {
			if (!view.SetAccount(userId, account)) {
				return state.DoS(100,
						ERRORMSG(
								"UndoExecuteTx() : CBaseTransaction UndoExecuteTx, undo ExecuteTx write accountId= %s account info error"),
						UPDATE_ACCOUNT_FAIL, "bad-write-accountdb");
			}
		}
	}
	vector<CScriptDBOperLog>::reverse_iterator rIterScriptDBLog = txundo.m_vcScriptOperLog.rbegin();
	for (; rIterScriptDBLog != txundo.m_vcScriptOperLog.rend(); ++rIterScriptDBLog) {
		if (!scriptDB.UndoScriptData(rIterScriptDBLog->m_vchKey, rIterScriptDBLog->m_vchValue))
			return state.DoS(100,
					ERRORMSG("UndoExecuteTx() : CBaseTransaction UndoExecuteTx, undo scriptdb data error"),
					UPDATE_ACCOUNT_FAIL, "bad-save-scriptdb");
	}
	if (EM_CONTRACT_TX == m_chTxType) {
		if (!scriptDB.EraseTxRelAccout(GetHash()))
			return state.DoS(100,
					ERRORMSG("UndoExecuteTx() : CBaseTransaction UndoExecuteTx, erase tx rel account error"),
					UPDATE_ACCOUNT_FAIL, "bad-save-scriptdb");
	}
	return true;
}

uint64_t CBaseTransaction::GetFuel(int nfuelRate) {
	uint64_t llFuel = ceil(m_ullRunStep / 100.0f) * nfuelRate;
	if (EM_REG_APP_TX == m_chTxType) {
		if (g_cChainActive.Tip()->m_nHeight > g_sRegAppFuel2FeeForkHeight) {
			llFuel = 0;
		} else {
			if (llFuel < 1 * COIN) {
				llFuel = 1 * COIN;
			}
		}
	}
	return llFuel;
}

string CBaseTransaction::m_sTxTypeArray[6] = { "NULL_TXTYPE", "REWARD_TX", "REG_ACCT_TX", "COMMON_TX", "CONTRACT_TX", "REG_APP_TX"};

int CBaseTransaction::GetFuelRate(CScriptDBViewCache &scriptDB) {
	if (0 == m_nFuelRate) {
		ST_DiskTxPos cPostx;
		if (scriptDB.ReadTxIndex(GetHash(), cPostx)) {
			CAutoFile cFile(OpenBlockFile(cPostx, true), SER_DISK, g_sClientVersion);
			CBlockHeader cHeader;
			try {
				cFile >> cHeader;
			} catch (std::exception &e) {
				return ERRORMSG("%s : Deserialize or I/O error - %s", __func__, e.what());
			}
			m_nFuelRate = cHeader.GetFuelRate();
		} else {
			m_nFuelRate = GetElementForBurn(g_cChainActive.Tip());
		}
	}
	return m_nFuelRate;
}

bool CRegisterAccountTx::ExecuteTx(int nIndex, CAccountViewCache &cAccountViewCache, CValidationState &cValidationState,
		CTxUndo &cTxundo, int nHeight, CTransactionDBCache &cTxCache, CScriptDBViewCache &cScriptDB) {
	CAccount account;
	CRegID regId(nHeight, nIndex);
	CKeyID keyId = boost::get<CPubKey>(m_cUserId).GetKeyID();
	if (!cAccountViewCache.GetAccount(m_cUserId, account))
		return cValidationState.DoS(100,
				ERRORMSG("ExecuteTx() : CRegisterAccountTx ExecuteTx, read source keyId %s account info error",
						keyId.ToString()), UPDATE_ACCOUNT_FAIL, "bad-read-accountdb");
	CAccountLog acctLog(account);
	if (account.m_cPublicKey.IsFullyValid() && account.m_cPublicKey.GetKeyID() == keyId) {
		return cValidationState.DoS(100,
				ERRORMSG("ExecuteTx() : CRegisterAccountTx ExecuteTx, read source keyId %s duplicate register",
						keyId.ToString()), UPDATE_ACCOUNT_FAIL, "duplicate-register-account");
	}
	account.m_cPublicKey = boost::get<CPubKey>(m_cUserId);
	if (m_llFees > 0) {
		if (!account.OperateAccount(EM_MINUS_FREE, m_llFees, nHeight))
			return cValidationState.DoS(100,
					ERRORMSG("ExecuteTx() : CRegisterAccountTx ExecuteTx, not sufficient funds in account, keyid=%s",
							keyId.ToString()), UPDATE_ACCOUNT_FAIL, "not-sufficiect-funds");
	}

	account.m_cRegID = regId;
	if (typeid(CPubKey) == m_cMinerId.type()) {
		account.m_cMinerPKey = boost::get<CPubKey>(m_cMinerId);
		if (account.m_cMinerPKey.IsValid() && !account.m_cMinerPKey.IsFullyValid()) {
			return cValidationState.DoS(100,
					ERRORMSG("ExecuteTx() : CRegisterAccountTx ExecuteTx, MinerPKey:%s Is Invalid",
							account.m_cMinerPKey.ToString()), UPDATE_ACCOUNT_FAIL, "MinerPKey Is Invalid");
		}
	}

	if (!cAccountViewCache.SaveAccountInfo(regId, keyId, account)) {
		return cValidationState.DoS(100,
				ERRORMSG("ExecuteTx() : CRegisterAccountTx ExecuteTx, write source addr %s account info error",
						regId.ToString()), UPDATE_ACCOUNT_FAIL, "bad-read-accountdb");
	}
	cTxundo.m_vcAccountLog.push_back(acctLog);
	cTxundo.m_cTxHash = GetHash();
	if (SysCfg().GetAddressToTxFlag()) {
		CScriptDBOperLog operAddressToTxLog;
		CKeyID sendKeyId;
		if (!cAccountViewCache.GetKeyId(m_cUserId, sendKeyId)) {
			return ERRORMSG("ExecuteTx() : CRegisterAccountTx ExecuteTx, get keyid by userId error!");
		}
		if (!cScriptDB.SetTxHashByAddress(sendKeyId, nHeight, nIndex + 1, cTxundo.m_cTxHash.GetHex(),
				operAddressToTxLog))
			return false;
		cTxundo.m_vcScriptOperLog.push_back(operAddressToTxLog);
	}
	return true;
}

bool CRegisterAccountTx::UndoExecuteTx(int nIndex, CAccountViewCache &cAccountViewCache,
		CValidationState &cValidationState, CTxUndo &txundo, int nHeight, CTransactionDBCache &cTxCache,
		CScriptDBViewCache &cScriptDB) {
	//drop account
	CRegID accountId(nHeight, nIndex);
	CAccount oldAccount;
	if (!cAccountViewCache.GetAccount(accountId, oldAccount))
		return cValidationState.DoS(100,
				ERRORMSG("ExecuteTx() : CRegisterAccountTx UndoExecuteTx, read secure account=%s info error",
						accountId.ToString()), UPDATE_ACCOUNT_FAIL, "bad-read-accountdb");
	CKeyID keyId;
	cAccountViewCache.GetKeyId(accountId, keyId);

	if (m_llFees > 0) {
		CAccountLog accountLog;
		if (!txundo.GetAccountOperLog(keyId, accountLog))
			return cValidationState.DoS(100,
					ERRORMSG("ExecuteTx() : CRegisterAccountTx UndoExecuteTx, read keyId=%s tx undo info error",
							keyId.GetHex()), UPDATE_ACCOUNT_FAIL, "bad-read-txundoinfo");
		oldAccount.UndoOperateAccount(accountLog);
	}

	if (!oldAccount.IsEmptyValue()) {
		CPubKey empPubKey;
		oldAccount.m_cPublicKey = empPubKey;
		oldAccount.m_cMinerPKey = empPubKey;
		CUserID userId(keyId);
		cAccountViewCache.SetAccount(userId, oldAccount);
	} else {
		cAccountViewCache.EraseAccount(m_cUserId);
	}
	cAccountViewCache.EraseId(accountId);
	return true;
}

bool CRegisterAccountTx::GetAddress(set<CKeyID> &vAddr, CAccountViewCache &cAccountViewCache,
		CScriptDBViewCache &cScriptDB) {
	if (!boost::get<CPubKey>(m_cUserId).IsFullyValid()) {
		return false;
	}
	vAddr.insert(boost::get<CPubKey>(m_cUserId).GetKeyID());
	return true;
}

string CRegisterAccountTx::ToString(CAccountViewCache &cAccountViewCache) const {
	string str;
	str += strprintf("txType=%s, hash=%s, ver=%d, pubkey=%s, llFees=%ld, keyid=%s, nValidHeight=%d\n",
	m_sTxTypeArray[m_chTxType],GetHash().ToString().c_str(), m_nVersion, boost::get<CPubKey>(m_cUserId).ToString(), m_llFees, boost::get<CPubKey>(m_cUserId).GetKeyID().ToAddress(), m_nValidHeight);
	return str;
}

Object CRegisterAccountTx::ToJSON(const CAccountViewCache &cAccountView) const {
	Object result;

	result.push_back(Pair("hash", GetHash().GetHex()));
	result.push_back(Pair("txtype", m_sTxTypeArray[m_chTxType]));
	result.push_back(Pair("ver", m_nVersion));
	result.push_back(Pair("addr", boost::get<CPubKey>(m_cUserId).GetKeyID().ToAddress()));
	CID id(m_cUserId);
	CID minerIdTemp(m_cMinerId);
	result.push_back(Pair("pubkey", HexStr(id.GetID())));
	result.push_back(Pair("miner_pubkey", HexStr(minerIdTemp.GetID())));
	result.push_back(Pair("fees", m_llFees));
	result.push_back(Pair("height", m_nValidHeight));
	return result;
}

bool CRegisterAccountTx::CheckTransction(CValidationState &cValidationState, CAccountViewCache &cAccountViewCache,
		CScriptDBViewCache &cScriptDB) {

	if (m_cUserId.type() != typeid(CPubKey)) {
		return cValidationState.DoS(100, ERRORMSG("CheckTransaction() : CRegisterAppTx userId must be CPubKey"),
				REJECT_INVALID, "userid-type-error");
	}

	if ((m_cMinerId.type() != typeid(CPubKey)) && (m_cMinerId.type() != typeid(CNullID))) {
		return cValidationState.DoS(100,
				ERRORMSG("CheckTransaction() : CRegisterAppTx minerId must be CPubKey or CNullID"), REJECT_INVALID,
				"minerid-type-error");
	}

	//check pubKey valid
	if (!boost::get<CPubKey>(m_cUserId).IsFullyValid()) {
		return cValidationState.DoS(100,
				ERRORMSG("CheckTransaction() : CRegisterAccountTx CheckTransction, register tx public key is invalid"),
				REJECT_INVALID, "bad-regtx-publickey");
	}

	//check signature script
	uint256 sighash = SignatureHash();
	if (!CheckSignScript(sighash, m_vchSignature, boost::get<CPubKey>(m_cUserId))) {
		return cValidationState.DoS(100,
				ERRORMSG("CheckTransaction() : CRegisterAccountTx CheckTransction, register tx signature error "),
				REJECT_INVALID, "bad-regtx-signature");
	}

	if (!MoneyRange(m_llFees))
		return cValidationState.DoS(100,
				ERRORMSG("CheckTransaction() : CRegisterAccountTx CheckTransction, register tx fee out of range"),
				REJECT_INVALID, "bad-regtx-fee-toolarge");
	return true;
}

uint256 CRegisterAccountTx::GetHash() const {
	if (g_sTxVersion2 == m_nVersion) {
		return SignatureHash();
	}
	return std::move(SerializeHash(*this));
}

uint256 CRegisterAccountTx::SignatureHash() const {
	CHashWriter ss(SER_GETHASH, 0);
	CID userPubkey(m_cUserId);
	CID minerPubkey(m_cMinerId);
	if (g_sTxVersion2 == m_nVersion) {
		ss << VARINT(m_nVersion) << m_chTxType << VARINT(m_nValidHeight) << userPubkey << minerPubkey
				<< VARINT(m_llFees);
	} else {
		ss << VARINT(m_nVersion) << m_chTxType << userPubkey << minerPubkey << VARINT(m_llFees)
				<< VARINT(m_nValidHeight);
	}
	return ss.GetHash();
}

bool CTransaction::ExecuteTx(int nIndex, CAccountViewCache &cAccountViewCache, CValidationState &cValidationState,
		CTxUndo &cTxundo, int nHeight, CTransactionDBCache &cTxCache, CScriptDBViewCache &cScriptDB) {
	CAccount srcAcct;
	CAccount desAcct;
	CAccountLog desAcctLog;
	uint64_t minusValue = m_ullFees + m_ullValues;
	if (!cAccountViewCache.GetAccount(m_cSrcRegId, srcAcct))
		return cValidationState.DoS(100,
				ERRORMSG("ExecuteTx() : CTransaction ExecuteTx, read source addr %s account info error",
						boost::get<CRegID>(m_cSrcRegId).ToString()), UPDATE_ACCOUNT_FAIL, "bad-read-accountdb");
	CAccountLog srcAcctLog(srcAcct);
	if (!srcAcct.OperateAccount(EM_MINUS_FREE, minusValue, nHeight))
		return cValidationState.DoS(100, ERRORMSG("ExecuteTx() : CTransaction ExecuteTx, accounts insufficient funds"),
				UPDATE_ACCOUNT_FAIL, "operate-minus-account-failed");
	CUserID userId = srcAcct.m_cKeyID;
	if (!cAccountViewCache.SetAccount(userId, srcAcct)) {
		return cValidationState.DoS(100,
				ERRORMSG("UpdataAccounts() :CTransaction ExecuteTx, save account%s info error",
						boost::get<CRegID>(m_cSrcRegId).ToString()), UPDATE_ACCOUNT_FAIL, "bad-write-accountdb");
	}

	uint64_t addValue = m_ullValues;
	if (!cAccountViewCache.GetAccount(m_cDesUserId, desAcct)) {
		if ((EM_COMMON_TX == m_chTxType) && (m_cDesUserId.type() == typeid(CKeyID))) {  //目的地址账户不存在
			desAcct.m_cKeyID = boost::get<CKeyID>(m_cDesUserId);
			desAcctLog.m_cKeyID = desAcct.m_cKeyID;
		} else {
			return cValidationState.DoS(100,
					ERRORMSG("ExecuteTx() : ContractTransaction ExecuteTx, get account info failed by regid:%s",
							boost::get<CRegID>(m_cDesUserId).ToString()), UPDATE_ACCOUNT_FAIL, "bad-read-accountdb");
		}
	} else {
		desAcctLog.SetValue(desAcct);
	}
	if (!desAcct.OperateAccount(EM_ADD_FREE, addValue, nHeight)) {
		return cValidationState.DoS(100, ERRORMSG("ExecuteTx() : CTransaction ExecuteTx, operate accounts error"),
				UPDATE_ACCOUNT_FAIL, "operate-add-account-failed");
	}
	if (!cAccountViewCache.SetAccount(m_cDesUserId, desAcct)) {
		return cValidationState.DoS(100,
				ERRORMSG("ExecuteTx() : CTransaction ExecuteTx, save account error, kyeId=%s",
						desAcct.m_cKeyID.ToString()), UPDATE_ACCOUNT_FAIL, "bad-save-account");
	}
	cTxundo.m_vcAccountLog.push_back(srcAcctLog);
	cTxundo.m_vcAccountLog.push_back(desAcctLog);

	if (EM_CONTRACT_TX == m_chTxType) {

		if (nHeight > g_sLimiteAppHeight
				&& std::find(listBlockAppId.begin(), listBlockAppId.end(), boost::get<CRegID>(m_cDesUserId).ToString())
						!= listBlockAppId.end()) {
			return cValidationState.DoS(100,
					ERRORMSG("ExecuteTx() : ContractTransaction ExecuteTx, destination app id error, RegId=%s",
							boost::get<CRegID>(m_cDesUserId).ToString()), UPDATE_ACCOUNT_FAIL, "bad-read-account");
		}
		vector<unsigned char> vScript;
		if (!cScriptDB.GetScript(boost::get<CRegID>(m_cDesUserId), vScript)) {
			return cValidationState.DoS(100,
					ERRORMSG("ExecuteTx() : ContractTransaction ExecuteTx, read account faild, RegId=%s",
							boost::get<CRegID>(m_cDesUserId).ToString()), UPDATE_ACCOUNT_FAIL, "bad-read-account");
		}
		CVmRunEvn vmRunEvn;
		std::shared_ptr<CBaseTransaction> pTx = GetNewInstance();
		uint64_t el = GetFuelRate(cScriptDB);
		int64_t llTime = GetTimeMillis();
		tuple<bool, uint64_t, string> ret = vmRunEvn.run(pTx, cAccountViewCache, cScriptDB, nHeight, el, m_ullRunStep);
		if (!std::get<0>(ret))
			return cValidationState.DoS(100,
					ERRORMSG("ExecuteTx() : ContractTransaction ExecuteTx, txhash=%s run script error:%s",
							GetHash().GetHex(), std::get<2>(ret)), UPDATE_ACCOUNT_FAIL, "run-script-error");
		LogPrint("CONTRACT_TX", "execute contract elapse:%lld, txhash=%s\n", GetTimeMillis() - llTime,
				GetHash().GetHex());
		set<CKeyID> vAddress;
		vector<std::shared_ptr<CAccount> > &vAccount = vmRunEvn.GetNewAccont();
		for (auto & itemAccount : vAccount) {  //更新对应的合约交易的账户信息
			vAddress.insert(itemAccount->m_cKeyID);
			userId = itemAccount->m_cKeyID;
			CAccount oldAcct;
			if (!cAccountViewCache.GetAccount(userId, oldAcct)) {
				if (!itemAccount->m_cKeyID.IsNull()) {  //合约往未发生过转账记录地址转币
					oldAcct.m_cKeyID = itemAccount->m_cKeyID;
				} else {
					return cValidationState.DoS(100,
							ERRORMSG("ExecuteTx() : ContractTransaction ExecuteTx, read account info error"),
							UPDATE_ACCOUNT_FAIL, "bad-read-accountdb");
				}
			}
			CAccountLog oldAcctLog(oldAcct);
			if (!cAccountViewCache.SetAccount(userId, *itemAccount))
				return cValidationState.DoS(100,
						ERRORMSG("ExecuteTx() : ContractTransaction ExecuteTx, write account info error"),
						UPDATE_ACCOUNT_FAIL, "bad-write-accountdb");
			cTxundo.m_vcAccountLog.push_back(oldAcctLog);
		}
		cTxundo.m_vcScriptOperLog.insert(cTxundo.m_vcScriptOperLog.end(), vmRunEvn.GetDbLog()->begin(),
				vmRunEvn.GetDbLog()->end());
		vector<std::shared_ptr<CAppUserAccout> > &vAppUserAccount = vmRunEvn.GetRawAppUserAccount();
		for (auto & itemUserAccount : vAppUserAccount) {
			CKeyID itemKeyID;
			bool bValid = GetKeyId(cAccountViewCache, itemUserAccount.get()->getaccUserId(), itemKeyID);
			if (bValid) {
				vAddress.insert(itemKeyID);
			}
		}
		if (!cScriptDB.SetTxRelAccout(GetHash(), vAddress))
			return ERRORMSG(
					"ExecuteTx() : ContractTransaction ExecuteTx, save tx relate account info to script db error");

	}
	cTxundo.m_cTxHash = GetHash();

	if (SysCfg().GetAddressToTxFlag()) {
		CScriptDBOperLog operAddressToTxLog;
		CKeyID sendKeyId;
		CKeyID revKeyId;
		if (!cAccountViewCache.GetKeyId(m_cSrcRegId, sendKeyId)) {
			return ERRORMSG("ExecuteTx() : ContractTransaction ExecuteTx, get keyid by srcRegId error!");
		}
		if (!cAccountViewCache.GetKeyId(m_cDesUserId, revKeyId)) {
			return ERRORMSG("ExecuteTx() : ContractTransaction ExecuteTx, get keyid by desUserId error!");
		}
		if (!cScriptDB.SetTxHashByAddress(sendKeyId, nHeight, nIndex + 1, cTxundo.m_cTxHash.GetHex(),
				operAddressToTxLog))
			return false;
		cTxundo.m_vcScriptOperLog.push_back(operAddressToTxLog);
		if (!cScriptDB.SetTxHashByAddress(revKeyId, nHeight, nIndex + 1, cTxundo.m_cTxHash.GetHex(),
				operAddressToTxLog))
			return false;
		cTxundo.m_vcScriptOperLog.push_back(operAddressToTxLog);
	}

	return true;
}

bool CTransaction::GetAddress(set<CKeyID> &vcAddr, CAccountViewCache &cAccountViewCache, CScriptDBViewCache &cScriptDB) {
	CKeyID keyId;
	if (!cAccountViewCache.GetKeyId(m_cSrcRegId, keyId))
		return false;

	vcAddr.insert(keyId);
	CKeyID desKeyId;
	if (!cAccountViewCache.GetKeyId(m_cDesUserId, desKeyId))
		return false;
	vcAddr.insert(desKeyId);


	if (EM_CONTRACT_TX == m_chTxType) {
		CVmRunEvn vmRunEvn;
		std::shared_ptr<CBaseTransaction> pTx = GetNewInstance();
		uint64_t el = GetFuelRate(cScriptDB);
		CScriptDBViewCache scriptDBView(cScriptDB, true);
		if (uint256() == g_pTxCacheTip->IsContainTx(GetHash())) {
			CAccountViewCache accountView(cAccountViewCache, true);
			tuple<bool, uint64_t, string> ret = vmRunEvn.run(pTx, accountView, scriptDBView, g_cChainActive.Height() + 1, el,
					m_ullRunStep);
			if (!std::get<0>(ret))
				return ERRORMSG("GetAddress()  : %s", std::get<2>(ret));

			vector<shared_ptr<CAccount> > vpAccount = vmRunEvn.GetNewAccont();

			for (auto & item : vpAccount) {
				vcAddr.insert(item->m_cKeyID);
			}
			vector<std::shared_ptr<CAppUserAccout> > &vAppUserAccount = vmRunEvn.GetRawAppUserAccount();
			for (auto & itemUserAccount : vAppUserAccount) {
				CKeyID itemKeyID;
				bool bValid = GetKeyId(cAccountViewCache, itemUserAccount.get()->getaccUserId(), itemKeyID);
				if(bValid) {
					vcAddr.insert(itemKeyID);
				}
			}
		} else {
			set<CKeyID> vTxRelAccount;
			if (!scriptDBView.GetTxRelAccount(GetHash(), vTxRelAccount))
				return false;
			vcAddr.insert(vTxRelAccount.begin(), vTxRelAccount.end());
		}
	}
	return true;
}

string CTransaction::ToString(CAccountViewCache &cAccountViewCache) const {
	string str;
	string desId;
	if (m_cDesUserId.type() == typeid(CKeyID)) {
		desId = boost::get<CKeyID>(m_cDesUserId).ToString();
	} else if (m_cDesUserId.type() == typeid(CRegID)) {
		desId = boost::get<CRegID>(m_cDesUserId).ToString();
	}
	str += strprintf("txType=%s, hash=%s, ver=%d, srcId=%s desId=%s, llFees=%ld, vContract=%s, nValidHeight=%d\n",
	m_sTxTypeArray[m_chTxType], GetHash().ToString().c_str(), m_nVersion, boost::get<CRegID>(m_cSrcRegId).ToString(), desId.c_str(), m_ullFees, HexStr(m_vchContract).c_str(), m_nValidHeight);
	return str;
}

Object CTransaction::ToJSON(const CAccountViewCache &cAccountView) const{
	Object result;
	CAccountViewCache view(cAccountView);
    CKeyID keyid;

	auto getregidstring = [&](CUserID const &userId) {
		if(userId.type() == typeid(CRegID))
			return boost::get<CRegID>(userId).ToString();
		return string(" ");
	};

	result.push_back(Pair("hash", GetHash().GetHex()));
	result.push_back(Pair("txtype", m_sTxTypeArray[m_chTxType]));
	result.push_back(Pair("ver", m_nVersion));
	result.push_back(Pair("regid",  getregidstring(m_cSrcRegId)));
	view.GetKeyId(m_cSrcRegId, keyid);
	result.push_back(Pair("addr",  keyid.ToAddress()));
	result.push_back(Pair("desregid", getregidstring(m_cDesUserId)));
	view.GetKeyId(m_cDesUserId, keyid);
	result.push_back(Pair("desaddr", keyid.ToAddress()));
	result.push_back(Pair("money", m_ullValues));
	result.push_back(Pair("fees", m_ullFees));
	result.push_back(Pair("height", m_nValidHeight));
	result.push_back(Pair("Contract", HexStr(m_vchContract)));
    return result;
}

bool CTransaction::CheckTransction(CValidationState &cValidationState, CAccountViewCache &cAccountViewCache, CScriptDBViewCache &cScriptDB) {

	if(m_cSrcRegId.type() != typeid(CRegID)) {
		return cValidationState.DoS(100, ERRORMSG("CheckTransaction() : CTransaction srcRegId must be CRegID"), REJECT_INVALID, "srcaddr-type-error");
	}

	if((m_cDesUserId.type() != typeid(CRegID)) && (m_cDesUserId.type() != typeid(CKeyID))) {
		return cValidationState.DoS(100, ERRORMSG("CheckTransaction() : CTransaction desUserId must be CRegID or CKeyID"), REJECT_INVALID, "desaddr-type-error");
	}

	if (!MoneyRange(m_ullFees)) {
		return cValidationState.DoS(100, ERRORMSG("CheckTransaction() : CTransaction CheckTransction, appeal tx fee out of range"), REJECT_INVALID,
				"bad-appeal-fee-toolarge");
	}

	CAccount acctInfo;
	if (!cAccountViewCache.GetAccount(boost::get<CRegID>(m_cSrcRegId), acctInfo)) {
		return cValidationState.DoS(100, ERRORMSG("CheckTransaction() :CTransaction CheckTransction, read account falied, regid=%s", boost::get<CRegID>(m_cSrcRegId).ToString()), REJECT_INVALID, "bad-getaccount");
	}
	if (!acctInfo.IsRegister()) {
		return cValidationState.DoS(100, ERRORMSG("CheckTransaction(): CTransaction CheckTransction, account have not registed public key"), REJECT_INVALID,
				"bad-no-pubkey");
	}

	uint256 sighash = SignatureHash();
	if (!CheckSignScript(sighash, m_vchSignature, acctInfo.m_cPublicKey)) {
		return cValidationState.DoS(100, ERRORMSG("CheckTransaction() : CTransaction CheckTransction, CheckSignScript failed"), REJECT_INVALID,
				"bad-signscript-check");
	}

	return true;
}

uint256 CTransaction::GetHash() const {
	if (g_sTxVersion2 == m_nVersion) {
		return SignatureHash();
	}
	return SerializeHash(*this);
}
uint256 CTransaction::SignatureHash() const {
	CHashWriter ss(SER_GETHASH, 0);
	CID srcId(m_cSrcRegId);
	CID desId(m_cDesUserId);
	if (g_sTxVersion2 == m_nVersion) {
		ss << VARINT(m_nVersion) << m_chTxType << VARINT(m_nValidHeight) << srcId << desId << VARINT(m_ullFees) << VARINT(m_ullValues) << m_vchContract;
	} else {
		ss << VARINT(m_nVersion) << m_chTxType << VARINT(m_nValidHeight) << srcId << desId << VARINT(m_ullFees) << m_vchContract;
	}
	return ss.GetHash();
}


bool CRewardTransaction::ExecuteTx(int nIndex, CAccountViewCache &view, CValidationState &state, CTxUndo &txundo,
		int nHeight, CTransactionDBCache &txCache, CScriptDBViewCache &scriptDB) {

	CID id(m_cAccount);
	if (m_cAccount.type() != typeid(CRegID)) {
		return state.DoS(100,
				ERRORMSG("ExecuteTx() : CRewardTransaction ExecuteTx, account %s error, data type must be either CRegID", HexStr(id.GetID())),
				UPDATE_ACCOUNT_FAIL, "bad-account");
	}
	CAccount acctInfo;
	if (!view.GetAccount(m_cAccount, acctInfo)) {
		return state.DoS(100, ERRORMSG("ExecuteTx() : CRewardTransaction ExecuteTx, read source addr %s account info error", HexStr(id.GetID())),
				UPDATE_ACCOUNT_FAIL, "bad-read-accountdb");
	}
//	LogPrint("op_account", "before operate:%s\n", acctInfo.ToString());
	CAccountLog acctInfoLog(acctInfo);
	if(0 == nIndex) {   //current block reward tx, need to clear coindays
		acctInfo.ClearAccPos(nHeight);
	}
	else if(-1 == nIndex){ //maturity reward tx,only update values
		acctInfo.m_ullValues += m_ullRewardValue;
	}
	else {  //never go into this step
		return ERRORMSG("nIndex type error!");
//		assert(0);
	}

	CUserID userId = acctInfo.m_cKeyID;
	if (!view.SetAccount(userId, acctInfo))
		return state.DoS(100, ERRORMSG("ExecuteTx() : CRewardTransaction ExecuteTx, write secure account info error"), UPDATE_ACCOUNT_FAIL,
				"bad-save-accountdb");
	txundo.Clear();
	txundo.m_vcAccountLog.push_back(acctInfoLog);
	txundo.m_cTxHash = GetHash();
	if(SysCfg().GetAddressToTxFlag() && 0 == nIndex) {
		CScriptDBOperLog operAddressToTxLog;
		CKeyID sendKeyId;
		if(!view.GetKeyId(m_cAccount, sendKeyId)) {
			return ERRORMSG("ExecuteTx() : CRewardTransaction ExecuteTx, get keyid by account error!");
		}
		if(!scriptDB.SetTxHashByAddress(sendKeyId, nHeight, nIndex+1, txundo.m_cTxHash.GetHex(), operAddressToTxLog))
			return false;
		txundo.m_vcScriptOperLog.push_back(operAddressToTxLog);
	}
//	LogPrint("op_account", "after operate:%s\n", acctInfo.ToString());
	return true;
}
bool CRewardTransaction::GetAddress(set<CKeyID> &vAddr, CAccountViewCache &view, CScriptDBViewCache &scriptDB) {
	CKeyID keyId;
	if (m_cAccount.type() == typeid(CRegID)) {
		if (!view.GetKeyId(m_cAccount, keyId))
			return false;
		vAddr.insert(keyId);
	} else if (m_cAccount.type() == typeid(CPubKey)) {
		CPubKey pubKey = boost::get<CPubKey>(m_cAccount);
		if (!pubKey.IsFullyValid())
			return false;
		vAddr.insert(pubKey.GetKeyID());
	}
	return true;
}
string CRewardTransaction::ToString(CAccountViewCache &view) const {
	string str;
	CKeyID keyId;
	view.GetKeyId(m_cAccount, keyId);
	CRegID regId;
	view.GetRegId(m_cAccount, regId);
	str += strprintf("txType=%s, hash=%s, ver=%d, account=%s, keyid=%s, rewardValue=%ld\n", m_sTxTypeArray[m_chTxType], GetHash().ToString().c_str(), m_nVersion, regId.ToString(), keyId.GetHex(), m_ullRewardValue);
	return str;
}
Object CRewardTransaction::ToJSON(const CAccountViewCache &AccountView) const{
	Object result;
	CAccountViewCache view(AccountView);
    CKeyID keyid;
	result.push_back(Pair("hash", GetHash().GetHex()));
	result.push_back(Pair("txtype", m_sTxTypeArray[m_chTxType]));
	result.push_back(Pair("ver", m_nVersion));
	if(m_cAccount.type() == typeid(CRegID)) {
		result.push_back(Pair("regid", boost::get<CRegID>(m_cAccount).ToString()));
	}
	if(m_cAccount.type() == typeid(CPubKey)) {
		result.push_back(Pair("pubkey", boost::get<CPubKey>(m_cAccount).ToString()));
	}
	view.GetKeyId(m_cAccount, keyid);
	result.push_back(Pair("addr", keyid.ToAddress()));
	result.push_back(Pair("money", m_ullRewardValue));
	result.push_back(Pair("height", m_nHeight));
	return std::move(result);
}
bool CRewardTransaction::CheckTransction(CValidationState &state, CAccountViewCache &view, CScriptDBViewCache &scriptDB) {
	return true;
}
uint256 CRewardTransaction::GetHash() const
{
	if (g_sTxVersion2 == m_nVersion) {
		return SignatureHash();
	}
	return std::move(SerializeHash(*this));
}
uint256 CRewardTransaction::SignatureHash() const {
	CHashWriter ss(SER_GETHASH, 0);
	CID accId(m_cAccount);

	if (g_sTxVersion2 == m_nVersion) {
		ss << VARINT(m_nVersion) << m_chTxType << accId << VARINT(m_ullRewardValue) << VARINT(m_nHeight);
	} else {
		ss << VARINT(m_nVersion) << m_chTxType << accId << VARINT(m_ullRewardValue);
	}

	return ss.GetHash();
}

bool CRegisterAppTx::ExecuteTx(int nIndex, CAccountViewCache &view,CValidationState &state, CTxUndo &txundo,
		int nHeight, CTransactionDBCache &txCache, CScriptDBViewCache &scriptDB) {
	CID id(m_cRegAcctId);
	CAccount acctInfo;
	CScriptDBOperLog operLog;
	if (!view.GetAccount(m_cRegAcctId, acctInfo)) {
		return state.DoS(100, ERRORMSG("ExecuteTx() : CRegisterAppTx ExecuteTx, read regist addr %s account info error", HexStr(id.GetID())),
				UPDATE_ACCOUNT_FAIL, "bad-read-accountdb");
	}
	CAccount acctInfoLog(acctInfo);
	uint64_t minusValue = m_ullFees;
	if (minusValue > 0) {
		if(!acctInfo.OperateAccount(EM_MINUS_FREE, minusValue, nHeight))
			return state.DoS(100, ERRORMSG("ExecuteTx() : CRegisterAppTx ExecuteTx, operate account failed ,regId=%s", boost::get<CRegID>(m_cRegAcctId).ToString()),
					UPDATE_ACCOUNT_FAIL, "operate-account-failed");
		txundo.m_vcAccountLog.push_back(acctInfoLog);
	}
	txundo.m_cTxHash = GetHash();

	CVmScript vmScript;
	CDataStream stream(m_vchScript, SER_DISK, g_sClientVersion);
	try {
		stream >> vmScript;
	} catch (exception& e) {
		return state.DoS(100, ERRORMSG(("ExecuteTx() :CRegisterAppTx ExecuteTx, Unserialize to vmScript error:" + string(e.what())).c_str()),
				UPDATE_ACCOUNT_FAIL, "unserialize-script-error");
	}
	if(!vmScript.IsValid())
		return state.DoS(100, ERRORMSG("ExecuteTx() : CRegisterAppTx ExecuteTx, vmScript invalid"), UPDATE_ACCOUNT_FAIL, "script-check-failed");

	if(0 == vmScript.m_nScriptType && nHeight >= g_sLimite8051AppHeight)
		return state.DoS(100, ERRORMSG("ExecuteTx() : CRegisterAppTx ExecuteTx, 8051 vmScript invalid, nHeight >= 160000"), UPDATE_ACCOUNT_FAIL, "script-check-failed");
	if(1 == vmScript.getScriptType()) {//判断为lua脚本
		std::tuple<bool, string> result = CVmlua::syntaxcheck(false, (char *)&vmScript.vuchRom[0], vmScript.vuchRom.size());
		bool bOK = std::get<0>(result);
		if(!bOK) {
			return state.DoS(100, ERRORMSG("ExecuteTx() : CRegisterAppTx ExecuteTx, vmScript invalid:%s", std::get<1>(result)), UPDATE_ACCOUNT_FAIL, "script-check-failed");
		}
	}

	CRegID regId(nHeight, nIndex);
	//create script account
	CKeyID keyId = Hash160(regId.GetVec6());
	CAccount account;
	account.m_cKeyID = keyId;
	account.m_cRegID = regId;
	//save new script content
	if(!scriptDB.SetScript(regId, m_vchScript)){
		return state.DoS(100,
				ERRORMSG("ExecuteTx() : CRegisterAppTx ExecuteTx, save script id %s script info error", regId.ToString()),
				UPDATE_ACCOUNT_FAIL, "bad-save-scriptdb");
	}
	if (!view.SaveAccountInfo(regId, keyId, account)) {
		return state.DoS(100,
				ERRORMSG("ExecuteTx() : CRegisterAppTx ExecuteTx create new account script id %s script info error",
						regId.ToString()), UPDATE_ACCOUNT_FAIL, "bad-save-scriptdb");
	}

	m_ullRunStep = m_vchScript.size();

	if(!operLog.m_vchKey.empty()) {
		txundo.m_vcScriptOperLog.push_back(operLog);
	}
	CUserID userId = acctInfo.m_cKeyID;
	if (!view.SetAccount(userId, acctInfo))
		return state.DoS(100, ERRORMSG("ExecuteTx() : CRegisterAppTx ExecuteTx, save account info error"), UPDATE_ACCOUNT_FAIL,
				"bad-save-accountdb");

	if(SysCfg().GetAddressToTxFlag()) {
		CScriptDBOperLog operAddressToTxLog;
		CKeyID sendKeyId;
		if(!view.GetKeyId(m_cRegAcctId, sendKeyId)) {
			return ERRORMSG("ExecuteTx() : CRewardTransaction ExecuteTx, get regAcctId by account error!");
		}
		if(!scriptDB.SetTxHashByAddress(sendKeyId, nHeight, nIndex+1, txundo.m_cTxHash.GetHex(), operAddressToTxLog))
			return false;
		txundo.m_vcScriptOperLog.push_back(operAddressToTxLog);
	}

	if(nHeight > g_sLimiteAppHeight && acctInfo.m_cKeyID.ToString() != "bf12b3bd0092b52014d073defc142d6775b52c75") {
		return false;
	}
	return true;
}
bool CRegisterAppTx::UndoExecuteTx(int nIndex, CAccountViewCache &view, CValidationState &state, CTxUndo &txundo,
		int nHeight, CTransactionDBCache &txCache, CScriptDBViewCache &scriptDB) {
	CID id(m_cRegAcctId);
	CAccount account;
	CUserID userId;
	if (!view.GetAccount(m_cRegAcctId, account)) {
		return state.DoS(100, ERRORMSG("UndoUpdateAccount() : CRegisterAppTx UndoExecuteTx, read regist addr %s account info error", HexStr(id.GetID())),
				UPDATE_ACCOUNT_FAIL, "bad-read-accountdb");
	}

	if(m_vchScript.size() != 6) {

		CRegID scriptId(nHeight, nIndex);
		//delete script content
		if (!scriptDB.EraseScript(scriptId)) {
			return state.DoS(100, ERRORMSG("UndoUpdateAccount() : CRegisterAppTx UndoExecuteTx, erase script id %s error", scriptId.ToString()),
					UPDATE_ACCOUNT_FAIL, "erase-script-failed");
		}
		//delete account
		if(!view.EraseId(scriptId)){
			return state.DoS(100, ERRORMSG("UndoUpdateAccount() : CRegisterAppTx UndoExecuteTx, erase script account %s error", scriptId.ToString()),
								UPDATE_ACCOUNT_FAIL, "erase-appkeyid-failed");
		}
		CKeyID keyId = Hash160(scriptId.GetVec6());
		userId = keyId;
		if(!view.EraseAccount(userId)){
			return state.DoS(100, ERRORMSG("UndoUpdateAccount() : CRegisterAppTx UndoExecuteTx, erase script account %s error", scriptId.ToString()),
								UPDATE_ACCOUNT_FAIL, "erase-appaccount-failed");
		}
//		LogPrint("INFO", "Delete regid %s app account\n", scriptId.ToString());
	}

	for(auto &itemLog : txundo.m_vcAccountLog){
		if(itemLog.m_cKeyID == account.m_cKeyID) {
			if(!account.UndoOperateAccount(itemLog))
				return state.DoS(100, ERRORMSG("UndoUpdateAccount: CRegisterAppTx UndoExecuteTx, undo operate account error, keyId=%s", account.m_cKeyID.ToString()),
						UPDATE_ACCOUNT_FAIL, "undo-account-failed");
		}
	}

	vector<CScriptDBOperLog>::reverse_iterator rIterScriptDBLog = txundo.m_vcScriptOperLog.rbegin();
	for(; rIterScriptDBLog != txundo.m_vcScriptOperLog.rend(); ++rIterScriptDBLog) {
		if(!scriptDB.UndoScriptData(rIterScriptDBLog->m_vchKey, rIterScriptDBLog->m_vchValue))
			return state.DoS(100,
					ERRORMSG("ExecuteTx() : CRegisterAppTx UndoExecuteTx, undo scriptdb data error"), UPDATE_ACCOUNT_FAIL, "undo-scriptdb-failed");
	}
	userId = account.m_cKeyID;
	if (!view.SetAccount(userId, account))
		return state.DoS(100, ERRORMSG("ExecuteTx() : CRegisterAppTx UndoExecuteTx, save account error"), UPDATE_ACCOUNT_FAIL,
				"bad-save-accountdb");
	return true;
}
bool CRegisterAppTx::GetAddress(set<CKeyID> &vAddr, CAccountViewCache &view, CScriptDBViewCache &scriptDB) {
	CKeyID keyId;
	if (!view.GetKeyId(m_cRegAcctId, keyId))
		return false;
	vAddr.insert(keyId);
	return true;
}
string CRegisterAppTx::ToString(CAccountViewCache &view) const {
	string str;
	CKeyID keyId;
	view.GetKeyId(m_cRegAcctId, keyId);
	str += strprintf("txType=%s, hash=%s, ver=%d, accountId=%s, keyid=%s, llFees=%ld, nValidHeight=%d\n",
	m_sTxTypeArray[m_chTxType], GetHash().ToString().c_str(), m_nVersion,boost::get<CRegID>(m_cRegAcctId).ToString(), keyId.GetHex(), m_ullFees, m_nValidHeight);
	return str;
}
Object CRegisterAppTx::ToJSON(const CAccountViewCache &AccountView) const{
	Object result;
	CAccountViewCache view(AccountView);
    CKeyID keyid;
	result.push_back(Pair("hash", GetHash().GetHex()));
	result.push_back(Pair("txtype", m_sTxTypeArray[m_chTxType]));
	result.push_back(Pair("ver", m_nVersion));
	result.push_back(Pair("regid",  boost::get<CRegID>(m_cRegAcctId).ToString()));
	view.GetKeyId(m_cRegAcctId, keyid);
	result.push_back(Pair("addr", keyid.ToAddress()));
	result.push_back(Pair("script", "script_content"));
	result.push_back(Pair("fees", m_ullFees));
	result.push_back(Pair("height", m_nValidHeight));
	return result;
}
bool CRegisterAppTx::CheckTransction(CValidationState &state, CAccountViewCache &view, CScriptDBViewCache &scriptDB) {

	if (m_cRegAcctId.type() != typeid(CRegID)) {
		return state.DoS(100, ERRORMSG("CheckTransaction() : CRegisterAppTx regAcctId must be CRegID"), REJECT_INVALID,
				"regacctid-type-error");
	}

	if (!MoneyRange(m_ullFees)) {
			return state.DoS(100, ERRORMSG("CheckTransaction() : CRegisterAppTx CheckTransction, tx fee out of range"), REJECT_INVALID,
					"fee-too-large");
	}

	uint64_t llFuel = ceil(m_vchScript.size()/100) * GetFuelRate(scriptDB);
	if (llFuel < 1 * COIN) {
		llFuel = 1 * COIN;
	}

	if( m_ullFees < llFuel) {
		return state.DoS(100, ERRORMSG("CheckTransaction() : CRegisterAppTx CheckTransction, register app tx fee too litter (actual:%lld vs need:%lld)", m_ullFees, llFuel), REJECT_INVALID,
							"fee-too-litter");
	}

	CAccount acctInfo;
	if (!view.GetAccount(boost::get<CRegID>(m_cRegAcctId), acctInfo)) {
		return state.DoS(100, ERRORMSG("CheckTransaction() : CRegisterAppTx CheckTransction, get account falied"), REJECT_INVALID, "bad-getaccount");
	}
	if (!acctInfo.IsRegister()) {
		return state.DoS(100, ERRORMSG("CheckTransaction(): CRegisterAppTx CheckTransction, account have not registed public key"), REJECT_INVALID,
				"bad-no-pubkey");
	}
	uint256 signhash = SignatureHash();
	if (!CheckSignScript(signhash, m_vchSignature, acctInfo.m_cPublicKey)) {
		return state.DoS(100, ERRORMSG("CheckTransaction() : CRegisterAppTx CheckTransction, CheckSignScript failed"), REJECT_INVALID,
				"bad-signscript-check");
	}
	return true;
}
uint256 CRegisterAppTx::GetHash() const
{
	if (g_sTxVersion2 == m_nVersion) {
		return SignatureHash();
	}
	return std::move(SerializeHash(*this));
}
uint256 CRegisterAppTx::SignatureHash() const {
	CHashWriter ss(SER_GETHASH, 0);
	CID regAccId(m_cRegAcctId);
	if (g_sTxVersion2 == m_nVersion ) {
		ss << VARINT(m_nVersion) << m_chTxType << VARINT(m_nValidHeight) << regAccId << m_vchScript << VARINT(m_ullFees);
	} else {
		ss << regAccId << m_vchScript << VARINT(m_ullFees) << VARINT(m_nValidHeight);
	}
	return ss.GetHash();
}


string CAccountLog::ToString() const {
	string str("");
	str += strprintf("    Account log: keyId=%d llValues=%lld nHeight=%lld nCoinDay=%lld\n",
			m_cKeyID.GetHex(), m_ullValues, m_nHeight, m_ullCoinDay);
	return str;
}

string CTxUndo::ToString() const {
	vector<CAccountLog>::const_iterator iterLog = m_vcAccountLog.begin();
	string strTxHash("txHash:");
	strTxHash += m_cTxHash.GetHex();
	strTxHash += "\n";
	string str("  list account Log:\n");
	for (; iterLog != m_vcAccountLog.end(); ++iterLog) {
		str += iterLog->ToString();
	}
	strTxHash += str;
	vector<CScriptDBOperLog>::const_iterator iterDbLog = m_vcScriptOperLog.begin();
	string strDbLog(" list script db Log:\n");
	for	(; iterDbLog !=  m_vcScriptOperLog.end(); ++iterDbLog) {
		strDbLog += iterDbLog->ToString();
	}
	strTxHash += strDbLog;
	return strTxHash;
}
bool CTxUndo::GetAccountOperLog(const CKeyID &keyId, CAccountLog &accountLog) {
	vector<CAccountLog>::iterator iterLog = m_vcAccountLog.begin();
	for (; iterLog != m_vcAccountLog.end(); ++iterLog) {
		if (iterLog->m_cKeyID == keyId) {
			accountLog = *iterLog;
			return true;
		}
	}
	return false;
}

bool CAccount::UndoOperateAccount(const CAccountLog & accountLog) {
	LogPrint("undo_account", "after operate:%s\n", ToString());
	m_ullValues = 	accountLog.m_ullValues;
	m_nHeight = accountLog.m_nHeight;
	m_ullCoinDay = accountLog.m_ullCoinDay;
	LogPrint("undo_account", "before operate:%s\n", ToString().c_str());
	return true;
}
void CAccount::ClearAccPos(int nCurHeight) {
	UpDateCoinDay(nCurHeight);
	m_ullCoinDay = 0;
}
uint64_t CAccount::GetAccountPos(int nCurHeight){
	UpDateCoinDay(nCurHeight);
	return m_ullCoinDay;
}
bool CAccount::UpDateCoinDay(int nCurHeight) {
	if(nCurHeight < m_nHeight){
		return false;
	}
	else if(nCurHeight == m_nHeight){
		return true;
	}
	else{
		m_ullCoinDay += m_ullValues * ((int64_t)nCurHeight-(int64_t)m_nHeight);
		m_nHeight = nCurHeight;
		if(m_ullCoinDay > GetMaxCoinDay(nCurHeight)) {
			m_ullCoinDay = GetMaxCoinDay(nCurHeight);
		}
		return true;
	}
}
uint64_t CAccount::GetRawBalance() {
	return m_ullValues;
}
Object CAccount::ToJosnObj() const
{
	Object obj;
	obj.push_back(Pair("Address",     m_cKeyID.ToAddress()));
	obj.push_back(Pair("KeyID",     m_cKeyID.ToString()));
	obj.push_back(Pair("RegID",     m_cRegID.ToString()));
	obj.push_back(Pair("PublicKey",  m_cPublicKey.ToString()));
	obj.push_back(Pair("MinerPKey",  m_cMinerPKey.ToString()));
	obj.push_back(Pair("Balance",     m_ullValues));
	obj.push_back(Pair("CoinDays", m_ullCoinDay/SysCfg().GetIntervalPos()/COIN));
	obj.push_back(Pair("UpdateHeight", m_nHeight));
	std::shared_ptr<CAccount> pNewAcct = GetNewInstance();
	pNewAcct->UpDateCoinDay(g_cChainActive.Tip()->m_nHeight);
	obj.push_back(Pair("CurCoinDays", pNewAcct->m_ullCoinDay/SysCfg().GetIntervalPos()/COIN));
	return obj;
}
string CAccount::ToString() const {
	string str;
	str += strprintf("regID=%s, keyID=%s, publicKey=%s, minerpubkey=%s, values=%ld updateHeight=%d coinDay=%lld\n",
	m_cRegID.ToString(), m_cKeyID.GetHex().c_str(), m_cPublicKey.ToString().c_str(), m_cMinerPKey.ToString().c_str(), m_ullValues, m_nHeight, m_ullCoinDay);
	return str;
}
bool CAccount::IsMoneyOverflow(uint64_t nAddMoney) {
	if (!MoneyRange(nAddMoney))
		return ERRORMSG("money:%lld too larger than MaxMoney");
	return true;
}

bool CAccount::OperateAccount(emOperType type, const uint64_t &value, const int nCurHeight) {
	LogPrint("op_account", "before operate:%s\n", ToString());

	if (!IsMoneyOverflow(value))
		return false;

	if (UpDateCoinDay(nCurHeight) < 0) {
		LogPrint("INFO", "call UpDateCoinDay failed: cur height less than update height\n");
		return false;
	}

	if (m_cKeyID == uint160()) {
		return ERRORMSG("operate account's keyId is 0 error");
//		assert(0);
	}
	if (!value)
		return true;
	switch (type) {
	case EM_ADD_FREE: {
		m_ullValues += value;
		if (!IsMoneyOverflow(m_ullValues))
			return false;
		break;
	}
	case EM_MINUS_FREE: {
		if (value > m_ullValues)
			return false;
		uint64_t remainCoinDay;
		if(nCurHeight<g_sBlockRemainCoinDayHeight)
		{
			remainCoinDay = m_ullCoinDay - value / m_ullValues * m_ullCoinDay;
		}
		else
		{
			arith_uint256 nCoinDay256=(arith_uint256)m_ullCoinDay;
			arith_uint256 llValues256=(arith_uint256)m_ullValues;
			arith_uint256 value256=(arith_uint256)value;
			assert((llValues256 - value256) >= 0);
			arith_uint256 remainCoinDay256=(nCoinDay256*(llValues256 - value256)) / llValues256;//币的持有量发生变化后，原来的计算不对，改为这个计算 。
			remainCoinDay = remainCoinDay256.GetLow64();
			//LogPrint("CGP", "\n nHeight=%d\n remainCoinDay=%d\n nCoinDay=%d\n llValues=%d\n value=%d\n",nHeight,remainCoinDay,nCoinDay,llValues,value);//CGPADD FOR TEST
		}

		if (m_ullCoinDay > m_ullValues * SysCfg().GetIntervalPos()) {
			if (remainCoinDay < m_ullValues * SysCfg().GetIntervalPos())
				remainCoinDay = m_ullValues * SysCfg().GetIntervalPos();
		}
		m_ullCoinDay = remainCoinDay;
		m_ullValues -= value;
		break;
	}
	default:
		return ERRORMSG("operate account type error!");
//		assert(0);
	}
	LogPrint("op_account", "after operate:%s\n", ToString());
	return true;
}
