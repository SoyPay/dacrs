/*
 * ScriptCheck.cpp
 *
 *  Created on: Sep 2, 2014
 *      Author: ranger.shi
 */
#include "VmScriptRun.h"
#include "tx.h"
#include "util.h"
#include <boost/foreach.hpp>
CVmScriptRun::CVmScriptRun() {
	RawAccont.clear();
	NewAccont.clear();
	height = 0;
	;
}
vector<shared_ptr<CAccount> > &CVmScriptRun::GetRawAccont() {
	return RawAccont;
}
vector<shared_ptr<CAccount> > &CVmScriptRun::GetNewAccont() {
	return NewAccont;
}

bool CVmScriptRun::intial(shared_ptr<CBaseTransaction> & Tx, CAccountViewCache& view, int nheight) {

	listTx = Tx;
	height = nheight;
	vector<unsigned char> vScript;

	if (Tx.get()->nTxType != CONTRACT_TX) {
		LogPrint("vm", "%s\r\n", "err param");
		return false;
	}

	CContractTransaction* secure = static_cast<CContractTransaction*>(Tx.get());
	if (pScriptDBTip->GetScript(secure->scriptRegId, vScript)) {
		CDataStream stream(vScript, SER_DISK, CLIENT_VERSION);
		try {
			stream >> vmScript;
		} catch (exception& e) {
			throw runtime_error("CVmScriptRun::intial() Unserialize to vmScript error:" + string(e.what()));
		}
	}

	if (vmScript.IsValid() == false)
		return false;

	for (auto& tx : secure->vAccountRegId) {
		auto tem = make_shared<CAccount>();
		view.GetAccount(tx, *tem.get());
		RawAccont.push_back(tem);
	}

	vector<unsigned char> strContract;
	pMcu = make_shared<CVir8051>(vmScript.Rom, strContract);
	return true;
}

CVmScriptRun::~CVmScriptRun() {

}
tuple<bool, uint64_t, string> CVmScriptRun:: run(shared_ptr<CBaseTransaction>& Tx, CAccountViewCache& view, int nheight,
		uint64_t nBurnFactor) {

	CContractTransaction* tx = static_cast<CContractTransaction*>(Tx.get());
	int maxstep = tx->llFees/nBurnFactor;
	tuple<bool, uint64_t, string> mytuple;
	if (!intial(Tx, view, nheight)) {
		mytuple = std::make_tuple (false, 0, string("VmScript inital Failed\n"));
		return mytuple;
	}
	int step = pMcu.get()->run(maxstep);
	if (!step) {
		mytuple = std::make_tuple (false, 0, string("VmScript run Failed\n"));
		return mytuple;
	}
	shared_ptr<vector<unsigned char>> retData = pMcu.get()->GetRetData();
	CDataStream Contractstream(*retData.get(), SER_DISK, CLIENT_VERSION);
	vector<CVmOperate> retvmcode;
	Contractstream >> retvmcode;

	if (!CheckOperate(retvmcode)) {
		mytuple = std::make_tuple (false, 0, string("VmScript CheckOperate Failed \n"));
		return mytuple;
	}
	if (!OpeatorAccount(retvmcode, view)) {
		mytuple = std::make_tuple (false, 0, string("VmScript OpeatorSecureAccount Failed\n"));
		return mytuple;
	}
	uint64_t spend = step*nBurnFactor;
	mytuple = std::make_tuple (true, spend, string("VmScript Sucess\n"));
	return mytuple;
}

shared_ptr<CAccount> CVmScriptRun::GetNewAccount(shared_ptr<CAccount>& vOldAccount) {
	if (NewAccont.size() == 0)
		return NULL;
	vector<shared_ptr<CAccount> >::iterator Iter;
	for (Iter = NewAccont.begin(); Iter != NewAccont.end(); Iter++) {
		shared_ptr<CAccount> temp = *Iter;
		if (temp.get()->keyID == vOldAccount.get()->keyID) {
			NewAccont.erase(Iter);
			return temp;
		}
	}
	return NULL;
}
shared_ptr<CAccount> CVmScriptRun::GetAccount(shared_ptr<CAccount>& Account) {
	if (RawAccont.size() == 0)
		return NULL;
	vector<shared_ptr<CAccount> >::iterator Iter;
	for (Iter = RawAccont.begin(); Iter != RawAccont.end(); Iter++) {
		shared_ptr<CAccount> temp = *Iter;
		if (Account.get()->keyID == temp.get()->keyID) {
			return temp;
		}
	}
	return NULL;
}

bool CVmScriptRun::CheckOperate(const vector<CVmOperate> &listoperate) const {
	// judge contract rulue
	uint64_t addmoey, miusmoney;
	for (auto& it : listoperate) {

		if (it.opeatortype == ADD_FREE || it.opeatortype == ADD_SELF_FREEZD || it.opeatortype == ADD_FREEZD) {
			addmoey += atoi64((char*) it.money);
		}
		if (it.opeatortype == MINUS_FREE || it.opeatortype == MINUS_SELF_FREEZD || it.opeatortype == MINUS_FREEZD) {
			miusmoney += atoi64((char*) it.money);
		}
		if (addmoey != miusmoney)
			return false;

	}
	return true;
}
vector_unsigned_char CVmScriptRun::GetAccountID(CVmOperate value) {
	vector_unsigned_char accountid;
	if (value.type == ACCOUNTID) {
		accountid.assign(value.accountid, value.accountid + 6);
	} else if (value.type == KEYID) {
		accountid.assign(value.accountid, value.accountid + 20);
	} else {
		return accountid;
	}
}
shared_ptr<vector<CVmOperate>> CVmScriptRun::GetOperate() const {
	auto tem = make_shared<vector<CVmOperate>>();
	shared_ptr<vector<unsigned char>> retData = pMcu.get()->GetRetData();
	CDataStream Contractstream(*retData.get(), SER_DISK, CLIENT_VERSION);
	vector<CVmOperate> retvmcode;
	;
	Contractstream >> retvmcode;
	return tem;
}

bool CVmScriptRun::OpeatorAccount(const vector<CVmOperate>& listoperate, CAccountViewCache& view) {

	NewAccont.clear();
	for (auto& it : listoperate) {
		CFund fund;
		fund.value = atoi64((char*) it.money);
		fund.nHeight = it.outheight + height;
		fund.uTxHash = listTx.get()->GetHash();

		auto tem = make_shared<CAccount>();
		vector_unsigned_char accountid = GetAccountID(it);
		;
		if (accountid.size() == 0) {
			return false;
		}

		view.GetAccount(accountid, *tem.get());
		shared_ptr<CAccount> vmAccount = GetAccount(tem);
		if (vmAccount.get() == NULL) {
			RawAccont.push_back(tem);
		}
		shared_ptr<CAccount> vnewAccount = GetNewAccount(tem);
		if (vnewAccount.get() != NULL) {
			vmAccount = vnewAccount;
		}

		if (fund.nFundType == FREEZD_FUND) {
			CFund vFind = vmAccount.get()->FindFund(vmAccount.get()->vFreeze, fund.uTxHash);
			fund.nHeight = vFind.nHeight;
		}

		LogPrint("vm", "muls account:%s\r\n", vmAccount.get()->ToString().c_str());
		LogPrint("vm", "fund:%s\r\n", fund.ToString().c_str());
		// about operate account undo
		uint64_t retValue;
		bool flag = true;//vmAccount.get()->OperateAccount((OperType) it.opeatortype, fund, &retValue);
		LogPrint("vm", "after muls account:%s\r\n", vmAccount.get()->ToString().c_str());
		if (flag) {
			return false;
		}
		NewAccont.push_back(vmAccount);

	}
	return true;
}

