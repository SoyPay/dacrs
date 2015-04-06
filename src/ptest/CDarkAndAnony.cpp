/*
 * CDarkAndAnony.cpp
 *
 *  Created on: 2014年12月30日
 *      Author: ranger.shi
 */

#include "CDarkAndAnony.h"

CDarkAndAnony::CDarkAndAnony() {
	step  = 0;
	buyerhash = "";
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
    	 SendBuyerPackage();
    	 break;
     case 2:
    	 SendSellerPackage();
    	 break;
     case 3:
    	 SendBuyerConfirmedPackage();
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
	BOOST_CHECK(basetest.GetHashFromCreatedTx(regscript, sritpthash));
	BOOST_CHECK(basetest.GenerateOneBlock());
	if (basetest.GetTxConfirmedRegID(sritpthash, scriptid)) {
			step++;
			return true;
	}
	return false;
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

		BOOST_CHECK(basetest.GetHashFromCreatedTx(buyerpack, buyerhash));
		BOOST_CHECK(basetest.GenerateOneBlock());

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

	string sellerhash = "";
	BOOST_CHECK(basetest.GetHashFromCreatedTx(Sellerpack, sellerhash));
	BOOST_CHECK(basetest.GenerateOneBlock());

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

	string sellerhash = "";
	BOOST_CHECK(basetest.GetHashFromCreatedTx(Sellerpack, sellerhash));
	BOOST_CHECK(basetest.GenerateOneBlock());

	string index = "";
	if (basetest.GetTxConfirmedRegID(sellerhash, index)) {
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

	string sellerhash = "";
	BOOST_CHECK(basetest.GetHashFromCreatedTx(Sellerpack, sellerhash));
	BOOST_CHECK(basetest.GenerateOneBlock());

	string index = "";
	if (basetest.GetTxConfirmedRegID(sellerhash, index)) {
		step = 1;
			return true;
	}
	return true;
}
