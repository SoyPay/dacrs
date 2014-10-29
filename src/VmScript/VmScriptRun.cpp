/*
 * ScriptCheck.cpp
 *
 *  Created on: Sep 2, 2014
 *      Author: ranger.shi
 */
#include "VmScriptRun.h"
#include "tx.h"
#include "CFundOpeator.h"
#include "util.h"
#include <boost/foreach.hpp>

vector<shared_ptr<CAccountInfo> > &CVmScriptRun::GetRawAccont() {
	return RawAccont;
}
vector<shared_ptr<CAccountInfo> > &CVmScriptRun::GetNewAccont() {
	return NewAccont;
}

bool CVmScriptRun::intial(shared_ptr<CBaseTransaction> & Tx,CAccountViewCache& view) {

	listTx = Tx;
	pView = view;
	vector<unsigned char> vScript;

	if (Tx.get()->nTxType != SECURE_TX) {
		LogPrint("vm", "%s\r\n", "err param");
		return false;
	}

	CSecureTransaction* secure = static_cast<CSecureTransaction*>(Tx.get());
	if (pContractScriptTip->GetScript(HexStr(secure->regScriptId), vScript)) {
		CDataStream stream(vScript, SER_DISK, CLIENT_VERSION);
		try {
			stream >> vmScript;
		}catch(exception& e) {
			 throw runtime_error("CVmScriptRun::intial() Unserialize to vmScript error:"+string(e.what()));
		}
	}

	if (vmScript.IsValid() == false)
		return false;

	for (auto& tx : secure->vRegAccountId) {
		auto tem = make_shared<CAccountInfo>();
		view.GetAccount(tx, *tem.get());
		RawAccont.push_back(tem);
	}

	vector<unsigned char> strContract;
	pMcu = make_shared<CVir8051>(vmScript.Rom, strContract);
	return true;
}

CVmScriptRun::~CVmScriptRun() {

}

bool CVmScriptRun::run(shared_ptr<CBaseTransaction>& Tx, CAccountViewCache& view) {

	if (!intial(Tx, view)) {
		LogPrint("vm", "VmScript inital Failed\n");
		return false;
	}
	if (!pMcu.get()->run()) {
		LogPrint("vm", "VmScript run Failed\n");
		return false;
	}
	shared_ptr<vector<unsigned char>> retData = pMcu.get()->GetRetData();
	CDataStream Contractstream(*retData.get(), SER_DISK, CLIENT_VERSION);
	vector<CVmOperate> retvmcode;;
	Contractstream >> retvmcode;


	if (!CheckOperate(retvmcode)) {
		LogPrint("vm", "VmScript CheckOperate Failed \n");//,HexStr(retData.get()->begin(),retData.get()->end()));
		return false;
	}
	if (!OpeatorSecureAccount(retvmcode)) {
		LogPrint("vm", "VmScript OpeatorSecureAccount Failed\n");
		return false;
	}
	return true;
}
shared_ptr<CAccountInfo> CVmScriptRun::GetNewAccount(shared_ptr<CAccountInfo>& vOldAccount) {
	if (NewAccont.size() == 0)
		return NULL;
	vector<shared_ptr<CAccountInfo> >::iterator Iter;
	for (Iter = NewAccont.begin(); Iter != NewAccont.end(); Iter++) {
		shared_ptr<CAccountInfo> temp = *Iter;
		if (temp.get()->keyID == vOldAccount.get()->keyID) {
			NewAccont.erase(Iter);
			return temp;
		}
	}
	return NULL;
}
shared_ptr<CAccountInfo> CVmScriptRun::GetAccount(shared_ptr<CAccountInfo>& Account)
{
	if (RawAccont.size() == 0)
		return NULL;
	vector<shared_ptr<CAccountInfo> >::iterator Iter;
	for (Iter = RawAccont.begin(); Iter != RawAccont.end(); Iter++) {
		shared_ptr<CAccountInfo> temp = *Iter;
		if (Account.get()->keyID == temp.get()->keyID) {
					return temp;
				}
		}
	}
	return NULL;
}
bool CVmScriptRun::CheckOperate(const vector<CVmOperate> &listoperate) const {
	// judge contract rulue
	uint64_t addmoey,miusmoney;
	for (auto& it : listoperate) {

		if(it == ADD_FREE || it == ADD_SELF_FREEZD || it == ADD_FREEZD)
		{
			addmoey += atoi64((char*)it.money);
		}
		if(it == MINUS_FREE || it == MINUS_SELF_FREEZD || it == MINUS_FREEZD)
		{
			miusmoney += atoi64((char*)it.money);
		}
		if(addmoey != miusmoney)
			return false;

	}
	return true;
}
vector_unsigned_char GetAccountID(CVmOperate value)
{
	vector_unsigned_char accountid;
	if(value.TYPE == ACCOUNTID)
	{
		accountid.assign(value.accountid,value.accountid+6);
	}
	else if(value.TYPE == KEYID)
	{
		accountid.assign(value.accountid,value.accountid+20);
	}
	else
	{
		return accountid;
	}
}
shared_ptr<vector<CVmOperate>> CVmScriptRun::GetOperate() const
{
	auto tem = make_shared<vector<CVmOperate>>();
	shared_ptr<vector<unsigned char>> retData = pMcu.get()->GetRetData();
	CDataStream Contractstream(*retData.get(), SER_DISK, CLIENT_VERSION);
	vector<CVmOperate> retvmcode;;
	Contractstream >> retvmcode;
	return tem;
}

bool CVmScriptRun::OpeatorSecureAccount(const vector<CVmOperate>& listoperate) {

	NewAccont.clear();
	for (auto& it : listoperate) {
		CFund fund;
		fund.value = atoi64((char*)it.money);
		fund.nHeight = it.outheight + chainActive.Height();
		fund.uTxHash = listTx.get()->GetHash();

		auto tem = make_shared<CAccountInfo>();
		vector_unsigned_char accountid = GetAccountID(it);;
		if(accountid.size() == 0)
		{
			return false;
		}

		view.GetAccount(accountid, *tem.get());
		shared_ptr<CAccountInfo> vmAccount = GetAccount(tem);
		if(vmAccount.get() == NULL)
		{
			RawAccont.push_back(tem);
		}
		shared_ptr<CAccountInfo> vnewAccount = GetNewAccount(tem);
		if (vnewAccount.get() != NULL) {
			vmAccount = vnewAccount;
		}

		if(fund.nFundType == FREEZD_FUND)
		{
			CFund vFind = vmAccount.get()->FindFund(vmAccount.get()->vFreeze,fund.uTxHash);
			fund.nHeight = vFind.nHeight;
		}

		LogPrint("vm","muls account:%s\r\n",vmAccount.get()->ToString().c_str());
		LogPrint("vm","fund:%s\r\n",fund.ToString().c_str());
//		cout<<mulsAccount.get()->ToString().c_str()<<endl;
//		cout<<"fund:"<<mulsfund.ToString().c_str()<<endl;
		uint64_t retValue;
		bool flag = vmAccount.get()->OperateAccount((OperType) it.opeatortype, fund, &retValue);
		LogPrint("vm","after muls account:%s\r\n",vmAccount.get()->ToString().c_str());
		if (flag) {
			return false;
		}
		NewAccont.push_back(vmAccount);

	}
	return true;
}

