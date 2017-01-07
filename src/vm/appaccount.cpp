/*
 * appuseraccout.cpp
 *
 *  Created on: 2015年3月30日
 *      Author: ranger.shi
 */

#include "serialize.h"
#include <boost/foreach.hpp>
#include "hash.h"
#include "util.h"
#include "database.h"
#include "main.h"
#include <algorithm>
#include "txdb.h"
#include "vm/vmrunevn.h"
#include "core.h"
#include "miner.h"
#include "json/json_spirit_utils.h"
#include "json/json_spirit_value.h"
#include "json/json_spirit_writer_template.h"
#include "appaccount.h"
#include "SafeInt3.hpp"
using namespace json_spirit;

CAppCFund::CAppCFund() {
	m_vTag.clear();
	m_nllValue = 0;
	m_nHeight = 0;
}

CAppCFund::CAppCFund(const CAppCFund &cAppCFund) {
	m_vTag = cAppCFund.getTag();
	m_nllValue = cAppCFund.getValue();
	m_nHeight = cAppCFund.getHeight();
}

CAppCFund::CAppCFund(const vector<unsigned char>& vuchTag, uint64_t ullVal, int nHight) {
	m_vTag = vuchTag;
	m_nllValue = ullVal;
	m_nHeight = nHight;
}

inline bool CAppCFund::MergeCFund(const CAppCFund &cFund) {
	assert(cFund.getTag() == this->getTag());
	assert(cFund.getHeight() == this->getHeight() && cFund.getValue() > 0);
	//value = fund.getValue()+value;
	uint64_t ullTempValue = 0;
	if (!SafeAdd(cFund.getValue(), m_nllValue, ullTempValue)) {
		return ERRORMSG("Operate overflow %lu, %lu!", cFund.getValue(), m_nllValue);
	}
	m_nllValue = ullTempValue;
	return true;
}

CAppCFund::CAppCFund(const CAppFundOperate& cOp) {
	assert(cOp.m_unOutHeight > 0);
	m_vTag = cOp.GetFundTagV();
	m_nllValue = cOp.GetUint64Value();					//!< amount of money
	m_nHeight = cOp.m_unOutHeight;
}

CAppUserAccout::CAppUserAccout() {
	m_vuchAccUserID.clear();
	m_ullValues = 0;
	m_vcFreezedFund.clear();
}

CAppUserAccout::CAppUserAccout(const vector<unsigned char> &vuchUserId) {
	m_vuchAccUserID.clear();
	m_vuchAccUserID = vuchUserId;
	m_ullValues = 0;
	m_vcFreezedFund.clear();
}

bool CAppUserAccout::GetAppCFund(CAppCFund& cOutFound, const vector<unsigned char>& vuchTag , int nHight) {
	auto it = find_if(m_vcFreezedFund.begin(), m_vcFreezedFund.end(), [&](const CAppCFund& CfundIn) {
		return nHight ==CfundIn.getHeight() && CfundIn.getTag()== vuchTag;});
	if (it != m_vcFreezedFund.end()) {
		cOutFound = *it;
		return true;
	}
	return false;
}

bool CAppUserAccout::AddAppCFund(const CAppCFund& cInFound) {
	//需要找到超时高度和tag 都相同的才可以合并
	auto it = find_if(m_vcFreezedFund.begin(), m_vcFreezedFund.end(), [&](const CAppCFund& CfundIn) {
		return CfundIn.getTag()== cInFound.getTag() && CfundIn.getHeight() ==cInFound.getHeight();});
	if (it != m_vcFreezedFund.end()) { //如果找到了
		it->MergeCFund(cInFound);
		return true;
	}
	//没有找到就加一个新的
	m_vcFreezedFund.insert(m_vcFreezedFund.end(), cInFound);
	return true;

}

uint64_t CAppUserAccout::GetAllFreezedValues() {
	uint64_t ullTotal = 0;
	for (auto &Fund : m_vcFreezedFund) {
		ullTotal += Fund.getValue();
	}

	return ullTotal;
}

bool CAppUserAccout::AutoMergeFreezeToFree(uint32_t unAppHeight, int nHeight) {
	int nHeightOrTime = nHeight;
	if (unAppHeight >= g_sBlockTime4AppAccountHeight) {
		if (nHeight >= g_sBlockTime4AppAccountHeight) {
			if (nHeight <= g_cChainActive.Tip()->m_nHeight) {
				nHeightOrTime = g_cChainActive[nHeight]->m_unTime;
			} else {
				nHeightOrTime = g_cChainActive.Tip()->m_unTime;
			}
		}
	}
	bool bIsNeedRemvoe = false;
	for (auto &Fund : m_vcFreezedFund) {
		if (Fund.getHeight() <= nHeightOrTime) {
			//m_ullValues += Fund.getvalue();
			uint64_t ullTempValue = 0;
			if (!SafeAdd(m_ullValues, Fund.getValue(), ullTempValue)) {
				return ERRORMSG("Operate overflow !");
			}
			m_ullValues = ullTempValue;
			bIsNeedRemvoe = true;
		}
	}
	if (bIsNeedRemvoe) {
		m_vcFreezedFund.erase(remove_if(m_vcFreezedFund.begin(), m_vcFreezedFund.end(), [&](const CAppCFund& CfundIn) {
			return (CfundIn.getHeight() <= nHeightOrTime);}), m_vcFreezedFund.end());
	}
	return true;

}

bool CAppUserAccout::ChangeAppCFund(const CAppCFund& cInFound) {
	//需要找到超时高度和tag 都相同的才可以合并
	assert(cInFound.getHeight() > 0);
	auto it = find_if(m_vcFreezedFund.begin(), m_vcFreezedFund.end(), [&](const CAppCFund& CfundIn) {
		return CfundIn.getTag()== cInFound.getTag() && CfundIn.getHeight() ==cInFound.getHeight();});
	if (it != m_vcFreezedFund.end()) { //如果找到了
		*it = cInFound;
		return true;
	}
	return false;
}

bool CAppUserAccout::MinusAppCFund(const CAppCFund& cInFound) {
	assert(cInFound.getHeight() > 0);
	auto it = find_if(m_vcFreezedFund.begin(), m_vcFreezedFund.end(), [&](const CAppCFund& CfundIn) {
		return CfundIn.getTag()== cInFound.getTag() && CfundIn.getHeight() ==cInFound.getHeight();});
	if (it != m_vcFreezedFund.end()) { //如果找到了
		if (it->getValue() >= cInFound.getValue()) {
			if (it->getValue() == cInFound.getValue()) {
				m_vcFreezedFund.erase(it);
				return true;
			}
			it->setValue(it->getValue() - cInFound.getValue());
			return true;
		}
	}
	return false;
}

bool CAppUserAccout::MinusAppCFund(const vector<unsigned char> &vuchTag, uint64_t ullVal, int nHight) {
	CAppCFund fund(vuchTag, ullVal, nHight);
	return MinusAppCFund(fund);
}

bool CAppUserAccout::AddAppCFund(const vector<unsigned char>& vuchTag, uint64_t ullVal, int nHight) {
	CAppCFund fund(vuchTag, ullVal, nHight);
	return AddAppCFund(fund);
}

CAppUserAccout::~CAppUserAccout() {
}

bool CAppUserAccout::Operate(const vector<CAppFundOperate> &vcOp) {
	assert(vcOp.size() > 0);
	//LogPrint("acc","before:%s",toString());
	for (auto const op : vcOp) {
		if (!Operate(op)) {
			return false;
		}
	}
	//LogPrint("acc","after:%s",toString());
	return true;
}

bool CAppUserAccout::Operate(const CAppFundOperate& cOp) {
	//LogPrint("acc","vcOp:%s",vcOp.toString());
	if (cOp.m_uchOpeatorType == EM_ADD_FREE_OP) {
		//m_ullValues += vcOp.GetUint64Value();
		uint64_t ullTempValue = 0;
		if (!SafeAdd(m_ullValues, cOp.GetUint64Value(), ullTempValue)) {
			return ERRORMSG("Operate overflow !");
		}
		m_ullValues = ullTempValue;
		return true;
	} else if (cOp.m_uchOpeatorType == EM_SUB_FREE_OP) {
		uint64_t ullTem = cOp.GetUint64Value();
		if (m_ullValues >= ullTem) {
			m_ullValues -= ullTem;
			return true;
		}
	} else if (cOp.m_uchOpeatorType == EM_ADD_TAG_OP) {
		CAppCFund tep(cOp);
		return AddAppCFund(tep);
	} else if (cOp.m_uchOpeatorType == EM_SUB_TAG_OP) {
		CAppCFund tep(cOp);
		return MinusAppCFund(tep);
	} else {
		return ERRORMSG("CAppUserAccout operate type error!");
		//		assert(0);
	}
	return false;
}

CAppFundOperate::CAppFundOperate() {
	m_uchFundTaglen = 0;
	m_uchAppuserIDlen = 0;
	m_uchOpeatorType = 0;
	m_unOutHeight = 0;
	m_llMoney = 0;
}

Object CAppCFund::toJSON() const {
	Object result;
	result.push_back(Pair("value", m_nllValue));
	result.push_back(Pair("Height", m_nHeight));
	result.push_back(Pair("vTag", HexStr(m_vTag)));
	return std::move(result);
}

string CAppCFund::toString() const {
	return write_string(Value(toJSON()), true);
}

Object CAppUserAccout::toJSON() const {
	Object result;
	result.push_back(Pair("m_vuchAccUserID", HexStr(m_vuchAccUserID)));
	result.push_back(Pair("FreeValues", m_ullValues));
	Array arry;
	for (auto const te : m_vcFreezedFund) {
		arry.push_back(te.toJSON());
	}
	result.push_back(Pair("m_vcFreezedFund", arry));
	return std::move(result);
}

string CAppUserAccout::toString() const {
	return write_string(Value(toJSON()), true);
}

Object CAppFundOperate::toJSON() const {
	Object result;
	int nTimout = m_unOutHeight;
	string strTep[] = { "error type", "EM_ADD_FREE_OP ", "EM_SUB_FREE_OP", "EM_ADD_TAG_OP", "EM_SUB_TAG_OP" };
	result.push_back(Pair("userid", HexStr(GetAppUserV())));
	result.push_back(Pair("vTag", HexStr(GetFundTagV())));
	result.push_back(Pair("m_uchOpeatorType", strTep[m_uchOpeatorType]));
	result.push_back(Pair("m_nOutHeight", nTimout));
	//	result.push_back(Pair("m_nOutHeight", m_nOutHeight));
	result.push_back(Pair("m_llMoney", m_llMoney));
	return std::move(result);
}

string CAppFundOperate::toString() const {
	return write_string(Value(toJSON()), true);
}


