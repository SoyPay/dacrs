#ifndef TX_H
#define TX_H
/*! \mainpage Developer documentation
 *
 * \section intro_sec Introduction
 *
 * This is the developer documentation of the reference client for an experimental new digital currency called Bitcoin (http://www.bitcoin.org/),
 * which enables instant payments to anyone, anywhere in the world. Bitcoin uses peer-to-peer technology to operate
 * with no central authority: managing transactions and issuing money are carried out collectively by the network.
 *
 * The software is a community-driven open source project, released under the MIT license.
 *
 Testing automatic link generation.

 * \section FuntionList
 *    bool CAccount::AddMoney(OperType type, const CFund &fund).\n
 *    bool CAccount::MinusMoney(OperType type, int nHeight, const CFund &fund, const vector_unsigned_char& scriptID); \n
 *	  bool CAccount::HasAuthorityToMinus(uint64_t nMoney,int nHeight,const vector_unsigned_char& scriptID); \n

 \section Navigation
 * Use the buttons <code>Namespaces</code>, <code>Classes</code> or <code>Files</code> at the top of the page to start navigating the code.
 */
#include "serialize.h"
#include <memory>
#include "uint256.h"
#include "key.h"
#include "hash.h"
#include <vector>
#include <string>

using namespace std;

class CTxUndo;
class CValidationState;
class CAccountViewCache;
class CScriptDB;
class CBlock;
class CTransactionCacheDB;
class CTransactionCache;
class CContractScriptCache;
class CAuthorizate;

typedef vector<unsigned char> vector_unsigned_char;

class CRegID {
public:
	vector<unsigned char> vRegID;

	CRegID(string strRegID);

	CRegID(uint32_t nHeight = 0, uint16_t nIndex = 0);

	string ToString() const;

	IMPLEMENT_SERIALIZE
	(
			READWRITE(vRegID);
	)
};

enum TxType {
	REG_ACCT_TX = 1,
	NORMAL_TX = 2,
	CONTRACT_TX = 3,
	FREEZE_TX = 4,
	REWARD_TX = 5,
	REG_SCRIPT_TX = 6,
	NULL_TX,
};

enum RegScriptType {
	SCRIPT_ID = 0, SCRIPT_CONTENT = 1, NULL_TYPE,
};

class CBaseTransaction {

public:
	static int64_t nMinTxFee;
	static int64_t nMinRelayTxFee;
	static const int CURRENT_VERSION = 1;

	unsigned char nTxType;
	int nVersion;
public:
	CBaseTransaction(const CBaseTransaction &other) {
		*this = other;
	}

	CBaseTransaction(int _nVersion, unsigned char _nTxType) :
			nVersion(_nVersion), nTxType(_nTxType) {
	}

	CBaseTransaction() :
			nVersion(CURRENT_VERSION), nTxType(NORMAL_TX) {
	}

	virtual ~CBaseTransaction() {
	}

	virtual unsigned int GetSerializeSize(int nType, int nVersion) const = 0;

	virtual uint256 GetHash() const = 0;

	virtual const vector_unsigned_char& GetvContract() {
		return *((vector_unsigned_char*) nullptr);
	}

	virtual const vector_unsigned_char& GetvSigAcountList() {
		return *((vector_unsigned_char*) nullptr);
	}

	virtual uint64_t GetFee() const = 0;

	virtual double GetPriority() const = 0;

	virtual uint256 SignatureHash() const = 0;

	virtual std::shared_ptr<CBaseTransaction> GetNewInstance() = 0;

	virtual string ToString(CAccountViewCache &view) const = 0;

	virtual bool GetAddress(vector<CKeyID> &vAddr, CAccountViewCache &view) = 0;

	virtual bool IsValidHeight(int nCurHeight, int nTxCacheHeight) const = 0;

	bool IsCoinBase() {
		return (nTxType == REWARD_TX);
	}

	virtual bool UpdateAccount(int nIndex, CAccountViewCache &view, CValidationState &state, CTxUndo &txundo,
			int nHeight, CTransactionCache &txCache, CContractScriptCache &scriptCache) = 0;

	virtual bool UndoUpdateAccount(int nIndex, CAccountViewCache &view, CValidationState &state, CTxUndo &txundo,
			int nHeight, CTransactionCache &txCache, CContractScriptCache &scriptCache) = 0;

	virtual bool CheckTransction(CValidationState &state, CAccountViewCache &view) = 0;

};

class CRegisterAccountTx: public CBaseTransaction {

public:
	CPubKey pubKey;
	int64_t llFees;
	int nValidHeight;
	vector<unsigned char> signature;

public:
	CRegisterAccountTx(const CBaseTransaction *pBaseTx) {
		assert(REG_ACCT_TX == pBaseTx->nTxType);
		*this = *(CRegisterAccountTx *) pBaseTx;
	}

	CRegisterAccountTx() {
		nTxType = REG_ACCT_TX;
	}

	~CRegisterAccountTx() {
	}

	IMPLEMENT_SERIALIZE
	(
			READWRITE(this->nVersion);
			nVersion = this->nVersion;
			READWRITE(pubKey);
			READWRITE(llFees);
			READWRITE(nValidHeight);
			READWRITE(signature);
	)

	uint64_t GetFee() const {
		return llFees;
	}

	uint256 GetHash() const {
		return SerializeHash(*this);
	}

	double GetPriority() const {
		return llFees / GetSerializeSize(SER_NETWORK, PROTOCOL_VERSION);
	}

	uint256 SignatureHash() const {
		CHashWriter ss(SER_GETHASH, 0);
		ss << pubKey << llFees << nValidHeight;
		return ss.GetHash();
	}

	std::shared_ptr<CBaseTransaction> GetNewInstance() {
		return make_shared<CRegisterAccountTx>(this);
	}

	bool GetAddress(vector<CKeyID> &vAddr, CAccountViewCache &view);

	bool IsValidHeight(int nCurHeight, int nTxCacheHeight) const;

	string ToString(CAccountViewCache &view) const;

	bool UpdateAccount(int nIndex, CAccountViewCache &view, CValidationState &state, CTxUndo &txundo, int nHeight,
			CTransactionCache &txCache, CContractScriptCache &scriptCache);

	bool UndoUpdateAccount(int nIndex, CAccountViewCache &view, CValidationState &state, CTxUndo &txundo, int nHeight,
			CTransactionCache &txCache, CContractScriptCache &scriptCache);

	bool CheckTransction(CValidationState &state, CAccountViewCache &view);
};

class CTransaction: public CBaseTransaction {

public:
	vector_unsigned_char srcRegAccountId;
	uint64_t llFees;
	uint64_t llValues;
	vector_unsigned_char desRegAccountId;
	int nValidHeight;
	vector_unsigned_char signature;
public:

	CTransaction(const CBaseTransaction *pBaseTx) {
		assert(NORMAL_TX == pBaseTx->nTxType);
		*this = *(CTransaction *) pBaseTx;
	}

	CTransaction() {
		srcRegAccountId.clear();
		desRegAccountId.clear();
		signature.clear();
		nTxType = NORMAL_TX;
	}

	~CTransaction() {

	}

	IMPLEMENT_SERIALIZE
	(
			READWRITE(this->nVersion);
			nVersion = this->nVersion;
			READWRITE(srcRegAccountId);
			READWRITE(llFees);
			READWRITE(llValues);
			READWRITE(desRegAccountId);
			READWRITE(nValidHeight);
			READWRITE(signature);
	)

	uint64_t GetFee() const {
		return llFees;
	}

	uint256 GetHash() const {
		return SerializeHash(*this);
	}

	double GetPriority() const {
		return llFees / GetSerializeSize(SER_NETWORK, PROTOCOL_VERSION);
	}

	uint256 SignatureHash() const {
		CHashWriter ss(SER_GETHASH, 0);
		ss << srcRegAccountId << llFees << llValues << desRegAccountId << nValidHeight;
		return ss.GetHash();
	}

	std::shared_ptr<CBaseTransaction> GetNewInstance() {
		return make_shared<CTransaction>(this);
	}

	bool GetAddress(vector<CKeyID> &vAddr, CAccountViewCache &view);

	bool IsValidHeight(int nCurHeight, int nTxCacheHeight) const;

	string ToString(CAccountViewCache &view) const;

	bool UpdateAccount(int nIndex, CAccountViewCache &view, CValidationState &state, CTxUndo &txundo, int nHeight,
			CTransactionCache &txCache, CContractScriptCache &scriptCache);

	bool UndoUpdateAccount(int nIndex, CAccountViewCache &view, CValidationState &state, CTxUndo &txundo, int nHeight,
			CTransactionCache &txCache, CContractScriptCache &scriptCache);

	bool CheckTransction(CValidationState &state, CAccountViewCache &view);
};

class CContractTransaction : public CBaseTransaction {
public:
	vector_unsigned_char scriptRegId;
	vector<vector_unsigned_char> vAccountRegId;
	uint64_t llFees;
	vector_unsigned_char vContract;
	int nValidHeight;
	vector<vector_unsigned_char> vSignature;
public:
	CContractTransaction(const CBaseTransaction *pBaseTx) {
		assert(CONTRACT_TX == pBaseTx->nTxType);
		*this = *(CContractTransaction *) pBaseTx;
	}

	CContractTransaction() {
		nTxType = CONTRACT_TX;
		llFees = 0;
		scriptRegId.clear();
		vAccountRegId.clear();
		vContract.clear();
		vContract.clear();
		nValidHeight = 0;
		vSignature.clear();
	}

	~CContractTransaction() {

	}

	IMPLEMENT_SERIALIZE
	(
			READWRITE(this->nVersion);
			nVersion = this->nVersion;
			READWRITE(scriptRegId);
			READWRITE(vAccountRegId);
			READWRITE(llFees);
			READWRITE(vContract);
			READWRITE(nValidHeight);
			READWRITE(vSignature);
	)

	uint256 GetHash() const {
		return SerializeHash(*this);
	}

	uint64_t GetFee() const {
		return llFees;
	}

	double GetPriority() const {
		return llFees / GetSerializeSize(SER_NETWORK, PROTOCOL_VERSION);
	}

	uint256 SignatureHash() const {
		CHashWriter ss(SER_GETHASH, 0);
		ss << scriptRegId << vAccountRegId << llFees << vContract << nValidHeight ;
		return ss.GetHash();
	}

	std::shared_ptr<CBaseTransaction> GetNewInstance() {
		return make_shared<CContractTransaction>(this);
	}

	string ToString(CAccountViewCache &view) const;

	bool GetAddress(vector<CKeyID> &vAddr, CAccountViewCache &view);

	bool IsValidHeight(int nCurHeight, int nTxCacheHeight) const;

	const vector_unsigned_char& GetvContract() {
		return vContract;
	}

	bool UpdateAccount(int nIndex, CAccountViewCache &view, CValidationState &state, CTxUndo &txundo, int nHeight,
			CTransactionCache &txCache, CContractScriptCache &scriptCache);

	bool UndoUpdateAccount(int nIndex, CAccountViewCache &view, CValidationState &state, CTxUndo &txundo, int nHeight,
			CTransactionCache &txCache, CContractScriptCache &scriptCache);

	bool CheckTransction(CValidationState &state, CAccountViewCache &view);

};
class CFreezeTransaction: public CBaseTransaction {

public:
	vector_unsigned_char regAccountId;
	uint64_t llFees;
	uint64_t llFreezeFunds;
	int nValidHeight;
	int nUnfreezeHeight;
	vector_unsigned_char signature;
public:
	CFreezeTransaction(const CBaseTransaction *pBaseTx) {
		assert(FREEZE_TX == pBaseTx->nTxType);
		*this = *(CFreezeTransaction*) pBaseTx;
	}

	CFreezeTransaction() {
		nTxType = FREEZE_TX;
		llFees = 0;
		llFreezeFunds = 0;
		nUnfreezeHeight = 0;
	}

	~CFreezeTransaction() {

	}

	uint256 GetHash() const {
		return SerializeHash(*this);
	}

	IMPLEMENT_SERIALIZE
	(
			READWRITE(this->nVersion);
			nVersion = this->nVersion;
			READWRITE(regAccountId);
			READWRITE(llFees);
			READWRITE(llFreezeFunds);
			READWRITE(nValidHeight);
			READWRITE(nUnfreezeHeight);
			READWRITE(signature);
	)

	uint64_t GetFee() const {
		return llFees;
	}

	double GetPriority() const {
		return llFees / GetSerializeSize(SER_NETWORK, PROTOCOL_VERSION);
	}

	std::shared_ptr<CBaseTransaction> GetNewInstance() {
		return make_shared<CFreezeTransaction>(this);
	}

	uint256 SignatureHash() const {
		CHashWriter ss(SER_GETHASH, 0);
		ss << regAccountId << llFees << llFreezeFunds << nValidHeight << nUnfreezeHeight;
		return ss.GetHash();
	}

	string ToString(CAccountViewCache &view) const;

	bool GetAddress(vector<CKeyID> &vAddr, CAccountViewCache &view);

	bool IsValidHeight(int nCurHeight, int nTxCacheHeight) const;

	bool UpdateAccount(int nIndex, CAccountViewCache &view, CValidationState &state, CTxUndo &txundo, int nHeight,
			CTransactionCache &txCache, CContractScriptCache &scriptCache);

	bool UndoUpdateAccount(int nIndex, CAccountViewCache &view, CValidationState &state, CTxUndo &txundo, int nHeight,
			CTransactionCache &txCache, CContractScriptCache &scriptCache);

	bool CheckTransction(CValidationState &state, CAccountViewCache &view);
};

class CRewardTransaction: public CBaseTransaction {

public:
	vector_unsigned_char account;   // in genesis block are pubkey, otherwise are account id
	uint64_t rewardValue;
	int nHeight;
public:
	CRewardTransaction(const CBaseTransaction *pBaseTx) {
		assert(REWARD_TX == pBaseTx->nTxType);
		*this = *(CRewardTransaction*) pBaseTx;
	}

	CRewardTransaction(const vector_unsigned_char &accountIn, const uint64_t rewardValueIn, const int _nHeight) {
		nTxType = REWARD_TX;
		account = accountIn;
		rewardValue = rewardValueIn;
		nHeight = _nHeight;
	}

	CRewardTransaction() {
		nTxType = REWARD_TX;
		rewardValue = 0;
		nHeight = 0;
	}

	~CRewardTransaction() {
	}

	IMPLEMENT_SERIALIZE
	(
			READWRITE(this->nVersion);
			nVersion = this->nVersion;
			READWRITE(account);
			READWRITE(rewardValue);
			READWRITE(nHeight);
	)

	uint256 GetHash() const {
		return SerializeHash(*this);
	}

	std::shared_ptr<CBaseTransaction> GetNewInstance() {
		return make_shared<CRewardTransaction>(this);
	}

	uint256 SignatureHash() const {
		CHashWriter ss(SER_GETHASH, 0);
		ss << account << rewardValue << nHeight;
		return ss.GetHash();
	}

	uint64_t GetFee() const {
		return 0;
	}

	double GetPriority() const {
		return 0.0f;
	}

	string ToString(CAccountViewCache &view) const;

	bool GetAddress(vector<CKeyID> &vAddr, CAccountViewCache &view);

	bool IsValidHeight(int nCurHeight, int nTxCacheHeight) const {
		return true;
	}

	bool UpdateAccount(int nIndex, CAccountViewCache &view, CValidationState &state, CTxUndo &txundo, int nHeight,
			CTransactionCache &txCache, CContractScriptCache &scriptCache);

	bool UndoUpdateAccount(int nIndex, CAccountViewCache &view, CValidationState &state, CTxUndo &txundo, int nHeight,
			CTransactionCache &txCache, CContractScriptCache &scriptCache);

	bool CheckTransction(CValidationState &state, CAccountViewCache &view);
};

class CRegistScriptTx: public CBaseTransaction {

public:
	vector_unsigned_char regAccountId;
	unsigned char nFlag; //0: exist scriptId, 1 new script content
	vector_unsigned_char script;
	uint64_t llFees;
	int nValidHeight;
	CAuthorizate aAuthorizate;
	vector_unsigned_char signature;
public:
	CRegistScriptTx(const CBaseTransaction *pBaseTx) {
		assert(REG_SCRIPT_TX == pBaseTx->nTxType);
		*this = *(CRegistScriptTx*) pBaseTx;
	}

	CRegistScriptTx() {
		nTxType = REG_SCRIPT_TX;
		llFees = 0;
		nFlag = 0;
		nValidHeight = 0;
	}

	~CRegistScriptTx() {
	}

	IMPLEMENT_SERIALIZE
	(
			READWRITE(this->nVersion);
			nVersion = this->nVersion;
			READWRITE(regAccountId);
			READWRITE(nFlag);
			READWRITE(script);
			READWRITE(llFees);
			READWRITE(nValidHeight);
			READWRITE(signature);
	)

	uint256 GetHash() const {
		return SerializeHash(*this);
	}

	std::shared_ptr<CBaseTransaction> GetNewInstance() {
		return make_shared<CRegistScriptTx>(this);
	}

	uint256 SignatureHash() const {
		CHashWriter ss(SER_GETHASH, 0);
		ss << regAccountId << nFlag << script << llFees << nValidHeight;
		return ss.GetHash();
	}

	uint64_t GetFee() const {
		return llFees;
	}

	double GetPriority() const {
		return llFees / GetSerializeSize(SER_NETWORK, PROTOCOL_VERSION);
	}

	string ToString(CAccountViewCache &view) const;

	bool GetAddress(vector<CKeyID> &vAddr, CAccountViewCache &view);

	bool IsValidHeight(int nCurHeight, int nTxCacheHeight) const;

	bool UpdateAccount(int nIndex, CAccountViewCache &view, CValidationState &state, CTxUndo &txundo, int nHeight,
			CTransactionCache &txCache, CContractScriptCache &scriptCache);

	bool UndoUpdateAccount(int nIndex, CAccountViewCache &view, CValidationState &state, CTxUndo &txundo, int nHeight,
			CTransactionCache &txCache, CContractScriptCache &scriptCache);

	bool CheckTransction(CValidationState &state, CAccountViewCache &view);
};

enum FundType {
	FREEDOM = 1,
	REWARD_FUND,
	FREEDOM_FUND,
	FREEZD_FUND,
	SELF_FREEZD_FUND,
	NULL_FUNDTYPE,
};

enum OperType {
	ADD_FREE = 1,  		//!< add money to freedom
	MINUS_FREE, 		//!< minus money from freedom
	ADD_SELF_FREEZD,  	//!< add money to self_freezd
	MINUS_SELF_FREEZD,	//!< minus money from self_freeze
	ADD_FREEZD,			//!< add money to to freezed
	MINUS_FREEZD,		//!< minus money from freezed
	NULL_OPERTYPE,		//!< invalid operate type
};

class CFund {

public:
	unsigned char nFundType;
	uint256 uTxHash;
	uint64_t value;
	int nHeight;
public:
	CFund() {
		nFundType = 0;
		uTxHash = 0;
		value = 0;
		nHeight = 0;
	}
	CFund(uint64_t _value) {
		nFundType = 0;
		uTxHash = 0;
		value = _value;
		nHeight = 0;
	}
	CFund(unsigned char _type, uint256 _hash, uint64_t _value, int _Height) {
		nFundType = _type;
		uTxHash = _hash;
		value = _value;
		nHeight = _Height;
	}
	CFund(const CFund &fund) {
		nFundType = fund.nFundType;
		uTxHash = fund.uTxHash;
		value = fund.value;
		nHeight = fund.nHeight;
	}
	CFund & operator =(const CFund &fund) {
		if (this == &fund) {
			return *this;
		}
		this->nFundType = fund.nFundType;
		this->uTxHash = fund.uTxHash;
		this->value = fund.value;
		this->nHeight = fund.nHeight;
		return *this;
	}
	~CFund() {
	}

	bool IsMergeFund(const int & nCurHeight, int &mergeType) const;

	friend bool operator <(const CFund &fa, const CFund &fb) {
		if (fa.nFundType < fb.nFundType)
			return true;
		else if (fa.nFundType == fb.nFundType) {
			if (fa.nHeight <= fb.nHeight)
				return true;
			else {
				return false;
			}
		} else
			return false;
	}

	friend bool operator >(const CFund &fa, const CFund &fb) {
		return !operator<(fa, fb);
	}

	friend bool operator ==(const CFund &fa, const CFund &fb) {
		if (fa.nFundType != fb.nFundType)
			return false;
		if (fa.uTxHash != fb.uTxHash)
			return false;
		if (fa.value != fb.value)
			return false;
		if (fa.nHeight != fb.nHeight)
			return false;
		return true;
	}

	IMPLEMENT_SERIALIZE
	(
			READWRITE(nFundType);
			READWRITE(uTxHash);
			READWRITE(value);
			READWRITE(nHeight);

	)

	string ToString() const;
};

enum AccountOper {
	ADD_FUND = 1, 	//!< add operate
	MINUS_FUND = 2, //!< minus operate
	NULL_OPER,		//!< invalid
};

class COperFund {
public:
	unsigned char operType;  //1:ADD_VALUE 2:MINUS_VALUE
	vector<CFund> vFund;

	IMPLEMENT_SERIALIZE
	(
			READWRITE(operType);
			READWRITE(vFund);
	)
public:
	COperFund() {
		operType = NULL_OPER;
		vFund.clear();
	}

	COperFund(unsigned char nType, const vector<CFund>& vOperFund) {
		operType = nType;
		vFund = vOperFund;
	}

	COperFund(unsigned char nType, const CFund& fund) {
		operType = nType;
		vFund.push_back(fund);
	}

	string ToString() const;

};

class CAccountOperLog {

public:
	CKeyID keyID;
	vector<COperFund> vOperFund;//
	IMPLEMENT_SERIALIZE(
			READWRITE(keyID);
			READWRITE(vOperFund);
	)
public:
	void InsertOperateLog(const COperFund& op) {
		vOperFund.push_back(op);
	}

	string ToString() const;

};

class CTxUndo {
public:
	vector<CAccountOperLog> vAccountOperLog;

	IMPLEMENT_SERIALIZE(
			READWRITE(vAccountOperLog);
	)

public:
	bool GetAccountOperLog(const CKeyID &keyId, CAccountOperLog &accountOperLog);
	string ToString() const;
};

class CAccount {
public:
	CKeyID keyID;
	CPubKey publicKey;
	uint64_t llValues;
	vector<CFund> vRewardFund;
	vector<CFund> vFreedomFund;
	vector<CFund> vFreeze;
	vector<CFund> vSelfFreeze;
	map<vector_unsigned_char,CAuthorizate> mapAuthorizate;	//!< Key:scriptID,value :CAuthorizate
	CAccountOperLog accountOperLog;							//!< record operlog, write at undoinfo
public :
	/**
	 * @brief add money to account
	 * @param type:	must be ADD_FREE or ADD_SELF_FREEZD or ADD_FREEZD
	 * @param fund:	member value in fund is the amount of money to add
	 * @return return true if operate successfully,otherwise return false
	 */
	bool AddMoney(OperType type, const CFund &fund);

	/**
	 * @brief :	minus money from account
	 * @param type:	must be MINUS_FREE or MINUS_SELF_FREEZD or MINUS_FREEZD
	 * @param nHeight:	the height that block connected into chain
	 * @param fund:	member value in fund is the amount of money to add
	 * @param scriptID:
	 * @return return true if operate successfully,otherwise return false
	 */
	bool MinusMoney(OperType type, int nHeight, const CFund &fund, const vector_unsigned_char& scriptID);

	/**
	 * @brief:	test if we can minus money in the script
	 * @param nMoney:	the amount of money to minus
	 * @param nHeight:	the height that block connect into the chain
	 * @param scriptID:
	 * @return if we can minus the money then return ture,otherwise return false
	 */
	bool HasAuthorityToMinus(uint64_t nMoney,int nHeight,const vector_unsigned_char& scriptID);

public:
	CAccount(CKeyID &keyId, CPubKey &pubKey) :
			keyID(keyId), publicKey(pubKey) {
		llValues = 0;
		accountOperLog.keyID = keyID;
		vFreedomFund.clear();
		vSelfFreeze.clear();
	}
	CAccount() :
			keyID(uint160(0)), llValues(0) {
		publicKey = CPubKey();
		accountOperLog.keyID = keyID;
		vFreedomFund.clear();
		vSelfFreeze.clear();
	}
	std::shared_ptr<CAccount> GetNewInstance() {
		return make_shared<CAccount>(*this);
	}
	uint64_t GetInterest() const {
		uint64_t rest = 0;

		return rest;
	}
	bool IsRegister() const {
		return (publicKey.IsFullyValid() && publicKey.GetID() == keyID);
	}
	uint64_t GetMatureAmount(int nCurHeight);
	uint64_t GetForzenAmount(int nCurHeight);
	uint64_t GetBalance(int nCurHeight);
	uint256 BuildMerkleTree(int prevBlockHeight) const;
	void ClearAccPos(uint256 hash, int prevBlockHeight, int nIntervalPos);
	uint64_t GetSecureAccPos(int prevBlockHeight) const;
	~CAccount() {

	}
	string ToString() const;
	bool IsEmptyValue() const {
		return !(llValues > 0 || !vFreedomFund.empty() || !vFreeze.empty() || !vSelfFreeze.empty());
	}
	void CompactAccount(int nCurHeight);

	bool UndoOperateAccount(const CAccountOperLog & accountOperLog);
	CFund& FindFund(const vector<CFund>& vFund, const uint256 &hash);

	IMPLEMENT_SERIALIZE
	(
			READWRITE(keyID);
			READWRITE(publicKey);
			READWRITE(llValues);
			READWRITE(vRewardFund);
			READWRITE(vFreedomFund);
			READWRITE(vFreeze);
			READWRITE(vSelfFreeze);
	)

private:
	void MergerFund(vector<CFund> &vFund, int nCurHeight);
	void WriteOperLog(AccountOper emOperType, const CFund &fund);
	void WriteOperLog(const COperFund &operLog);
	bool CheckAddFund(OperType type, const CFund& fund);
	bool MinusFreezed(const CFund& fund);
	bool MinusFree(const CFund &fund);
	bool MinusSelf(const CFund &fund);
	bool IsMoneyOverflow(uint64_t nAddMoney);
	void AddToFreeze(const CFund &fund);
	void UpdateAuthority(int nHeight,uint64_t nMoney, const vector_unsigned_char& scriptID);
	uint64_t GetVecMoney(const vector<CFund>& vFund);
};

class CNetAuthorizate {
public:
	uint32_t GetAuthorizeTime() const {
		return nAuthorizeTime;
	}
	uint64_t GetMaxMoneyPerTime() const {
		return nMaxMoneyPerTime;
	}
	uint64_t GetMaxMoneyTotal() const {
		return nMaxMoneyTotal;
	}
	uint64_t GetMaxMoneyPerDay() const {
		return nMaxMoneyPerDay;
	}

	void SetAuthorizeTime(uint32_t nTime) {
		nAuthorizeTime = nTime;
	}
	void SetMaxMoneyPerTime(uint64_t nMoney) {
		nMaxMoneyPerTime = nMoney;
	}
	void SetMaxMoneyTotal(uint64_t nMoney) {
		nMaxMoneyTotal = nMoney;
	}
	void SetMaxMoneyPerDay(uint64_t nMoney) {
		nMaxMoneyPerDay = nMoney;
	}

	IMPLEMENT_SERIALIZE
	(
			READWRITE(VARINT(nAuthorizeTime));
			READWRITE(VARINT(nUserDefine));
			READWRITE(VARINT(nMaxMoneyPerTime));
			READWRITE(VARINT(nMaxMoneyTotal));
			READWRITE(VARINT(nMaxMoneyPerDay));
	)

protected:
	uint64_t nAuthorizeTime;
	uint32_t nUserDefine;
	uint64_t nMaxMoneyPerTime;
	uint64_t nMaxMoneyTotal;
	uint64_t nMaxMoneyPerDay;
};


class CAuthorizate :public CNetAuthorizate{
public:
	uint64_t GetCurMaxMoneyPerDay() const {
		return nCurMaxMoneyPerDay;
	}
	uint32_t GetLastOperHeight() const {
		return nLastOperHeight;
	}

	void SetCurMaxMoneyPerDay(uint64_t nMoney) {
		nCurMaxMoneyPerDay = nMoney;
	}
	void SetLastOperHeight(uint32_t nHeight) {
		nLastOperHeight = nHeight;
	}

	IMPLEMENT_SERIALIZE
	(
			READWRITE(VARINT(nLastOperHeight));
			READWRITE(VARINT(nAuthorizeTime));
			READWRITE(VARINT(nMaxMoneyPerTime));
			READWRITE(VARINT(nUserDefine));
			READWRITE(VARINT(nMaxMoneyTotal));
			READWRITE(VARINT(nMaxMoneyPerDay));
			READWRITE(VARINT(nCurMaxMoneyPerDay));
	)

private:
	uint32_t nLastOperHeight;
	uint64_t nCurMaxMoneyPerDay;
};

class CContractScript {
public:
	vector_unsigned_char scriptId;
	vector_unsigned_char scriptContent;
	set<string> setArbitratorAccId;
public:
	bool IsContainScript(const vector_unsigned_char &scriptContent,
			const map<string, CContractScript> &mapScript) const;
};

class CTransactionCache {
private:
	CTransactionCacheDB *base;
	map<uint256, vector<uint256> > mapTxHashByBlockHash;  // key:block hash  value:tx hash
	map<uint256, vector<uint256> > mapTxHashCacheByPrev;  // key:pre tx hash  value:relay tx hash
public:
	CTransactionCache(CTransactionCacheDB *pTxCacheDB);
	void SetTxCacheSize(int nSize);
	int GetTxCacheSize(void) const;
	bool AddBlockToCache(const CBlock &block);
	bool DeleteBlockFromCache(const CBlock &block);
	bool IsContainTx(const uint256 & txHash);
	vector<uint256> GetRelayTx(const uint256 & txHash);
	const map<uint256, vector<uint256> > &GetRelayTx(void) const;
	const map<uint256, vector<uint256> > &GetTxHashCache(void) const;
	void AddTxHashCache(const uint256 & blockHash, const vector<uint256> &vTxHash);
	void AddRelayTx(const uint256 preTxHash, const vector<uint256> &vTxHash);
	bool Flush();
	bool LoadTransaction();
	void Clear();
};

class CContractScriptCache {
private:
	CScriptDB *base;
	map<string, CContractScript> mapScript;
public:
	CContractScriptCache(CScriptDB *base);
	bool GetContractScript(const string &strKey, CContractScript &contractScript);
	bool IsContainContractScript(const string &strKey);
	bool AddContractScript(const string &strKey, const CContractScript &script);
	bool DeleteContractScript(const string &strKey);
	bool LoadRegScript();
    bool Flush();
    map<string, CContractScript> &GetScriptCache();
    bool GetScript(const string &strKey, vector<unsigned char> &vscript);
    bool GetArbitrator(const string &strKey, set<string> &setArbId);
    bool SetArbitrator(const string &strKey, const set<string> &setArbitrator);
};

inline unsigned int GetSerializeSize(const std::shared_ptr<CBaseTransaction> &pa, int nType, int nVersion) {
	return pa->GetSerializeSize(nType, nVersion) + 1;
}

template<typename Stream>
void Serialize(Stream& os, const std::shared_ptr<CBaseTransaction> &pa, int nType, int nVersion) {
	unsigned char ntxType = pa->nTxType;
	Serialize(os, ntxType, nType, nVersion);
	if (pa->nTxType == REG_ACCT_TX) {
		Serialize(os, *((CRegisterAccountTx *) (pa.get())), nType, nVersion);
	}
	else if (pa->nTxType == NORMAL_TX) {
		Serialize(os, *((CTransaction *) (pa.get())), nType, nVersion);
	}
	else if (pa->nTxType == CONTRACT_TX) {
		Serialize(os, *((CContractTransaction *) (pa.get())), nType, nVersion);
	}
	else if (pa->nTxType == FREEZE_TX) {
		Serialize(os, *((CFreezeTransaction *) (pa.get())), nType, nVersion);
	}
	else if (pa->nTxType == REWARD_TX) {
		Serialize(os, *((CRewardTransaction *) (pa.get())), nType, nVersion);
	}
	else if (pa->nTxType == REG_SCRIPT_TX) {
		Serialize(os, *((CRegistScriptTx *) (pa.get())), nType, nVersion);
	}
	else {
		assert(0);
	}

}

template<typename Stream>
void Unserialize(Stream& is, std::shared_ptr<CBaseTransaction> &pa, int nType, int nVersion) {
	char nTxType;
	is.read((char*) &(nTxType), sizeof(nTxType));
	if (nTxType == REG_ACCT_TX) {
		pa = make_shared<CRegisterAccountTx>();
		Unserialize(is, *((CRegisterAccountTx *) (pa.get())), nType, nVersion);
	}
	else if (nTxType == NORMAL_TX) {
		pa = make_shared<CTransaction>();
		Unserialize(is, *((CTransaction *) (pa.get())), nType, nVersion);
	}
	else if (nTxType == CONTRACT_TX) {
		pa = make_shared<CContractTransaction>();
		Unserialize(is, *((CContractTransaction *) (pa.get())), nType, nVersion);
	}
	else if (nTxType == FREEZE_TX) {
		pa = make_shared<CFreezeTransaction>();
		Unserialize(is, *((CFreezeTransaction *) (pa.get())), nType, nVersion);
	}
	else if (nTxType == REWARD_TX) {
		pa = make_shared<CRewardTransaction>();
		Unserialize(is, *((CRewardTransaction *) (pa.get())), nType, nVersion);
	}
	else if (nTxType == REG_SCRIPT_TX) {
		pa = make_shared<CRegistScriptTx>();
		Unserialize(is, *((CRegistScriptTx *) (pa.get())), nType, nVersion);
	}
	else {
		assert(0);
	}
	pa->nTxType = nTxType;
}

#endif
