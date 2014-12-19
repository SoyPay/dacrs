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
	m_ScriptDBTip = NULL;
	m_view = NULL;
	m_dblog = std::make_shared<std::vector<CScriptDBOperLog> >();
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
	m_view = &view;
	vector<unsigned char> vScript;

	if (Tx.get()->nTxType != CONTRACT_TX) {
		LogPrint("vm", "%s\r\n", "err param");
		return false;
	}

	CContractTransaction* secure = static_cast<CContractTransaction*>(Tx.get());
	if (m_ScriptDBTip->GetScript(boost::get<CRegID>(secure->scriptRegId), vScript)) {
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

	pMcu = make_shared<CVir8051>(vmScript.Rom, secure->vContract);
	return true;
}

CVmScriptRun::~CVmScriptRun() {

}
tuple<bool, uint64_t, string> CVmScriptRun::run(shared_ptr<CBaseTransaction>& Tx, CAccountViewCache& view, CScriptDBViewCache& VmDB, int nheight,
		uint64_t nBurnFactor) {

	if(nBurnFactor == 0)
	{
		assert(0);
		return std::make_tuple (false, 0, string("VmScript nBurnFactor == 0 \n"));
	}
	m_ScriptDBTip = &VmDB;

	CContractTransaction* tx = static_cast<CContractTransaction*>(Tx.get());
	uint64_t maxstep = tx->llFees;///nBurnFactor;
	tuple<bool, uint64_t, string> mytuple;
	if (!intial(Tx, view, nheight)) {
		return std::make_tuple (false, 0, string("VmScript inital Failed\n"));

	}
	int64_t  step = pMcu.get()->run(maxstep,this);
	if (0 == step) {
		mytuple = std::make_tuple (false, 0, string("VmScript run Failed\n"));
		return mytuple;
	}else if(-1 == step){
		mytuple = std::make_tuple (false, 0, string("the fee not enough \n"));
		return mytuple;
	}

	shared_ptr<vector<unsigned char>> retData = pMcu.get()->GetRetData();
	CDataStream Contractstream(*retData.get(), SER_DISK, CLIENT_VERSION);
	vector<CVmOperate> retvmcode;
	Contractstream >> retvmcode;

	if (!CheckOperate(m_output)) {
		return std::make_tuple (false, 0, string("VmScript CheckOperate Failed \n"));

	}
	if (!OpeatorAccount(m_output, view)) {
		return std::make_tuple (false, 0, string("VmScript OpeatorSecureAccount Failed\n"));
	}
	uint64_t spend = step*nBurnFactor;
	return std::make_tuple (true, spend, string("VmScript Sucess\n"));

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
vector_unsigned_char CVmScriptRun::GetAccountID(CVmOperate value) {
	vector_unsigned_char accountid;
	//if (value.type == ACCOUNTID) {
	//	accountid.assign(value.accountid, value.accountid + 6);
	//} else if (value.type == KEYID) {
		//accountid.assign(value.accountid, value.accountid + 20);
	//}
	return accountid;
}
bool CVmScriptRun::CheckOperate(const vector<CVmOperate> &listoperate) const {
	// judge contract rulue
	uint64_t addmoey = 0, miusmoney = 0;
	uint64_t temp = 0;
	for (auto& it : listoperate) {

		if (it.opeatortype == ADD_FREE || it.opeatortype == ADD_SELF_FREEZD || it.opeatortype == ADD_FREEZD) {
			memcpy(&temp,it.money,sizeof(it.money));
			addmoey += temp;
		}
		if (it.opeatortype == MINUS_FREE || it.opeatortype == MINUS_SELF_FREEZD || it.opeatortype == MINUS_FREEZD) {

			/// 从冻结金额里面扣钱，超时高度必须大于当前tip高度
			if(it.opeatortype == MINUS_FREEZD && it.outheight <height)
			{
				return false;
			}
			vector<unsigned char > accountid(it.accountid,it.accountid+sizeof(it.accountid));
			CRegID regId(accountid);
			CContractTransaction* secure = static_cast<CContractTransaction*>(listTx.get());
			/// current tx's script cant't mius other script's regid
			if(m_ScriptDBTip->HaveScript(regId) && regId != boost::get<CRegID>(secure->scriptRegId))
			{
				return false;
			}
			memcpy(&temp,it.money,sizeof(it.money));
			miusmoney += temp;
		}
		vector<unsigned char> accountid(it.accountid, it.accountid + sizeof(it.accountid));
		CRegID regId(accountid);
		if(regId.IsEmpty() || regId.getKeyID( *m_view) == uint160(0))
			return false;
		/// if account script id ,the it.opeatortype must be ADD_FREE or MINUS_FREE
		if (m_ScriptDBTip->HaveScript(regId) && it.opeatortype != ADD_FREE && it.opeatortype != MINUS_FREE) {
			return false;
		}
	}
	if (addmoey != miusmoney)
	{
		return false;
	}
	return true;
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
		CContractTransaction* tx = static_cast<CContractTransaction*>(listTx.get());
		CFund fund;
		memcpy(&fund.value,it.money,sizeof(it.money));
		fund.nHeight = it.outheight;
		fund.scriptID = boost::get<CRegID>(tx->scriptRegId).GetVec6();

		auto tem = make_shared<CAccount>();
//		vector_unsigned_char accountid = GetAccountID(it);
//		if (accountid.size() == 0) {
//			return false;
//		}
		vector_unsigned_char accountid(it.accountid,it.accountid+sizeof(it.accountid));
		CRegID regid(accountid);
		CUserID userid(regid);
		view.GetAccount(userid, *tem.get());
		shared_ptr<CAccount> vmAccount = GetAccount(tem);
		if (vmAccount.get() == NULL) {
			RawAccont.push_back(tem);
			vmAccount = tem;
		}
		shared_ptr<CAccount> vnewAccount = GetNewAccount(tem);
		//// 这个账号已经存在，需要合并
		if (vnewAccount.get() != NULL) {
			vmAccount = vnewAccount;
			vmAccount.get()->CompactAccount(height);
		}
		if ((OperType) it.opeatortype == ADD_FREE) {
			fund.nFundType = FREEDOM_FUND;
		} else if ((OperType) it.opeatortype == ADD_FREEZD || (OperType) it.opeatortype == MINUS_FREEZD) {
			fund.nFundType = FREEZD_FUND;
		} else if ((OperType) it.opeatortype == ADD_SELF_FREEZD) {
			fund.nFundType = SELF_FREEZD_FUND;
		}
		//// the script account if ADD_FREE must merge
		if (m_ScriptDBTip->HaveScript(vmAccount.get()->regID)
				&& vmAccount.get()->regID.GetVec6() != fund.scriptID) {
			if (fund.nFundType == ADD_FREE) {
				CFund vFind;
				if (vmAccount.get()->FindFund(vmAccount.get()->vFreeze, fund.scriptID, vFind)) {
					fund.nHeight = vFind.nHeight;
				}

			}
		}


//		LogPrint("vm", "account id:%s\r\n", HexStr(accountid).c_str());
//		LogPrint("vm", "befer account:%s\r\n", vmAccount.get()->ToString().c_str());
//		LogPrint("vm", "fund:%s\r\n", fund.ToString().c_str());
		bool ret = false;
		if(IsSignatureAccount(vmAccount.get()->regID) || vmAccount.get()->regID == boost::get<CRegID>(tx->scriptRegId))
		{
			ret = vmAccount.get()->OperateAccount((OperType)it.opeatortype,fund,height);
		}else{
			ret = vmAccount.get()->OperateAccount((OperType)it.opeatortype,fund,height,&GetScriptRegID().GetVec6(),true);
		}

//		LogPrint("vm", "after account:%s\r\n", vmAccount.get()->ToString().c_str());
		if (!ret) {
			return false;
		}
		NewAccont.push_back(vmAccount);

	}
	return true;
}

const CRegID& CVmScriptRun::GetScriptRegID()
{
	CContractTransaction* tx = static_cast<CContractTransaction*>(listTx.get());
	return boost::get<CRegID>(tx->scriptRegId);
}
const vector<CUserID>& CVmScriptRun::GetTxAccount()
{
	CContractTransaction* tx = static_cast<CContractTransaction*>(listTx.get());
		return tx->vAccountRegId;
}
const BOOL CVmScriptRun::IsSignatureAccount(CRegID account)
{
	vector<CUserID> regid =GetTxAccount();

	vector<unsigned char> item;
	auto tem =  make_shared<std::vector< vector<unsigned char> > >();
		for (auto& it : regid) {
			if(account.GetVec6() == boost::get<CRegID>(it).GetVec6())
			return true;
		}
		return false;
}
const vector<unsigned char>& CVmScriptRun::GetTxContact()
{
	CContractTransaction* tx = static_cast<CContractTransaction*>(listTx.get());
		return tx->vContract;
}
int CVmScriptRun::GetComfirHeight()
{
	return height;
}
uint256 CVmScriptRun::GetCurTxHash()
{
	return listTx.get()->GetHash();
}
CScriptDBViewCache* CVmScriptRun::GetScriptDB()
{
	return m_ScriptDBTip;
}
CAccountViewCache * CVmScriptRun::GetCatchView()
{
	return m_view;
}
void CVmScriptRun::InsertOutputData(vector<CVmOperate> source)
{
	m_output.insert(m_output.end(),source.begin(),source.end());
}
shared_ptr<vector<CScriptDBOperLog> > CVmScriptRun::GetDbLog()
{
	return m_dblog;
}
