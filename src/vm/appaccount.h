/*
 * appuseraccout.h
 *
 *  Created on: 2015��3��30��
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
	CAppCFund(const CAppFundOperate &Op);
	CAppCFund(const CAppCFund &fund);
	CAppCFund(const vector<unsigned char> &vtag,uint64_t val,int nhight);
	bool MergeCFund( const CAppCFund &fund);
	Object toJSON()const;
	string toString()const;

	const vector<unsigned char> GetTag() const {
		return vTag;
	}

	int getheight() const {
		return nHeight;
	}

	void setHeight(int height) {
		nHeight = height;
	}

	uint64_t getvalue() const {
		return value;
	}

	void setValue(uint64_t value) {
		this->value = value;
	}

//	const vector<unsigned char>& gettag() const {
//		return vTag;
//	}

	void setTag(const vector<unsigned char>& tag) {
		vTag = tag;
	}

public:
	IMPLEMENT_SERIALIZE
	(
		READWRITE(VARINT(value));
		READWRITE(VARINT(nHeight));
		READWRITE(vTag);
	)

private:
	uint64_t value;					//!< amount of money
	int nHeight;					//!< time-out height
	vector<unsigned char> vTag;	//!< vTag of the tx which create the fund

};
enum APP_OP_TYPE{
	ADD_FREE_OP = 1,
	SUB_FREE_OP,
	ADD_TAG_OP,
	SUB_TAG_OP
}__attribute__((aligned(1)));


class CAppFundOperate {
public:
	CAppFundOperate();

	unsigned char opeatortype;		//!OperType
	unsigned int outheight;		    //!< the transacion Timeout height
	int64_t mMoney;			        //!<The transfer amount
	unsigned char appuserIDlen;
	unsigned char vAppuser[CAppCFund::MAX_TAG_SIZE ];				//!< accountid
	unsigned char FundTaglen;
	unsigned char vFundTag[CAppCFund::MAX_TAG_SIZE ];				//!< accountid

	CAppFundOperate(const vector<unsigned char> &AppTag,const vector<unsigned char> &FundTag,APP_OP_TYPE opType,int timeout,int64_t money)
	{
		assert(sizeof(vAppuser) >= AppTag.size());
		assert(sizeof(vFundTag) >= FundTag.size());
		appuserIDlen = AppTag.size();
		FundTaglen = FundTag.size();
		memcpy(&vAppuser[0],&AppTag[0],AppTag.size());
		memcpy(&vFundTag[0],&FundTag[0],FundTag.size());
		mMoney = money;
		outheight = timeout;
		assert((opType >= ADD_FREE_OP) && (opType <= SUB_TAG_OP));
		opeatortype = opType;
	}

	IMPLEMENT_SERIALIZE
	(
		READWRITE(opeatortype);
		READWRITE(outheight);
		READWRITE(mMoney);
		READWRITE(appuserIDlen);
		for(unsigned int i = 0;i < sizeof(vAppuser);++i)
		READWRITE(vAppuser[i]);
		READWRITE(FundTaglen);
		for(unsigned int i = 0;i < sizeof(vFundTag);++i)
		READWRITE(vFundTag[i]);
	)
	Object toJSON()const;
	string toString()const;

	uint64_t GetUint64Value() const {
		return mMoney;
	}
	const vector<unsigned char> GetFundTagV() const {
		assert(sizeof(vFundTag) >= FundTaglen );
		vector<unsigned char> tag(&vFundTag[0], &vFundTag[FundTaglen]);
		return (tag);
	}
	const vector<unsigned char> GetAppUserV() const {
	//	cout<<appuserIDlen<<endl;
		assert(sizeof(vAppuser) >= appuserIDlen && appuserIDlen > 0);
		vector<unsigned char> tag(&vAppuser[0], &vAppuser[appuserIDlen]);
		return (tag);
	}

	unsigned char getopeatortype() const {
		return opeatortype;
	}

	bool setOpeatortype(unsigned char opeatortype) {
		if((opeatortype >= ADD_FREE_OP) && (opeatortype <= SUB_TAG_OP))
		{
			this->opeatortype = opeatortype;
			return true;
		}
		else
		{
			return false;
		}
	}

	unsigned int getoutheight() const {
		return outheight;
	}

	void setOutheight(unsigned int outheight) {
		this->outheight = outheight;
	}
};


class CAppUserAccout {
public:
	CAppUserAccout();
	CAppUserAccout(const vector<unsigned char> &userId);
	bool Operate(const vector<CAppFundOperate> &Op);
	bool GetAppCFund(CAppCFund &outFound,const vector<unsigned char> &vtag,int nhight);

	bool AutoMergeFreezeToFree(int hight);

	virtual ~CAppUserAccout();
	Object toJSON()const;
	string toString()const;
	uint64_t getllValues() const {
		return llValues;
	}

	void setLlValues(uint64_t llValues) {
		this->llValues = llValues;
	}

	const vector<unsigned char>& getaccUserId() const {
		return mAccUserID;
	}

	void setAccUserId(const vector<unsigned char>& accUserId) {
		mAccUserID = accUserId;
	}

	IMPLEMENT_SERIALIZE
	(
		READWRITE(VARINT(llValues));
		READWRITE(mAccUserID);
		READWRITE(vFreezedFund);
	)

private:
	bool MinusAppCFund(const vector<unsigned char> &vtag,uint64_t val,int nhight);
	bool AddAppCFund(const vector<unsigned char>& vtag, uint64_t val, int nhight);
	bool MinusAppCFund(const CAppCFund &inFound);
	bool AddAppCFund(const CAppCFund &inFound);
	bool ChangeAppCFund(const CAppCFund &inFound);
	bool Operate(const CAppFundOperate &Op);
	uint64_t llValues;       //���ɽ��
	vector<unsigned char>  mAccUserID;
	vector<CAppCFund> vFreezedFund;
};



#endif /* APPUSERACCOUT_H_ */
