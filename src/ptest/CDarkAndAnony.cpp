/*
 * CDarkAndAnony.cpp
 *
 *  Created on: 2014Äê12ÔÂ30ÈÕ
 *      Author: ranger.shi
 */

#include "CDarkAndAnony.h"

CDarkAndAnony::CDarkAndAnony() {
	step  = 0;
	 darkhash = "";
	 anonyhahs= "";
	 dest1[0] ="000000001400";
	dest1[1] ="000000000e00";
	 dest1[2] ="000000000100";
}

CDarkAndAnony::~CDarkAndAnony() {
	// todo Auto-generated destructor stub
}

string CDarkAndAnony::CreateDarkTx(string scriptid, string buyeraddr, string selleraddr, string nfee,
		uint64_t paymoney) {
	std::vector<std::string> vInputParams;
	vInputParams.clear();
	vInputParams.push_back(scriptid);
	string temp1 = "[";
		temp1+="\""+buyeraddr+"\""+","+"\""+selleraddr+"\""+"]";
	vInputParams.push_back(temp1);

	FIRST_CONTRACT contact;
	contact.dnType = 0x01;
	contact.nHeight = 1000000;
	contact.nPayMoney = paymoney;
	CRegID regIdb(buyeraddr) ;
	CRegID regIds(selleraddr) ;
	memcpy(contact.buyer,&regIdb.GetVec6().at(0),sizeof(contact.buyer));
	memcpy(contact.seller,&regIds.GetVec6().at(0),sizeof(contact.seller));

	CDataStream scriptData(SER_DISK, CLIENT_VERSION);
	scriptData << contact;
	string temp = HexStr(scriptData);
//	cout<<"first:"<<temp<<endl;
	vInputParams.push_back(temp);
	vInputParams.push_back(nfee);
	vInputParams.push_back("0");
	std::string strReturn("");
	if(TestCallRPC("createcontracttx", vInputParams, strReturn)){
			strReturn= Parsejson(strReturn);
			vInputParams.clear();
			vInputParams.push_back(strReturn);
			if (TestCallRPC("signcontracttx", vInputParams, strReturn) > 0) {
				strReturn= Parsejson(strReturn);
			}
		}
	return strReturn;
}

string CDarkAndAnony::CreateSecondDarkTx(string scriptid, string hash, string buyeraddr, string nfee) {
	{
			std::vector<std::string> vInputParams;
			vInputParams.clear();
			vInputParams.push_back(scriptid);
			string temp1 = "[";
				temp1+="\""+buyeraddr+"\""+"]";
			vInputParams.push_back(temp1);

			uint256 hash1(hash.c_str());
			string param ="02";
			param += HexStr(hash1);
			vInputParams.push_back(param);
			vInputParams.push_back(nfee);
			vInputParams.push_back("0");
			std::string strReturn("");
			TestCallRPC("createcontracttx", vInputParams, strReturn);
			return strReturn;
		}
}


string CDarkAndAnony::Createanony(string scriptid, string addr, string toaddress1, string toaddress2, string nfee,
		uint64_t paymoney) 	{
	std::vector<std::string> vInputParams;
	vInputParams.clear();
	vInputParams.push_back(scriptid);
	string temp1 = "[";
	temp1+="\""+addr+"\""+"]";
	vInputParams.push_back(temp1);

	string temp = "";
	CONTRACT_ANONY contact;
	contact.nHeight = 10;
	contact.nPayMoney = paymoney;
//	cout<<"first:"<<paymoney<<endl;
	CRegID regIdb(addr);
	memcpy(contact.Sender,&regIdb.GetVec6().at(0),sizeof(contact.Sender));

	ACCOUNT_INFO info;
	info.nReciMoney = paymoney/2;
//	cout<<"first:"<<info.nReciMoney<<endl;
	CRegID regId1(toaddress1);
	memcpy(info.account,&regId1.GetVec6().at(0),sizeof(info.account));

	ACCOUNT_INFO info1;
	info1.nReciMoney =  paymoney/2;
	CRegID regId2(toaddress2);
	memcpy(info1.account,&regId2.GetVec6().at(0),sizeof(info1.account));

	contact.len = ::GetSerializeSize(info, SER_DISK, CLIENT_VERSION)*2;
//	cout<<contact.len<<endl;
	CDataStream scriptData(SER_DISK, CLIENT_VERSION);
	scriptData << contact;
	scriptData << info;
	scriptData << info1;
	temp = HexStr(scriptData);
//	cout<<"cotx:"<<temp<<endl;
	vInputParams.push_back(temp);
	vInputParams.push_back(nfee);
	vInputParams.push_back("0");
	std::string strReturn("");
	TestCallRPC("createcontracttx", vInputParams, strReturn);

	return strReturn;
}

TEST_STATE CDarkAndAnony::run()
{
     switch(step)
     {
     case 0:
    	 step0RegistScript();
    	 break;
     case 1:
    	 step1RegistScript();
    	 break;
     case 2:
          step2RegistScript();
          break;
     }
	return next_state;
}


string CDarkAndAnony::GetScript(string hash) 	{
	std::vector<std::string> vInputParams;
	vInputParams.push_back(hash);
	std::string strReturn("");
	if(TestCallRPC("getscriptid", vInputParams, strReturn) > 0)
	{
		strReturn = Parsejson(strReturn);
	}
	return strReturn;
}

string CDarkAndAnony::Parsejson(string str) 	{
	json_spirit::Value val;
	json_spirit::read(str, val);
	if (val.type() != obj_type)
	{
		return "";
	}
	json_spirit::Value::Object obj=  val.get_obj();
	string ret;
	for(int i = 0; i < obj.size(); ++i)
	{
		const json_spirit::Pair& pair = obj[i];
		const std::string& str_name = pair.name_;
		const json_spirit::Value& val_val = pair.value_;
		if(str_name =="rawtx")
		{
			if(val_val.get_str() != "")
			{
				ret = val_val.get_str();
			}
		}else if(str_name =="hash")
		{
			ret = val_val.get_str();
		}else if(str_name =="script")
				{
					ret = val_val.get_str();
				}

	}
	return ret;
}

string CDarkAndAnony::CreateScript(char* vmpath, string addr, string nfee)	{
	std::vector<std::string> vInputParams;
	vInputParams.push_back(addr);
	vInputParams.push_back("0");
	vInputParams.push_back(vmpath);
	vInputParams.push_back(nfee);
	vInputParams.push_back("0");
	std::string strReturn("");
	TestCallRPC("registerscripttx", vInputParams, strReturn);
	return strReturn;
}

void CDarkAndAnony::SendDarkTx(string scriptid) {
	string buyaddr,selleraddr;
	GetAddress(buyaddr,selleraddr);
	string nfee;
	nfee = strprintf("%d",GetRandomFee());
	GetAddress(buyaddr,selleraddr);
	nfee = strprintf("%d",GetRandomFee());
	uint64_t paymoney =GetPayMoney();
	string txhash = CreateDarkTx(scriptid,buyaddr,selleraddr,nfee,paymoney);

	BOOST_CHECK(txhash != "");

	if(txhash != "")
	{
	  string hash = CreateSecondDarkTx(scriptid,txhash,buyaddr,nfee);
	  BOOST_CHECK(Parsejson(hash) != "");
	}
}

int CDarkAndAnony::TestCallRPC(std::string strMethod, const std::vector<std::string>& vParams, std::string& strRet) {

	string strPrint;
	int nRet;
	Array params = RPCConvertValues(strMethod, vParams);

	Object reply = CallRPC(strMethod, params);

	// Parse reply
	const Value& result = find_value(reply, "result");
	const Value& error = find_value(reply, "error");

	if (error.type() != null_type) {
		// Error
		strPrint = "error: " + write_string(error, false);
		int code = find_value(error.get_obj(), "code").get_int();
		nRet = abs(code);
	} else {
		// Result
		if (result.type() == null_type)
			strPrint = "";
		else if (result.type() == str_type)
			strPrint = result.get_str();
		else
			strPrint = write_string(result, true);
	}
	strRet = strPrint;
	BOOST_MESSAGE(strPrint);
	//cout << strPrint << endl;
	return nRet;
}
void CDarkAndAnony::SendanonyTx(string scriptid) {
	string sendaddr,recviaddr1,reciveaddr2;
	GetAddress(sendaddr,recviaddr1,reciveaddr2);
	string nfee;
	nfee = strprintf("%d",GetRandomFee());
	uint64_t paymoney =GetPayMoney();
//	cout<<sendaddr<<endl;
//	cout<<recviaddr1<<endl;
//	cout<<reciveaddr2<<endl;
	string txhash = Createanony(scriptid,sendaddr,recviaddr1,reciveaddr2,nfee,paymoney);
	cout<<txhash<<endl;
	txhash = Parsejson(txhash);
	BOOST_CHECK(txhash != "");

}
