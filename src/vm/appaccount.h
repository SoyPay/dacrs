/*
 * appuseraccout.h
 *
 *  Created on: 2015年3月30日
 *      Author: ranger.shi
 */

#ifndef APPUSERACCOUT_H_
#define APPUSERACCOUT_H_

#include "tx.h"

class CAppFundOperate;

class CAppCFund {
 public:
	static const int MAX_TAG_SIZE  = 40;
	CAppCFund();
	CAppCFund(const CAppFundOperate &vcOp);
	CAppCFund(const CAppCFund &fund);
	CAppCFund(const vector<unsigned char> &vchTag,uint64_t ullVal,int nHight);
	bool MergeCFund( const CAppCFund &cFund);
	Object toJSON()const;
	string toString()const;

	const vector<unsigned char> getTag() const {
		return m_vTag;
	}
	void setTag(const vector<unsigned char>& tag) {
		m_vTag = tag;
	}
	int getHeight() const {
		return m_nHeight;
	}

	void setHeight(int nHeight) {
		m_nHeight = nHeight;
	}

	uint64_t getValue() const {
		return m_nllValue;
	}

	void setValue(uint64_t ullValue) {
		this->m_nllValue = ullValue;
	}

 public:
	IMPLEMENT_SERIALIZE
	(
		READWRITE(VARINT(m_nllValue));
		READWRITE(VARINT(m_nHeight));
		READWRITE(m_vTag);
	)

private:
	uint64_t m_nllValue;					//!< amount of money
	int m_nHeight;					//!< time-out height
	vector<unsigned char> m_vTag;	//!< m_vTag of the tx which create the fund

};
enum emAPP_OP_TYPE{
	EM_ADD_FREE_OP = 1,
	EM_SUB_FREE_OP,
	EM_ADD_TAG_OP,
	EM_SUB_TAG_OP
}__attribute__((aligned(1)));


class CAppFundOperate {
public:
	CAppFundOperate();

	unsigned char m_uchOpeatorType;		//!OperType
	unsigned int m_nOutHeight;		    //!< the transacion Timeout height
	int64_t m_llMoney;			        //!<The transfer amount
	unsigned char m_uchAppuserIDlen;
	unsigned char m_arruchAppuser[CAppCFund::MAX_TAG_SIZE ];				//!< accountid
	unsigned char m_uchFundTaglen;
	unsigned char m_arruchFundTag[CAppCFund::MAX_TAG_SIZE ];				//!< accountid

	CAppFundOperate(const vector<unsigned char> &vuchAppTag, const vector<unsigned char> &vuchFundTag, emAPP_OP_TYPE opType,
			int nTimeout, int64_t llMoney) {
		assert(sizeof(m_arruchAppuser) >= vuchAppTag.size());
		assert(sizeof(m_arruchFundTag) >= vuchFundTag.size());
		m_uchAppuserIDlen = vuchAppTag.size();
		m_uchFundTaglen = vuchFundTag.size();
		memcpy(&m_arruchAppuser[0], &vuchAppTag[0], vuchAppTag.size());
		memcpy(&m_arruchFundTag[0], &vuchFundTag[0], vuchFundTag.size());
		m_llMoney = llMoney;
		m_nOutHeight = nTimeout;
		assert((opType >= EM_ADD_FREE_OP) && (opType <= EM_SUB_TAG_OP));
		m_uchOpeatorType = opType;
	}

	IMPLEMENT_SERIALIZE
	(
			READWRITE(m_uchOpeatorType);
			READWRITE(m_nOutHeight);
			READWRITE(m_llMoney);
			READWRITE(m_uchAppuserIDlen);
			for(unsigned int i = 0;i < sizeof(m_arruchAppuser);++i) {
				READWRITE(m_arruchAppuser[i]);
			}
			READWRITE(m_uchFundTaglen);
			for(unsigned int i = 0;i < sizeof(m_arruchFundTag);++i) {
				READWRITE(m_arruchFundTag[i]);
			}
	)
	Object toJSON() const;
	string toString() const;

	uint64_t GetUint64Value() const {
		return m_llMoney;
	}
	const vector<unsigned char> GetFundTagV() const;
	const vector<unsigned char> GetAppUserV() const {
		assert(sizeof(m_arruchAppuser) >= m_uchAppuserIDlen && m_uchAppuserIDlen > 0);
		vector<unsigned char> vuchTag(&m_arruchAppuser[0], &m_arruchAppuser[m_uchAppuserIDlen]);
		return (vuchTag);
	}

	unsigned char getopeatortype() const {
		return m_uchOpeatorType;
	}

	bool setOpeatortype(unsigned char opeatortype) {
		if ((opeatortype >= EM_ADD_FREE_OP) && (opeatortype <= EM_SUB_TAG_OP)) {
			this->m_uchOpeatorType = opeatortype;
			return true;
		} else {
			return false;
		}
	}

	unsigned int getoutheight() const {
		return m_nOutHeight;
	}

	void setOutheight(unsigned int outheight) {
		this->m_nOutHeight = outheight;
	}
};


class CAppUserAccout {
 public:
	CAppUserAccout();
	CAppUserAccout(const vector<unsigned char> &vuchUserId);
	bool Operate(const vector<CAppFundOperate> &vcOp);
	bool GetAppCFund(CAppCFund &cOutFound,const vector<unsigned char> &vuchTag,int nHight);

	bool AutoMergeFreezeToFree(uint32_t unAppHeight, int nHeight);

	virtual ~CAppUserAccout();
	Object toJSON()const;
	string toString()const;
	uint64_t getllValues() const {
		return m_ullValues;
	}

	void setLlValues(uint64_t llValues) {
		this->m_ullValues = llValues;
	}

	const vector<unsigned char>& getaccUserId() const {
		return m_vuchAccUserID;
	}

	void setAccUserId(const vector<unsigned char>& accUserId) {
		m_vuchAccUserID = accUserId;
	}

	vector<CAppCFund>& getFreezedFund() {
		return m_vcFreezedFund;
	}

	void setFreezedFund(const vector<CAppCFund>& vtmp)
	{
		m_vcFreezedFund.clear();
		for(int i = 0; i < (int)vtmp.size(); i++)
		{
			m_vcFreezedFund.push_back(vtmp[i]);
		}
	}

	uint64_t GetAllFreezedValues();
	IMPLEMENT_SERIALIZE
	(
		READWRITE(VARINT(m_ullValues));
		READWRITE(m_vuchAccUserID);
		READWRITE(m_vcFreezedFund);
	)

	bool MinusAppCFund(const vector<unsigned char> &vuchTag,uint64_t ullVal,int nHight);
	bool AddAppCFund(const vector<unsigned char>& vuchTag, uint64_t ullVal, int nHight);
	bool MinusAppCFund(const CAppCFund &cInFound);
	bool AddAppCFund(const CAppCFund &cInFound);
	bool ChangeAppCFund(const CAppCFund &cInFound);
	bool Operate(const CAppFundOperate &Op);
private:
	uint64_t m_ullValues;       //自由金额
	vector<unsigned char>  m_vuchAccUserID;
	vector<CAppCFund> m_vcFreezedFund;
};

class CAssetOperate
{
 public:
	CAssetOperate() {
		m_uchFundTaglen = 0;
		m_unOutHeight = 0;
		m_ullMoney = 0;
	}

	uint64_t GetUint64Value() const {
		return m_ullMoney;
	}

	int getheight() const {
		return m_unOutHeight;
	}

	const vector<unsigned char> GetFundTagV() const {
		assert(sizeof(m_vuchFundTag) >= m_uchFundTaglen);
		vector<unsigned char> vuchTag(&m_vuchFundTag[0], &m_vuchFundTag[m_uchFundTaglen]);
		return (vuchTag);
	}

	IMPLEMENT_SERIALIZE
	(
			READWRITE(m_unOutHeight);
			READWRITE(m_ullMoney);
			READWRITE(m_uchFundTaglen);
			for(unsigned int i = 0;i < sizeof(m_vuchFundTag);++i) {
				READWRITE(m_vuchFundTag[i]);
			}
	)
public:
	unsigned int m_unOutHeight;		    //!< the transacion Timeout height
	uint64_t m_ullMoney;			        //!<The transfer amount
	unsigned char m_uchFundTaglen;
	unsigned char m_vuchFundTag[CAppCFund::MAX_TAG_SIZE ];				//!< accountid
};

inline const vector<unsigned char> CAppFundOperate::GetFundTagV() const {
	assert(sizeof(m_arruchFundTag) >= m_uchFundTaglen);
	vector<unsigned char> tag(&m_arruchFundTag[0], &m_arruchFundTag[m_uchFundTaglen]);
	return (tag);
}

#endif /* APPUSERACCOUT_H_ */
