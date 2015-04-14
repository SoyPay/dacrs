/*
 * CDarkAndAnony.cpp
 *
 *  Created on: 2014年12月30日
 *      Author: ranger.shi
 */

#include "CDarkAndAnony.h"

CDarkAndAnony::CDarkAndAnony() {
	 step =0 ;
	 sritpthash = "";
	 buyerhash = "";
	 sellerhash= "";
	 buyerconfiredhash = "";
	 buyercancelhash = "";
	 scriptid = "";
	 sendmonye = 0;
}

CDarkAndAnony::~CDarkAndAnony() {
	// todo Auto-generated destructor stub
}

TEST_STATE CDarkAndAnony::run()
{
     switch(step)
     {
     case 0:
    	 RegistScript();
    	 break;
     case 1:
    	 WaitRegistScript();
    	 break;
     case 2:
    	 SendBuyerPackage();
    	 break;
     case 3:
    	 WaitSendBuyerPackage();
    	 break;
     case 4:
    	 SendSellerPackage();
    	 break;
     case 5:
    	 WaitSendSellerPackage();
    	 break;
     case 6:
    	 SendBuyerConfirmedPackage();
          break;
     case 7:
          WaitSendBuyerConfirmedPackage();
          break;
     }
	return next_state;
}

bool CDarkAndAnony::RegistScript(){
	const char* pKey[] = { "cNcJkU44oG3etbWoEvY46i5qWPeE8jVb7K44keXxEQxsXUZ85MKU",
			"cVFWoy8jmJVVSNnMs3YRizkR7XEekMTta4MzvuRshKuQEEJ4kbNg", };
	int nCount = sizeof(pKey) / sizeof(char*);
	basetest.ImportWalletKey(pKey, nCount);

	string strFileName("darksecure.bin");
	int nFee = GetRandomFee();
	int nCurHight;
	basetest.GetBlockHeight(nCurHight);
	//注册对赌脚本
	Value regscript = basetest.RegisterScriptTx(BUYER_A, strFileName, nCurHight, nFee);
	if(basetest.GetHashFromCreatedTx(regscript, sritpthash)){
		step++;
		return true;
	}
	return true;
}
bool CDarkAndAnony::WaitRegistScript(){
	if (basetest.GetTxConfirmedRegID(sritpthash, scriptid)) {
			step++;
			return true;
	}
	return true;
}

bool CDarkAndAnony::SendBuyerPackage(){
	if(scriptid == "")
		return false;

		FIRST_CONTRACT senddata;
		senddata.dnType = 0x01;
		string strregid = "";
		string selleraddr = SELLER_B;
		BOOST_CHECK(basetest.GetRegID(selleraddr,strregid));
		CRegID Sellerregid(strregid);
		memcpy(senddata.seller,&Sellerregid.GetVec6().at(0),sizeof(senddata.seller));

		CDataStream scriptData(SER_DISK, CLIENT_VERSION);
		scriptData << senddata;
		string sendcontract = HexStr(scriptData);
		cout <<sendcontract<<endl;
		sendmonye = GetPayMoney();
		Value  buyerpack= basetest.CreateContractTx(scriptid,BUYER_A,sendcontract,0,0,sendmonye);

		if(basetest.GetHashFromCreatedTx(buyerpack, buyerhash)){
			step++;
			return true;
		}
		return true;
}
bool CDarkAndAnony::WaitSendBuyerPackage(){
	string index = "";
	if (basetest.GetTxConfirmedRegID(buyerhash, index)) {
		step++;
		return true;
	}
	return true;
}

bool CDarkAndAnony::SendSellerPackage(){
	if(scriptid == "")
		return false;

	NEXT_CONTRACT Seller;
	unsigned int Size = ::GetSerializeSize(Seller, SER_DISK, CLIENT_VERSION);
	Seller.dnType = 0x02;
	memcpy(Seller.hash, uint256(buyerhash).begin(), sizeof(Seller.hash));

	CDataStream scriptData(SER_DISK, CLIENT_VERSION);
	scriptData << Seller;
	string sendcontract = HexStr(scriptData);
	cout<<"size:"<<Size<<" " <<sendcontract<<endl;

	Value  Sellerpack= basetest.CreateContractTx(scriptid,SELLER_B,sendcontract,0,0,sendmonye/2);

	if(basetest.GetHashFromCreatedTx(Sellerpack, sellerhash)){
		step++;
		return true;
	}

	return true;
}
bool CDarkAndAnony::WaitSendSellerPackage(){
	string index = "";
	if (basetest.GetTxConfirmedRegID(sellerhash, index)) {
		step++;
			return true;
	}
	return true;
}

bool CDarkAndAnony::SendBuyerConfirmedPackage(){
	if(scriptid == "")
		return false;

	NEXT_CONTRACT Seller;
	Seller.dnType = 0x03;
	memcpy(Seller.hash, uint256(buyerhash).begin(), sizeof(Seller.hash));

	CDataStream scriptData(SER_DISK, CLIENT_VERSION);
	scriptData << Seller;
	string sendcontract = HexStr(scriptData);
	Value  Sellerpack= basetest.CreateContractTx(scriptid,BUYER_A,sendcontract,0);

	if(basetest.GetHashFromCreatedTx(Sellerpack, buyerconfiredhash)){
		step++;
		return true;
	}

	return true;
}
bool CDarkAndAnony::WaitSendBuyerConfirmedPackage(){
	string index = "";
	if (basetest.GetTxConfirmedRegID(buyerconfiredhash, index)) {
		step = 1;
			return true;
	}
	return true;
}

bool CDarkAndAnony::SendBuyerCancelPackage(){
	if(scriptid == "")
		return false;

	NEXT_CONTRACT Seller;
	Seller.dnType = 0x04;
	memcpy(Seller.hash, uint256(buyerhash).begin(), sizeof(Seller.hash));

	CDataStream scriptData(SER_DISK, CLIENT_VERSION);
	scriptData << Seller;
	string sendcontract = HexStr(scriptData);
	Value  Sellerpack= basetest.CreateContractTx(scriptid,BUYER_A,sendcontract,0);

	if(basetest.GetHashFromCreatedTx(Sellerpack, buyercancelhash)){
		step++;
		return true;
	}
	return true;
}
bool CDarkAndAnony::WaitSendBuyerCancelPackage(){
	string index = "";
	if (basetest.GetTxConfirmedRegID(buyercancelhash, index)) {
		step = 1;
		return true;
	}
	return true;
}
