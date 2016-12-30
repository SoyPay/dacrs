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
#include "SafeInt3.hpp"

 //CVmRunEvn *pVmRunEvn = NULL;

#define MAX_OUTPUT_COUNT 100

CVmRunEvn::CVmRunEvn() {
	RawAccont.clear();
	NewAccont.clear();
	RawAppUserAccout.clear();
	NewAppUserAccout.clear();
	RunTimeHeight = 0;
	m_ScriptDBTip = NULL;
	m_view = NULL;
	m_dblog = std::make_shared<std::vector<CScriptDBOperLog> >();
	m_bIsCheckAccount = false;
	m_nScriptType = 0; //默认跑8051脚本
}

vector<shared_ptr<CAccount> > &CVmRunEvn::GetRawAccont() {
	return RawAccont;
}

vector<shared_ptr<CAccount> > &CVmRunEvn::GetNewAccont() {
	return NewAccont;
}

vector<shared_ptr<CAppUserAccout>> &CVmRunEvn::GetNewAppUserAccount() {
	return NewAppUserAccout;
}

vector<shared_ptr<CAppUserAccout>> &CVmRunEvn::GetRawAppUserAccount() {
	return RawAppUserAccout;
}

bool CVmRunEvn::intial(shared_ptr<CBaseTransaction> & Tx, CAccountViewCache& view, int nheight) {

	m_output.clear();
	listTx = Tx;
	RunTimeHeight = nheight;
	m_view = &view;
	vector<unsigned char> vuchScript;

	if (Tx.get()->m_chTxType != EM_CONTRACT_TX) {
		LogPrint("ERROR", "%s\r\n", "err param");
		return false;
	}

	CTransaction* pcSecure = static_cast<CTransaction*>(Tx.get());

	if (!m_ScriptDBTip->GetScript(boost::get<CRegID>(pcSecure->m_cDesUserId), vuchScript)) {
		LogPrint("ERROR", "Script is not Registed %s\r\n", boost::get<CRegID>(pcSecure->m_cDesUserId).ToString());
		return false;
	}

	CDataStream pcStream(vuchScript, SER_DISK, g_sClientVersion);
	try {
		pcStream >> vmScript;
	} catch (exception& e) {
		LogPrint("ERROR", "%s\r\n", "CVmScriptRun::intial() Unserialize to vmScript error");
		throw runtime_error("CVmScriptRun::intial() Unserialize to vmScript error:" + string(e.what()));
	}

	if (vmScript.IsValid() == false) {
		LogPrint("ERROR", "%s\r\n", "CVmScriptRun::intial() vmScript.IsValid error");
		return false;
	}
	m_bIsCheckAccount = vmScript.IsCheckAccount();
	m_nScriptType = vmScript.getScriptType(); //初始化脚本类型
	if (pcSecure->m_vchContract.size() >= 4 * 1024) {
		LogPrint("ERROR", "%s\r\n", "CVmScriptRun::intial() vContract context size lager 4096");
		return false;
	}
	try {
		if (0 == m_nScriptType) {
			pMcu = std::make_shared<CVm8051>(vmScript.vuchRom, pcSecure->m_vchContract);
			LogPrint("vm", "%s\r\n", "CVmScriptRun::intial() MCU");
		} else {
			pLua = std::make_shared<CVmlua>(vmScript.vuchRom, pcSecure->m_vchContract);
			//pVmRunEvn = this; //传CVmRunEvn对象指针给lmylib.cpp库使用
			LogPrint("vm", "%s\r\n", "CVmScriptRun::intial() LUA");
		}
	} catch (exception& e) {
		LogPrint("ERROR", "%s\r\n", "CVmScriptRun::intial() init error");
		return false;
	}

	return true;
}

CVmRunEvn::~CVmRunEvn() {

}

tuple<bool, uint64_t, string> CVmRunEvn::run(shared_ptr<CBaseTransaction>& Tx, CAccountViewCache& cView,
		CScriptDBViewCache& cVmDB, int nHeight, uint64_t ullBurnFactor, uint64_t &ullRunStep) {

	if (ullBurnFactor == 0) {
//		assert(0);
		return std::make_tuple(false, 0, string("VmScript nBurnFactor == 0 \n"));
	}
	m_ScriptDBTip = &cVmDB;

	CTransaction* pcTx = static_cast<CTransaction*>(Tx.get());
	if (pcTx->m_ullFees <= CBaseTransaction::m_sMinTxFee) {
		return std::make_tuple(false, 0, string("vm run evn fee too litter\n"));
	}
	uint64_t ullMaxStep = ((pcTx->m_ullFees - CBaseTransaction::m_sMinTxFee) / ullBurnFactor) * 100;

	if (ullMaxStep > MAX_BLOCK_RUN_STEP) {
		ullMaxStep = MAX_BLOCK_RUN_STEP;
	}
	LogPrint("vm", "pcTx hash:%s fees=%lld fuelrate=%lld ullMaxStep:%d\n", Tx->GetHash().GetHex(), pcTx->m_ullFees,
			ullBurnFactor, ullMaxStep);
	if (!intial(Tx, cView, nHeight)) {
		return std::make_tuple(false, 0, string("VmScript inital Failed\n"));
	}

	int64_t llStep = 0;
	if (0 == m_nScriptType) {
		llStep = pMcu.get()->run(ullMaxStep, this);
		LogPrint("vm", "%s\r\n", "CVmScriptRun::run() MCU");
	} else {
		llStep = pLua.get()->run(ullMaxStep, this);
		LogPrint("vm", "%s\r\n", "CVmScriptRun::run() LUA");
	}
	if (0 == llStep) {
		return std::make_tuple(false, 0, string("VmScript run Failed\n"));
	} else if (-1 == llStep) {
		return std::make_tuple(false, 0, string("execure tx contranct run llStep exceed the max llStep limit\n"));
	} else {
		ullRunStep = llStep;
	}

	LogPrint("vm", "tx:%s,llStep:%ld\n", pcTx->ToString(cView), ullRunStep);

	if (!CheckOperate(m_output)) {
		return std::make_tuple(false, 0, string("VmScript CheckOperate Failed \n"));
	}

	if (!OpeatorAccount(m_output, cView, nHeight)) {
		return std::make_tuple(false, 0, string("VmScript OpeatorAccount Failed\n"));
	}

	LogPrint("vm", "m_bIsCheckAccount:%d\n", m_bIsCheckAccount);
	if (m_bIsCheckAccount) {
		LogPrint("vm", "m_bIsCheckAccount is true\n");
		if (!CheckAppAcctOperate(pcTx)) {
			return std::make_tuple(false, 0, string("VmScript CheckAppAcct Failed\n"));
		}
	}

	if (!OpeatorAppAccount(MapAppOperate, *m_ScriptDBTip)) {
		return std::make_tuple(false, 0, string("OpeatorApp Account Failed\n"));
	}

	if (SysCfg().GetOutPutLog() && m_output.size() > 0) {
		CScriptDBOperLog cOperlog;
		uint256 txhash = GetCurTxHash();
		if (!m_ScriptDBTip->WriteTxOutPut(txhash, m_output, cOperlog)) {
			return std::make_tuple(false, 0, string("write tx out put Failed \n"));
		}
		m_dblog->push_back(cOperlog);
	}

	/*
	 uint64_t ullSpend = uRunStep * nBurnFactor;
	 if((ullSpend < uRunStep) || (ullSpend < nBurnFactor)){
	 return std::make_tuple (false, 0, string("mul error\n"));
	 }
	 */
	uint64_t ullSpend = 0;
	if (!SafeMultiply(ullRunStep, ullBurnFactor, ullSpend)) {
		return std::make_tuple(false, 0, string("mul error\n"));
	}
	return std::make_tuple(true, ullSpend, string("VmScript Sucess\n"));

}

shared_ptr<CAccount> CVmRunEvn::GetNewAccount(shared_ptr<CAccount>& vOldAccount) {
	if (NewAccont.size() == 0) {
		return NULL;
	}
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
	if (value.m_uchNaccType == EM_REGID) {
		accountid.assign(value.m_arruchAccountId, value.m_arruchAccountId + 6);
	} else if (value.m_uchNaccType == EM_BASE_58_ADDR) {
		string addr(value.m_arruchAccountId, value.m_arruchAccountId + sizeof(value.m_arruchAccountId));
		CKeyID cKeyId = CKeyID(addr);
		CRegID cRegid;
		if (m_view->GetRegId(CUserID(cKeyId), cRegid)) {
			accountid.assign(cRegid.GetVec6().begin(), cRegid.GetVec6().end());
		} else {
			accountid.assign(value.m_arruchAccountId, value.m_arruchAccountId + 34);
		}
	}
	return accountid;
}

shared_ptr<CAppUserAccout> CVmRunEvn::GetAppAccount(shared_ptr<CAppUserAccout>& AppAccount) {
	if (RawAppUserAccout.size() == 0)
		return NULL;
	vector<shared_ptr<CAppUserAccout> >::iterator Iter;
	for (Iter = RawAppUserAccout.begin(); Iter != RawAppUserAccout.end(); Iter++) {
		shared_ptr<CAppUserAccout> temp = *Iter;
		if (AppAccount.get()->getaccUserId() == temp.get()->getaccUserId()) {
			return temp;
		}
	}
	return NULL;
}

bool CVmRunEvn::CheckOperate(const vector<CVmOperate> &listoperate) {
	// judge contract rulue
	uint64_t ullAddMoney = 0, ullMiusMoney = 0;
	uint64_t ullOperValue = 0;
	if (listoperate.size() > MAX_OUTPUT_COUNT) {
		return false;
	}

	for (auto& it : listoperate) {

		if (it.m_uchNaccType != EM_REGID && it.m_uchNaccType != EM_BASE_58_ADDR) {
			return false;
		}
		if (it.m_uchOpeatorType == ADD_FREE) {
			memcpy(&ullOperValue, it.m_arruchMoney, sizeof(it.m_arruchMoney));
			/*
			 uint64_t temp = ullAddMoney;
			 temp += operValue;
			 if(temp < operValue || temp<ullAddMoney) {
			 return false;
			 }
			 */
			uint64_t ullTemp = 0;
			if (!SafeAdd(ullAddMoney, ullOperValue, ullTemp)) {
				return false;
			}
			ullAddMoney = ullTemp;
		} else if (it.m_uchOpeatorType == MINUS_FREE) {

			//vector<unsigned char > accountid(it.accountid,it.accountid+sizeof(it.accountid));
			vector_unsigned_char accountid = GetAccountID(it);
			if (accountid.size() != 6)
				return false;
			CRegID cRegid(accountid);
			CTransaction* pcTx = static_cast<CTransaction*>(listTx.get());
			/// current pcTx's script cant't mius other script's cRegid
			if (m_ScriptDBTip->HaveScript(cRegid) && cRegid != boost::get<CRegID>(pcTx->m_cDesUserId)) {
				return false;
			}

			memcpy(&ullOperValue, it.m_arruchMoney, sizeof(it.m_arruchMoney));
			uint64_t ullTemp = 0;
			if (!SafeAdd(ullMiusMoney, ullOperValue, ullTemp)) {
				return false;
			}
			ullMiusMoney = ullTemp;
		} else {
			return false; // 输入数据错
		}

		//vector<unsigned char> accountid(it.accountid, it.accountid + sizeof(it.accountid));
		vector_unsigned_char accountid = GetAccountID(it);
		if (accountid.size() == 6) {
			CRegID cRegid(accountid);
			if (cRegid.IsEmpty() || cRegid.getKeyID(*m_view) == uint160())
				return false;
			//  app only be allowed minus self m_arruchMoney
			if (!m_ScriptDBTip->HaveScript(cRegid) && it.m_uchOpeatorType == MINUS_FREE) {
				return false;
			}
		}

	}
	if (ullAddMoney != ullMiusMoney) {
		return false;
	}
	return true;
}

bool CVmRunEvn::CheckAppAcctOperate(CTransaction* pcTx) {
	int64_t addValue(0), minusValue(0), sumValue(0);
	for(auto  vOpItem : MapAppOperate) {
		for(auto appFund : vOpItem.second) {
			if(EM_ADD_FREE_OP == appFund.m_uchOpeatorType || EM_ADD_TAG_OP == appFund.m_uchOpeatorType) {
				/*
				int64_t temp = appFund.mMoney;
				temp += addValue;
				if(temp < addValue || temp<appFund.mMoney) {
					return false;
				}
				*/
				int64_t temp = 0;
				if(!SafeAdd(appFund.m_llMoney, addValue, temp)) {
					return false;
				}
				addValue = temp;
			}
			else if(EM_SUB_FREE_OP == appFund.m_uchOpeatorType || EM_SUB_TAG_OP == appFund.m_uchOpeatorType) {
				/*
				int64_t temp = appFund.mMoney;
				temp += minusValue;
				if(temp < minusValue || temp<appFund.mMoney) {
					return false;
				}
				*/
				int64_t temp = 0;
				if(!SafeAdd(appFund.m_llMoney, minusValue, temp)) {
					return false;
				}
				minusValue = temp;
			}
		}
	}
	/*
	sumValue = addValue - minusValue;
	if(sumValue > addValue) {
		return false;
	}
	*/
	if(!SafeSubtract(addValue, minusValue, sumValue)) {
		return false;
	}

	uint64_t sysContractAcct(0);
	for(auto item : m_output) {
		vector_unsigned_char vAccountId = GetAccountID(item);
		if(vAccountId == boost::get<CRegID>(pcTx->m_cDesUserId).GetVec6() && item.m_uchOpeatorType == MINUS_FREE) {
			uint64_t value;
			memcpy(&value, item.m_arruchMoney, sizeof(item.m_arruchMoney));
			int64_t temp = value;
			if(temp < 0) {
				return false;
			}
			/*
			temp += sysContractAcct;
			if(temp < sysContractAcct || temp < (int64_t)value)
				return false;
			*/
			uint64_t tempValue = temp;
			uint64_t tempOut = 0;
			if(!SafeAdd(tempValue, sysContractAcct, tempOut)) {
				return false;
			}
			sysContractAcct = tempOut;
		}
	}

	/*
	 int64_t sysAcctSum = tx->llValues - sysContractAcct;
	 if(sysAcctSum > (int64_t)tx->llValues) {
	 	 return false;
	 }
	*/
	int64_t sysAcctSum = 0;
	if (!SafeSubtract((int64_t)pcTx->m_ullValues, (int64_t)sysContractAcct, sysAcctSum)) {
		return false;
	}

	if(sumValue != (int64_t)sysAcctSum){
		LogPrint("vm", "CheckAppAcctOperate:addValue=%lld, minusValue=%lld, txValue=%lld, sysContractAcct=%lld sumValue=%lld, sysAcctSum=%lld\n", addValue, minusValue, pcTx->m_ullValues, sysContractAcct,sumValue, sysAcctSum);
		return false;
	}
	return true;
}

//shared_ptr<vector<CVmOperate>> CVmRunEvn::GetOperate() const {
//	auto tem = make_shared<vector<CVmOperate>>();
//	shared_ptr<vector<unsigned char>> retData = pMcu.get()->GetRetData();
//	CDataStream Contractstream(*retData.get(), SER_DISK, g_sClientVersion);
//	vector<CVmOperate> retvmcode;
//	;
//	Contractstream >> retvmcode;
//	return tem;
//}

bool CVmRunEvn::OpeatorAccount(const vector<CVmOperate>& listoperate, CAccountViewCache& view, const int nCurHeight) {

	NewAccont.clear();
	for (auto& it : listoperate) {
		uint64_t value;
		memcpy(&value, it.m_arruchMoney, sizeof(it.m_arruchMoney));

		auto tem = std::make_shared<CAccount>();
		vector_unsigned_char accountid = GetAccountID(it);
		CRegID cUserRegId;
		CKeyID cUserkeyId;

		if (accountid.size() == 6) {
			cUserRegId.SetRegID(accountid);
			if (!view.GetAccount(CUserID(cUserRegId), *tem.get())) {
				return false;                                           /// 账户不存在
			}
		} else {
			string strAddr(accountid.begin(), accountid.end());
			cUserkeyId = CKeyID(strAddr);
			if (!view.GetAccount(CUserID(cUserkeyId), *tem.get())) {
				tem->keyID = cUserkeyId;
				//return false;                                           /// 未产生过交易记录的账户
			}
		}

		shared_ptr<CAccount> vmAccount = GetAccount(tem);
		if (vmAccount.get() == NULL) {
			RawAccont.push_back(tem);
			vmAccount = tem;
		}
		LogPrint("vm", "account id:%s\r\n", HexStr(accountid).c_str());
		LogPrint("vm", "befer account:%s\r\n", vmAccount.get()->ToString().c_str());
		bool bRet = false;
//		vector<CScriptDBOperLog> vAuthorLog;
		//todolist
//		if(IsSignatureAccount(vmAccount.get()->cRegid) || vmAccount.get()->cRegid == boost::get<CRegID>(tx->appRegId))
		{
			bRet = vmAccount.get()->OperateAccount((OperType) it.m_uchOpeatorType, value, nCurHeight);
		}
//		else{
//			ret = vmAccount.get()->OperateAccount((OperType)it.m_uchOpeatorType, fund, *m_ScriptDBTip, vAuthorLog,  height, &GetScriptRegID().GetVec6(), true);
//		}

//		LogPrint("vm", "after account:%s\r\n", vmAccount.get()->ToString().c_str());
		if (!bRet) {
			return false;
		}
		NewAccont.push_back(vmAccount);
//		m_dblog->insert(m_dblog->end(), vAuthorLog.begin(), vAuthorLog.end());

	}
	return true;
}

const CRegID& CVmRunEvn::GetScriptRegID() {   //获取目的账户ID
	CTransaction* pcTx = static_cast<CTransaction*>(listTx.get());
	return boost::get<CRegID>(pcTx->m_cDesUserId);
}

const CRegID &CVmRunEvn::GetTxAccount() {
	CTransaction* pcTx = static_cast<CTransaction*>(listTx.get());
	return boost::get<CRegID>(pcTx->m_cSrcRegId);
}

uint64_t CVmRunEvn::GetValue() const {
	CTransaction* pcTx = static_cast<CTransaction*>(listTx.get());
	return pcTx->m_ullValues;
}

const vector<unsigned char>& CVmRunEvn::GetTxContact() {
	CTransaction* pcTx = static_cast<CTransaction*>(listTx.get());
	return pcTx->m_vchContract;
}

int CVmRunEvn::GetComfirHeight() {
	return RunTimeHeight;
}

uint256 CVmRunEvn::GetCurTxHash() {
	return listTx.get()->GetHash();
}

CScriptDBViewCache* CVmRunEvn::GetScriptDB() {
	return m_ScriptDBTip;
}

CAccountViewCache * CVmRunEvn::GetCatchView() {
	return m_view;
}

void CVmRunEvn::InsertOutAPPOperte(const vector<unsigned char>& vuchUserId, const CAppFundOperate &vcSource) {
	if (MapAppOperate.count(vuchUserId)) {
		MapAppOperate[vuchUserId].push_back(vcSource);
	} else {
		vector<CAppFundOperate> it;
		it.push_back(vcSource);
		MapAppOperate[vuchUserId] = it;
	}

}

bool CVmRunEvn::InsertOutputData(const vector<CVmOperate>& vcSource) {
	m_output.insert(m_output.end(), vcSource.begin(), vcSource.end());
	if (m_output.size() < MAX_OUTPUT_COUNT) {
		return true;
	}
	return false;
}

shared_ptr<vector<CScriptDBOperLog> > CVmRunEvn::GetDbLog() {
	return m_dblog;
}

/**
 * 从脚本数据库中，取指定账户的 应用账户信息,同时解冻冻结金额到自由金额
 * @param vAppUserId   账户地址或regId
 * @param sptrAcc
 * @return
 */
bool CVmRunEvn::GetAppUserAccout(const vector<unsigned char> &vuchAppUserId, shared_ptr<CAppUserAccout> &sptrAcc) {
	assert(m_ScriptDBTip != NULL);
	shared_ptr<CAppUserAccout> tem = std::make_shared<CAppUserAccout>();
	if (!m_ScriptDBTip->GetScriptAcc(GetScriptRegID(), vuchAppUserId, *tem.get())) {
		tem = std::make_shared<CAppUserAccout>(vuchAppUserId);
		sptrAcc = tem;
		return true;
	}
	if (!tem.get()->AutoMergeFreezeToFree(GetScriptRegID().getHight(), RunTimeHeight)) {
		return false;
	}
	sptrAcc = tem;
	return true;
}

bool CVmRunEvn::OpeatorAppAccount(const map<vector<unsigned char>, vector<CAppFundOperate> > opMap,
		CScriptDBViewCache& view) {
	NewAppUserAccout.clear();
	if ((MapAppOperate.size() > 0)) {
		for (auto const tem : opMap) {
			shared_ptr<CAppUserAccout> sptrAcc;
			if (!GetAppUserAccout(tem.first, sptrAcc)) {
				LogPrint("vm", "GetAppUserAccout(tem.first, sptrAcc, true) failed \r\n appuserid :%s\r\n",
						HexStr(tem.first));
				return false;
			}
			if (!sptrAcc.get()->AutoMergeFreezeToFree(GetScriptRegID().getHight(), RunTimeHeight)) {
				LogPrint("vm", "AutoMergeFreezeToFreefailed \r\n appuser :%s\r\n", sptrAcc.get()->toString());
				return false;

			}
			shared_ptr<CAppUserAccout> vmAppAccount = GetAppAccount(sptrAcc);
			if (vmAppAccount.get() == NULL) {
				RawAppUserAccout.push_back(sptrAcc);
				vmAppAccount = sptrAcc;
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
			NewAppUserAccout.push_back(sptrAcc);
			CScriptDBOperLog log;
			view.SetScriptAcc(GetScriptRegID(), *sptrAcc.get(), log);
			shared_ptr<vector<CScriptDBOperLog> > m_dblog = GetDbLog();
			m_dblog.get()->push_back(log);

		}
	}
	return true;
}

void CVmRunEvn::SetCheckAccount(bool bCheckAccount) {
	m_bIsCheckAccount = bCheckAccount;
}

Object CVmOperate::ToJson() {
	Object obj;
	if (m_uchNaccType == EM_REGID) {
		vector<unsigned char> vuchRegId(m_arruchAccountId, m_arruchAccountId + 6);
		CRegID cRegid(vuchRegId);
		obj.push_back(Pair("cRegid", cRegid.ToString()));
	} else if (m_uchNaccType == EM_BASE_58_ADDR) {
		string strAddr(m_arruchAccountId, m_arruchAccountId + sizeof(m_arruchAccountId));
		obj.push_back(Pair("addr", strAddr));
	}
	if (m_uchOpeatorType == ADD_FREE) {
		obj.push_back(Pair("opertype", "add"));
	} else if (m_uchOpeatorType == MINUS_FREE) {
		obj.push_back(Pair("opertype", "minus"));
	}
	if (m_unOutHeight > 0)
		obj.push_back(Pair("freezeheight", (int) m_unOutHeight));
	uint64_t amount;
	memcpy(&amount, m_arruchMoney, sizeof(m_arruchMoney));
	obj.push_back(Pair("amount", amount));
	return obj;
}
