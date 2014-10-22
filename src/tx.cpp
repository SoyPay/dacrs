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

static string txTypeArray[] = { "NULL_TXTYPE", "REG_ACCT_TX", "NORMAL_TX", "APPEAL_TX", "SECURE_TX", "FREEZE_TX",
		"REWARD_TX", "REG_SCRIPT_TX" };


CRegID::CRegID(string strRegID) {
	vRegID.clear();
	vRegID = ::ParseHex(strRegID);
}
CRegID::CRegID(uint32_t nHeight, uint16_t nIndex) {
	vRegID.clear();
	vRegID.insert(vRegID.end(), BEGIN(nHeight), END(nHeight));
	vRegID.insert(vRegID.end(), BEGIN(nIndex), END(nIndex));
}
string CRegID::ToString() const {
	return ::HexStr(vRegID);
}

bool CRegisterAccountTx::UpdateAccount(int nIndex, CAccountViewCache &view, CValidationState &state, CTxUndo &txundo,
		int nHeight, CTransactionCache &txCache, CContractScriptCache &scriptCache) {
	CSecureAccount sourceAccount;
	CRegID accountId(nHeight, nIndex);
	CKeyID keyId = pubKey.GetID();
	if (!view.GetAccount(keyId, sourceAccount))
		return state.DoS(100, ERROR("UpdateAccounts() : read source addr %s account info error", accountId.ToString()),
				UPDATE_ACCOUNT_FAIL, "bad-read-accountdb");

	sourceAccount.publicKey = pubKey;
	if (llFees > 0) {
		CFund fund(llFees);
		sourceAccount.OperateAccount(MINUS_FREE, fund);
	}
	if (!view.SaveAccountInfo(accountId.vRegID, keyId, sourceAccount)) {
		return state.DoS(100, ERROR("UpdateAccounts() : write source addr %s account info error", accountId.ToString()),
				UPDATE_ACCOUNT_FAIL, "bad-read-accountdb");
	}
	txundo.vAccountOperLog.push_back(sourceAccount.accountOperLog);
	return true;
}
bool CRegisterAccountTx::UndoUpdateAccount(int nIndex, CAccountViewCache &view, CValidationState &state,
		CTxUndo &txundo, int nHeight, CTransactionCache &txCache, CContractScriptCache &scriptCache) {
	//drop account
	CRegID accountId(nHeight, nIndex);
	CSecureAccount oldAccount;
	if (!view.GetAccount(accountId.vRegID, oldAccount))
		return state.DoS(100,
				ERROR("UpdateAccounts() : read secure account=%s info error", HexStr(accountId.vRegID).c_str()),
				UPDATE_ACCOUNT_FAIL, "bad-read-accountdb");
	CKeyID keyId;
	view.GetKeyId(accountId.vRegID, keyId);
	if (!oldAccount.IsEmptyValue()) {
		CPubKey empPubKey;
		oldAccount.publicKey = empPubKey;
		if (llFees > 0) {
			CAccountOperLog accountOperLog;
			if (!txundo.GetAccountOperLog(keyId, accountOperLog))
				return state.DoS(100, ERROR("UpdateAccounts() : read keyId=%s tx undo info error", keyId.GetHex()),
						UPDATE_ACCOUNT_FAIL, "bad-read-txundoinfo");
			oldAccount.UndoOperateAccount(accountOperLog);
		}
		view.SetAccount(keyId, oldAccount);
	} else {
		view.EraseAccount(keyId);
	}
	view.EraseKeyId(accountId.vRegID);
	return true;
}
bool CRegisterAccountTx::IsValidHeight(int nCurHeight, int nTxCacheHeight) const {
	if (nValidHeight > nCurHeight + nTxCacheHeight / 2)
		return false;
	if (nValidHeight < nCurHeight - nTxCacheHeight / 2)
		return false;
	return true;
}
bool CRegisterAccountTx::GetAddress(vector<CKeyID> &vAddr, CAccountViewCache &view) const {
	if (!pubKey.IsFullyValid()) {
		return false;
	}
	vAddr.push_back(pubKey.GetID());
	return true;
}
string CRegisterAccountTx::ToString(CAccountViewCache &view) const {
	string str;
	str += strprintf("txType=%s, hash=%s, ver=%d, pubkey=%s, llFees=%ld, keyid=%s, nValidHeight=%d\n",
	txTypeArray[nTxType],GetHash().ToString().c_str(), nVersion, HexStr(pubKey.begin(), pubKey.end()).c_str(), llFees, pubKey.GetID().GetHex(), nValidHeight);
	return str;
}
bool CRegisterAccountTx::CheckTransction(CValidationState &state, CAccountViewCache &view) {
	//check pubKey valid
	if (!pubKey.IsFullyValid()) {
		return state.DoS(100, ERROR("CheckTransaction() : register tx public key is invalid"), REJECT_INVALID,
				"bad-regtx-publickey");
	}

	//check signature script
	uint256 sighash = SignatureHash();
	if (!pubKey.Verify(sighash, signature))
		return state.DoS(100, ERROR("CheckTransaction() : register tx signature error "), REJECT_INVALID,
				"bad-regtx-signature");

	if (!MoneyRange(llFees))
		return state.DoS(100, ERROR("CheckTransaction() : register tx fee out of range"), REJECT_INVALID,
				"bad-regtx-fee-toolarge");
	return true;
}

bool CTransaction::UpdateAccount(int nIndex, CAccountViewCache &view, CValidationState &state, CTxUndo &txundo,
		int nHeight, CTransactionCache &txCache, CContractScriptCache &scriptCache) {
	CSecureAccount sourceAccount;
	CSecureAccount desAccount;
	if (!view.GetAccount(srcRegAccountId, sourceAccount))
		return state.DoS(100,
				ERROR("UpdateAccounts() : read source addr %s account info error", HexStr(srcRegAccountId)),
				UPDATE_ACCOUNT_FAIL, "bad-read-accountdb");

	if (desRegAccountId.size() == 6) {
		if (!view.GetAccount(desRegAccountId, desAccount))
			return state.DoS(100,
					ERROR("UpdateAccounts() : read des addr %s account info error", HexStr(desRegAccountId)),
					UPDATE_ACCOUNT_FAIL, "bad-read-accountdb");;
	} else {
		desAccount.keyID = CKeyID(uint160(desRegAccountId));
		view.GetAccount(desAccount.keyID, desAccount);
	}

	uint64_t minusValue = llFees + llValues;
	CFund minusFund(minusValue);
	sourceAccount.CompactAccount(nHeight - 1);
	if (!sourceAccount.OperateAccount(MINUS_FREE, minusFund))
		return state.DoS(100, ERROR("UpdateAccounts() : secure accounts insufficient funds"), UPDATE_ACCOUNT_FAIL,
				"bad-read-accountdb");
	uint64_t addValue = llValues;
	CFund addFund(FREEDOM_FUND, GetHash(), addValue, nHeight);
	desAccount.CompactAccount(nHeight - 1);
	desAccount.OperateAccount(ADD_FREE, addFund);
	vector<CSecureAccount> vSecureAccounts;
	vSecureAccounts.push_back(sourceAccount);
	vSecureAccounts.push_back(desAccount);
	if (!view.BatchWrite(vSecureAccounts))
		return state.DoS(100, ERROR("UpdateAccounts() : batch write secure accounts info error"), UPDATE_ACCOUNT_FAIL,
				"bad-read-accountdb");
	txundo.vAccountOperLog.push_back(sourceAccount.accountOperLog);
	txundo.vAccountOperLog.push_back(desAccount.accountOperLog);
	return true;
}
bool CTransaction::UndoUpdateAccount(int nIndex, CAccountViewCache &view, CValidationState &state, CTxUndo &txundo,
		int nHeight, CTransactionCache &txCache, CContractScriptCache &scriptCache) {
	CSecureAccount sourceAccount;
	CSecureAccount desAccount;
	if (!view.GetAccount(srcRegAccountId, sourceAccount))
		return state.DoS(100,
				ERROR("UpdateAccounts() : read source addr %s account info error", HexStr(srcRegAccountId)),
				UPDATE_ACCOUNT_FAIL, "bad-read-accountdb");

	if (desRegAccountId.size() == 20) {
		CKeyID keyId = uint160(desRegAccountId);
		if (!view.GetAccount(keyId, desAccount))
			return state.DoS(100,
					ERROR("UpdateAccounts() : read destination addr %s account info error", HexStr(desRegAccountId)),
					UPDATE_ACCOUNT_FAIL, "bad-read-accountdb");
	} else {
		if (!view.GetAccount(desRegAccountId, desAccount))
			return state.DoS(100,
					ERROR("UpdateAccounts() : read destination addr %s account info error", HexStr(desRegAccountId)),
					UPDATE_ACCOUNT_FAIL, "bad-read-accountdb");
	}
	CAccountOperLog accountOperLog;
	if (!txundo.GetAccountOperLog(sourceAccount.keyID, accountOperLog))
		return state.DoS(100,
				ERROR("UpdateAccounts() : read source keyid=%s undo info error", sourceAccount.keyID.GetHex()),
				UPDATE_ACCOUNT_FAIL, "bad-read-txundoinfo");
	sourceAccount.UndoOperateAccount(accountOperLog);
	if (!txundo.GetAccountOperLog(desAccount.keyID, accountOperLog))
		return state.DoS(100,
				ERROR("UpdateAccounts() : read destination keyid=%s tx undo info error", desAccount.keyID.GetHex()),
				UPDATE_ACCOUNT_FAIL, "bad-read-txundoinfo");
	desAccount.UndoOperateAccount(accountOperLog);
	vector<CSecureAccount> vSecureAccounts;
	vSecureAccounts.push_back(sourceAccount);
	vSecureAccounts.push_back(desAccount);

	if (!view.BatchWrite(vSecureAccounts))
		return state.DoS(100, ERROR("UpdateAccounts() : batch write secure accounts info error"), UPDATE_ACCOUNT_FAIL,
				"bad-read-accountdb");
	return true;
}
bool CTransaction::GetAddress(vector<CKeyID> &vAddr, CAccountViewCache &view) const {
	CKeyID srcKeyId;
	if (!view.GetKeyId(srcRegAccountId, srcKeyId))
		return false;

	CKeyID desKeyId;
	if (desRegAccountId.size() == 6) {
		if (!view.GetKeyId(desRegAccountId, desKeyId))
			return false;
	} else if (desRegAccountId.size() == 20) {
		memcpy(desKeyId.begin(), &desRegAccountId[0], 20);
	} else {
		return false;
	}

	vAddr.push_back(srcKeyId);
	vAddr.push_back(desKeyId);
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
	view.GetKeyId(srcRegAccountId, srcKeyId);
	if (desRegAccountId.size() == 20) {
		desKeyId = CKeyID(uint160(desRegAccountId));
		str += strprintf("txType=%s, hash=%s, nVersion=%d, srcAccountId=%s, llFees=%ld, llValues=%ld, desKeyId=%s, nValidHeight=%d\n",
		txTypeArray[nTxType], GetHash().ToString().c_str(), nVersion, HexStr(srcRegAccountId).c_str(), llFees, llValues, desKeyId.GetHex(), nValidHeight);
	} else if(desRegAccountId.size() == 6 ) {
		view.GetKeyId(desRegAccountId, srcKeyId);
		str += strprintf("txType=%s, hash=%s, nVersion=%d, srcAccountId=%s, srcKeyId=%s, llFees=%ld, llValues=%ld, desAccountId=%s, desKeyId=%s, nValidHeight=%d\n",
		txTypeArray[nTxType], GetHash().ToString().c_str(), nVersion, HexStr(srcRegAccountId).c_str(), srcKeyId.GetHex(), llFees, llValues, HexStr(desRegAccountId).c_str(), desKeyId.GetHex(), nValidHeight);
	}

	return str;
}
bool CTransaction::CheckTransction(CValidationState &state, CAccountViewCache &view) {
	//check source addr, destination addr
	CBitcoinAddress fromAddr, toAddr;
	if (srcRegAccountId.size() != 6) {
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
	if (!CheckSignScript(srcRegAccountId, sighash, signature, state, view)) {
		return state.DoS(100, ERROR("CheckTransaction() :CheckSignScript failed"), REJECT_INVALID,
				"bad-signscript-check");
	}

	if (desRegAccountId.size() == 20) {
		CSecureAccount acctDesInfo;
		if (view.GetAccount(desRegAccountId, acctDesInfo)) {
			return state.DoS(100,
					ERROR(
							"CheckTransaction() : normal tx des account have regested, destination addr must be account id"),
					REJECT_INVALID, "bad-normal-desaddr error");
		}
	}
	return true;
}

bool CAppealTransaction::UpdateAccount(int nIndex, CAccountViewCache &view, CValidationState &state, CTxUndo &txundo,
		int nHeight, CTransactionCache &txCache, CContractScriptCache &scriptCache) {
	vector<std::shared_ptr<CBaseTransaction> > vTx;
	std::shared_ptr<CBaseTransaction> preTx;
	if (!GetTransaction(preTx, preTxHash))
		return state.DoS(100,
				ERROR("UpdateAccounts() : AppealTransaction UpdateAccount txhash=%s get tx detail info error",
						preTxHash.GetHex()), UPDATE_ACCOUNT_FAIL, "get-tx-error");
	vTx.push_back(preTx);
	vector<uint256> relayTx = txCache.GetRelayTx(preTxHash);
	for (auto & item : relayTx) {
		std::shared_ptr<CBaseTransaction> pTx;
		if (!GetTransaction(pTx, item))
			return state.DoS(100,
					ERROR("UpdateAccounts() : AppealTransaction UpdateAccount txhash=%s get tx detail info error",
							preTxHash.GetHex()), UPDATE_ACCOUNT_FAIL, "get-tx-error");
		vTx.push_back(pTx);
	}
	vTx.push_back(GetNewInstance());
	CVmScriptRun vmRun;
	if (!vmRun.run(vTx,view))
		return state.DoS(100,
				ERROR("UpdateAccounts() : AppealTransaction UpdateAccount txhash=%s run script error",
						GetHash().GetHex()), UPDATE_ACCOUNT_FAIL, "run-script-error");
	vector<std::shared_ptr<CSecureAccount> > &vAccount = vmRun.GetNewAccont();
	for (auto & itemAccount : vAccount) {
		if (!view.SetAccount(itemAccount->keyID, *itemAccount))
			return state.DoS(100,
					ERROR("UpdateAccounts() : AppealTransaction Udateaccount write secure account info error"),
					UPDATE_ACCOUNT_FAIL, "bad-write-accountdb");
		txundo.vAccountOperLog.push_back((itemAccount->accountOperLog));
	}
	return true;
}
bool CAppealTransaction::UndoUpdateAccount(int nIndex, CAccountViewCache &view, CValidationState &state,
		CTxUndo &txundo, int nHeight, CTransactionCache &txCache, CContractScriptCache &scriptCache) {
	vector<CKeyID> vKeyId;
	if (!GetAddress(vKeyId, view))
		return state.DoS(100, ERROR("UpdateAccounts() : AppealTransaction undo updateaccount get key id error"),
				UPDATE_ACCOUNT_FAIL, "get-keyid-error");
	for (auto & keyId : vKeyId) {
		CSecureAccount secureAccount;
		if (!view.GetAccount(keyId, secureAccount)) {
			return state.DoS(100,
					ERROR("UpdateAccounts() : AppealTransaction undo updateaccount read keyid= %s info error",
							keyId.GetHex()), UPDATE_ACCOUNT_FAIL, "bad-read-accountdb");
		}
		CAccountOperLog accountOperLog;
		if (txundo.GetAccountOperLog(keyId, accountOperLog)) {
			secureAccount.UndoOperateAccount(accountOperLog);
			if (!view.SetAccount(keyId, secureAccount))
				return state.DoS(100,
						ERROR(
								"UpdateAccounts() : AppealTransaction undo updateaccount write accountId= %s account info error"),
						UPDATE_ACCOUNT_FAIL, "bad-read-accountdb");
		}
	}
	return true;
}
bool CAppealTransaction::GetAddress(vector<CKeyID> &vAddr, CAccountViewCache &view) const {
	CKeyID keyId;
	std::shared_ptr<CBaseTransaction> preTx;
	if (!GetTransaction(preTx, preTxHash))
		return false;
	vector<CKeyID> vPreTxAddr;
	if (!preTx->GetAddress(vPreTxAddr, view))
		return false;
	for (auto & item : vPreAcountIndex) {
		vAddr.push_back(vPreTxAddr[item]);
	}
	return true;
}
bool CAppealTransaction::IsValidHeight(int nCurHeight, int nTxCacheHeight) const {
	if (pTxCacheTip->IsContainTx(preTxHash)) {
		std::shared_ptr<CBaseTransaction> pBaseTx;
		if(!GetTransaction(pBaseTx, preTxHash))
			return ERROR("read pre tx from database error");
		return pBaseTx->IsValidHeight(nCurHeight, nTxCacheHeight);
	}
	return ERROR("can't find pre tx hash in txcache");

}
string CAppealTransaction::ToString(CAccountViewCache &view) const {
	string str;
	str += strprintf("txType=%s, hash=%s, ver=%d, vPreAcountIndex=%s, preTxHash=%s, llFees=%ld, vContract=%s\n",
	txTypeArray[nTxType], GetHash().ToString().c_str(), nVersion, HexStr(vPreAcountIndex).c_str(), preTxHash.GetHex(), llFees, HexStr(vContract).c_str());
	return str;
}
bool CAppealTransaction::CheckTransction(CValidationState &state, CAccountViewCache &view) {
	if (!MoneyRange(llFees)) {
		return state.DoS(100, ERROR("CheckTransaction() : appeal tx fee out of range"), REJECT_INVALID,
				"bad-appeal-fee-toolarge");
	}

	if (vPreAcountIndex.size() != signature.size()) {
		return state.DoS(100, ERROR("CheckTransaction() :vPre size not equal to sign size"), REJECT_INVALID,
				"bad-vpre-size ");
	}

	std::shared_ptr<CBaseTransaction> pPrevSecureTx = CreateNewEmptyTransaction(SECURE_TX);
	if (!GetTransaction(pPrevSecureTx, preTxHash)) {
		return state.DoS(100, ERROR("CheckTransaction() :GetTransaction failed"), REJECT_INVALID, "bad-get-tx");
	}

	unsigned char nIndex = 0;
	vector<unsigned char> accountID;
	uint256 sighash = SignatureHash();
	std::shared_ptr<CSecureTransaction> pSecureTx = dynamic_pointer_cast<CSecureTransaction>(pPrevSecureTx);
	for (int i = 0; i < vPreAcountIndex.size(); i++) {
		nIndex = vPreAcountIndex.at(i);
		if(nIndex >= pSecureTx->vArbitratorRegAccId.size()) {
			nIndex -= pSecureTx->vArbitratorRegAccId.size();
			accountID = pSecureTx->vRegAccountId.at(nIndex);
		} else {
			accountID = pSecureTx->vArbitratorRegAccId.at(nIndex);
		}

		if (!CheckSignScript(accountID, sighash, signature.at(i), state, view)) {
			return state.DoS(100, ERROR("CheckTransaction() :CheckSignScript failed"), REJECT_INVALID,
					"bad-signscript-check");
		}
	}

	//for VerifyDB checkblock return true
	if (pTxCacheTip->IsContainTx(GetHash())) {
		return true;
	}
	vector<std::shared_ptr<CBaseTransaction> > vTx;
	std::shared_ptr<CBaseTransaction> preTx;
	vTx.push_back(pSecureTx);
	vector<uint256> relayTx = pTxCacheTip->GetRelayTx(preTxHash);
	for (auto & itemTxHash : relayTx) {
		std::shared_ptr<CBaseTransaction> pRelayTx;
		if (!GetTransaction(pRelayTx, itemTxHash))
			return state.DoS(100,
					ERROR("CheckTransaction() : AppealTransaction txhash=%s get relay tx detail info error",
							itemTxHash.GetHex()), UPDATE_ACCOUNT_FAIL, "get-tx-error");
		vTx.push_back(pRelayTx);
	}
	vTx.push_back(GetNewInstance());
	CVmScriptRun vmRun;
	if (!vmRun.run(vTx,view))
		return state.DoS(100,
				ERROR("CheckTransaction() : AppealTransaction txhash=%s run script error",
						GetHash().GetHex()), UPDATE_ACCOUNT_FAIL, "run-script-error");
	return true;
}

bool CSecureTransaction::UpdateAccount(int nIndex, CAccountViewCache &view, CValidationState &state, CTxUndo &txundo,
		int nHeight, CTransactionCache &txCache, CContractScriptCache &scriptCache) {
	vector<std::shared_ptr<CBaseTransaction> > vTx;
	vTx.push_back(GetNewInstance());
	CVmScriptRun vmRun;
	if (!vmRun.run(vTx,view))
		return state.DoS(100,
				ERROR("UpdateAccounts() : SecureTransaction udateaccount txhash=%s run script error",
						GetHash().GetHex()), UPDATE_ACCOUNT_FAIL, "run-script-error");
	vector<std::shared_ptr<CSecureAccount> > &vAccount = vmRun.GetNewAccont();
	for (auto & itemAccount : vAccount) {
		LogPrint("INFO", "after run script securetx:%s\n", itemAccount->ToString());
		if (!view.SetAccount(itemAccount->keyID, *itemAccount))
		{
			return state.DoS(100,
					ERROR("UpdateAccounts() : SecureTransaction udateaccount write secure account info error"),
					UPDATE_ACCOUNT_FAIL, "bad-write-accountdb");
		}
		txundo.vAccountOperLog.push_back((itemAccount->accountOperLog));
	}
	return true;
}
bool CSecureTransaction::UndoUpdateAccount(int nIndex, CAccountViewCache &view, CValidationState &state,
		CTxUndo &txundo, int nHeight, CTransactionCache &txCache, CContractScriptCache &scriptCache) {
	vector<vector_unsigned_char>::iterator iterAccount = vRegAccountId.begin();
	for (; iterAccount != vRegAccountId.end(); ++iterAccount) {
		CSecureAccount secureAccount;
		if (!view.GetAccount(*iterAccount, secureAccount)) {
			return state.DoS(100,
					ERROR("UpdateAccounts() : SecureTransaction undo updateaccount read accountId= %s info error",
							HexStr(*iterAccount)), UPDATE_ACCOUNT_FAIL, "bad-read-accountdb");
		}
		CAccountOperLog accountOperLog;
		if (txundo.GetAccountOperLog(secureAccount.keyID, accountOperLog)) {
			secureAccount.UndoOperateAccount(accountOperLog);
			if (!view.SetAccount(*iterAccount, secureAccount))
				return state.DoS(100,
						ERROR(
								"UpdateAccounts() : SecureTransaction undo updateaccount write accountId= %s account info error"),
						UPDATE_ACCOUNT_FAIL, "bad-read-accountdb");
		}
	}
	return true;
}
bool CSecureTransaction::GetAddress(vector<CKeyID> &vAddr, CAccountViewCache &view) const {
	for (int i = 0; i < vArbitratorRegAccId.size(); ++i) {
		CKeyID keyId;
		if (!view.GetKeyId(vArbitratorRegAccId[i], keyId))
			return false;
		vAddr.push_back(keyId);
	}
	for (int i = 0; i < vRegAccountId.size(); ++i) {
		CKeyID keyId;
		if (!view.GetKeyId(vRegAccountId[i], keyId))
			return false;
		vAddr.push_back(keyId);
	}
	return true;
}
bool CSecureTransaction::IsValidHeight(int nCurHeight, int nTxCacheHeight) const {
	if (nValidHeight > nCurHeight + nTxCacheHeight / 2)
		return false;
	if (nValidHeight < max(0,nCurHeight - nTxCacheHeight / 2))
		return false;
	return true;
}
string CSecureTransaction::ToString(CAccountViewCache &view) const {
	string str;
	str += strprintf("txType=%s, hash=%s, ver=%d, regScriptId=%s, vArbitrator.size=%d, vAccountId.size=%d, llFees=%ld, vContract=%s, nValidHeight=%d\n",
	txTypeArray[nTxType], GetHash().ToString().c_str(), nVersion, HexStr(regScriptId).c_str(), vArbitratorRegAccId.size(), vRegAccountId.size(), llFees, HexStr(vContract).c_str(), nValidHeight);
	for (unsigned int i = 0; i < vArbitratorRegAccId.size(); ++i) {
		if (0 == i)
			str += "arbitrator account id: \n";
		str += "    " + HexStr(vArbitratorRegAccId[i]) + "\n";
	}
	for (unsigned int i = 0; i < vRegAccountId.size(); ++i) {
		if (0 == i)
			str += "transaction account id: \n";
		str += "    " + HexStr(vRegAccountId[i]) + "\n";
	}
	return str;
}
bool CSecureTransaction::CheckTransction(CValidationState &state, CAccountViewCache &view) {
	if (!MoneyRange(llFees)) {
		return state.DoS(100, ERROR("CheckTransaction() : secure tx fee out of range"), REJECT_INVALID,
				"bad-secure-fee-toolarge");
	}

	if (vRegAccountId.size() != vScripts.size() || vRegAccountId.size() < 2) {
		return state.DoS(100, ERROR("CheckTransaction() :vAccountID size error"), REJECT_INVALID, "bad-sec_vAcc-size");
	}

	vector<unsigned char> vscript;
	set<string> sob;
	if (!pContractScriptTip->GetScript(HexStr(regScriptId), vscript)) {
		return state.DoS(100, ERROR("CheckTransaction() :GetScript failed"), REJECT_INVALID, "bad-sec_regScriptId");
	}

	if (!pContractScriptTip->GetArbitrator(HexStr(regScriptId), sob)) {
		return state.DoS(100, ERROR("CheckTransaction() :GetArbitrator falied"), REJECT_INVALID, "bad-sec_regScriptId");
	}

	CSecureAccount secureAcc;
	for (auto& vregid : vArbitratorRegAccId) {
		if (!view.GetAccount(vregid, secureAcc)) {
			return state.DoS(100, ERROR("CheckTransaction() :unregister ID in vArbitratorAccId"), REJECT_INVALID,
					"bad-sec-vAr");
		}

		if (sob.end() == sob.find(HexStr(vregid))) {
			return state.DoS(100, ERROR("CheckTransaction() :unregister script ob RegID in vArbitratorAccId"),
					REJECT_INVALID, "bad-sec_vAr");
		}
	}

	int i = 0;
	uint256 signhash = SignatureHash();
	for (auto& vregid : vRegAccountId) {
		if (!CheckSignScript(vregid, signhash, vScripts[i], state, view)) {
			return state.DoS(100, ERROR("CheckTransaction() :CheckSignScript failed"), REJECT_INVALID,
					"bad-signscript-check");
		}
		i++;
	}

	//for VerifyDB checkblock return true
	if (pTxCacheTip->IsContainTx(GetHash())) {
		return true;
	}

	vector<std::shared_ptr<CBaseTransaction> > vTx;
	vTx.push_back(GetNewInstance());
	CVmScriptRun vmRun;
	if (!vmRun.run(vTx,view))
		return state.DoS(100,
				ERROR("CheckTransaction() : SecureTransaction txhash=%s run script error",
						GetHash().GetHex()), UPDATE_ACCOUNT_FAIL, "run-script-error");
	return true;
}

bool CFreezeTransaction::UpdateAccount(int nIndex, CAccountViewCache &view, CValidationState &state, CTxUndo &txundo,
		int nHeight, CTransactionCache &txCache, CContractScriptCache &scriptCache) {
	uint64_t minusValue = llFees;
	uint64_t freezeValue = llFreezeFunds;
	CSecureAccount secureAccount;
	if (!view.GetAccount(regAccountId, secureAccount)) {
		return state.DoS(100, ERROR("UpdateAccounts() : read source addr %s account info error", HexStr(regAccountId)),
				UPDATE_ACCOUNT_FAIL, "bad-read-accountdb");
	}
	secureAccount.CompactAccount(nHeight - 1);
	CFund minusFund(minusValue);
	if (!secureAccount.OperateAccount(MINUS_FREE, minusFund))
		return state.DoS(100, ERROR("UpdateAccounts() : secure accounts insufficient funds"), UPDATE_ACCOUNT_FAIL,
				"bad-read-accountdb");
	CFund selfFund(SELF_FREEZD_FUND, GetHash(), freezeValue, nUnfreezeHeight);
	if (!secureAccount.OperateAccount(ADD_SELF_FREEZD, selfFund))
		return state.DoS(100, ERROR("UpdateAccounts() : secure accounts insufficient funds"), UPDATE_ACCOUNT_FAIL,
				"bad-read-accountdb");
	if (!view.SetAccount(regAccountId, secureAccount))
		return state.DoS(100, ERROR("UpdateAccounts() : batch write secure account info error"), UPDATE_ACCOUNT_FAIL,
				"bad-read-accountdb");
	txundo.vAccountOperLog.push_back(secureAccount.accountOperLog);
	return true;
}
bool CFreezeTransaction::UndoUpdateAccount(int nIndex, CAccountViewCache &view, CValidationState &state,
		CTxUndo &txundo, int nHeight, CTransactionCache &txCache, CContractScriptCache &scriptCache) {
	CSecureAccount secureAccount;
	if (!view.GetAccount(regAccountId, secureAccount))
		return state.DoS(100, ERROR("UpdateAccounts() : read source addr %s account info error", HexStr(regAccountId)),
				UPDATE_ACCOUNT_FAIL, "bad-read-accountdb");
	CAccountOperLog accountOperLog;
	if (!txundo.GetAccountOperLog(secureAccount.keyID, accountOperLog))
		return state.DoS(100, ERROR("UpdateAccounts() : read keyid=%s undo info error", secureAccount.keyID.GetHex()),
				UPDATE_ACCOUNT_FAIL, "bad-read-txundoinfo");
	secureAccount.UndoOperateAccount(accountOperLog);
	if (!view.SetAccount(regAccountId, secureAccount))
		return state.DoS(100, ERROR("UpdateAccounts() : write secure account info error"), UPDATE_ACCOUNT_FAIL,
				"bad-read-accountdb");
	return true;
}
bool CFreezeTransaction::GetAddress(vector<CKeyID> &vAddr, CAccountViewCache &view) const {
	CKeyID keyId;
	if (!view.GetKeyId(regAccountId, keyId))
		return false;
	vAddr.push_back(keyId);
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
	txTypeArray[nTxType], GetHash().ToString().c_str(), nVersion, HexStr(regAccountId).c_str(), llFees, keyId.GetHex(), llFreezeFunds, nValidHeight, nUnfreezeHeight);
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

	if (!CheckSignScript(regAccountId, SignatureHash(), signature, state, view)) {
		return state.DoS(100, ERROR("CheckTransaction() :CheckSignScript failed"), REJECT_INVALID,
				"bad-signscript-check");
	}
	return true;
}

bool CRewardTransaction::UpdateAccount(int nIndex, CAccountViewCache &view, CValidationState &state, CTxUndo &txundo,
		int nHeight, CTransactionCache &txCache, CContractScriptCache &scriptCache) {
	CSecureAccount secureAccount;
	if (!view.GetAccount(account, secureAccount)) {
		return state.DoS(100, ERROR("UpdateAccounts() : read source addr %s account info error", HexStr(account)),
				UPDATE_ACCOUNT_FAIL, "bad-read-accountdb");
	}
	LogPrint("INFO", "before rewardtx confirm account:%s\n", secureAccount.ToString());
	secureAccount.ClearAccPos(GetHash(), nHeight - 1, Params().GetIntervalPos());
	CFund fund(REWARD_FUND, GetHash(), rewardValue, nHeight);
	secureAccount.OperateAccount(ADD_FREE, fund);
	LogPrint("INFO", "after rewardtx confirm account:%s\n", secureAccount.ToString());
	if (!view.SetAccount(account, secureAccount))
		return state.DoS(100, ERROR("UpdateAccounts() : write secure account info error"), UPDATE_ACCOUNT_FAIL,
				"bad-save-accountdb");
	txundo.vAccountOperLog.push_back(secureAccount.accountOperLog);
	return true;
}
bool CRewardTransaction::UndoUpdateAccount(int nIndex, CAccountViewCache &view, CValidationState &state,
		CTxUndo &txundo, int nHeight, CTransactionCache &txCache, CContractScriptCache &scriptCache) {
	if (account.size() != 6 && account.size() != 65) {
		return state.DoS(100,
				ERROR("UpdateAccounts() : account  %s error, either accountId 6 bytes, or pubkey 65 bytes",
						HexStr(account)), UPDATE_ACCOUNT_FAIL, "bad-read-accountdb");
	}
	CSecureAccount secureAccount;
	if (!view.GetAccount(account, secureAccount)) {
		return state.DoS(100, ERROR("UpdateAccounts() : read source addr %s account info error", HexStr(account)),
				UPDATE_ACCOUNT_FAIL, "bad-read-accountdb");
	}
	CAccountOperLog accountOperLog;
	if (!txundo.GetAccountOperLog(secureAccount.keyID, accountOperLog))
		return state.DoS(100, ERROR("UpdateAccounts() : read keyid=%s undo info error", secureAccount.keyID.GetHex()),
				UPDATE_ACCOUNT_FAIL, "bad-read-txundoinfo");
	secureAccount.UndoOperateAccount(accountOperLog);
	if (!view.SetAccount(account, secureAccount))
		return state.DoS(100, ERROR("UpdateAccounts() : write secure account info error"), UPDATE_ACCOUNT_FAIL,
				"bad-read-accountdb");
	return true;
}
bool CRewardTransaction::GetAddress(vector<CKeyID> &vAddr, CAccountViewCache &view) const {
	CKeyID keyId;
	if (account.size() == 6) {
		if (!view.GetKeyId(account, keyId))
			return false;
		vAddr.push_back(keyId);
	} else {
		CPubKey pubKey(account);
		if (!pubKey.IsFullyValid())
			return false;
		vAddr.push_back(pubKey.GetID());
	}
	return true;
}
string CRewardTransaction::ToString(CAccountViewCache &view) const {
	string str;
	CKeyID keyId;
	view.GetKeyId(account, keyId);
	str += strprintf("txType=%s, hash=%s, ver=%d, account=%s, keyid=%s, rewardValue=%ld\n", txTypeArray[nTxType], GetHash().ToString().c_str(), nVersion, HexStr(account).c_str(), keyId.GetHex(), rewardValue);
	return str;
}
bool CRewardTransaction::CheckTransction(CValidationState &state, CAccountViewCache &view) {
	return true;
}

bool CRegistScriptTx::UpdateAccount(int nIndex, CAccountViewCache &view, CValidationState &state, CTxUndo &txundo,
		int nHeight, CTransactionCache &txCache, CContractScriptCache &scriptCache) {
	LogPrint("INFO" ,"registscript UpdateAccount\n");
	CSecureAccount secureAccount;
	if (!view.GetAccount(regAccountId, secureAccount)) {
		return state.DoS(100, ERROR("UpdateAccounts() : read regist addr %s account info error", HexStr(regAccountId)),
				UPDATE_ACCOUNT_FAIL, "bad-read-accountdb");
	}

	uint64_t minusValue = llFees;
	if (minusValue > 0) {
		CFund fund(minusValue);
		secureAccount.OperateAccount(MINUS_FREE, fund);
		txundo.vAccountOperLog.push_back(secureAccount.accountOperLog);
		if (!view.SetAccount(regAccountId, secureAccount))
			return state.DoS(100, ERROR("UpdateAccounts() : write secure account info error"), UPDATE_ACCOUNT_FAIL,
					"bad-save-accountdb");
	}

	if (SCRIPT_ID == nFlag) {
		vector<unsigned char> vScript;
		if (!scriptCache.GetScript(HexStr(script), vScript)) {
			return state.DoS(100,
					ERROR("UpdateAccounts() : Get script id=%s error", HexStr(script.begin(), script.end())),
					UPDATE_ACCOUNT_FAIL, "bad-query-scriptdb");
		}
		if (0 == nIndex)
			return true;
		set<string> setArbitrator;
		scriptCache.GetArbitrator(HexStr(script), setArbitrator);
		if (setArbitrator.count(HexStr(regAccountId.begin(), regAccountId.end())))
			return state.DoS(100,
					ERROR("UpdateAccounts() : accountid %s have regested scriptid %s",
							HexStr(regAccountId.begin(), regAccountId.end()), HexStr(script.begin(), script.end())),
					UPDATE_ACCOUNT_FAIL, "bad-regest-script");
		setArbitrator.insert(HexStr(regAccountId.begin(), regAccountId.end()));
		if (!scriptCache.SetArbitrator(HexStr(script), setArbitrator)) {
			return state.DoS(100,
					ERROR("UpdateAccounts() : save script id %s error", HexStr(script.begin(), script.end())),
					UPDATE_ACCOUNT_FAIL, "bad-save-scriptdb");
		}
	} else if (SCRIPT_CONTENT == nFlag) {
		if (0 == nIndex)
			return true;
		CRegID scriptId(nHeight, nIndex);
		CContractScript contractScript;
		contractScript.scriptId = scriptId.vRegID;
		contractScript.scriptContent = script;
		contractScript.setArbitratorAccId.insert(HexStr(regAccountId.begin(), regAccountId.end()));
		if (!scriptCache.AddContractScript(HexStr(scriptId.vRegID), contractScript)) {
			return state.DoS(100,
					ERROR("UpdateAccounts() : save script id %s script info error", HexStr(scriptId.vRegID)),
					UPDATE_ACCOUNT_FAIL, "bad-save-scriptdb");
		}
	}
	return true;
}
bool CRegistScriptTx::UndoUpdateAccount(int nIndex, CAccountViewCache &view, CValidationState &state, CTxUndo &txundo,
		int nHeight, CTransactionCache &txCache, CContractScriptCache &scriptCache) {
	CSecureAccount secureAccount;
	if (!view.GetAccount(regAccountId, secureAccount)) {
		return state.DoS(100, ERROR("UpdateAccounts() : read regist addr %s account info error", HexStr(regAccountId)),
				UPDATE_ACCOUNT_FAIL, "bad-read-accountdb");
	}
	if (SCRIPT_ID == nFlag) {
		set<string> setArbitrator;
		scriptCache.GetArbitrator(HexStr(script), setArbitrator);
		setArbitrator.erase(HexStr(regAccountId.begin(), regAccountId.end()));
		if (!scriptCache.SetArbitrator(HexStr(script), setArbitrator)) {
			return state.DoS(100,
					ERROR("UpdateAccounts() : save script id %s error", HexStr(script.begin(), script.end())),
					UPDATE_ACCOUNT_FAIL, "bad-save-scriptdb");
		}
	} else if (SCRIPT_CONTENT == nFlag) {
		CRegID scriptId(nHeight, nIndex);
		if (!scriptCache.DeleteContractScript(HexStr(scriptId.vRegID))) {
			return state.DoS(100, ERROR("UpdateAccounts() : erase script id %s error", HexStr(scriptId.vRegID)),
					UPDATE_ACCOUNT_FAIL, "bad-save-scriptdb");
		}
	}

	CAccountOperLog accountOperLog;
	if (!txundo.GetAccountOperLog(secureAccount.keyID, accountOperLog))
		return state.DoS(100, ERROR("UpdateAccounts() : read keyid=%s undo info error", secureAccount.keyID.GetHex()),
				UPDATE_ACCOUNT_FAIL, "bad-read-txundoinfo");
	secureAccount.UndoOperateAccount(accountOperLog);

	if (!view.SetAccount(regAccountId, secureAccount))
		return state.DoS(100, ERROR("UpdateAccounts() : write secure account info error"), UPDATE_ACCOUNT_FAIL,
				"bad-save-accountdb");
	return true;
}
bool CRegistScriptTx::GetAddress(vector<CKeyID> &vAddr, CAccountViewCache &view) const {
	CKeyID keyId;
	if (!view.GetKeyId(regAccountId, keyId))
		return false;
	vAddr.push_back(keyId);
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
	txTypeArray[nTxType], GetHash().ToString().c_str(), nVersion, HexStr(regAccountId).c_str(), keyId.GetHex(), llFees, nValidHeight);
	return str;
}
bool CRegistScriptTx::CheckTransction(CValidationState &state, CAccountViewCache &view) {
	CSecureAccount  account;
	if(!view.GetAccount(regAccountId, account)) {
		return state.DoS(100, ERROR("CheckTransaction() : register script tx get registe account info error"), REJECT_INVALID,
				"bad-read-account-info");
	}
	if(!nFlag) {
		vector<unsigned char> vScriptContent;
		if(!pContractScriptTip->GetScript(HexStr(script), vScriptContent)) {
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
	if (!CheckSignScript(regAccountId, signhash, signature, state, view)) {
		return state.DoS(100, ERROR("CheckTransaction() :CheckSignScript failed"), REJECT_INVALID,
				"bad-signscript-check");
	}

	return true;
}

bool CFund::IsMergeFund(const int & nCurHeight, int &nMergeType) const {
	if (nCurHeight - nHeight > nMaxCoinDay / nTargetSpacing) {
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
	case INPUT_FREEZD_FUND:
	case OUTPUT_FREEZD_FUND:
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

string CFund::ToString() const {
	string str;
	string fundTypeArray[] = { "NULL_FUNDTYPE", "FREEDOM", "REWARD_FUND", "FREEDOM_FUND", "IN_FREEZD_FUND",
			"OUT_FREEZD_FUND", "SELF_FREEZD_FUND" };
	str += strprintf("            nType=%s, uTxHash=%d, value=%ld, nHeight=%d\n",
	fundTypeArray[nFundType], uTxHash.ToString().c_str(), value, nHeight);
	//LogPrint("INFO", "%s", str.c_str());
	return str;
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

void CSecureAccount::CompactAccount(int nCurHeight) {
	MergerFund(vRewardFund, nCurHeight);
	MergerFund(vInputFreeze, nCurHeight);
	MergerFund(vOutputFreeze, nCurHeight);
	MergerFund(vSelfFreeze, nCurHeight);
	MergerFund(vFreedomFund, nCurHeight);
}

void CSecureAccount::MergerFund(vector<CFund> &vFund, int nCurHeight) {
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
				vAddFunds.push_back(fund);
				vFreedomFund.push_back(fund);
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

void CSecureAccount::WriteOperLog(const COperFund &operLog) {
	accountOperLog.InsertOperateLog(operLog);
}

bool CSecureAccount::OperateAccount(OperType type, const CFund &fund, uint64_t* pOperatedValue) {
	if (keyID != accountOperLog.keyID)
		accountOperLog.keyID = keyID;

	if (!IsFundValid(type, fund))
		return false;

	if (!fund.value){
		if(pOperatedValue)
			*pOperatedValue = 0;
		return true;
	}

	bool bRet = true;
	uint64_t nOperateValue = 0;
	switch (type) {
	case ADD_FREE: {
		if (REWARD_FUND == fund.nFundType)
			vRewardFund.push_back(fund);
		else
			vFreedomFund.push_back(fund);

		WriteOperLog(ADD_FUND, fund);
		break;
	}

	case MINUS_FREE: {
		bRet = MinusFree(fund, nOperateValue);
		break;
	}

	case ADD_SELF_FREEZD: {
		bRet = MinusFree(fund, nOperateValue);
		if (!bRet)
			break;

		vSelfFreeze.push_back(fund);
		WriteOperLog(ADD_FUND, fund);
		break;
	}

	case ADD_INPUT_FREEZD: {
		vInputFreeze.push_back(fund);
		WriteOperLog(ADD_FUND, fund);
		break;
	}

	case MINUS_FREE_TO_OUTPUT: {
		bRet = MinusFreeToOutput(fund);
		break;
	}

	case MINUS_FREE_OR_SELF:{
		bRet = MinusFreeOrSelf(fund,nOperateValue);
		break;
	}

	case MINUS_OUTPUT: {
		bRet = MinusInputOutput(vOutputFreeze, fund, nOperateValue);
		break;
	}

	case MINUS_OUTPUT_OR_FREE: {
		bRet = MinusFund(vOutputFreeze, fund, nOperateValue);
		break;
	}

	case MINUS_OUTPUT_OR_FREE_OR_SELF: {
		bRet = MinusFundEx(vOutputFreeze, fund, nOperateValue);
		break;
	}

	case MINUS_INPUT: {
		bRet = MinusInputOutput(vInputFreeze, fund, nOperateValue);
		break;
	}

	case MINUS_INPUT_OR_FREE: {
		bRet = MinusFund(vInputFreeze, fund, nOperateValue);
		break;
	}

	case MINUS_INPUT_OR_FREE_OR_SELF: {
		bRet = MinusFundEx(vInputFreeze, fund, nOperateValue);
		break;
	}

	default:
		bRet = false;
		break;
	}

	if (pOperatedValue) {
		*pOperatedValue = nOperateValue;
	}

	return bRet;
}

bool CSecureAccount::MinusFree(const CFund &fund, uint64_t& nOperateValue, bool bLastMinus) {
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
		vOperFund.clear();
		vOperFund.insert(vOperFund.end(), vFreedomFund.begin(), iterFound + 1);

		COperFund operLog(MINUS_FUND, vOperFund);
		WriteOperLog(operLog);

		uint64_t remainValue = nCandidateValue - fund.value;
		CFund fundAdd(*iterFound);
		vFreedomFund.erase(vFreedomFund.begin(), iterFound + 1);
		if (remainValue > 0) {
			fundAdd.value = remainValue;
			WriteOperLog(ADD_FUND, fundAdd);
			vFreedomFund.push_back(fundAdd);
		}
		nOperateValue = fund.value;
		return true;

	} else {
		bool bRet = true;
		CFund freedom;
		freedom.nFundType = FREEDOM;
		if (llValues < fund.value - nCandidateValue) {
			nOperateValue = llValues + nCandidateValue;
			freedom.value = llValues;
			llValues = 0;

			if (bLastMinus)
				bRet = false;
		} else {
			nOperateValue = fund.value;
			freedom.value = fund.value - nCandidateValue;
			llValues -= fund.value - nCandidateValue;
		}

		vOperFund.clear();
		vOperFund.insert(vOperFund.end(), vFreedomFund.begin(), vFreedomFund.end());
		vFreedomFund.clear();
		vOperFund.push_back(freedom);
		COperFund operLog(MINUS_FUND, vOperFund);
		WriteOperLog(operLog);

		return bRet;
	}

}

bool CSecureAccount::UndoOperateAccount(const CAccountOperLog & accountOperLog) {
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
					vFreedomFund.erase(remove(vFreedomFund.begin(), vFreedomFund.end(), *iterFund), vFreedomFund.end());
				} else if (MINUS_FUND == iterOperFundLog->operType)
					vFreedomFund.push_back(*iterFund);
				break;
			case INPUT_FREEZD_FUND:
				if (ADD_FUND == iterOperFundLog->operType)
					vInputFreeze.erase(remove(vInputFreeze.begin(), vInputFreeze.end(), *iterFund), vInputFreeze.end());
				else if (MINUS_FUND == iterOperFundLog->operType)
					vInputFreeze.push_back(*iterFund);
				break;
			case OUTPUT_FREEZD_FUND:
				if (ADD_FUND == iterOperFundLog->operType)
					vOutputFreeze.erase(remove(vOutputFreeze.begin(), vOutputFreeze.end(), *iterFund),
							vOutputFreeze.end());
				else if (MINUS_FUND == iterOperFundLog->operType)
					vOutputFreeze.push_back(*iterFund);
				break;
			case SELF_FREEZD_FUND:
				if (ADD_FUND == iterOperFundLog->operType)
					vSelfFreeze.erase(remove(vSelfFreeze.begin(), vSelfFreeze.end(), *iterFund), vSelfFreeze.end());
				else if (MINUS_FUND == iterOperFundLog->operType)
					vSelfFreeze.push_back(*iterFund);
				break;
			default:
				return false;
			}
		}
	}

	return true;
}

//caculate pos
void CSecureAccount::ClearAccPos(uint256 hash, int prevBlockHeight, int nIntervalPos) {
	int days = 0;
	uint64_t money = 0;
	money = llValues;
	{
		COperFund acclog;
		acclog.operType = MINUS_FUND;
		{
			CFund fund(FREEDOM, 0, llValues, 0);
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
			COperFund acclog;
			acclog.operType = ADD_FUND;
			CFund fund(FREEDOM_FUND, hash, money, prevBlockHeight + 1);
			vFreedomFund.push_back(fund);
			acclog.vFund.push_back(fund);
			WriteOperLog(acclog);
		}
	}
}

//caculate pos
uint64_t CSecureAccount::GetSecureAccPos(int prevBlockHeight) const {
	uint64_t accpos = 0;
	int days = 0;

	accpos = llValues * 30;
	for (const auto &freeFund :vFreedomFund) {

		int nIntervalPos = Params().GetIntervalPos();
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


uint64_t CSecureAccount::GetMatureAmount(int nCurHeight) {
	CompactAccount(nCurHeight);
	uint64_t balance = 0;

	for(auto &fund:vRewardFund) {
		balance += fund.value;
	}
	return balance;
}

uint64_t CSecureAccount::GetForzenAmount(int nCurHeight) {
	CompactAccount(nCurHeight);
	uint64_t balance = 0;

	for(auto &fund:vInputFreeze) {
		balance += fund.value;
	}
	for(auto &fund:vOutputFreeze) {
		balance += fund.value;
	}
	for(auto &fund:vSelfFreeze) {
		balance += fund.value;
	}
	return balance;
}

uint64_t CSecureAccount::GetBalance(int nCurHeight) {
	CompactAccount(nCurHeight);
	uint64_t balance = llValues;

	for(auto &fund:vFreedomFund) {
		balance += fund.value;
	}
	return balance;
}

uint256 CSecureAccount::BuildMerkleTree(int prevBlockHeight) const {
	vector<uint256> vMerkleTree;
	vMerkleTree.clear();

	for (const auto &freeFund : vFreedomFund) {
		//at least larger than 100 height
		//if (prevBlockHeight < freeFund.confirmHeight + 100) {
		vMerkleTree.push_back(freeFund.uTxHash);
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

string CSecureAccount::ToString() const {
	string str;
	str += strprintf("keyID=%s, publicKey=%d, values=%ld\n",
	HexStr(keyID).c_str(), HexStr(publicKey).c_str(), llValues);
	for (unsigned int i = 0; i < vRewardFund.size(); ++i) {
		str += "    " + vRewardFund[i].ToString() + "\n";
	}
	for (unsigned int i = 0; i < vFreedomFund.size(); ++i) {
		str += "    " + vFreedomFund[i].ToString() + "\n";
	}
	for (unsigned int i = 0; i < vInputFreeze.size(); ++i) {
		str += "    " + vInputFreeze[i].ToString() + "\n";
	}
	for (unsigned int i = 0; i < vOutputFreeze.size(); ++i) {
		str += "    " + vOutputFreeze[i].ToString() + "\n";
	}
	for (unsigned int i = 0; i < vSelfFreeze.size(); ++i) {
		str += "    " + vSelfFreeze[i].ToString() + "\n";
	}
	return str;
}

bool CSecureAccount::MinusFreeToOutput(const CFund& fund) {
	vector<CFund>::iterator it = vFreedomFund.begin();
	for (; it != vFreedomFund.end(); it++) {
		if (it->uTxHash == fund.uTxHash) {
			break;
		}
	}

	if (it == vFreedomFund.end()) {
		return false;
	}

	CFund fundOutput(*it);
	fundOutput.nFundType = OUTPUT_FREEZD_FUND;
	vOutputFreeze.push_back(fundOutput);

	WriteOperLog(MINUS_FUND, *it);
	WriteOperLog(ADD_FUND, fundOutput);
	vFreedomFund.erase(it);

	return true;
}

void CSecureAccount::WriteOperLog(AccountOper emOperType, const CFund &fund) {
	vector<CFund> vFund;
	vFund.push_back(fund);
	COperFund operLog(emOperType, vFund);
	WriteOperLog(operLog);
}

bool CSecureAccount::MinusInputOutput(vector<CFund>& vFund, const CFund& fund, uint64_t& nOperateValue,
		bool bLastMinus) {
	vector<CFund>::iterator it = vFund.begin();
	for (; it != vFund.end(); it++) {
		if (it->uTxHash == fund.uTxHash) {
			break;
		}
	}

	if (it == vFund.end()) {
		return false;
	}

	if (fund.value >= it->value) {
		uint64_t nFundValue = it->value;
		nOperateValue = nFundValue;
		WriteOperLog(MINUS_FUND, *it);
		vFund.erase(it);

		if (fund.value > nFundValue) {
			if (bLastMinus)
				return false;
			else
				return true;
		} else
			return true;
	} else {
		nOperateValue = fund.value;
		WriteOperLog(MINUS_FUND, *it);

		CFund logfund(*it);
		logfund.value = it->value - fund.value;
		WriteOperLog(ADD_FUND, logfund);
		vFund.erase(it);
		vFund.push_back(logfund);
		return true;
	}
}

bool CSecureAccount::MinusFund(vector<CFund>& vFund, const CFund& fund, uint64_t& nOperateValue, bool bLastMinus) {
	uint64_t nValueSpecific = 0;
	if (MinusInputOutput(vFund, fund, nValueSpecific, false)) {
		if (fund.value > nValueSpecific) {
			CFund fundRemain(fund);
			fundRemain.value = fund.value - nValueSpecific;

			uint64_t nValueFree = 0;
			bool bRet = MinusFree(fundRemain, nValueFree, bLastMinus);
			nOperateValue = nValueSpecific + nValueFree;
			return bRet;
		}

		nOperateValue = nValueSpecific;
		return true;

	} else {
		return false;
	}
}

bool CSecureAccount::MinusFundEx(vector<CFund>& vFund, const CFund& fund, uint64_t& nOperateValue) {
	uint64_t nValue = 0;
	if (MinusFund(vFund, fund, nValue, false)) {
		if (fund.value > nValue) {
			CFund fundRemain(fund);
			fundRemain.value = fund.value - nValue;

			uint64_t nValueSelf = 0;
			bool bRet = MinusSelf(fundRemain, nValueSelf);
			nOperateValue = nValue + nValueSelf;
			return bRet;
		}

		nOperateValue = nValue;
		return true;
	} else {
		return false;
	}
}

bool CSecureAccount::MinusSelf(const CFund &fund, uint64_t& nOperateValue) {
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
		nOperateValue = fund.value;
		vOperFund.clear();
		vOperFund.insert(vOperFund.end(), vSelfFreeze.begin(), iterFound + 1);
		COperFund operLog(MINUS_FUND, vOperFund);
		WriteOperLog(operLog);

		uint64_t remainValue = nCandidateValue - fund.value;
		CFund fundAdd(*iterFound);
		vSelfFreeze.erase(vSelfFreeze.begin(), iterFound + 1);
		if (remainValue > 0) {
			fundAdd.value = remainValue;
			WriteOperLog(ADD_FUND, fundAdd);
			vSelfFreeze.push_back(fundAdd);
		}

		return true;
	} else {
		nOperateValue = nCandidateValue;
		vOperFund.clear();
		vOperFund.insert(vOperFund.end(), vSelfFreeze.begin(), vSelfFreeze.end());
		vSelfFreeze.clear();
		COperFund operLog(MINUS_FUND, vOperFund);
		WriteOperLog(operLog);
		return false;
	}
}

bool CSecureAccount::MinusFreeOrSelf(const CFund& fund, uint64_t& nOperateValue) {
	uint64_t nValueFree = 0;
	if (MinusFree(fund, nValueFree, false)) {
		if (fund.value > nValueFree) {
			CFund fundRemain(fund);
			fundRemain.value = fund.value - nValueFree;

			uint64_t nValueSelf = 0;
			bool bRet = MinusSelf(fundRemain, nValueSelf);
			nOperateValue = nValueFree + nValueSelf;
			return bRet;
		} else {
			nOperateValue = nValueFree;
			return true;
		}
	} else {
		return false;
	}
}

bool CSecureAccount::IsFundValid(OperType type, const CFund& fund) {
	switch (type) {
	case ADD_FREE:
		if (!IsFundValid(fund))
			return false;
		if (REWARD_FUND != fund.nFundType && FREEDOM_FUND != fund.nFundType)
			return false;
		if (!IsMoneyOverflow(fund.value))
			return false;
		break;

	case ADD_SELF_FREEZD:
		if (!IsFundValid(fund))
			return false;
		if (SELF_FREEZD_FUND != fund.nFundType)
			return false;
		if (!IsMoneyOverflow(fund.value))
			return false;
		break;

	case ADD_INPUT_FREEZD:
		if (!IsFundValid(fund))
			return false;
		if (INPUT_FREEZD_FUND != fund.nFundType)
			return false;
		if (!IsMoneyOverflow(fund.value))
			return false;
		break;

	case MINUS_OUTPUT:
	case MINUS_OUTPUT_OR_FREE:
	case MINUS_OUTPUT_OR_FREE_OR_SELF:
		if (!IsHashValidInFund(vOutputFreeze, fund))
			return false;
		break;

	case MINUS_INPUT:
	case MINUS_INPUT_OR_FREE:
	case MINUS_INPUT_OR_FREE_OR_SELF:
		if (!IsHashValidInFund(vInputFreeze, fund))
			return false;
		break;

	default:
		break;
	}

	return true;
}

bool CSecureAccount::IsHashValidInFund(const vector<CFund>& vFund, const CFund& fund) {
	vector<CFund>::const_iterator it = vFund.begin();
	for (; it != vFund.end(); it++) {
		if (it->uTxHash == fund.uTxHash)
			return true;
	}

	return false;
}

bool CSecureAccount::IsFundValid(const CFund& fund) {
	if (fund.nFundType < FREEDOM || fund.nFundType >= NULL_FUNDTYPE)
		return false;

	if (!fund.uTxHash)		//|| fund.nHeight != chainActive.Tip()->nHeight + 1)
		return false;

	return true;
}

bool CSecureAccount::IsMoneyOverflow(uint64_t nAddMoney) {
	if (!MoneyRange(nAddMoney))
		return false;

	uint64_t nTotalMoney = 0;
	nTotalMoney = GetVecMoney(vFreedomFund)+GetVecMoney(vRewardFund)+GetVecMoney(vInputFreeze)\
			+GetVecMoney(vOutputFreeze)+GetVecMoney(vSelfFreeze)+llValues+nAddMoney;
	return MoneyRange(static_cast<int64_t>(nTotalMoney) );
}

uint64_t CSecureAccount::GetVecMoney(const vector<CFund>& vFund){
	uint64_t nTotal = 0;
	for(vector<CFund>::const_iterator it = vFund.begin();it != vFund.end();it++){
		nTotal += it->value;
	}

	return nTotal;
}

CFund& CSecureAccount::FindFund(const vector<CFund>& vFund, const uint256 &hash)
{
	CFund vret;
	for(vector<CFund>::const_iterator it = vFund.begin();it != vFund.end();it++){
		if(it->uTxHash == hash)
		{
			vret = *it;
		}
	}
	return vret;
}

bool CContractScript::IsContainScript(const vector_unsigned_char &scriptContent,
		const map<string, CContractScript> &mapScript) const {
	map<string, CContractScript>::const_iterator iterScript = mapScript.begin();
	for (; iterScript != mapScript.end(); ++iterScript) {
		if (scriptContent == iterScript->second.scriptContent)
			return true;
	}
	return false;
}

CTransactionCache::CTransactionCache(CTransactionCacheDB *pTxCacheDB) {
	base = pTxCacheDB;
}

bool CTransactionCache::AddBlockToCache(const CBlock &block) {
	vector<uint256> vTxHash;
	vTxHash.clear();
	for (auto &ptx : block.vptx) {
		vTxHash.push_back(ptx->GetHash());
		if (ptx->nTxType == APPEAL_TX) {
			CAppealTransaction *pAppealTx = (CAppealTransaction*) ptx.get();
			if (mapTxHashCacheByPrev.count(pAppealTx->preTxHash)) {
				mapTxHashCacheByPrev[pAppealTx->preTxHash].push_back(pAppealTx->GetHash());
			} else {
				LogPrint("INFO", "add appeal tx to map:%s\n", pAppealTx->GetHash().GetHex());
				vector<uint256> vAppealTx;
				vAppealTx.clear();
				vAppealTx.push_back(pAppealTx->GetHash());
				mapTxHashCacheByPrev[pAppealTx->preTxHash] = vAppealTx;
			}
		}
	}
	LogPrint("INFO", "mapTxHashByBlockHash size:%d\n", mapTxHashByBlockHash.size());
	mapTxHashByBlockHash.insert(make_pair(block.GetHash(), vTxHash));
	for(auto &item : mapTxHashByBlockHash) {
		LogPrint("INFO", "blockhash:%s\n", item.first.GetHex());
		for(auto &txHash : item.second)
			LogPrint("INFO", "txhash:%s\n", txHash.GetHex());
	}
	for(auto &item : mapTxHashCacheByPrev) {
		LogPrint("INFO", "prehash:%s\n", item.first.GetHex());
		for(auto &relayTx : item.second)
			LogPrint("INFO", "relay tx hash:%s\n", relayTx.GetHex());
	}
	return true;
}

bool CTransactionCache::DeleteBlockFromCache(const CBlock &block) {
	for (auto &ptx : block.vptx) {
		vector<uint256> vTxHash;
		vTxHash.clear();
		mapTxHashByBlockHash[block.GetHash()] = vTxHash;
		if (ptx->nTxType == APPEAL_TX) {
			CAppealTransaction *pAppealTx = (CAppealTransaction*) ptx.get();
			if (mapTxHashCacheByPrev.count(pAppealTx->preTxHash)) {
				vector<uint256>::iterator itr = find(mapTxHashCacheByPrev[pAppealTx->preTxHash].begin(),
						mapTxHashCacheByPrev[pAppealTx->preTxHash].end(), ptx->GetHash());
				mapTxHashCacheByPrev[pAppealTx->preTxHash].erase(itr);
			}
		}
	}
	return true;
}

bool CTransactionCache::IsContainTx(const uint256 & txHash) {
	for(auto & item : mapTxHashByBlockHash) {
		vector<uint256>::iterator it = find(item.second.begin(), item.second.end(), txHash);
		if(it != item.second.end())
			return true;
	}
	return false;
}

vector<uint256> CTransactionCache::GetRelayTx(const uint256 & txHash) {
	return mapTxHashCacheByPrev[txHash];
}

const map<uint256, vector<uint256> > &CTransactionCache::GetRelayTx(void) const {
	return mapTxHashCacheByPrev;
}

const map<uint256, vector<uint256> > &CTransactionCache::GetTxHashCache(void) const {
	return mapTxHashByBlockHash;
}

bool CTransactionCache::Flush() {
	bool bRet = base->Flush(mapTxHashByBlockHash, mapTxHashCacheByPrev);
//	if (bRet) {
//		mapTxHashByBlockHash.clear();
//		mapTxHashCacheByPrev.clear();
//	}
	return bRet;
}

void CTransactionCache::AddTxHashCache(const uint256 & blockHash, const vector<uint256> &vTxHash) {
	mapTxHashByBlockHash[blockHash] = vTxHash;
}

void CTransactionCache::AddRelayTx(const uint256 preTxHash, const vector<uint256> &vTxHash) {
	mapTxHashCacheByPrev[preTxHash].clear();
	mapTxHashCacheByPrev[preTxHash].assign(vTxHash.begin(), vTxHash.end());
}

bool CTransactionCache::LoadTransaction() {
	return base->LoadTransaction(mapTxHashByBlockHash, mapTxHashCacheByPrev);
}

void CTransactionCache::Clear() {
	mapTxHashByBlockHash.clear();
	mapTxHashCacheByPrev.clear();
}

CContractScriptCache::CContractScriptCache(CScriptDB * pScriptDB) {
	base = pScriptDB;
}

bool CContractScriptCache::GetContractScript(const string &strKey, CContractScript &contractScript) {
	if(mapScript.count(strKey)) {
		contractScript = mapScript[strKey];
		return true;
	}
	else {
		if(base->Exists(strKey)) {
			if(!base->GetContractScript(ParseHex(strKey), contractScript))
				return false;
			mapScript[strKey] = contractScript;
			return true;
		}
	}
	return false;
}

bool CContractScriptCache::IsContainContractScript(const string &strKey) {
	if(mapScript.count(strKey) || base->Exists(strKey)) {
		return true;
	}
	return false;
}

bool CContractScriptCache::AddContractScript(const string &strKey, const CContractScript &contractScript) {
	mapScript[strKey] = contractScript;
	return true;
}

bool CContractScriptCache::DeleteContractScript(const string &strKey) {
	if(mapScript.count(strKey)) {
		mapScript[strKey].scriptId.clear();
	}
	else {
		CContractScript contractScript;
		if(base->GetContractScript(ParseHex(strKey), contractScript)) {
			contractScript.scriptId.clear();
			mapScript[strKey] = contractScript;
		}
	}
	return true;
}

bool CContractScriptCache::LoadRegScript() {
	return base->LoadRegScript(mapScript);
}

bool CContractScriptCache::Flush() {
	return base->Flush(mapScript);
}

map<string, CContractScript> &CContractScriptCache::GetScriptCache() {
	return mapScript;
}

bool CContractScriptCache::GetScript(const string &strKey, vector<unsigned char> &vscript) {
	if(mapScript.count(strKey)) {
		vscript = mapScript[strKey].scriptContent;
		return true;
	}
	else {
		if(base->Exists(strKey)) {
			CContractScript contractScript;
			if(base->GetContractScript(ParseHex(strKey), contractScript))
			{
				mapScript[strKey] = contractScript;
				vscript = contractScript.scriptContent;
				return true;
			}
		}
	}
	return false;
}

bool CContractScriptCache::GetArbitrator(const string &strKey, set<string> &setArbId) {
	if(mapScript.count(strKey)) {
		setArbId = mapScript[strKey].setArbitratorAccId;
		return true;
	}
	else {
		if(base->Exists(strKey)) {
			CContractScript contractScript;
			if(base->GetContractScript(ParseHex(strKey), contractScript))
			{
				mapScript[strKey] = contractScript;
				setArbId = contractScript.setArbitratorAccId;
				return true;
			}
		}
	}
	return false;
}

bool CContractScriptCache::SetArbitrator(const string &strKey, const set<string> &setArbitrator) {
	if(mapScript.count(strKey)) {
		mapScript[strKey].setArbitratorAccId = setArbitrator;
		return true;
	}
//	else {
//		if(base->Exists(strKey)) {
//			CContractScript contractScript;
//			if(base->GetContractScript(ParseHex(strKey), contractScript))
//			{
//				contractScript.setArbitratorAccId = setArbitrator;
//				mapScript[strKey] = contractScript;
//				return true;
//			}
//		}
//	}
	return false;
}

