/*
 * ScriptCheck.h
 *
 *  Created on: Sep 2, 2014
 *      Author: ranger.shi
 */

#ifndef SCRIPTCHECK_H_
#define SCRIPTCHECK_H_
#include "vm8051.h"
#include "serialize.h"
#include "script.h"
#include "main.h"
#include "txdb.h"
#include <memory>
using namespace std;
class CVmOperate;
class CVmRunEvn {
	/**
	 * Run the script object
	 */
	shared_ptr<CVm8051> pMcu;
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
	shared_ptr<CBaseTransaction> listTx;
	/**
	 * run the script
	 */
	CVmScript vmScript;
	/**
	 * the block height
	 */
	unsigned int RunTimeHeight;
	CScriptDBViewCache *m_ScriptDBTip;
	CAccountViewCache *m_view;
	vector<CVmOperate> m_output;


	map<vector<unsigned char >,vector<CAppFundOperate> > MapAppOperate;
	shared_ptr<vector<CScriptDBOperLog> > m_dblog;


private:
	/**
	 * @brief The initialization function
	 * @param Tx: run the tx's contact
	 * @param view: Cache holds account
	 *  @param nheight: run the Environment the block's height
	 * @return : check the the tx and account is Legal true is legal false is unlegal
	 */
	bool intial(shared_ptr<CBaseTransaction> & Tx, CAccountViewCache& view, int nheight);
	/**
	 *@brief check aciton
	 * @param listoperate: run the script return the code,check the code
	 * @return : true check success
	 */
	bool CheckOperate(const vector<CVmOperate> &listoperate);
	/**
	 *
	 * @param listoperate: through the vm return code ,The accounts plus money and less money
	 * @param view:
	 * @return true operate account success
	 */
	bool OpeatorAccount(const vector<CVmOperate>& listoperate, CAccountViewCache& view);
	/**
	 * @brief find the vOldAccount from NewAccont if find success remove it from NewAccont
	 * @param vOldAccount: the argument
	 * @return:Return the object
	 */
	std::shared_ptr<CAccount> GetNewAccount(shared_ptr<CAccount>& vOldAccount);
	/**
	 * @brief find the Account from NewAccont
	 * @param Account: argument
	 * @return:Return the object
	 */
	std::shared_ptr<CAccount> GetAccount(shared_ptr<CAccount>& Account);
	/**
	 * @brief get the account id
	 * @param value: argument
	 * @return:Return account id
	 */
	vector_unsigned_char GetAccountID(CVmOperate value);
	bool IsSignatureAccount(CRegID account);
	bool OpeatorAppAccount(const map<vector<unsigned char >,vector<CAppFundOperate> > opMap, CScriptDBViewCache& view) ;
public:
	/**
	 * A constructor.
	 */
	CVmRunEvn();
	/**
	 *@brief get be operate the account
	 * @return the variable RawAccont
	 */
	vector<shared_ptr<CAccount> > &GetRawAccont();
	/**
	 *@brief get after operate the account
	 * @return :the variable NewAccont
	 */
	vector<shared_ptr<CAccount> > &GetNewAccont();
	/**
	 * @brief  start to run the script
	 * @param Tx: run the tx
	 * @param view: the second argument
	 * @param nheight: block height
	 * @param nBurnFactor: Executing a step script to spending
	 * @return: tuple<bool,uint64_t,string>  bool represent the script run success
	 * uint64_t if the script run sucess Run the script calls the money ,string represent run the failed's  Reason
	 */
	tuple<bool,uint64_t,string> run(shared_ptr<CBaseTransaction>& Tx, CAccountViewCache& view,CScriptDBViewCache& VmDB,
			int nheight,uint64_t nBurnFactor, uint64_t &uRunStep);
	/**
	 * @brief just for test
	 * @return:
	 */
	shared_ptr<vector<CVmOperate> > GetOperate() const;
	const CRegID& GetScriptRegID();
	const CRegID &GetTxAccount();
	uint64_t GetValue() const;
	const vector<unsigned char>& GetTxContact();
	CScriptDBViewCache* GetScriptDB();
	CAccountViewCache * GetCatchView();
	int GetComfirHeight();
	uint256 GetCurTxHash();
	void InsertOutputData(const vector<CVmOperate> &source);
	void InsertOutAPPOperte(const vector<unsigned char>& userId,const CAppFundOperate &source);
	shared_ptr<vector<CScriptDBOperLog> > GetDbLog();

	bool GetAppUserAccout(const vector<unsigned char> &id,shared_ptr<CAppUserAccout> &sptrAcc);

	virtual ~CVmRunEvn();
};

enum ACCOUNT_TYPE {
	// account type
	regid = 0x01,			//!< Registration accountid
	base58addr = 0x02,			    //!< pulickey
};
/**
 * @brief after run the script,the script output the code
 */
class CVmOperate{
public:
	unsigned char nacctype;
	unsigned char accountid[34];	//!< accountid
	unsigned char opeatortype;		//!OperType
	unsigned int  outheight;		//!< the transacion Timeout height
	unsigned char money[8];			//!<The transfer amount
	IMPLEMENT_SERIALIZE
	(
			READWRITE(nacctype);
			for(int i = 0;i < 34;i++)
			READWRITE(accountid[i]);
			READWRITE(opeatortype);
			READWRITE(outheight);
			for(int i = 0;i < 8;i++)
			READWRITE(money[i]);
	)
};

#endif /* SCRIPTCHECK_H_ */
