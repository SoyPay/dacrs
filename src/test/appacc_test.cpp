/*
 * appacc_test.cpp
 *
 *  Created on: 2015年3月29日
 *      Author: ranger.shi
 */


#include <boost/foreach.hpp>
#include <boost/test/unit_test.hpp>
#include "txdb.h"
#include "account.h"
#include <iostream>
#include  "boost/filesystem/operations.hpp"
#include  "boost/filesystem/path.hpp"
#include  "../vm/appuseraccout.h"

using namespace std;




// appacc_tests/key_test1

BOOST_AUTO_TEST_SUITE(appacc_tests)

BOOST_AUTO_TEST_CASE(key_test1)
 {
	auto StrTVector = [&](string tag)
	{
		return vector<unsigned char>(tag.begin(),tag.end());
	};

	srand((int) time(NULL));

	vector<unsigned char> AppuserId = StrTVector("test1");
	vector<unsigned char> fundtag = StrTVector("foundtag");
	vector<unsigned char> fundtag2 = StrTVector("foundtag2");

	CAppFundOperate opTe(AppuserId,fundtag, ADD_TAG_OP, 500, 800000);
	BOOST_CHECK(opTe.GetFundTagV() == fundtag);
	BOOST_CHECK(opTe.GetUint64Value()== 800000);
	BOOST_CHECK(opTe.getopeatortype()== ADD_TAG_OP);


	vector<CAppFundOperate> OpArry;
	uint64_t allmony = 0;
	int timeout = (rand() % 15000) + 51;
	int loop = 500;
	int maxtimeout = timeout + loop+1;
	for (int i = 0; i < loop; i++) {
		int64_t temp = ((rand() * rand()) % 15000000) + 20;
		allmony += temp;
		CAppFundOperate op(AppuserId,fundtag, ADD_TAG_OP, timeout + i, temp);
		OpArry.insert(OpArry.end(), op);
	}

	CAppUserAccout AccCount(AppuserId);
	BOOST_CHECK(AccCount.getaccUserId() == AppuserId);      //初始化的ID 必须是
	BOOST_CHECK(AccCount.Operate(OpArry));               //执行所有的操作符合
	BOOST_CHECK(AccCount.getllValues() == 0);            //因为操作符全是加冻结的钱所以自由金额必须是0


	{
		CAppCFund tep;
		BOOST_CHECK(AccCount.GetAppCFund(tep, fundtag, timeout)); //获取相应的冻结项
		BOOST_CHECK(tep.getvalue() == OpArry[0].GetUint64Value());                    //冻结的金额需要没有问题
		CAppCFund tep2;
		BOOST_CHECK(AccCount.GetAppCFund(tep2, fundtag, maxtimeout + 5) == false); //获取相应的冻结项 超时时间不同 必须获取不到

		AccCount.AutoMergeFreezeToFree(timeout - 1);               	   //自动合并 超时高度没有到  这里的 50 是为了配合签名 time out de 51
		BOOST_CHECK(AccCount.GetAppCFund(tep, fundtag, timeout));			 //没有合并必须金额还是没有变动
		BOOST_CHECK(tep.getvalue() == OpArry[0].GetUint64Value());         	//没有合并必须金额还是没有变动
	}

	{
		vector<CAppFundOperate> OpArry2;
		CAppFundOperate subfreexeop(AppuserId,fundtag, SUB_TAG_OP, timeout, 8);
		OpArry2.insert(OpArry2.end(), subfreexeop);
		BOOST_CHECK(AccCount.Operate(OpArry2));               //执行所有的操作符合
	}

	{
		CAppCFund subtemptep;
		BOOST_CHECK(AccCount.GetAppCFund(subtemptep, fundtag, timeout));        //获取相应的冻结项
		BOOST_CHECK(subtemptep.getvalue() == (OpArry[0].GetUint64Value() - 8));    //上面减去了8  检查是否对
	}

	{
		vector<CAppFundOperate> OpArry2;
		CAppFundOperate revertfreexeop(AppuserId,fundtag, ADD_TAG_OP, timeout, 8);
		OpArry2.clear();
		OpArry2.insert(OpArry2.end(), revertfreexeop);
		BOOST_CHECK(AccCount.Operate(OpArry2));               //执行所有的操作符合
	}

	{
		CAppCFund reverttemptep;
		BOOST_CHECK(AccCount.GetAppCFund(reverttemptep, fundtag, timeout));			 //没有合并必须金额还是没有变动
		BOOST_CHECK(reverttemptep.getvalue() == OpArry[0].GetUint64Value());         	//没有合并必须金额还是没有变动
	}

	{         	//合并第一个
		CAppCFund tep;
		AccCount.AutoMergeFreezeToFree(timeout);                  				//自动合并 第0个
		BOOST_CHECK(AccCount.GetAppCFund(tep, fundtag, timeout) == false); 		//必须找不到数据
		BOOST_CHECK(AccCount.getllValues() == OpArry[0].GetUint64Value());;          				//合并后自由金额必须没有问题
	}

	{          				//减去全部
		CAppFundOperate subfreeop(AppuserId,fundtag, SUB_FREE_OP, timeout, OpArry[0].GetUint64Value());
		vector<CAppFundOperate> OpArry2;
		OpArry2.insert(OpArry2.end(), subfreeop);
		BOOST_CHECK(AccCount.Operate(OpArry2));               				//执行所有的操作符合
		BOOST_CHECK(AccCount.getllValues() == 0);;                           //钱必须可以核对
	}

	{
		vector<CAppFundOperate> OpArry2;
		CAppFundOperate addfreeop(AppuserId,fundtag, ADD_FREE_OP, timeout, OpArry[0].GetUint64Value());    //再次把数据加进去
		OpArry2.clear();
		OpArry2.insert(OpArry2.end(), addfreeop);
		BOOST_CHECK(AccCount.Operate(OpArry2));               				//执行所有的操作符合
		BOOST_CHECK(AccCount.getllValues() == OpArry[0].GetUint64Value());                //加上后 就回来了
	}

	AccCount.AutoMergeFreezeToFree(maxtimeout);     				//全部合并
	BOOST_CHECK(AccCount.getllValues() == allmony);                //余额平账

}

BOOST_AUTO_TEST_SUITE_END()
