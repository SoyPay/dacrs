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
	vector<shared_ptr<CAccount> > RawAccont;
	/**
	 * vm operate the account  state
	 */
	vector<shared_ptr<CAccount> > NewAccont;
	/**
	 * current run the tx
	 */
	shared_ptr<CBaseTransaction>  listTx;
	/**
	 * run the script
	 */
	CVmScript vmScript;
	/**
	 * the block height
	 */
	int height;

private:
	/**
	 * @brief The initialization function
	 * @param Tx: run the tx's contact
	 * @param view:Cache holds account
	 * @return : check the the tx and account is Legal true is legal false is unlegal
	 */
	bool intial(shared_ptr<CBaseTransaction> & Tx,CAccountViewCache& view,int nheight);
	/**
	 *
	 * @param listoperate:run the script return the code,check the code
	 * @return :true check success
	 */
	bool CheckOperate(const vector<CVmOperate> &listoperate) const;
	/**
	 *
	 * @param listoperate:through the vm return code ,The accounts plus money and less money
	 * @return
	 */
	bool OpeatorSecureAccount(const vector<CVmOperate>& listoperate,CAccountViewCache& view);
	/**
	 * @brief find the vOldAccount from NewAccont if find success remove it from NewAccont
	 * @param vOldAccount:the argument
	 * @return:Return the object
	 */
	shared_ptr<CAccount> GetNewAccount(shared_ptr<CAccount>& vOldAccount);
	/**
	 * @brief find the Account from NewAccont
	 * @param Account argument
	 * @return:Return the object
	 */
	shared_ptr<CAccount> GetAccount(shared_ptr<CAccount>& Account);
	/**
	 * @brief get the account id
	 * @param value: argument
	 * @return:Return account id
	 */
	vector_unsigned_char GetAccountID(CVmOperate value);

public:
	/**
	 * A constructor.
	 */
	CVmScriptRun();
	/**
	 *
	 * @return :the variable RawAccont
	 */
	vector<shared_ptr<CAccount> > &GetRawAccont();
	/**
	 *
	 * @return :the variable NewAccont
	 */
	vector<shared_ptr<CAccount> > &GetNewAccont();
	/**
	 * @brief Is beginning to run the script
	 * @param Tx: run the tx
	 * @param view:the second argument
	 * @param nheight: block height
	 * @return:true run success
	 */
	bool run(shared_ptr<CBaseTransaction>& Tx,CAccountViewCache& view,int nheight);
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
public:
	unsigned char type;
	unsigned char accountid[20];
	unsigned char opeatortype;
	unsigned int  outheight;
	unsigned char money[8];
	IMPLEMENT_SERIALIZE
	(
			READWRITE(type);
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
