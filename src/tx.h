#ifndef DACRS_TX_H_
#define DACRS_TX_H_

#include "serialize.h"
#include <memory>
#include "uint256.h"
#include "key.h"
#include "hash.h"
#include <vector>
#include <string>
#include <boost/variant.hpp>
#include "chainparams.h"
#include "json/json_spirit_utils.h"
#include "json/json_spirit_value.h"
using namespace json_spirit;

using namespace std;

class CTxUndo;
class CValidationState;
class CAccountViewCache;
class CScriptDB;
class CBlock;
class CTransactionDBCache;
class CScriptDBViewCache;
class CRegID;
class CID;
class CAccountLog;

static const int g_sTxVersion1 = 1;    //交易初始版本。
static const int g_sTxVersion2 = 2;    //交易版本升级后，其签名hash和交易hash算法升级为新的算法。

typedef vector<unsigned char> vector_unsigned_char;

#define SCRIPT_ID_SIZE (6)

enum emTxType {
	EM_REWARD_TX = 1,    	//!< reward tx
	EM_REG_ACCT_TX = 2,  	//!< tx that used to register account
	EM_COMMON_TX = 3,    	//!< transfer money from one account to another
	EM_CONTRACT_TX = 4,  	//!< contract tx
	EM_REG_APP_TX = 5,		//!< register app
	EM_NULL_TX,          	//!< NULL_TX
};

/**
 * brief:	kinds of fund type
 */
enum emFundType {
	EM_FREEDOM = 1,	    	//!< FREEDOM
	EM_REWARD_FUND,     	//!< REWARD_FUND
	EM_NULL_FUNDTYPE,   	//!< NULL_FUNDTYPE
};

enum emOperType {
	EM_ADD_FREE = 1,  		//!< add money to freedom
	EM_MINUS_FREE, 		//!< minus money from freedom
	EM_NULL_OPERTYPE,		//!< invalid operate type
};

enum emAccountOper {
	EM_ADD_FUND = 1, 		//!< add operate
	EM_MINUS_FUND = 2, 	//!< minus operate
	EM_NULL_OPER,			//!< invalid
};

class CNullID {
 public:
	friend bool operator==(const CNullID &a, const CNullID &b) {
		return true;
	}
	friend bool operator<(const CNullID &a, const CNullID &b) {
		return true;
	}
};

typedef boost::variant<CNullID, CRegID, CKeyID, CPubKey> CUserID;
/*CRegID 是地址激活后，分配的账户ID*/
class CRegID {
 public:
	friend class CID;
	CRegID(string strRegID);
	CRegID(const vector<unsigned char> &vchIn) ;
	CRegID(uint32_t uHeight = 0, uint16_t ushIndex = 0);

	const vector<unsigned char> &GetVec6() const {
		assert(m_vchRegID.size() == 6);
		return m_vchRegID;
	}
	void SetRegID(const vector<unsigned char> &vchIn) ;
    CKeyID getKeyID(const CAccountViewCache &cAccountViewCache)const;
    uint32_t getHight()const { return m_uHeight;};

	bool operator ==(const CRegID& cRegID) const {
		return (this->m_uHeight == cRegID.m_uHeight && this->m_ushIndex == cRegID.m_ushIndex);
	}
	bool operator !=(const CRegID& cRegID) const {
		return (this->m_uHeight != cRegID.m_uHeight || this->m_ushIndex != cRegID.m_ushIndex);
	}
	static bool IsSimpleRegIdStr(const string & str);
	static bool IsRegIdStr(const string & str);
	static bool GetKeyID(const string & str,CKeyID &cKeyId);

	bool IsEmpty() const {
		return (m_uHeight == 0 && m_ushIndex == 0);
	}

    bool clean();

	string ToString() const;

	IMPLEMENT_SERIALIZE
	(
		READWRITE(VARINT(m_uHeight));
		READWRITE(VARINT(m_ushIndex));
		if(fRead) {
			m_vchRegID.clear();
			m_vchRegID.insert(m_vchRegID.end(), BEGIN(m_uHeight), END(m_uHeight));
			m_vchRegID.insert(m_vchRegID.end(), BEGIN(m_ushIndex), END(m_ushIndex));
		}
	)

 private:
 	uint32_t m_uHeight;
 	uint16_t m_ushIndex;
 	mutable vector<unsigned char> m_vchRegID;
 	void SetRegIDByCompact(const vector<unsigned char> &vchIn);
 	void SetRegID(string strRegID);
};

/*CID是一个vector 存放CRegID,CKeyID,CPubKey*/
class CID {
 public:
	const vector_unsigned_char &GetID() {
		return m_vchData;
	}
	static const vector_unsigned_char & UserIDToVector(const CUserID &cUserid) {
		return CID(cUserid).GetID();
	}
	bool Set(const CRegID &cRegId);
	bool Set(const CKeyID &cKeyID);
	bool Set(const CPubKey &cPubKey);
	bool Set(const CNullID &cNullID);
	bool Set(const CUserID &cUserid);
	CID() {
	}
	CID(const CUserID &cUserIDDest) {
		Set(cUserIDDest);
	}
	CUserID GetUserId();IMPLEMENT_SERIALIZE
	(
			READWRITE(m_vchData);
	)

 private:
 	vector_unsigned_char m_vchData;
};

class CIDVisitor: public boost::static_visitor<bool> {
 public:
	CIDVisitor(CID *pIdIn) :
		m_pId(pIdIn) {
	}
	bool operator()(const CRegID &cRegId) const {
			return m_pId->Set(cRegId);
	}
	bool operator()(const CKeyID &cKeyId) const {
		return m_pId->Set(cKeyId);
	}
	bool operator()(const CPubKey &cPubKey) const {
		return m_pId->Set(cPubKey);
	}
	bool operator()(const CNullID &cNullId) const {
		return true;
	}

 private:
	CID *m_pId;
};

class CBaseTransaction {
 public:
	CBaseTransaction(const CBaseTransaction &cOther) {
		*this = cOther;
	}

	CBaseTransaction(int nVersion, unsigned char chTxType) :
			m_chTxType(chTxType), m_nVersion(nVersion), m_nValidHeight(0), m_ullRunStep(0), m_nFuelRate(0){
	}

	CBaseTransaction() :
			m_chTxType(EM_COMMON_TX), m_nVersion(m_sCurrentVersion), m_nValidHeight(0), m_ullRunStep(0), m_nFuelRate(0){
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

	virtual Object ToJSON(const CAccountViewCache &AccountView) const = 0;

	virtual bool GetAddress(std::set<CKeyID> &vAddr, CAccountViewCache &view, CScriptDBViewCache &scriptDB) = 0;

	virtual bool IsValidHeight(int nCurHeight, int nTxCacheHeight) const;

	bool IsCoinBase() {
		return (m_chTxType == EM_REWARD_TX);
	}

	virtual bool ExecuteTx(int nIndex, CAccountViewCache &view, CValidationState &state, CTxUndo &txundo,
			int nHeight, CTransactionDBCache &txCache, CScriptDBViewCache &scriptDB) = 0;

	virtual bool UndoExecuteTx(int nIndex, CAccountViewCache &view, CValidationState &state, CTxUndo &txundo,
			int nHeight, CTransactionDBCache &txCache, CScriptDBViewCache &scriptDB);

	virtual bool CheckTransction(CValidationState &state, CAccountViewCache &view, CScriptDBViewCache &scriptDB) = 0;

	virtual uint64_t GetFuel(int nfuelRate);
	virtual uint64_t GetValue() const = 0;
	int GetFuelRate(CScriptDBViewCache &scriptDB);

 public:
 	static uint64_t m_sMinTxFee;
 	static int64_t m_sMinRelayTxFee;
 	static const int m_sCurrentVersion = g_sTxVersion2;

 	unsigned char m_chTxType;
 	int m_nVersion;
 	int m_nValidHeight;
 	uint64_t m_ullRunStep;  	//only in memory
 	int m_nFuelRate;      		//only in memory

  protected:
  	static string m_sTxTypeArray[6];
};

class CRegisterAccountTx: public CBaseTransaction {
 public:
	CRegisterAccountTx(const CBaseTransaction *pBaseTx) {
		assert(EM_REG_ACCT_TX == pBaseTx->m_chTxType);
		*this = *(CRegisterAccountTx *) pBaseTx;
	}
	CRegisterAccountTx(const CUserID &uId,const CUserID &cMinerID,int64_t llFees,int nHeight) {
		m_chTxType 		= EM_REG_ACCT_TX;
		m_llFees 		= llFees;
		m_nValidHeight 	= nHeight;
		m_cUserId 		= uId;
		m_cMinerId		=cMinerID;
		m_vchSignature.clear();
	}
	CRegisterAccountTx() {
		m_chTxType 		= EM_REG_ACCT_TX;
		m_llFees 		= 0;
		m_nValidHeight 	= 0;
	}

	~CRegisterAccountTx() {
	}

	IMPLEMENT_SERIALIZE
	(
		READWRITE(VARINT(this->m_nVersion));
		nVersion = this->m_nVersion;
		READWRITE(VARINT(m_nValidHeight));
		CID id(m_cUserId);
		READWRITE(id);
		CID mMinerid(m_cMinerId);
		READWRITE(mMinerid);
		if(fRead) {
			m_cUserId = id.GetUserId();
			m_cMinerId = mMinerid.GetUserId();
		}
		READWRITE(VARINT(m_llFees));
		READWRITE(m_vchSignature);
	)
	uint64_t GetValue() const {return 0;}
	uint64_t GetFee() const {
		return m_llFees;
	}

	uint256 GetHash() const;

	double GetPriority() const {
		return m_llFees / GetSerializeSize(SER_NETWORK, g_sProtocolVersion);
	}

	uint256 SignatureHash() const;

	std::shared_ptr<CBaseTransaction> GetNewInstance() {
		return std::make_shared<CRegisterAccountTx>(this);
	}

	bool GetAddress(set<CKeyID> &vAddr, CAccountViewCache &cAccountViewCache, CScriptDBViewCache &cScriptDB);

	string ToString(CAccountViewCache &cAccountViewCache) const;

	Object ToJSON(const CAccountViewCache &cAccountView) const;

	bool ExecuteTx(int nIndex, CAccountViewCache &cAccountViewCache, CValidationState &cValidationState, CTxUndo &cTxundo, int nHeight,
			CTransactionDBCache &cTxCache, CScriptDBViewCache &cScriptDB);

	bool UndoExecuteTx(int nIndex, CAccountViewCache &cAccountViewCache, CValidationState &cValidationState, CTxUndo &txundo, int nHeight,
			CTransactionDBCache &cTxCache, CScriptDBViewCache &cScriptDB);

	bool CheckTransction(CValidationState &cValidationState, CAccountViewCache &cAccountViewCache, CScriptDBViewCache &cScriptDB);

 public:
 	mutable CUserID m_cUserId;      //pubkey
 	mutable CUserID m_cMinerId;     //Miner pubkey
 	int64_t m_llFees;
 	vector<unsigned char> m_vchSignature;
};

class CTransaction : public CBaseTransaction {
 public:
	CTransaction(const CBaseTransaction *pBaseTx) {
		assert(EM_CONTRACT_TX == pBaseTx->m_chTxType || EM_COMMON_TX == pBaseTx->m_chTxType);
		*this = *(CTransaction *) pBaseTx;
	}
	CTransaction(const CUserID& cInUserRegId, CUserID cInDesUserId, uint64_t ullFee, uint64_t ullValue, int nHeight, vector_unsigned_char& vchContract)
	{
		if (cInUserRegId.type() == typeid(CRegID)) {
			assert(!boost::get<CRegID>(cInUserRegId).IsEmpty());
		}
		if (cInDesUserId.type() == typeid(CRegID)) {
			assert(!boost::get<CRegID>(cInDesUserId).IsEmpty());
		}
		m_chTxType 			= EM_CONTRACT_TX;
		m_cSrcRegId 		= cInUserRegId;
		m_cDesUserId 		= cInDesUserId;
		m_vchContract 		= vchContract;
		m_nValidHeight 		= nHeight;
		m_ullFees 			= ullFee;
		m_ullValues 		= ullValue;
		m_vchSignature.clear();
	}
	CTransaction(const CUserID& cInUserRegId, CUserID cInDesUserId, uint64_t ullFee, uint64_t ullValue, int nHeight)
	{
		m_chTxType = EM_COMMON_TX;
		if (cInUserRegId.type() == typeid(CRegID)) {
			assert(!boost::get<CRegID>(cInUserRegId).IsEmpty());
		}
		if (cInDesUserId.type() == typeid(CRegID)) {
			assert(!boost::get<CRegID>(cInDesUserId).IsEmpty());
		}
		m_cSrcRegId 		= cInUserRegId;
		m_cDesUserId 		= cInDesUserId;
		m_nValidHeight 		= nHeight;
		m_ullFees 			= ullFee;
		m_ullValues 		= ullValue;
		m_vchSignature.clear();
	}
	CTransaction() {
		m_chTxType 		= EM_COMMON_TX;
		m_ullFees 		= 0;
		m_nValidHeight 	= 0;
		m_ullValues 	= 0;
		m_vchContract.clear();
		m_vchSignature.clear();
	}

	~CTransaction() {

	}

	IMPLEMENT_SERIALIZE
	(
			READWRITE(VARINT(this->m_nVersion));
			nVersion = this->m_nVersion;
			READWRITE(VARINT(m_nValidHeight));
			CID srcId(m_cSrcRegId);
			READWRITE(srcId);
			CID desId(m_cDesUserId);
			READWRITE(desId);
			READWRITE(VARINT(m_ullFees));
			READWRITE(VARINT(m_ullValues));
			READWRITE(m_vchContract);
			READWRITE(m_vchSignature);
			if(fRead) {
				m_cSrcRegId = srcId.GetUserId();
				m_cDesUserId = desId.GetUserId();
			}
	)
	uint64_t GetValue() const {return m_ullValues;}
	uint256 GetHash() const;

	uint64_t GetFee() const {
		return m_ullFees;
	}

	double GetPriority() const {
		return m_ullFees / GetSerializeSize(SER_NETWORK, g_sProtocolVersion);
	}

	uint256 SignatureHash() const;

	std::shared_ptr<CBaseTransaction> GetNewInstance() {
		return std::make_shared<CTransaction>(this);
	}

	string ToString(CAccountViewCache &cAccountViewCache) const;

	Object ToJSON(const CAccountViewCache &cAccountView) const;

	bool GetAddress(set<CKeyID> &vcAddr, CAccountViewCache &cAccountViewCache, CScriptDBViewCache &cScriptDB);

	const vector_unsigned_char& GetvContract() {
		return m_vchContract;
	}

	bool ExecuteTx(int nIndex, CAccountViewCache &cAccountViewCache, CValidationState &cValidationState, CTxUndo &cTxundo, int nHeight,
			CTransactionDBCache &cTxCache, CScriptDBViewCache &cScriptDB);

	bool CheckTransction(CValidationState &cValidationState, CAccountViewCache &cAccountViewCache, CScriptDBViewCache &cScriptDB);

 public:
 	mutable CUserID m_cSrcRegId;                   //src regid
 	mutable CUserID m_cDesUserId;                  //user regid or user key id or app regid
 	uint64_t m_ullFees;
 	uint64_t m_ullValues;                          //transfer amount
 	vector_unsigned_char m_vchContract;
 	vector_unsigned_char m_vchSignature;
};

class CRewardTransaction: public CBaseTransaction {
 public:
	CRewardTransaction(const CBaseTransaction *pBaseTx) {
		assert(EM_REWARD_TX == pBaseTx->m_chTxType);
		*this = *(CRewardTransaction*) pBaseTx;
	}

	CRewardTransaction(const vector_unsigned_char &accountIn, const uint64_t rewardValueIn, const int _nHeight) {
		m_chTxType = EM_REWARD_TX;
		if (accountIn.size() > 6) {
			m_cAccount = CPubKey(accountIn);
		} else {
			m_cAccount = CRegID(accountIn);
		}
		m_ullRewardValue = rewardValueIn;
		m_nHeight = _nHeight;
	}

	CRewardTransaction() {
		m_chTxType = EM_REWARD_TX;
		m_ullRewardValue = 0;
		m_nHeight = 0;
	}

	~CRewardTransaction() {
	}

	IMPLEMENT_SERIALIZE
	(
		READWRITE(VARINT(this->m_nVersion));
		nVersion = this->m_nVersion;
		CID acctId(m_cAccount);
		READWRITE(acctId);
		if(fRead) {
			m_cAccount = acctId.GetUserId();
		}
		READWRITE(VARINT(m_ullRewardValue));
		READWRITE(VARINT(m_nHeight));
	)
	uint64_t GetValue() const {return m_ullRewardValue;}
	uint256 GetHash() const;

	std::shared_ptr<CBaseTransaction> GetNewInstance() {
		return std::make_shared<CRewardTransaction>(this);
	}

	uint256 SignatureHash() const;

	uint64_t GetFee() const {
		return 0;
	}

	double GetPriority() const {
		return 0.0f;
	}

	string ToString(CAccountViewCache &view) const;

	Object ToJSON(const CAccountViewCache &AccountView) const;

	bool GetAddress(set<CKeyID> &vAddr, CAccountViewCache &view, CScriptDBViewCache &scriptDB);

	bool ExecuteTx(int nIndex, CAccountViewCache &view, CValidationState &state, CTxUndo &txundo, int nHeight,
			CTransactionDBCache &txCache, CScriptDBViewCache &scriptDB);

	bool CheckTransction(CValidationState &state, CAccountViewCache &view, CScriptDBViewCache &scriptDB);

 public:
 	mutable CUserID m_cAccount;   // in genesis block are pubkey, otherwise are m_cAccount id
 	uint64_t m_ullRewardValue;
 	int m_nHeight;
};

class CRegisterAppTx: public CBaseTransaction {
 public:
	CRegisterAppTx(const CBaseTransaction *pBaseTx) {
		assert(EM_REG_APP_TX == pBaseTx->m_chTxType);
		*this = *(CRegisterAppTx*) pBaseTx;
	}

	CRegisterAppTx() {
		m_chTxType = EM_REG_APP_TX;
		m_ullFees = 0;
		m_nValidHeight = 0;
	}

	~CRegisterAppTx() {
	}

	IMPLEMENT_SERIALIZE
	(
		READWRITE(VARINT(this->m_nVersion));
		nVersion = this->m_nVersion;
		READWRITE(VARINT(m_nValidHeight));
		CID regId(m_cRegAcctId);
		READWRITE(regId);
		if(fRead) {
			m_cRegAcctId = regId.GetUserId();
		}
		READWRITE(m_vchScript);
		READWRITE(VARINT(m_ullFees));
		READWRITE(m_vchSignature);
	)
	uint64_t GetValue() const {return 0;}
	uint256 GetHash() const;

	std::shared_ptr<CBaseTransaction> GetNewInstance() {
		return std::make_shared<CRegisterAppTx>(this);
	}

	uint256 SignatureHash() const;

	uint64_t GetFee() const {
		return m_ullFees;
	}

	double GetPriority() const {
		return m_ullFees / GetSerializeSize(SER_NETWORK, g_sProtocolVersion);
	}

	string ToString(CAccountViewCache &view) const;

	Object ToJSON(const CAccountViewCache &AccountView) const;

	bool GetAddress(set<CKeyID> &vAddr, CAccountViewCache &view, CScriptDBViewCache &scriptDB);

	bool ExecuteTx(int nIndex, CAccountViewCache &view, CValidationState &state, CTxUndo &txundo, int nHeight,
			CTransactionDBCache &txCache, CScriptDBViewCache &scriptDB);

	bool UndoExecuteTx(int nIndex, CAccountViewCache &view, CValidationState &state, CTxUndo &txundo, int nHeight,
			CTransactionDBCache &txCache, CScriptDBViewCache &scriptDB);

	bool CheckTransction(CValidationState &state, CAccountViewCache &view, CScriptDBViewCache &scriptDB);

 public:
 	mutable CUserID m_cRegAcctId;         //regid
 	vector_unsigned_char m_vchScript;          //m_vchScript content
 	uint64_t m_ullFees;
 	vector_unsigned_char m_vchSignature;
};

//class CFund {
//public:
//	uint64_t value;					//!< amount of money
//	int nHeight;					//!< confirm height
//public:
//	CFund() {
//		value = 0;
//		nHeight = 0;
//	}
//	CFund(uint64_t _value) {
//		value = _value;
//		nHeight = 0;
//	}
//	CFund(uint64_t _value, int _Height) {
//		value = _value;
//		nHeight = _Height;
//	}
//	CFund(const CFund &fund) {
//		value = fund.value;
//		nHeight = fund.nHeight;
//	}
//	CFund & operator =(const CFund &fund) {
//		if (this == &fund) {
//			return *this;
//		}
//		this->value = fund.value;
//		this->nHeight = fund.nHeight;
//		return *this;
//	}
//	~CFund() {
//	}
//	Object ToJosnObj() const;
//	bool IsMergeFund(const int & nCurHeight, int &mergeType) const;
//
//	uint256 GetHash() const {
//		CHashWriter ss(SER_GETHASH, 0);
//		ss << VARINT(value) << VARINT(nHeight);
//		return ss.GetHash();
//	}
//
//	friend bool operator <(const CFund &fa, const CFund &fb) {
//		if (fa.nHeight <= fb.nHeight)
//			return true;
//		else
//			return false;
//	}
//
//	friend bool operator >(const CFund &fa, const CFund &fb) {
//		return !operator<(fa, fb);
//	}
//
//	friend bool operator ==(const CFund &fa, const CFund &fb) {
//		if (fa.value != fb.value)
//			return false;
//		if (fa.nHeight != fb.nHeight)
//			return false;
//		return true;
//	}
//
//	IMPLEMENT_SERIALIZE
//	(
//		READWRITE(VARINT(value));
//		READWRITE(VARINT(m_nHeight));
//	)
//
//	string ToString() const;
//};

class CScriptDBOperLog {
public:
	vector<unsigned char> m_vchKey;
	vector<unsigned char> m_vchValue;

	CScriptDBOperLog (const vector<unsigned char> &vKeyIn, const vector<unsigned char> &vValueIn) {
		m_vchKey = vKeyIn;
		m_vchValue = vValueIn;
	}

	CScriptDBOperLog() {
		m_vchKey.clear();
		m_vchValue.clear();
	}

	IMPLEMENT_SERIALIZE
	(
		READWRITE(m_vchKey);
		READWRITE(m_vchValue);
	)

	string ToString() const {
		string str("");
		str += "vKey:";
		str += HexStr(m_vchKey);
		str += "\n";
		str +="vValue:";
		str += HexStr(m_vchValue);
		str += "\n";
		return str;
	}

	friend bool operator<(const CScriptDBOperLog &log1, const CScriptDBOperLog &log2) {
		return log1.m_vchKey < log2.m_vchKey;
	}
};

//class COperFund {
//public:
//	unsigned char operType;  		//!<1:ADD_VALUE 2:MINUS_VALUE
//	CFund fund;
//
//	IMPLEMENT_SERIALIZE
//	(
//		READWRITE(operType);
//		READWRITE(fund);
//	)
//public:
//	COperFund() {
//		operType = NULL_OPER;
//	}
//
//	COperFund(unsigned char nType, const CFund& operFund) {
//		operType = nType;
//		fund = operFund;
//	}
//
//	string ToString() const;
//
//};

class CTxUndo {
 public:
	bool GetAccountOperLog(const CKeyID &keyId, CAccountLog &accountLog);
	void Clear() {
		m_cTxHash = uint256();
		m_vcAccountLog.clear();
		m_vcScriptOperLog.clear();
	}
	string ToString() const;

 public:
	uint256 m_cTxHash;
	vector<CAccountLog> m_vcAccountLog;
	vector<CScriptDBOperLog> m_vcScriptOperLog;
	IMPLEMENT_SERIALIZE
	(
		READWRITE(m_cTxHash);
		READWRITE(m_vcAccountLog);
		READWRITE(m_vcScriptOperLog);
	)

};


class CAccount {
 public :
	/**
	 * @brief operate account
	 * @param type:	operate type
	 * @param values
	 * @param nCurHeight:the height that block connected into chain
	 * @return if operate successfully return ture,otherwise return false
	 */
	bool OperateAccount(emOperType type, const uint64_t &values,const int nCurHeight);

	bool UndoOperateAccount(const CAccountLog & accountLog);

 public:
	CAccount(CKeyID &keyId, CPubKey &pubKey) :
			m_cKeyID(keyId), m_cPublicKey(pubKey) {
		m_ullValues = 0;
		m_cMinerPKey =  CPubKey();
		m_nHeight = 0;
		m_cRegID.clean();
		m_ullCoinDay = 0;
	}
	CAccount() :m_cKeyID(uint160()), m_ullValues(0) {
		m_cPublicKey = CPubKey();
		m_cMinerPKey =  CPubKey();
		m_nHeight = 0;
		m_cRegID.clean();
		m_ullCoinDay = 0;
	}
	CAccount(const CAccount & other) {
		this->m_cRegID = other.m_cRegID;
		this->m_cKeyID = other.m_cKeyID;
		this->m_cPublicKey = other.m_cPublicKey;
		this->m_cMinerPKey = other.m_cMinerPKey;
		this->m_ullValues = other.m_ullValues;
		this->m_nHeight = other.m_nHeight;
		this->m_ullCoinDay = other.m_ullCoinDay;
	}
	CAccount &operator=(const CAccount & other) {
		if(this == &other)
			return *this;
		this->m_cRegID = other.m_cRegID;
		this->m_cKeyID = other.m_cKeyID;
		this->m_cPublicKey = other.m_cPublicKey;
		this->m_cMinerPKey = other.m_cMinerPKey;
		this->m_ullValues = other.m_ullValues;
		this->m_nHeight = other.m_nHeight;
		this->m_ullCoinDay = other.m_ullCoinDay;
		return *this;
	}
	std::shared_ptr<CAccount> GetNewInstance() const{
		return std::make_shared<CAccount>(*this);
	}

	bool IsMiner(int nCurHeight) {
		if(nCurHeight < 2*SysCfg().GetIntervalPos())
			return true;
		return m_ullCoinDay >= m_ullValues * SysCfg().GetIntervalPos();

	}
	bool IsRegister() const {
		return (m_cPublicKey.IsFullyValid() && m_cPublicKey.GetKeyID() == m_cKeyID);
	}
	bool SetRegId(const CRegID &regID){this->m_cRegID = regID;return true;};
	bool GetRegId(CRegID &regID)const {regID = this->m_cRegID;return !regID.IsEmpty();};
	uint64_t GetRawBalance();
	void ClearAccPos(int nCurHeight);
	uint64_t GetAccountPos(int prevBlockHeight);
	string ToString() const;
	Object ToJosnObj() const;
	bool IsEmptyValue() const {
		return !(m_ullValues > 0);
	}
	uint256 GetHash(){
		CHashWriter ss(SER_GETHASH, 0);
		ss << m_cRegID << m_cKeyID << m_cPublicKey << m_cMinerPKey << VARINT(m_ullValues)
		   << VARINT(m_nHeight) << VARINT(m_ullCoinDay);
		return ss.GetHash();
	}
	uint64_t GetMaxCoinDay(int nCurHeight) {
		return m_ullValues * SysCfg().GetMaxDay();
	}

	bool UpDateCoinDay(int nCurHeight);
	IMPLEMENT_SERIALIZE
	(
		READWRITE(m_cRegID);
		READWRITE(m_cKeyID);
		READWRITE(m_cPublicKey);
		READWRITE(m_cMinerPKey);
		READWRITE(VARINT(m_ullValues));
		READWRITE(VARINT(m_nHeight));
		READWRITE(VARINT(m_ullCoinDay));
	)

 public:
 	CRegID m_cRegID;
 	CKeyID m_cKeyID;											//!< keyID of the m_cAccount
 	CPubKey m_cPublicKey;										//!< public key of the m_cAccount
 	CPubKey m_cMinerPKey;									    //!< public key of the account for miner
 	uint64_t m_ullValues;										//!< total money
 	int m_nHeight;                                            //!< update height
 	uint64_t m_ullCoinDay;									    //!< coin day

 private:
	bool IsMoneyOverflow(uint64_t nAddMoney);
};


class CAccountLog {
 public:
	CAccountLog(const CAccount &acct) {
		m_cKeyID = acct.m_cKeyID;
		m_ullValues = acct.m_ullValues;
		m_nHeight = acct.m_nHeight;
		m_ullCoinDay = acct.m_ullCoinDay;
	}
	CAccountLog(CKeyID &keyId) {
		m_cKeyID = keyId;
		m_ullValues = 0;
		m_nHeight = 0;
		m_ullCoinDay = 0;
	}
	CAccountLog() {
		m_cKeyID = uint160();
		m_ullValues = 0;
		m_nHeight = 0;
		m_ullCoinDay = 0;
	}
	void SetValue(const CAccount &acct) {
		m_cKeyID = acct.m_cKeyID;
		m_ullValues = acct.m_ullValues;
		m_nHeight = acct.m_nHeight;
		m_ullCoinDay = acct.m_ullCoinDay;
	}
	string ToString() const;

 public:
 	CKeyID m_cKeyID;
 	uint64_t m_ullValues;										//!< freedom money which coinage greater than 30 days
 	int m_nHeight;                                            	//!< update height
 	uint64_t m_ullCoinDay;									    //!< coin day

 	IMPLEMENT_SERIALIZE
 	(
 		READWRITE(m_cKeyID);
 		READWRITE(VARINT(m_ullValues));
 		READWRITE(VARINT(m_nHeight));
 		READWRITE(VARINT(m_ullCoinDay));
 	)
};

inline unsigned int GetSerializeSize(const std::shared_ptr<CBaseTransaction> &pa, int nType, int nVersion) {
	return pa->GetSerializeSize(nType, nVersion) + 1;
}

template<typename Stream>
void Serialize(Stream& os, const std::shared_ptr<CBaseTransaction> &pa, int nType, int nVersion) {
	unsigned char ntxType = pa->m_chTxType;
	Serialize(os, ntxType, nType, nVersion);
	if (pa->m_chTxType == EM_REG_ACCT_TX) {
		Serialize(os, *((CRegisterAccountTx *) (pa.get())), nType, nVersion);
	}
	else if (pa->m_chTxType == EM_COMMON_TX) {
		Serialize(os, *((CTransaction *) (pa.get())), nType, nVersion);
	}
	else if (pa->m_chTxType == EM_CONTRACT_TX) {
		Serialize(os, *((CTransaction *) (pa.get())), nType, nVersion);
	}
	else if (pa->m_chTxType == EM_REWARD_TX) {
		Serialize(os, *((CRewardTransaction *) (pa.get())), nType, nVersion);
	}
	else if (pa->m_chTxType == EM_REG_APP_TX) {
		Serialize(os, *((CRegisterAppTx *) (pa.get())), nType, nVersion);
	}
	else {
		throw ios_base::failure("seiralize tx type value error, must be ranger(1...5)");
	}

}

template<typename Stream>
void Unserialize(Stream& is, std::shared_ptr<CBaseTransaction> &pa, int nType, int nVersion) {
	char nTxType;
	is.read((char*) &(nTxType), sizeof(nTxType));
	if (nTxType == EM_REG_ACCT_TX) {
		pa = std::make_shared<CRegisterAccountTx>();
		Unserialize(is, *((CRegisterAccountTx *) (pa.get())), nType, nVersion);
	}
	else if (nTxType == EM_COMMON_TX) {
		pa = std::make_shared<CTransaction>();
		Unserialize(is, *((CTransaction *) (pa.get())), nType, nVersion);
	}
	else if (nTxType == EM_CONTRACT_TX) {
		pa = std::make_shared<CTransaction>();
		Unserialize(is, *((CTransaction *) (pa.get())), nType, nVersion);
	}
	else if (nTxType == EM_REWARD_TX) {
		pa = std::make_shared<CRewardTransaction>();
		Unserialize(is, *((CRewardTransaction *) (pa.get())), nType, nVersion);
	}
	else if (nTxType == EM_REG_APP_TX) {
		pa = std::make_shared<CRegisterAppTx>();
		Unserialize(is, *((CRegisterAppTx *) (pa.get())), nType, nVersion);
	}
	else {
		throw ios_base::failure("unseiralize tx type value error, must be ranger(1...5)");
	}
	pa->m_chTxType = nTxType;
}

#endif
