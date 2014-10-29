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
	/**
	 * Run the script object
	 */
	shared_ptr<CVir8051> pMcu;
	/**
	 * vm before the account state
	 */
	vector<shared_ptr<CAccountInfo> > RawAccont;
	/**
	 * vm operate the account  state
	 */
	vector<shared_ptr<CAccountInfo> > NewAccont;
	/**
	 * current run the tx
	 */
	shared_ptr<CBaseTransaction>  listTx;
	CAccountViewCache pView;
	/**
	 * run the script
	 */
	CVmScript vmScript;

private:
	/**
	 * @brief The initialization function
	 * @param Tx: run the tx's contact
	 * @param view:Cache holds account
	 * @return : check the the tx and account is Legal true is legal false is unlegal
	 */
	bool intial(shared_ptr<CBaseTransaction> & Tx,CAccountViewCache& view);
	/**
	 *
	 * @param listoperate:run the script return the code,check the code
	 * @return :true check success
	 */
	bool CheckOperate(const vector<CVmOperate> &listoperate) const;
public:
	/**
	 * A constructor.
	 */
	CVmScriptRun(){};
	/**
	 *
	 * @return :the variable RawAccont
	 */
	vector<shared_ptr<CAccountInfo> > &GetRawAccont();
	/**
	 *
	 * @return :the variable NewAccont
	 */
	vector<shared_ptr<CAccountInfo> > &GetNewAccont();
	/**
	 *
	 * @param listoperate:through the vm return code ,The accounts plus money and less money
	 * @return
	 */
	bool OpeatorSecureAccount(const vector<CVmOperate>& listoperate);
	/**
	 * @brief find the vOldAccount from NewAccont if find success remove it from NewAccont
	 * @param vOldAccount:the argument
	 * @return:Return the object
	 */
	shared_ptr<CAccountInfo> GetNewAccount(shared_ptr<CAccountInfo>& vOldAccount);
	/**
	 * @brief find the Account from NewAccont
	 * @param Account argument
	 * @return:Return the object
	 */
	shared_ptr<CAccountInfo> GetAccount(shared_ptr<CAccountInfo>& Account);
	/**
	 * @brief get the account id
	 * @param value: argument
	 * @return:Return account id
	 */
	vector_unsigned_char GetAccountID(CVmOperate value);
	/**
	 * @brief Is beginning to run the script
	 * @param Tx: run the tx
	 * @param view:the second argument
	 * @return:true run success
	 */
	bool run(shared_ptr<CBaseTransaction>& Tx,CAccountViewCache& view);
	/**
	 * @brief just for test
	 * @return:
	 */
	shared_ptr<vector<CVmOperate>> GetOperate() const;
	virtual ~CVmScriptRun();
};

//#pragma pack(1)
enum ACCOUNT_TYPE {
	ACCOUNTID = 0,			//
	KEYID = 1,
};
class CVmOperate{
	ACCOUNT_TYPE TYPE;
	unsigned char accountid[20];
	unsigned char opeatortype;
	unsigned int  outheight;
	unsigned char money[8];
	IMPLEMENT_SERIALIZE
	(
			READWRITE(TYPE);
			for(int i = 0;i < 20;i++)
			READWRITE(accountid[i]);
			READWRITE(opeatortype);
			READWRITE(outheight);
			for(int i = 0;i < 8;i++)
			READWRITE(money[i]);
	)
};
//#pragma pack()
#endif /* SCRIPTCHECK_H_ */
