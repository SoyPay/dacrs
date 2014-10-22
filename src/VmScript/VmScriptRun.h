/*
 * ScriptCheck.h
 *
 *  Created on: Sep 2, 2014
 *      Author: ranger.shi
 */

#ifndef SCRIPTCHECK_H_
#define SCRIPTCHECK_H_
#include "CVir8051.h"
#include "serialize.h"
#include "VmScript.h"
#include "main.h"
#include "txdb.h"
#include <memory>
using namespace std;
class CVmOperate;
class CVmScriptRun {
	bool IsRun;
	shared_ptr<CVir8051> pMcu;
	vector<shared_ptr<CSecureAccount> > vArbitratorAcc;
	vector<shared_ptr<CSecureAccount> > vAccount;
	vector<shared_ptr<CSecureAccount> > RawAccont;
	vector<shared_ptr<CSecureAccount> > NewAccont;
	vector<shared_ptr<CBaseTransaction> > listTx;
	CVmScript vmScript;

private:
	bool intial(vector<shared_ptr<CBaseTransaction> >& Tx,CAccountViewCache& view);
	bool IsFirstTx() const {
		return listTx.size() == 1;
	}
	int maxTxIndex() const {
		return listTx.size();
	}
	int maxAccountIndex() const {
		return RawAccont.size();
	}
	bool CheckOperate(const vector<CVmOperate> &listoperate) const;
public:
	CVmScriptRun(){};

	vector<shared_ptr<CSecureAccount> > &GetRawAccont();
	vector<shared_ptr<CSecureAccount> > &GetNewAccont();
	bool OpeatorSecureAccount(const vector<CVmOperate>& listoperate);
	shared_ptr<CSecureAccount> GetNewAccount(shared_ptr<CSecureAccount>& vOldAccount);

	bool run(vector<shared_ptr<CBaseTransaction> >& Tx,CAccountViewCache& view);
	//just for debug
	CVmScriptRun(CAccountViewCache& view, vector<shared_ptr<CBaseTransaction> >& Tx, CVmScript& script);
	shared_ptr<vector<CVmOperate>> GetOperate() const;
	virtual ~CVmScriptRun();
};

//#pragma pack(1)
class CVmHeadData {
public:
	unsigned char ArbitratorAccCount;   //Arbitrator number
	unsigned char vAccountCount;		//account number
	unsigned char AppealTxCount;		//appeal tx number
	unsigned char vCurrentH[4];         // current block height
	unsigned char vSecureH[4];			//CSecureTransaction of the height in the block
public:
	IMPLEMENT_SERIALIZE
	(
			READWRITE(ArbitratorAccCount);
			READWRITE(vAccountCount);
			READWRITE(AppealTxCount);
			for(int i = 0;i < 4;i++)
			READWRITE(vCurrentH[i]);
			for(int i = 0;i < 4;i++)
			READWRITE(vSecureH[i]);
	)
};

class CVmSecureTxData {
public:
	vector<unsigned char> Contract;
	vector<unsigned char> sigaccountid;
public:
	IMPLEMENT_SERIALIZE
	(
			READWRITE(Contract);
			READWRITE(sigaccountid);
	)

};

class CVmAppealTxPackes {
public:
	vector<unsigned char> AppealTxContract;
	vector<unsigned char> sigaccountid;
public:
	IMPLEMENT_SERIALIZE
	(
			READWRITE(AppealTxContract);
			READWRITE(sigaccountid);
	)
};
class Cpackes {
public:
	char version[2];
	CVmHeadData vhead;
	CVmSecureTxData vsecuretx;
	vector<CVmAppealTxPackes> vAppealTxPacke;
public:
	IMPLEMENT_SERIALIZE
	(
			READWRITE(version[0]);
			READWRITE(version[1]);
			READWRITE(vhead);
			READWRITE(vsecuretx);
			READWRITE(vAppealTxPacke);
	)
};
struct OperateData {
	unsigned char txid;
	unsigned char Opeater;
	unsigned char ResultCheck;
	unsigned char accountid;
	unsigned short outheight;
	unsigned char money[20];
	IMPLEMENT_SERIALIZE
	(
			READWRITE(txid);
			READWRITE(Opeater);
			READWRITE(ResultCheck);
			READWRITE(accountid);
			READWRITE(outheight);
			for(int i = 0;i < 20;i++)
			READWRITE(money[i]);
	)

};


class CVmOperate {

public:
	OperateData muls;
	OperateData add;
public:
	CVmOperate() {
	}

	IMPLEMENT_SERIALIZE
	(
			READWRITE(muls);
			READWRITE(add);
	)

	virtual ~CVmOperate() {
	}

};
class CVmOperatePacke{
public:
	char version[2];
	vector<CVmOperate> vmpackets;
public:
	CVmOperatePacke() {
	}
	IMPLEMENT_SERIALIZE
	(
			READWRITE(version[0]);
			READWRITE(version[1]);
			READWRITE(vmpackets);
	)
	virtual ~CVmOperatePacke() {
	}
};
//#pragma pack()
#endif /* SCRIPTCHECK_H_ */
