/*
 * appacc_test.cpp
 *
 *  Created on: 2015年3月29日
 *      Author: ranger.shi
 */


#include <boost/foreach.hpp>
#include <boost/test/unit_test.hpp>
#include "txdb.h"
#include "database.h"
#include <iostream>
#include  "boost/filesystem/operations.hpp"
#include  "boost/filesystem/path.hpp"
#include  "../vm/appaccount.h"
#include "../vm/vmrunevn.h"
#include "tx.h"
#include "util.h"

using namespace std;

int64_t g_arrllOpValue[8][8] = {
						{100*COIN, 10*COIN, 10*COIN, 30*COIN, 40*COIN, 30*COIN ,30*COIN, 20*COIN},    //false
						{1000*COIN, 200*COIN, 20*COIN, 30*COIN, 40*COIN, 20*COIN, 10*COIN, 20*COIN},  //false
						{500*COIN, 200*COIN, 20*COIN, 100*COIN, 200*COIN, 100*COIN, 300*COIN, 100*COIN}, //false
						{100*COIN, 10*COIN, 20*COIN, 50*COIN, 50*COIN, 10*COIN, 20*COIN, 30*COIN}, //false
						{200*COIN, 20*COIN, 30*COIN, 40*COIN, 30*COIN, 30*COIN, 40*COIN, 40*COIN},  //false
						{1000*COIN, 200*COIN, 20*COIN, 500*COIN, 800*COIN, 400*COIN, 200*COIN, 100*COIN}, //true
						{500*COIN, 200*COIN, 200*COIN, 300*COIN, 200*COIN, 50*COIN, 100*COIN, 50*COIN}, //true
						{600*COIN, 200*COIN, 20*COIN, 30*COIN, 50*COIN, 60*COIN, 70*COIN, 20*COIN}  //false
						};

// appacc_tests/key_test1


bool CheckAppAcct(int64_t arrllOpValue[]) {
	CRegID cSrcRegId(100, 1);
	CRegID cDesRegId(100, 2);
	CRegID cDesUser1RegId(100, 3);
	CRegID cDesUser2RegId(100, 4);
	CAccount cContractAcct;
	cContractAcct.m_ullValues = 100 * COIN;  //这里将TX中100 COIN先充值到合约账户中，扮演系统操作账户角色
	cContractAcct.m_cRegID = cDesRegId;

	CUserID srcUserId = cSrcRegId;
	CUserID desUserId = cDesRegId;
	vector_unsigned_char pContract;
	CTransaction tx(srcUserId, cDesRegId, 10000, arrllOpValue[0], 1, pContract); //100 * COIN

	CVmRunEvn vmRunEvn;
	vector<CVmOperate> vAcctOper;

	vector_unsigned_char vDesUser1RegId = cDesUser1RegId.GetVec6();
	int64_t llTemp = arrllOpValue[1];  //10 * COIN
	CVmOperate cAcctAddOper;
	cAcctAddOper.m_uchNaccType = EM_REGID;
	cAcctAddOper.m_uchOpeatorType = EM_ADD_FREE;
	memcpy(cAcctAddOper.m_arruchAccountId, &vDesUser1RegId[0], 6);
	memcpy(cAcctAddOper.m_arruchMoney, &llTemp, sizeof(llTemp));
	vAcctOper.push_back(cAcctAddOper);

	vector_unsigned_char vDesUser2RegId = cDesUser2RegId.GetVec6();
	llTemp = arrllOpValue[2];   //20 * COIN
	cAcctAddOper.m_uchNaccType = EM_REGID;
	cAcctAddOper.m_uchOpeatorType = EM_ADD_FREE;
	memcpy(cAcctAddOper.m_arruchAccountId, &vDesUser2RegId[0], 6);
	memcpy(cAcctAddOper.m_arruchMoney, &llTemp, sizeof(llTemp));
	vAcctOper.push_back(cAcctAddOper);

	vector_unsigned_char vDesRegId = cDesRegId.GetVec6();
	llTemp = arrllOpValue[3];  //30 * COIN
	cAcctAddOper.m_uchNaccType = EM_REGID;
	cAcctAddOper.m_uchOpeatorType = EM_MINUS_FREE;
	memcpy(cAcctAddOper.m_arruchAccountId, &vDesRegId[0], 6);
	memcpy(cAcctAddOper.m_arruchMoney, &llTemp, sizeof(llTemp));
	vAcctOper.push_back(cAcctAddOper);
	vmRunEvn.InsertOutputData(vAcctOper);

	CAppFundOperate appFundOper;
	appFundOper.m_uchOpeatorType = EM_ADD_FREE_OP;
	appFundOper.m_llMoney = arrllOpValue[4];       //20 * COIN
	appFundOper.m_uchAppuserIDlen = 6;
	memcpy(appFundOper.m_arruchAppuser, &vDesUser1RegId[0], 6);
	appFundOper.m_uchFundTaglen = 6;
	memcpy(appFundOper.m_arruchFundTag, &vDesUser1RegId[0], 6);
	vmRunEvn.InsertOutAPPOperte(vDesUser1RegId, appFundOper);

	appFundOper.m_uchOpeatorType = EM_SUB_FREE_OP;
	appFundOper.m_llMoney = arrllOpValue[5];      //90 * COIN
	appFundOper.m_uchAppuserIDlen = 6;
	memcpy(appFundOper.m_arruchAppuser, &vDesUser2RegId[0], 6);
	appFundOper.m_uchFundTaglen = 6;
	memcpy(appFundOper.m_arruchFundTag, &vDesUser2RegId[0], 6);
	vmRunEvn.InsertOutAPPOperte(vDesUser2RegId, appFundOper);

	appFundOper.m_uchOpeatorType = EM_ADD_TAG_OP;
	appFundOper.m_llMoney = arrllOpValue[6];     // 90 * COIN
	appFundOper.m_uchAppuserIDlen = 6;
	memcpy(appFundOper.m_arruchAppuser, &vDesUser2RegId[0], 6);
	appFundOper.m_uchFundTaglen = 6;
	memcpy(appFundOper.m_arruchFundTag, &vDesUser2RegId[0], 6);
	vmRunEvn.InsertOutAPPOperte(vDesUser2RegId, appFundOper);

	appFundOper.m_uchOpeatorType = EM_SUB_TAG_OP;
	appFundOper.m_llMoney = arrllOpValue[7];  // 80 * COIN
	appFundOper.m_uchAppuserIDlen = 6;
	memcpy(appFundOper.m_arruchAppuser, &vDesUser1RegId[0], 6);
	appFundOper.m_uchFundTaglen = 6;
	memcpy(appFundOper.m_arruchFundTag, &vDesUser1RegId[0], 6);
	vmRunEvn.InsertOutAPPOperte(vDesUser1RegId, appFundOper);

	return vmRunEvn.CheckAppAcctOperate(&tx);
}

BOOST_AUTO_TEST_SUITE(appacc_tests)

BOOST_AUTO_TEST_CASE(key_test1) {
	auto StrTVector = [&](string tag)
	{
		return vector<unsigned char>(tag.begin(),tag.end());
	};

	srand((int) time(NULL));

	vector<unsigned char> vuchAppuserId = StrTVector("test1");
	vector<unsigned char> vuchFundtag = StrTVector("foundtag");
	vector<unsigned char> vuchFundtag2 = StrTVector("foundtag2");

	CAppFundOperate cOpTe(vuchAppuserId, vuchFundtag, EM_ADD_TAG_OP, 500, 800000);
	BOOST_CHECK(cOpTe.GetFundTagV() == vuchFundtag);
	BOOST_CHECK(cOpTe.GetUint64Value() == 800000);
	BOOST_CHECK(cOpTe.getopeatortype() == EM_ADD_TAG_OP);

	vector<CAppFundOperate> vcOpArry;
	uint64_t ullAllmoney = 0;
	int nTimeout = (rand() % 15000) + 51;
	int nLoop = 500;
	int nMaxtimeout = nTimeout + nLoop + 1;
	for (int i = 0; i < nLoop; i++) {
		int64_t llTemp = ((rand() * rand()) % 15000000) + 20;
		ullAllmoney += llTemp;
		CAppFundOperate op(vuchAppuserId, vuchFundtag, EM_ADD_TAG_OP, nTimeout + i, llTemp);
		vcOpArry.insert(vcOpArry.end(), op);
	}

	CAppUserAccout AccCount(vuchAppuserId);
	BOOST_CHECK(AccCount.getaccUserId() == vuchAppuserId);      //初始化的ID 必须是
	BOOST_CHECK(AccCount.Operate(vcOpArry));               //执行所有的操作符合
	BOOST_CHECK(AccCount.getllValues() == 0);            //因为操作符全是加冻结的钱所以自由金额必须是0

	{
		CAppCFund tep;
		BOOST_CHECK(AccCount.GetAppCFund(tep, vuchFundtag, nTimeout)); //获取相应的冻结项
		BOOST_CHECK(tep.getValue() == vcOpArry[0].GetUint64Value());                    //冻结的金额需要没有问题
		CAppCFund tep2;
		BOOST_CHECK(AccCount.GetAppCFund(tep2, vuchFundtag, nMaxtimeout + 5) == false); //获取相应的冻结项 超时时间不同 必须获取不到

		AccCount.AutoMergeFreezeToFree(0, nTimeout - 1);               	   //自动合并 超时高度没有到  这里的 50 是为了配合签名 time out de 51
		BOOST_CHECK(AccCount.GetAppCFund(tep, vuchFundtag, nTimeout));			 //没有合并必须金额还是没有变动
		BOOST_CHECK(tep.getValue() == vcOpArry[0].GetUint64Value());         	//没有合并必须金额还是没有变动
	}

	{
		vector<CAppFundOperate> OpArry2;
		CAppFundOperate subfreexeop(vuchAppuserId, vuchFundtag, EM_SUB_TAG_OP, nTimeout, 8);
		OpArry2.insert(OpArry2.end(), subfreexeop);
		BOOST_CHECK(AccCount.Operate(OpArry2));               //执行所有的操作符合
	}

	{
		CAppCFund subtemptep;
		BOOST_CHECK(AccCount.GetAppCFund(subtemptep, vuchFundtag, nTimeout));        //获取相应的冻结项
		BOOST_CHECK(subtemptep.getValue() == (vcOpArry[0].GetUint64Value() - 8));    //上面减去了8  检查是否对
	}

	{
		vector<CAppFundOperate> OpArry2;
		CAppFundOperate revertfreexeop(vuchAppuserId, vuchFundtag, EM_ADD_TAG_OP, nTimeout, 8);
		OpArry2.clear();
		OpArry2.insert(OpArry2.end(), revertfreexeop);
		BOOST_CHECK(AccCount.Operate(OpArry2));               //执行所有的操作符合
	}

	{
		CAppCFund reverttemptep;
		BOOST_CHECK(AccCount.GetAppCFund(reverttemptep, vuchFundtag, nTimeout));			 //没有合并必须金额还是没有变动
		BOOST_CHECK(reverttemptep.getValue() == vcOpArry[0].GetUint64Value());         	//没有合并必须金额还是没有变动
	}

	{         	//合并第一个
		CAppCFund tep;
		AccCount.AutoMergeFreezeToFree(0, nTimeout);                  				//自动合并 第0个
		BOOST_CHECK(AccCount.GetAppCFund(tep, vuchFundtag, nTimeout) == false); 		//必须找不到数据
		BOOST_CHECK(AccCount.getllValues() == vcOpArry[0].GetUint64Value());;          				//合并后自由金额必须没有问题
	}

	{          				//减去全部
		CAppFundOperate subfreeop(vuchAppuserId, vuchFundtag, EM_SUB_FREE_OP, nTimeout, vcOpArry[0].GetUint64Value());
		vector<CAppFundOperate> OpArry2;
		OpArry2.insert(OpArry2.end(), subfreeop);
		BOOST_CHECK(AccCount.Operate(OpArry2));               				//执行所有的操作符合
		BOOST_CHECK(AccCount.getllValues() == 0);;                           //钱必须可以核对
	}

	{
		vector<CAppFundOperate> OpArry2;
		CAppFundOperate addfreeop(vuchAppuserId, vuchFundtag, EM_ADD_FREE_OP, nTimeout, vcOpArry[0].GetUint64Value()); //再次把数据加进去
		OpArry2.clear();
		OpArry2.insert(OpArry2.end(), addfreeop);
		BOOST_CHECK(AccCount.Operate(OpArry2));               				//执行所有的操作符合
		BOOST_CHECK(AccCount.getllValues() == vcOpArry[0].GetUint64Value());                //加上后 就回来了
	}

	AccCount.AutoMergeFreezeToFree(0, nMaxtimeout);     				//全部合并
//	BOOST_CHECK_MESSAGE(AccCount.getllValues() == allmoney, "" << allmoney << ' ' << AccCount.getllValues());                //余额平账

}

BOOST_AUTO_TEST_CASE(checkappacct_test) {
	for (int j = 0; j < 8; ++j) {
		for (int i = 0; i < 8; ++i) {
			cout << g_arrllOpValue[j][i] << " ";
		}
		cout << endl;
		int64_t llTxValue = g_arrllOpValue[j][0];
		int64_t llAcctMinusValue = g_arrllOpValue[j][3];
		int64_t llAcctSum = llTxValue - llAcctMinusValue;
		int64_t llAppAcctSum = g_arrllOpValue[j][4] - g_arrllOpValue[j][5] + g_arrllOpValue[j][6] - g_arrllOpValue[j][7];
		bool bIsCheck = (llAcctSum == llAppAcctSum);
		cout << "ischeck:" << bIsCheck << endl;
		BOOST_CHECK(CheckAppAcct(g_arrllOpValue[j]) == bIsCheck);
	}

}
BOOST_AUTO_TEST_SUITE_END()



