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

vector<shared_ptr<CSecureAccount> > &CVmScriptRun::GetRawAccont() {
	return RawAccont;
}
vector<shared_ptr<CSecureAccount> > &CVmScriptRun::GetNewAccont() {
	return NewAccont;
}

bool CVmScriptRun::intial(vector<shared_ptr<CBaseTransaction> >& Tx,CAccountViewCache& view) {

	IsRun = false;
	listTx = Tx;
	vector<unsigned char> vScript;

	auto checkPara = [&]()
	{
		if(Tx.size() ==0)
		return false;
		auto tx = Tx[0].get();
		if(tx->nTxType != SECURE_TX)
		return false;
		for (auto& tx : Tx) {
			if (tx.get()->nTxType != SECURE_TX && tx.get()->nTxType != APPEAL_TX ) {
				return false;
			}
		}

		return true;
	};

	if (!checkPara()) {
		LogPrint("vm", "%s\r\n", "err param");
		return false;
	}

	CSecureTransaction* secure = static_cast<CSecureTransaction*>(Tx[0].get());
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

	CDataStream VmData(SER_DISK, CLIENT_VERSION);
	Cpackes Vmpacket;
	//// Write VmHeadData
	CVmHeadData headdata;
	headdata.ArbitratorAccCount = secure->vArbitratorRegAccId.size();
	headdata.AppealTxCount = Tx.size() - 1;
	headdata.vAccountCount = secure->vRegAccountId.size();
	Vmpacket.vhead = headdata;

	for (auto& tx : Tx) {
		if (tx.get()->nTxType == APPEAL_TX) {
			CAppealTransaction* appealTx = static_cast<CAppealTransaction*>(tx.get());
			CVmAppealTxPackes AppealTxData;

			AppealTxData.AppealTxContract.insert(AppealTxData.AppealTxContract.begin(), appealTx->vContract.begin(),
					appealTx->vContract.end());
			AppealTxData.sigaccountid.insert(AppealTxData.sigaccountid.begin(), appealTx->vPreAcountIndex.begin(),
					appealTx->vPreAcountIndex.end());

			Vmpacket.vAppealTxPacke.push_back(AppealTxData);
		} else if (tx.get()->nTxType == SECURE_TX) {
			CSecureTransaction* SecureTx = static_cast<CSecureTransaction*>(tx.get());
			vector<unsigned char> sigaccountlist;
			int count = SecureTx->vArbitratorRegAccId.size();
			for (int i = 0; i < SecureTx->vRegAccountId.size(); i++) {
				unsigned char ch;
				count += i;
				memcpy(&ch, &count, 1);
				sigaccountlist.push_back(ch);
			}

			CVmSecureTxData SecureTxData;
			SecureTxData.Contract.insert(SecureTxData.Contract.begin(), tx.get()->GetvContract().begin(),
					tx.get()->GetvContract().end());
			SecureTxData.sigaccountid.insert(SecureTxData.sigaccountid.begin(), sigaccountlist.begin(),
					sigaccountlist.end());
			Vmpacket.vsecuretx = SecureTxData;
		} else {
			assert(0);
			return false;
		}
	}

	VmData << Vmpacket;

	for (auto& tx : secure->vArbitratorRegAccId) {
		auto tem = make_shared<CSecureAccount>();
		view.GetAccount(tx, *tem.get());
		vArbitratorAcc.push_back(tem);
	}
	RawAccont.insert(RawAccont.end(), vArbitratorAcc.begin(), vArbitratorAcc.end());
	for (auto& tx : secure->vRegAccountId) {
		auto tem = make_shared<CSecureAccount>();
		view.GetAccount(tx, *tem.get());
		RawAccont.push_back(tem);
	}

	vector<unsigned char> strContract;
	strContract.assign(VmData.begin(), VmData.end());
	LogPrint("vm","Tx size:%d,Contract:%s\r\n",Tx.size(),HexStr(VmData).c_str());
	pMcu = make_shared<CVir8051>(vmScript.Rom, strContract);
	return true;
}

CVmScriptRun::~CVmScriptRun() {

}

bool CVmScriptRun::run(vector<shared_ptr<CBaseTransaction> >& Tx, CAccountViewCache& view) {

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
	CVmOperatePacke retpack;
	Contractstream >> retpack;
	vector<CVmOperate> retvmcode;
	retvmcode.assign(retpack.vmpackets.begin(), retpack.vmpackets.end());

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
shared_ptr<CSecureAccount> CVmScriptRun::GetNewAccount(shared_ptr<CSecureAccount>& vOldAccount) {
	if (NewAccont.size() == 0)
		return NULL;
	vector<shared_ptr<CSecureAccount> >::iterator Iter;
	for (Iter = NewAccont.begin(); Iter != NewAccont.end(); Iter++) {
		shared_ptr<CSecureAccount> temp = *Iter;
		if (temp.get()->keyID == vOldAccount.get()->keyID) {
			NewAccont.erase(Iter);
			return temp;
		}
	}
	return NULL;
}
bool CVmScriptRun::CheckOperate(const vector<CVmOperate> &listoperate) const {
	// judge contract rulue
	for (auto& it : listoperate) {
		if (it.muls.accountid >= maxAccountIndex() || it.muls.txid >= maxTxIndex()
				|| it.add.accountid >= maxAccountIndex() || it.add.txid >= maxTxIndex()) {
			LogPrint("vm", "VmScript OpeatorSecureAccount accountid not vaild\n");
			return false;
		}
		if (!(it.add.Opeater == ADD_FREE || it.add.Opeater == ADD_SELF_FREEZD || it.add.Opeater == ADD_INPUT_FREEZD)) {
			LogPrint("vm", "VmScript OpeatorSecureAccount operate not vaild\n");
			return false;
		}
		if (!(it.muls.Opeater == MINUS_FREE || it.muls.Opeater == MINUS_FREE_TO_OUTPUT
				|| it.muls.Opeater == MINUS_OUTPUT || it.muls.Opeater == MINUS_OUTPUT_OR_FREE
				|| it.muls.Opeater == MINUS_OUTPUT_OR_FREE_OR_SELF || it.muls.Opeater == MINUS_INPUT
				|| it.muls.Opeater == MINUS_INPUT_OR_FREE || it.muls.Opeater == MINUS_INPUT_OR_FREE_OR_SELF)) {
			LogPrint("vm", "VmScript OpeatorSecureAccount muls operate not vaild\n");
			return false;
		}
		if (atoi64((char*)it.muls.money) != atoi64((char*)it.add.money)) {
			LogPrint("vm", "VmScript OpeatorSecureAccount muls and add value not Equal\n");
			return false;
		}
		if (IsFirstTx()) {
			if (it.add.outheight > vmScript.rule.vpreOutHeihgt)
			{
				LogPrint("vm", "VmScript OpeatorSecureAccount add height not less rule height\n");
				return false;
			}
		} else {
			if (it.add.outheight > vmScript.rule.vNextOutHeight)
			{
				LogPrint("vm", "VmScript OpeatorSecureAccount appealtx add %d height not less rule %d height\n",it.add.outheight,vmScript.rule.vNextOutHeight);
				return false;
			}
		}
//		if (it.muls.accountid < (vArbitratorAcc.size() -1)) {
//			if (atoi64((char*)it.muls.money) > vmScript.rule.maxReSv)
//			{
//				LogPrint("vm", "VmScript OpeatorSecureAccount muls money not less rule maxReSv\n");
//				return false;
//			}
//		}
//		if (it.add.accountid < (vArbitratorAcc.size() -1)) {
//			if (atoi64((char*)it.muls.money) > vmScript.rule.maxPay)
//			{
//				LogPrint("vm", "VmScript OpeatorSecureAccount muls money not less rule maxPay\n");
//				return false;
//			}
//		}
	}
	return true;
}
shared_ptr<vector<CVmOperate>> CVmScriptRun::GetOperate() const
{
	auto tem = make_shared<vector<CVmOperate>>();
	shared_ptr<vector<unsigned char>> retData = pMcu.get()->GetRetData();
	CDataStream Contractstream(*retData.get(), SER_DISK, CLIENT_VERSION);
	CVmOperatePacke retpack;
	Contractstream >> retpack;
	tem.get()->assign(retpack.vmpackets.begin(), retpack.vmpackets.end());
	return tem;
}
bool CVmScriptRun::OpeatorSecureAccount(const vector<CVmOperate>& listoperate) {

	NewAccont.clear();
	for (auto& it : listoperate) {
		CFund mulsfund;
		mulsfund.value = atoi64((char*)it.muls.money);
		mulsfund.nHeight = it.muls.outheight + chainActive.Height();
		mulsfund.uTxHash = listTx[it.muls.txid].get()->GetHash();
		if ((OperType) it.muls.Opeater == MINUS_INPUT_OR_FREE_OR_SELF || (OperType) it.muls.Opeater == MINUS_INPUT) {
			mulsfund.nFundType = INPUT_FREEZD_FUND;

		}
		CFund addfund;
		addfund.value = atoi64((char*)it.add.money);
		addfund.nHeight = it.add.outheight + chainActive.Height();
		addfund.uTxHash = listTx[it.add.txid].get()->GetHash();
		if ((OperType) it.add.Opeater == ADD_INPUT_FREEZD) {
			addfund.nFundType = INPUT_FREEZD_FUND;
		}

		if ((OperType) it.add.Opeater == ADD_FREE) {
			addfund.nFundType = FREEDOM_FUND;
		}
		shared_ptr<CSecureAccount> mulsAccount = RawAccont[it.muls.accountid];
		shared_ptr<CSecureAccount> vnewAccount = GetNewAccount(mulsAccount);
		if (vnewAccount.get() != NULL) {
			mulsAccount = vnewAccount;
		}

		if(mulsfund.nFundType == INPUT_FREEZD_FUND)
		{
			CFund vFind = mulsAccount.get()->FindFund(mulsAccount.get()->vInputFreeze,mulsfund.uTxHash);
			mulsfund.nHeight = vFind.nHeight;
		}

		LogPrint("vm","muls account:%s\r\n",mulsAccount.get()->ToString().c_str());
		LogPrint("vm","fund:%s\r\n",mulsfund.ToString().c_str());
//		cout<<mulsAccount.get()->ToString().c_str()<<endl;
//		cout<<"fund:"<<mulsfund.ToString().c_str()<<endl;
		uint64_t retValue;
		bool retflag = mulsAccount.get()->OperateAccount((OperType) it.muls.Opeater, mulsfund, &retValue);
		LogPrint("vm","after muls account:%s\r\n",mulsAccount.get()->ToString().c_str());
		if (it.muls.ResultCheck == 0x01 && retValue != atoi64((char*)it.muls.money)) {
			return false;
		}
		addfund.value = retValue;

		shared_ptr<CSecureAccount> addAccount = RawAccont[it.add.accountid];
		shared_ptr<CSecureAccount> vaddnewAccount = GetNewAccount(addAccount);
		if (vaddnewAccount.get() != NULL) {
			addAccount = vaddnewAccount;
		}
		LogPrint("vm","add addAccount:%s\r\n",mulsAccount.get()->ToString().c_str());
		LogPrint("vm","fund:%s\r\n",addfund.ToString().c_str());
		addAccount.get()->OperateAccount((OperType) it.add.Opeater, addfund);
		LogPrint("vm","after addAccount:%s\r\n",mulsAccount.get()->ToString().c_str());
		if(mulsAccount.get()->keyID == addAccount.get()->keyID)
		{
			NewAccont.push_back(addAccount);
		}
		else
		{
			NewAccont.push_back(mulsAccount);
			NewAccont.push_back(addAccount);
		}

	}
	return true;
}

CVmScriptRun::CVmScriptRun(CAccountViewCache& view, vector<shared_ptr<CBaseTransaction> >& Tx,CVmScript& script)
{
	IsRun = false;
	listTx = Tx;
	vector<unsigned char> vScript;

	vmScript = script;
	auto checkPara = [&]()
	{
		if(Tx.size() ==0)
		return false;
		auto tx = Tx[0].get();
		if(tx->nTxType != SECURE_TX)
		{
			return false;
		}

		for (auto& tx : Tx) {
			if (tx.get()->nTxType != SECURE_TX && tx.get()->nTxType != APPEAL_TX) {
				return false;
			}
		}

		return true;
	};

	if (!checkPara()) {
		LogPrint("vm", "%s%s\r\n", getFilelineStr(), "err param");
		return;
	}

	CSecureTransaction* secure = static_cast<CSecureTransaction*>(Tx[0].get());

	CDataStream VmData(SER_DISK, CLIENT_VERSION);
	Cpackes Vmpacket;
	//// Write VmHeadData
	CVmHeadData headdata;
	headdata.ArbitratorAccCount = secure->vArbitratorRegAccId.size();
	headdata.AppealTxCount = Tx.size() - 1;
	headdata.vAccountCount = secure->vRegAccountId.size();
	sprintf((char*)headdata.vCurrentH,"%d",chainActive.Height());

	Vmpacket.vhead = headdata;

	for (auto& tx : Tx) {
		if (tx.get()->nTxType == APPEAL_TX) {
			CAppealTransaction* appealTx = static_cast<CAppealTransaction*>(tx.get());
			CVmAppealTxPackes AppealTxData;
			for (auto& ch : appealTx->vPreAcountIndex) {

			AppealTxData.AppealTxContract.insert(AppealTxData.AppealTxContract.begin(), appealTx->vContract.begin(),
					appealTx->vContract.end());
			AppealTxData.sigaccountid.insert(AppealTxData.sigaccountid.begin(), appealTx->vPreAcountIndex.begin(),
					appealTx->vPreAcountIndex.end());

			Vmpacket.vAppealTxPacke.push_back(AppealTxData);
			}
		}
		if (tx.get()->nTxType == SECURE_TX) {
			CSecureTransaction* SecureTx = static_cast<CSecureTransaction*>(tx.get());
			vector<unsigned char> sigaccountlist;
			int count = SecureTx->vArbitratorRegAccId.size();
			for (int i = 0; i < SecureTx->vRegAccountId.size(); i++) {
				unsigned char ch;
				count += i;
				memcpy(&ch, &count, 1);
				sigaccountlist.push_back(ch);
			}

			CVmSecureTxData SecureTxData;
			SecureTxData.Contract.insert(SecureTxData.Contract.begin(), tx.get()->GetvContract().begin(),
					tx.get()->GetvContract().end());
			SecureTxData.sigaccountid.insert(SecureTxData.sigaccountid.begin(), sigaccountlist.begin(),
					sigaccountlist.end());
			Vmpacket.vsecuretx = SecureTxData;
		}
	}

	VmData << Vmpacket;

	for (auto& tx : secure->vArbitratorRegAccId) {
		auto tem = make_shared<CSecureAccount>();
		view.GetAccount(tx, *tem.get());
		vArbitratorAcc.push_back(tem);
	}
	RawAccont.insert(RawAccont.end(), vArbitratorAcc.begin(), vArbitratorAcc.end());
	for (auto& tx : secure->vRegAccountId) {
		auto tem = make_shared<CSecureAccount>();
		view.GetAccount(tx, *tem.get());
		RawAccont.push_back(tem);
	}

	vector<unsigned char> strContract;
	strContract.assign(VmData.begin(), VmData.end());

//	cout << "contct:"<<HexStr(strContract).c_str() << endl;
	pMcu = make_shared<CVir8051>(vmScript.Rom, strContract);
}
