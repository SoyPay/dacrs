/*
 * Black_Halo_tests.cpp
 *
 *  Created on: 2014年12月30日
 *      Author: ranger.shi
 */

#include "black_halo_tests.h"

CBlackHalo::CBlackHalo() {
	m_nStep = 0;
	m_strSritpthash = "";
	m_strBuyerhash = "";
	m_strSellerhash = "";
	m_strBuyerconfiredhash = "";
	m_strBuyercancelhash = "";
	m_strScriptid = "";
	m_ullSendMonye = 0;
}

CBlackHalo::~CBlackHalo() {
	// todo Auto-generated destructor stub
}

emTEST_STATE CBlackHalo::Run() {
	switch (m_nStep) {
	case 0: {
		RegistScript();
		break;
	}
	case 1: {
		WaitRegistScript();
		break;
	}
	case 2: {
		SendBuyerPackage();
		break;
	}
	case 3: {
		WaitSendBuyerPackage();
		break;
	}
	case 4: {
		SendSellerPackage();
		break;
	}
	case 5: {
		WaitSendSellerPackage();
		break;
	}
	case 6: {
		SendBuyerConfirmedPackage();
		break;
	}
	case 7: {
		WaitSendBuyerConfirmedPackage();
		break;
	}
	default:
		break;
	}
	return EM_NEXT_STATE;
}

bool CBlackHalo::RegistScript() {
	const char* pKey[] = { "cNcJkU44oG3etbWoEvY46i5qWPeE8jVb7K44keXxEQxsXUZ85MKU",
			"cVFWoy8jmJVVSNnMs3YRizkR7XEekMTta4MzvuRshKuQEEJ4kbNg", };
	int nCount = sizeof(pKey) / sizeof(char*);
	m_cBasetest.ImportWalletKey(pKey, nCount);

	string strFileName("darksecure.bin");
	int nFee = GetRandomFee();
	int nCurHight;
	m_cBasetest.GetBlockHeight(nCurHight);
	//注册对赌脚本
	Value regscript = m_cBasetest.RegisterAppTx(BUYER_A, strFileName, nCurHight, nFee + COIN);
	if (m_cBasetest.GetHashFromCreatedTx(regscript, m_strSritpthash)) {
		m_nStep++;
		return true;
	}
	return true;
}

bool CBlackHalo::WaitRegistScript() {
	if (m_cBasetest.GetTxConfirmedRegID(m_strSritpthash, m_strScriptid)) {
		m_nStep++;
		return true;
	}
	return true;
}

bool CBlackHalo::SendBuyerPackage() {
	if (m_strScriptid == "") {
		return false;
	}
	ST_FIRST_CONTRACT tSenddata;
	tSenddata.uchDnType = 0x01;
	string strRegid = "";
	string strSellerAddr = SELLER_B;
	BOOST_CHECK(m_cBasetest.GetRegID(strSellerAddr, strRegid));
	CRegID cSellerRegId(strRegid);
	memcpy(tSenddata.arruchSeller, &cSellerRegId.GetVec6().at(0), sizeof(tSenddata.arruchSeller));

	CDataStream cScriptData(SER_DISK, g_sClientVersion);
	cScriptData << tSenddata;
	string strSendContract = HexStr(cScriptData);
	m_ullSendMonye = GetPayMoney();
	Value buyerpack = m_cBasetest.CreateContractTx(m_strScriptid, BUYER_A, strSendContract, 0, 0, m_ullSendMonye);

	if (m_cBasetest.GetHashFromCreatedTx(buyerpack, m_strBuyerhash)) {
		m_nStep++;
		return true;
	}
	return true;
}

bool CBlackHalo::WaitSendBuyerPackage() {
	string strIndex = "";
	if (m_cBasetest.GetTxConfirmedRegID(m_strBuyerhash, strIndex)) {
		m_nStep++;
		return true;
	}
	return true;
}

bool CBlackHalo::SendSellerPackage() {
	if (m_strScriptid == "") {
		return false;
	}

	ST_NEXT_CONTRACT tSeller;

	tSeller.uchDnType = 0x02;
	memcpy(tSeller.arruchHash, uint256S(m_strBuyerhash).begin(), sizeof(tSeller.arruchHash));

	CDataStream cScriptData(SER_DISK, g_sClientVersion);
	cScriptData << tSeller;
	string strSendContract = HexStr(cScriptData);

	Value Sellerpack = m_cBasetest.CreateContractTx(m_strScriptid, SELLER_B, strSendContract, 0, 0, m_ullSendMonye / 2);

	if (m_cBasetest.GetHashFromCreatedTx(Sellerpack, m_strSellerhash)) {
		m_nStep++;
		return true;
	}

	return true;
}

bool CBlackHalo::WaitSendSellerPackage() {
	string strIndex = "";
	if (m_cBasetest.GetTxConfirmedRegID(m_strSellerhash, strIndex)) {
		m_nStep++;
		return true;
	}
	return true;
}

bool CBlackHalo::SendBuyerConfirmedPackage() {
	if (m_strScriptid == "") {
		return false;
	}
	ST_NEXT_CONTRACT tSeller;
	tSeller.uchDnType = 0x03;
	memcpy(tSeller.arruchHash, uint256S(m_strBuyerhash).begin(), sizeof(tSeller.arruchHash));

	CDataStream cScriptData(SER_DISK, g_sClientVersion);
	cScriptData << tSeller;
	string strSendContract = HexStr(cScriptData);
	Value Sellerpack = m_cBasetest.CreateContractTx(m_strScriptid, BUYER_A, strSendContract, 0);

	if (m_cBasetest.GetHashFromCreatedTx(Sellerpack, m_strBuyerconfiredhash)) {
		m_nStep++;
		return true;
	}

	return true;
}

bool CBlackHalo::WaitSendBuyerConfirmedPackage() {
	string strIndex = "";
	if (m_cBasetest.GetTxConfirmedRegID(m_strBuyerconfiredhash, strIndex)) {
		m_nStep = 1;
		IncSentTotal();
		return true;
	}
	return true;
}

bool CBlackHalo::SendBuyerCancelPackage() {
	if (m_strScriptid == "") {
		return false;
	}

	ST_NEXT_CONTRACT tSeller;
	tSeller.uchDnType = 0x04;
	memcpy(tSeller.arruchHash, uint256S(m_strBuyerhash).begin(), sizeof(tSeller.arruchHash));

	CDataStream cScriptData(SER_DISK, g_sClientVersion);
	cScriptData << tSeller;
	string strSendContract = HexStr(cScriptData);
	Value Sellerpack = m_cBasetest.CreateContractTx(m_strScriptid, BUYER_A, strSendContract, 0);

	if (m_cBasetest.GetHashFromCreatedTx(Sellerpack, m_strBuyercancelhash)) {
		m_nStep++;
		return true;
	}
	return true;
}

bool CBlackHalo::WaitSendBuyerCancelPackage() {
	string strIndex = "";
	if (m_cBasetest.GetTxConfirmedRegID(m_strBuyercancelhash, strIndex)) {
		m_nStep = 1;
		return true;
	}
	return true;
}
