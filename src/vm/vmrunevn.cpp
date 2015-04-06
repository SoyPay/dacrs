/*
 * ScriptCheck.cpp
 *
 *  Created on: Sep 2, 2014
 *      Author: ranger.shi
 */
#include "vmrunevn.h"
#include "tx.h"
#include "util.h"
#include<algorithm>
#include <boost/foreach.hpp>
CVmRunEvn::CVmRunEvn() {
	RawAccont.clear();
	NewAccont.clear();
	RunTimeHeight = 0;
	m_ScriptDBTip = NULL;
	m_view = NULL;
	m_dblog = std::make_shared<std::vector<CScriptDBOperLog> >();
}
vector<shared_ptr<CAccount> > &CVmRunEvn::GetRawAccont() {
	return RawAccont;
}
vector<shared_ptr<CAccount> > &CVmRunEvn::GetNewAccont() {
	return NewAccont;
}

bool CVmRunEvn::intial(shared_ptr<CBaseTransaction> & Tx, CAccountViewCache& view, int nheight) {

	m_output.clear();
	listTx = Tx;
	RunTimeHeight = nheight;
	m_view = &view;
	vector<unsigned char> vScript;

	if (Tx.get()->nTxType != CONTRACT_TX) {
		LogPrint("ERROR", "%s\r\n", "err param");
		assert(0);
		return false;
	}

	CTransaction* secure = static_cast<CTransaction*>(Tx.get());
	if (!m_ScriptDBTip->GetScript(boost::get<CRegID>(secure->desUserId), vScript)) {
		LogPrint("ERROR", "Script is not Registed %s\r\n", boost::get<CRegID>(secure->desUserId).ToString());
		return false;
	}

	CDataStream stream(vScript, SER_DISK, CLIENT_VERSION);
	try {
		stream >> vmScript;
	} catch (exception& e) {
		LogPrint("ERROR", "%s\r\n", "CVmScriptRun::intial() Unserialize to vmScript error");
		throw runtime_error("CVmScriptRun::intial() Unserialize to vmScript error:" + string(e.what()));
	}

	if (vmScript.IsValid() == false){
		LogPrint("ERROR", "%s\r\n", "CVmScriptRun::intial() vmScript.IsValid error");
		return false;
	}

	pMcu = make_shared<CVm8051>(vmScript.Rom, secure->vContract);
	return true;
}

CVmRunEvn::~CVmRunEvn() {

}
tuple<bool, uint64_t, string> CVmRunEvn::run(shared_ptr<CBaseTransaction>& Tx, CAccountViewCache& view, CScriptDBViewCache& VmDB, int nheight,
		uint64_t nBurnFactor, uint64_t &uRunStep) {

	if(nBurnFactor == 0)
	{
		assert(0);
		return std::make_tuple (false, 0, string("VmScript nBurnFactor == 0 \n"));
	}
	m_ScriptDBTip = &VmDB;

	CTransaction* tx = static_cast<CTransaction*>(Tx.get());
	uint64_t maxstep = min((tx->llFees-CBaseTransaction::nMinTxFee)/nBurnFactor, MAX_BLOCK_RUN_STEP);
	tuple<bool, uint64_t, string> mytuple;
	if (!intial(Tx, view, nheight)) {
		return std::make_tuple (false, 0, string("VmScript inital Failed\n"));
	}

	int64_t step = pMcu.get()->run(maxstep,this);
	if (0 == step) {
		return std::make_tuple(false, 0, string("VmScript run Failed\n"));
	} else if (-1 == step) {
		return std::make_tuple(false, 0, string("execure tx contranct run step exceed the max step limit\n"));
		//uRunStep = maxstep;
	}else{
		uRunStep = step;
	}

	LogPrint("CONTRACT_TX", "tx:%s,step:%ld\n", tx->ToString(view), uRunStep);

	if (!CheckOperate(m_output)) {
		return std::make_tuple (false, 0, string("VmScript CheckOperate Failed \n"));

	}
	if (!OpeatorAccount(m_output, view)) {
		return std::make_tuple (false, 0, string("VmScript OpeatorAccount Failed\n"));
	}

	if(!OpeatorAppAccount(MapAppOperate, *m_ScriptDBTip))
	{
		return std::make_tuple (false, 0, string("OpeatorApp Account Failed\n"));
	}

	uint64_t spend = uRunStep * nBurnFactor;
	return std::make_tuple (true, spend, string("VmScript Sucess\n"));

}

shared_ptr<CAccount> CVmRunEvn::GetNewAccount(shared_ptr<CAccount>& vOldAccount) {
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
shared_ptr<CAccount> CVmRunEvn::GetAccount(shared_ptr<CAccount>& Account) {
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
vector_unsigned_char CVmRunEvn::GetAccountID(CVmOperate value) {
	vector_unsigned_char accountid;
	if (value.nacctype == regid) {
		accountid.assign(value.accountid, value.accountid + 6);
	} else if (value.nacctype == base58addr) {
		string addr(value.accountid,value.accountid+sizeof(value.accountid));
		CKeyID KeyId = CKeyID(addr);
		CRegID regid;
		if(m_view->GetRegId(CUserID(KeyId), regid)){
			accountid.assign(regid.GetVec6().begin(),regid.GetVec6().end());
		}else{
			accountid.assign(value.accountid, value.accountid + 34);
		}
	}
	return accountid;
}
bool CVmRunEvn::CheckOperate(const vector<CVmOperate> &listoperate) {
	// judge contract rulue
	uint64_t addmoey = 0, miusmoney = 0;
	uint64_t temp = 0;
	for (auto& it : listoperate) {

		if(it.nacctype != regid && it.nacctype != base58addr)
			return false;

		if (it.opeatortype == ADD_FREE ) {
			memcpy(&temp,it.money,sizeof(it.money));
			addmoey += temp;
		}

		if (it.opeatortype == MINUS_FREE) {

			//vector<unsigned char > accountid(it.accountid,it.accountid+sizeof(it.accountid));
			vector_unsigned_char accountid = GetAccountID(it);
			if(accountid.size() != 6)
				return false;
			CRegID regId(accountid);
			CTransaction* secure = static_cast<CTransaction*>(listTx.get());
			/// current tx's script cant't mius other script's regid
			if(m_ScriptDBTip->HaveScript(regId) && regId != boost::get<CRegID>(secure->desUserId))
			{
				return false;
			}

			memcpy(&temp,it.money,sizeof(it.money));
			miusmoney += temp;
		}
		//vector<unsigned char> accountid(it.accountid, it.accountid + sizeof(it.accountid));
		vector_unsigned_char accountid = GetAccountID(it);
		if(accountid.size() == 6){
			CRegID regId(accountid);
			if(regId.IsEmpty() || regId.getKeyID( *m_view) == uint160(0))
				return false;
			//  app only be allowed minus self money
			if (!m_ScriptDBTip->HaveScript(regId) && it.opeatortype == MINUS_FREE) {
				return false;
			}
		}

	}
	if (addmoey != miusmoney)
	{
		return false;
	}
	return true;
}

shared_ptr<vector<CVmOperate>> CVmRunEvn::GetOperate() const {
	auto tem = make_shared<vector<CVmOperate>>();
	shared_ptr<vector<unsigned char>> retData = pMcu.get()->GetRetData();
	CDataStream Contractstream(*retData.get(), SER_DISK, CLIENT_VERSION);
	vector<CVmOperate> retvmcode;
	;
	Contractstream >> retvmcode;
	return tem;
}

bool CVmRunEvn::OpeatorAccount(const vector<CVmOperate>& listoperate, CAccountViewCache& view) {

	NewAccont.clear();
	for (auto& it : listoperate) {
		CTransaction* tx = static_cast<CTransaction*>(listTx.get());
		CFund fund;
		memcpy(&fund.value,it.money,sizeof(it.money));
		fund.nHeight = it.outheight;
		fund.appId = boost::get<CRegID>(tx->desUserId).GetVec6();

		auto tem = make_shared<CAccount>();
//		vector_unsigned_char accountid = GetAccountID(it);
//		if (accountid.size() == 0) {
//			return false;
//		}
//		vector_unsigned_char accountid(it.accountid,it.accountid+sizeof(it.accountid));
		vector_unsigned_char accountid = GetAccountID(it);
		CRegID userregId;
		CKeyID userkeyid;

		if(accountid.size() == 6){
			userregId.SetRegID(accountid);
			view.GetAccount(CUserID(userregId), *tem.get());
		}else{
			string dacrsaddr(accountid.begin(), accountid.end());
			userkeyid = CKeyID(dacrsaddr);
			view.GetAccount(CUserID(userkeyid), *tem.get());
			}

		shared_ptr<CAccount> vmAccount = GetAccount(tem);
		if (vmAccount.get() == NULL) {
			RawAccont.push_back(tem);
			vmAccount = tem;
		}
		shared_ptr<CAccount> vnewAccount = GetNewAccount(tem);
		//// 这个账号已经存在，需要合并
		if (vnewAccount.get() != NULL) {
			vmAccount = vnewAccount;
		}else{
			vmAccount.get()->CompactAccount(RunTimeHeight);
		}
		if ((OperType) it.opeatortype == ADD_FREE) {
			fund.nFundType = FREEDOM_FUND;
		}

		LogPrint("vm", "account id:%s\r\n", HexStr(accountid).c_str());
		LogPrint("vm", "befer account:%s\r\n", vmAccount.get()->ToString().c_str());
		LogPrint("vm", "fund:%s\r\n", fund.ToString().c_str());
		bool ret = false;
//		vector<CScriptDBOperLog> vAuthorLog;
		//todolist
//		if(IsSignatureAccount(vmAccount.get()->regID) || vmAccount.get()->regID == boost::get<CRegID>(tx->appRegId))
		{
			ret = vmAccount.get()->OperateAccount((OperType)it.opeatortype, fund);
		}
//		else{
//			ret = vmAccount.get()->OperateAccount((OperType)it.opeatortype, fund, *m_ScriptDBTip, vAuthorLog,  height, &GetScriptRegID().GetVec6(), true);
//		}

//		LogPrint("vm", "after account:%s\r\n", vmAccount.get()->ToString().c_str());
		if (!ret) {
			return false;
		}
		NewAccont.push_back(vmAccount);
//		m_dblog->insert(m_dblog->end(), vAuthorLog.begin(), vAuthorLog.end());

	}
	return true;
}

const CRegID& CVmRunEvn::GetScriptRegID()
{
	CTransaction* tx = static_cast<CTransaction*>(listTx.get());
	return boost::get<CRegID>(tx->desUserId);
}

const CRegID &CVmRunEvn::GetTxAccount() {
	CTransaction* tx = static_cast<CTransaction*>(listTx.get());
	return boost::get<CRegID>(tx->srcRegId);
}
uint64_t CVmRunEvn::GetValue() const{
	CTransaction* tx = static_cast<CTransaction*>(listTx.get());
		return tx->llValues;
}
const vector<unsigned char>& CVmRunEvn::GetTxContact()
{
	CTransaction* tx = static_cast<CTransaction*>(listTx.get());
		return tx->vContract;
}
int CVmRunEvn::GetComfirHeight()
{
	return RunTimeHeight;
}
uint256 CVmRunEvn::GetCurTxHash()
{
	return listTx.get()->GetHash();
}
CScriptDBViewCache* CVmRunEvn::GetScriptDB()
{
	return m_ScriptDBTip;
}
CAccountViewCache * CVmRunEvn::GetCatchView()
{
	return m_view;
}
void CVmRunEvn::InsertOutAPPOperte(const vector<unsigned char>& userId,const CAppFundOperate &source)
{
	if(MapAppOperate.count(userId))
	{
		MapAppOperate[userId].push_back(source);
	}
	else
	{
		vector<CAppFundOperate> it;
		it.push_back(source);
		MapAppOperate[userId] = it;
	}

}
void CVmRunEvn::InsertOutputData(const vector<CVmOperate>& source)
{
	m_output.insert(m_output.end(),source.begin(),source.end());
}
shared_ptr<vector<CScriptDBOperLog> > CVmRunEvn::GetDbLog()
{
	return m_dblog;
}

bool CVmRunEvn::GetAppUserAccout(const vector<unsigned char> &vAppUserId, shared_ptr<CAppUserAccout> &sptrAcc) {
	assert(m_ScriptDBTip != NULL);
	shared_ptr<CAppUserAccout> tem = make_shared<CAppUserAccout>();
	if (!m_ScriptDBTip->GetScriptAcc(GetScriptRegID(), vAppUserId, *tem.get())) {
			tem = make_shared<CAppUserAccout>(vAppUserId);
			sptrAcc = tem;
			return true;
	}
	if (!tem.get()->AutoMergeFreezeToFree(RunTimeHeight)) {
		return false;
	};
	sptrAcc = tem;
	return true;
}

bool CVmRunEvn::OpeatorAppAccount(const map<vector<unsigned char >,vector<CAppFundOperate> > opMap, CScriptDBViewCache& view) {
	if ((MapAppOperate.size() > 0)) {
		for (auto const tem : opMap) {
			shared_ptr<CAppUserAccout> sptrAcc;
			if (!GetAppUserAccout(tem.first, sptrAcc)) {
				LogPrint("vm", "GetAppUserAccout(tem.first, sptrAcc, true) failed \r\n appuserid :%s\r\n",
						HexStr(tem.first));
				return false;
			}
			if (!sptrAcc.get()->AutoMergeFreezeToFree(RunTimeHeight)) {
				LogPrint("vm", "AutoMergeFreezeToFreefailed \r\n appuser :%s\r\n", sptrAcc.get()->toString());
				return false;

			}
			LogPrint("vm", "before user: %s\r\n", sptrAcc.get()->toString());
			if (!sptrAcc.get()->Operate(tem.second)) {

				int i = 0;
				for (auto const pint : tem.second) {
					LogPrint("vm", "GOperate failed \r\n Operate %d : %s\r\n", i++, pint.toString());
				}
				LogPrint("vm", "GetAppUserAccout(tem.first, sptrAcc, true) failed \r\n appuserid :%s\r\n",
						HexStr(tem.first));
				return false;
			}
			LogPrint("vm", "after user: %s\r\n", sptrAcc.get()->toString());
			CScriptDBOperLog log;
			view.SetScriptAcc(GetScriptRegID(), *sptrAcc.get(), log);
			shared_ptr<vector<CScriptDBOperLog> > m_dblog = GetDbLog();
			m_dblog.get()->push_back(log);

		}
	}
	return true;
}


