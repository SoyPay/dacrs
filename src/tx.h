#ifndef TX_H
#define TX_H

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

static const int nTxVersion1 = 1;    //交易初始版本。
static const int nTxVersion2 = 2;    //交易版本升级后，其签名hash和交易hash算法升级为新的算法。

typedef vector<unsigned char> vector_unsigned_char;

#define SCRIPT_ID_SIZE (6)

enum TxType {
	REWARD_TX = 1,    //!< reward tx
	REG_ACCT_TX = 2,  //!< tx that used to register account
	COMMON_TX = 3,    //!< transfer money from one account to another
	CONTRACT_TX = 4,  //!< contract tx
	REG_APP_TX = 5,//!< register app
	NULL_TX,          //!< NULL_TX
};

/**
 * brief:	kinds of fund type
 */
enum FundType {
	FREEDOM = 1,	    //!< FREEDOM
	REWARD_FUND,     	//!< REWARD_FUND
	NULL_FUNDTYPE,   	//!< NULL_FUNDTYPE
};

enum OperType {
	ADD_FREE = 1,  		//!< add money to freedom
	MINUS_FREE, 		//!< minus money from freedom
	NULL_OPERTYPE,		//!< invalid operate type
};

enum AccountOper {
	ADD_FUND = 1, 		//!< add operate
	MINUS_FUND = 2, 	//!< minus operate
	NULL_OPER,			//!< invalid
};

class CNullID {
public:
    friend bool operator==(const CNullID &a, const CNullID &b) { return true; }
    friend bool operator<(const CNullID &a, const CNullID &b) { return true; }
};

typedef boost::variant<CNullID, CRegID, CKeyID, CPubKey> CUserID;
/*CRegID 是地址激活后，分配的账户ID*/
class CRegID {
private:
	uint32_t nHeight;
	uint16_t nIndex;
	mutable vector<unsigned char> vRegID;
	void SetRegIDByCompact(const vector<unsigned char> &vIn);
	void SetRegID(string strRegID);
public:
	friend class CID;
	CRegID(string strRegID);
	CRegID(const vector<unsigned char> &vIn) ;
	CRegID(uint32_t nHeight = 0, uint16_t nIndex = 0);

	const vector<unsigned char> &GetVec6() const {assert(vRegID.size() ==6);return vRegID;}
	void SetRegID(const vector<unsigned char> &vIn) ;
    CKeyID getKeyID(const CAccountViewCache &view)const;
    uint32_t getHight()const { return nHeight;};

	bool operator ==(const CRegID& co) const {
		return (this->nHeight == co.nHeight && this->nIndex == co.nIndex);
	}
	bool operator !=(const CRegID& co) const {
		return (this->nHeight != co.nHeight || this->nIndex != co.nIndex);
	}
	static bool IsSimpleRegIdStr(const string & str);
	static bool IsRegIdStr(const string & str);
	static bool GetKeyID(const string & str,CKeyID &keyId);

    bool IsEmpty()const{return (nHeight == 0 && nIndex == 0);};

    bool clean();

	string ToString() const;

	IMPLEMENT_SERIALIZE
	(
		READWRITE(VARINT(nHeight));
		READWRITE(VARINT(nIndex));
		if(fRead) {
			vRegID.clear();
			vRegID.insert(vRegID.end(), BEGIN(nHeight), END(nHeight));
			vRegID.insert(vRegID.end(), BEGIN(nIndex), END(nIndex));
		}
	)

};
/*CID是一个vector 存放CRegID,CKeyID,CPubKey*/
class CID {
private:
	vector_unsigned_char vchData;
public:
	const vector_unsigned_char &GetID() {
		return vchData;
	}
	static const vector_unsigned_char & UserIDToVector(const CUserID &userid)
	{
		return CID(userid).GetID();
	}
	bool Set(const CRegID &id);
    bool Set(const CKeyID &id);
    bool Set(const CPubKey &id);
    bool Set(const CNullID &id);
    bool Set(const CUserID &userid);
    CID() {}
    CID(const CUserID &dest) {Set(dest);}
    CUserID GetUserId();
    IMPLEMENT_SERIALIZE
	(
		READWRITE(vchData);
	)
};

class CIDVisitor: public boost::static_visitor<bool> {
private:
	CID *pId;
public:
	CIDVisitor(CID *pIdIn) :
		pId(pIdIn) {
	}
	bool operator()(const CRegID &id) const {
			return pId->Set(id);
	}
	bool operator()(const CKeyID &id) const {
		return pId->Set(id);
	}
	bool operator()(const CPubKey &id) const {
		return pId->Set(id);
	}
	bool operator()(const CNullID &no) const {
		return true;
	}
};

class CBaseTransaction {
protected:
	static string txTypeArray[6];
public:
	static uint64_t nMinTxFee;
	static int64_t nMinRelayTxFee;
	static const int CURRENT_VERSION = nTxVersion2;

	unsigned char nTxType;
	int nVersion;
	int nValidHeight;
	uint64_t nRunStep;
	int nFuelRate;      //only in memory
public:

	CBaseTransaction(const CBaseTransaction &other) {
		*this = other;
	}

	CBaseTransaction(int _nVersion, unsigned char _nTxType) :
			nTxType(_nTxType), nVersion(_nVersion), nValidHeight(0), nRunStep(0), nFuelRate(0){
	}

	CBaseTransaction() :
			nTxType(COMMON_TX), nVersion(CURRENT_VERSION), nValidHeight(0), nRunStep(0), nFuelRate(0){
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
		return (nTxType == REWARD_TX);
	}

	virtual bool ExecuteTx(int nIndex, CAccountViewCache &view, CValidationState &state, CTxUndo &txundo,
			int nHeight, CTransactionDBCache &txCache, CScriptDBViewCache &scriptDB) = 0;

	virtual bool UndoExecuteTx(int nIndex, CAccountViewCache &view, CValidationState &state, CTxUndo &txundo,
			int nHeight, CTransactionDBCache &txCache, CScriptDBViewCache &scriptDB);

	virtual bool CheckTransction(CValidationState &state, CAccountViewCache &view, CScriptDBViewCache &scriptDB) = 0;

	virtual uint64_t GetFuel(int nfuelRate);

	int GetFuelRate(CScriptDBViewCache &scriptDB);
};

class CRegisterAccountTx: public CBaseTransaction {

public:
	mutable CUserID userId;      //pubkey
	mutable CUserID minerId;     //Miner pubkey
	int64_t llFees;
	vector<unsigned char> signature;

public:
	CRegisterAccountTx(const CBaseTransaction *pBaseTx) {
		assert(REG_ACCT_TX == pBaseTx->nTxType);
		*this = *(CRegisterAccountTx *) pBaseTx;
	}
	CRegisterAccountTx(const CUserID &uId,const CUserID &minerID,int64_t fees,int height) {
		nTxType = REG_ACCT_TX;
		llFees = fees;
		nValidHeight = height;
		userId = uId;
		minerId=minerID;
		signature.clear();
	}
	CRegisterAccountTx() {
		nTxType = REG_ACCT_TX;
		llFees = 0;
		nValidHeight = 0;
	}

	~CRegisterAccountTx() {
	}

	IMPLEMENT_SERIALIZE
	(
		READWRITE(VARINT(this->nVersion));
		nVersion = this->nVersion;
		READWRITE(VARINT(nValidHeight));
		CID id(userId);
		READWRITE(id);
		CID mMinerid(minerId);
		READWRITE(mMinerid);
		if(fRead) {
			userId = id.GetUserId();
			minerId = mMinerid.GetUserId();
		}
		READWRITE(VARINT(llFees));
		READWRITE(signature);
	)

	uint64_t GetFee() const {
		return llFees;
	}

	uint256 GetHash() const;

	double GetPriority() const {
		return llFees / GetSerializeSize(SER_NETWORK, PROTOCOL_VERSION);
	}

	uint256 SignatureHash() const;

	std::shared_ptr<CBaseTransaction> GetNewInstance() {
		return make_shared<CRegisterAccountTx>(this);
	}

	bool GetAddress(set<CKeyID> &vAddr, CAccountViewCache &view, CScriptDBViewCache &scriptDB);

	string ToString(CAccountViewCache &view) const;

	Object ToJSON(const CAccountViewCache &AccountView) const;

	bool ExecuteTx(int nIndex, CAccountViewCache &view, CValidationState &state, CTxUndo &txundo, int nHeight,
			CTransactionDBCache &txCache, CScriptDBViewCache &scriptDB);

	bool UndoExecuteTx(int nIndex, CAccountViewCache &view, CValidationState &state, CTxUndo &txundo, int nHeight,
			CTransactionDBCache &txCache, CScriptDBViewCache &scriptDB);

	bool CheckTransction(CValidationState &state, CAccountViewCache &view, CScriptDBViewCache &scriptDB);
};

class CTransaction : public CBaseTransaction {
public:
	mutable CUserID srcRegId;                   //src regid
	mutable CUserID desUserId;                  //user regid or user key id or app regid
	uint64_t llFees;
	uint64_t llValues;                          //transfer amount
	vector_unsigned_char vContract;
	vector_unsigned_char signature;

public:
	CTransaction(const CBaseTransaction *pBaseTx) {
		assert(CONTRACT_TX == pBaseTx->nTxType || COMMON_TX == pBaseTx->nTxType);
		*this = *(CTransaction *) pBaseTx;
	}
	CTransaction(const CUserID& in_UserRegId, CUserID in_desUserId, uint64_t Fee, uint64_t Value, int high, vector_unsigned_char& pContract)
	{
		if (in_UserRegId.type() == typeid(CRegID)) {
			assert(!boost::get<CRegID>(in_UserRegId).IsEmpty());
		}
		if (in_desUserId.type() == typeid(CRegID)) {
			assert(!boost::get<CRegID>(in_desUserId).IsEmpty());
		}
		nTxType = CONTRACT_TX;
		srcRegId = in_UserRegId;
		desUserId = in_desUserId;
		vContract = pContract;
		nValidHeight = high;
		llFees = Fee;
		llValues = Value;
		signature.clear();
	}
	CTransaction(const CUserID& in_UserRegId, CUserID in_desUserId, uint64_t Fee, uint64_t Value, int high)
	{
		nTxType = COMMON_TX;
		if (in_UserRegId.type() == typeid(CRegID)) {
			assert(!boost::get<CRegID>(in_UserRegId).IsEmpty());
		}
		if (in_desUserId.type() == typeid(CRegID)) {
			assert(!boost::get<CRegID>(in_desUserId).IsEmpty());
		}
		srcRegId = in_UserRegId;
		desUserId = in_desUserId;
		nValidHeight = high;
		llFees = Fee;
		llValues = Value;
		signature.clear();
	}
	CTransaction() {
		nTxType = COMMON_TX;
		llFees = 0;
		vContract.clear();
		nValidHeight = 0;
		llValues = 0;
		signature.clear();
	}

	~CTransaction() {

	}

	IMPLEMENT_SERIALIZE
	(
			READWRITE(VARINT(this->nVersion));
			nVersion = this->nVersion;
			READWRITE(VARINT(nValidHeight));
			CID srcId(srcRegId);
			READWRITE(srcId);
			CID desId(desUserId);
			READWRITE(desId);
			READWRITE(VARINT(llFees));
			READWRITE(VARINT(llValues));
			READWRITE(vContract);
			READWRITE(signature);
			if(fRead) {
				srcRegId = srcId.GetUserId();
				desUserId = desId.GetUserId();
			}
	)

	uint256 GetHash() const;

	uint64_t GetFee() const {
		return llFees;
	}

	double GetPriority() const {
		return llFees / GetSerializeSize(SER_NETWORK, PROTOCOL_VERSION);
	}

	uint256 SignatureHash() const;

	std::shared_ptr<CBaseTransaction> GetNewInstance() {
		return make_shared<CTransaction>(this);
	}

	string ToString(CAccountViewCache &view) const;

	Object ToJSON(const CAccountViewCache &AccountView) const;

	bool GetAddress(set<CKeyID> &vAddr, CAccountViewCache &view, CScriptDBViewCache &scriptDB);

	const vector_unsigned_char& GetvContract() {
		return vContract;
	}

	bool ExecuteTx(int nIndex, CAccountViewCache &view, CValidationState &state, CTxUndo &txundo, int nHeight,
			CTransactionDBCache &txCache, CScriptDBViewCache &scriptDB);

	bool CheckTransction(CValidationState &state, CAccountViewCache &view, CScriptDBViewCache &scriptDB);

};

class CRewardTransaction: public CBaseTransaction {

public:
	mutable CUserID account;   // in genesis block are pubkey, otherwise are account id
	uint64_t rewardValue;
	int nHeight;
public:
	CRewardTransaction(const CBaseTransaction *pBaseTx) {
		assert(REWARD_TX == pBaseTx->nTxType);
		*this = *(CRewardTransaction*) pBaseTx;
	}

	CRewardTransaction(const vector_unsigned_char &accountIn, const uint64_t rewardValueIn, const int _nHeight) {
		nTxType = REWARD_TX;
		if (accountIn.size() > 6) {
			account = CPubKey(accountIn);
		} else {
			account = CRegID(accountIn);
		}
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
		READWRITE(VARINT(this->nVersion));
		nVersion = this->nVersion;
		CID acctId(account);
		READWRITE(acctId);
		if(fRead) {
			account = acctId.GetUserId();
		}
		READWRITE(VARINT(rewardValue));
		READWRITE(VARINT(nHeight));
	)

	uint256 GetHash() const;

	std::shared_ptr<CBaseTransaction> GetNewInstance() {
		return make_shared<CRewardTransaction>(this);
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
};

class CRegisterAppTx: public CBaseTransaction {

public:
	mutable CUserID regAcctId;         //regid
	vector_unsigned_char script;          //script content
	uint64_t llFees;
	vector_unsigned_char signature;
public:
	CRegisterAppTx(const CBaseTransaction *pBaseTx) {
		assert(REG_APP_TX == pBaseTx->nTxType);
		*this = *(CRegisterAppTx*) pBaseTx;
	}

	CRegisterAppTx() {
		nTxType = REG_APP_TX;
		llFees = 0;
		nValidHeight = 0;
	}

	~CRegisterAppTx() {
	}

	IMPLEMENT_SERIALIZE
	(
		READWRITE(VARINT(this->nVersion));
		nVersion = this->nVersion;
		READWRITE(VARINT(nValidHeight));
		CID regId(regAcctId);
		READWRITE(regId);
		if(fRead) {
			regAcctId = regId.GetUserId();
		}
		READWRITE(script);
		READWRITE(VARINT(llFees));
		READWRITE(signature);
	)

	uint256 GetHash() const;

	std::shared_ptr<CBaseTransaction> GetNewInstance() {
		return make_shared<CRegisterAppTx>(this);
	}

	uint256 SignatureHash() const;

	uint64_t GetFee() const {
		return llFees;
	}

	double GetPriority() const {
		return llFees / GetSerializeSize(SER_NETWORK, PROTOCOL_VERSION);
	}

	string ToString(CAccountViewCache &view) const;

	Object ToJSON(const CAccountViewCache &AccountView) const;

	bool GetAddress(set<CKeyID> &vAddr, CAccountViewCache &view, CScriptDBViewCache &scriptDB);

	bool ExecuteTx(int nIndex, CAccountViewCache &view, CValidationState &state, CTxUndo &txundo, int nHeight,
			CTransactionDBCache &txCache, CScriptDBViewCache &scriptDB);

	bool UndoExecuteTx(int nIndex, CAccountViewCache &view, CValidationState &state, CTxUndo &txundo, int nHeight,
			CTransactionDBCache &txCache, CScriptDBViewCache &scriptDB);

	bool CheckTransction(CValidationState &state, CAccountViewCache &view, CScriptDBViewCache &scriptDB);
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
//		READWRITE(VARINT(nHeight));
//	)
//
//	string ToString() const;
//};

class CScriptDBOperLog {
public:
	vector<unsigned char> vKey;
	vector<unsigned char> vValue;

	CScriptDBOperLog (const vector<unsigned char> &vKeyIn, const vector<unsigned char> &vValueIn) {
		vKey = vKeyIn;
		vValue = vValueIn;
	}

	CScriptDBOperLog() {
		vKey.clear();
		vValue.clear();
	}

	IMPLEMENT_SERIALIZE
	(
		READWRITE(vKey);
		READWRITE(vValue);
	)

	string ToString() const {
		string str("");
		str += "vKey:";
		str += HexStr(vKey);
		str += "\n";
		str +="vValue:";
		str += HexStr(vValue);
		str += "\n";
		return str;
	}

	friend bool operator<(const CScriptDBOperLog &log1, const CScriptDBOperLog &log2) {
		return log1.vKey < log2.vKey;
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
	uint256 txHash;
	vector<CAccountLog> vAccountLog;
	vector<CScriptDBOperLog> vScriptOperLog;
	IMPLEMENT_SERIALIZE
	(
		READWRITE(txHash);
		READWRITE(vAccountLog);
		READWRITE(vScriptOperLog);
	)

public:
	bool GetAccountOperLog(const CKeyID &keyId, CAccountLog &accountLog);
	void Clear() {
		txHash = uint256();
		vAccountLog.clear();
		vScriptOperLog.clear();
	}
	string ToString() const;
};

class CAccount {
public:
	CRegID regID;
	CKeyID keyID;											//!< keyID of the account
	CPubKey PublicKey;										//!< public key of the account
	CPubKey MinerPKey;									    //!< public key of the account for miner
	uint64_t llValues;										//!< total money
	int nHeight;                                            //!< update height
	uint64_t nCoinDay;									    //!< coin day
public :
	/**
	 * @brief operate account
	 * @param type:	operate type
	 * @param fund
	 * @param nHeight:	the height that block connected into chain
	 * @param pscriptID
	 * @param bCheckAuthorized
	 * @return if operate successfully return ture,otherwise return false
	 */
	bool OperateAccount(OperType type, const uint64_t &values,const int nCurHeight);

	bool UndoOperateAccount(const CAccountLog & accountLog);
public:
	CAccount(CKeyID &keyId, CPubKey &pubKey) :
			keyID(keyId), PublicKey(pubKey) {
		llValues = 0;
		MinerPKey =  CPubKey();
		nHeight = 0;
		regID.clean();
		nCoinDay = 0;
	}
	CAccount() :keyID(uint160()), llValues(0) {
		PublicKey = CPubKey();
		MinerPKey =  CPubKey();
		nHeight = 0;
		regID.clean();
		nCoinDay = 0;
	}
	CAccount(const CAccount & other) {
		this->regID = other.regID;
		this->keyID = other.keyID;
		this->PublicKey = other.PublicKey;
		this->MinerPKey = other.MinerPKey;
		this->llValues = other.llValues;
		this->nHeight = other.nHeight;
		this->nCoinDay = other.nCoinDay;
	}
	CAccount &operator=(const CAccount & other) {
		if(this == &other)
			return *this;
		this->regID = other.regID;
		this->keyID = other.keyID;
		this->PublicKey = other.PublicKey;
		this->MinerPKey = other.MinerPKey;
		this->llValues = other.llValues;
		this->nHeight = other.nHeight;
		this->nCoinDay = other.nCoinDay;
		return *this;
	}
	std::shared_ptr<CAccount> GetNewInstance() const{
		return make_shared<CAccount>(*this);
	}

	bool IsMiner(int nCurHeight) {
		if(nCurHeight < 2*SysCfg().GetIntervalPos())
			return true;
		return nCoinDay >= llValues * SysCfg().GetIntervalPos();

	}
	bool IsRegister() const {
		return (PublicKey.IsFullyValid() && PublicKey.GetKeyID() == keyID);
	}
	bool SetRegId(const CRegID &regID){this->regID = regID;return true;};
	bool GetRegId(CRegID &regID)const {regID = this->regID;return !regID.IsEmpty();};
	uint64_t GetRawBalance();
	void ClearAccPos(int nCurHeight);
	uint64_t GetAccountPos(int prevBlockHeight);
	string ToString() const;
	Object ToJosnObj() const;
	bool IsEmptyValue() const {
		return !(llValues > 0);
	}
	bool IsBlackAccount() const;
	uint256 GetHash(){
		CHashWriter ss(SER_GETHASH, 0);
		ss << regID << keyID << PublicKey << MinerPKey << VARINT(llValues)
		   << VARINT(nHeight) << VARINT(nCoinDay);
		return ss.GetHash();
	}
	uint64_t GetMaxCoinDay(int nCurHeight) {
		return llValues * SysCfg().GetMaxDay();
	}

	bool UpDateCoinDay(int nCurHeight);
	IMPLEMENT_SERIALIZE
	(
		READWRITE(regID);
		READWRITE(keyID);
		READWRITE(PublicKey);
		READWRITE(MinerPKey);
		READWRITE(VARINT(llValues));
		READWRITE(VARINT(nHeight));
		READWRITE(VARINT(nCoinDay));
	)

private:
	bool IsMoneyOverflow(uint64_t nAddMoney);
};


class CAccountLog {
public:
	CKeyID keyID;
	uint64_t llValues;										//!< freedom money which coinage greater than 30 days
	int nHeight;                                            //!< update height
	uint64_t nCoinDay;									    //!< coin day

	IMPLEMENT_SERIALIZE
	(
		READWRITE(keyID);
		READWRITE(VARINT(llValues));
		READWRITE(VARINT(nHeight));
		READWRITE(VARINT(nCoinDay));
	)
public:
	CAccountLog(const CAccount &acct) {
		keyID = acct.keyID;
		llValues = acct.llValues;
		nHeight = acct.nHeight;
		nCoinDay = acct.nCoinDay;
	}
	CAccountLog(CKeyID &keyId) {
		keyID = keyId;
		llValues = 0;
		nHeight = 0;
		nCoinDay = 0;
	}
	CAccountLog() {
		keyID = uint160();
		llValues = 0;
		nHeight = 0;
		nCoinDay = 0;
	}
	void SetValue(const CAccount &acct) {
		keyID = acct.keyID;
		llValues = acct.llValues;
		nHeight = acct.nHeight;
		nCoinDay = acct.nCoinDay;
	}
	string ToString() const;
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
	else if (pa->nTxType == COMMON_TX) {
		Serialize(os, *((CTransaction *) (pa.get())), nType, nVersion);
	}
	else if (pa->nTxType == CONTRACT_TX) {
		Serialize(os, *((CTransaction *) (pa.get())), nType, nVersion);
	}
	else if (pa->nTxType == REWARD_TX) {
		Serialize(os, *((CRewardTransaction *) (pa.get())), nType, nVersion);
	}
	else if (pa->nTxType == REG_APP_TX) {
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
	if (nTxType == REG_ACCT_TX) {
		pa = make_shared<CRegisterAccountTx>();
		Unserialize(is, *((CRegisterAccountTx *) (pa.get())), nType, nVersion);
	}
	else if (nTxType == COMMON_TX) {
		pa = make_shared<CTransaction>();
		Unserialize(is, *((CTransaction *) (pa.get())), nType, nVersion);
	}
	else if (nTxType == CONTRACT_TX) {
		pa = make_shared<CTransaction>();
		Unserialize(is, *((CTransaction *) (pa.get())), nType, nVersion);
	}
	else if (nTxType == REWARD_TX) {
		pa = make_shared<CRewardTransaction>();
		Unserialize(is, *((CRewardTransaction *) (pa.get())), nType, nVersion);
	}
	else if (nTxType == REG_APP_TX) {
		pa = make_shared<CRegisterAppTx>();
		Unserialize(is, *((CRegisterAppTx *) (pa.get())), nType, nVersion);
	}
	else {
		throw ios_base::failure("unseiralize tx type value error, must be ranger(1...5)");
	}
	pa->nTxType = nTxType;
}




#endif
