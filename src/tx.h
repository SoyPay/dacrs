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
#include "tx.h"
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
//class CTransactionDBCache;
class CScriptDBViewCache;
class CRegID;
class CID;
typedef vector<unsigned char> vector_unsigned_char;

class CNullID {
public:
    friend bool operator==(const CNullID &a, const CNullID &b) { return true; }
    friend bool operator<(const CNullID &a, const CNullID &b) { return true; }
};

typedef boost::variant<CNullID, CRegID, CKeyID, CPubKey> CUserID;

class CRegID {
private:
	uint32_t nHeight;
	uint16_t nIndex;
	mutable vector<unsigned char> vRegID;
	void SetRegIDByCompact(const vector<unsigned char> &vIn);
public:
	friend class CID;
	const vector<unsigned char> &GetVec6() const {assert(vRegID.size() ==6);return vRegID;}
	void SetRegID(const vector<unsigned char> &vIn) ;
	void SetRegID(string strRegID);
    CKeyID getKeyID(const CAccountViewCache &view)const;
    uint32_t getHight()const { return nHeight;};
	CRegID(string strRegID);
	bool operator ==(const CRegID& co) const {
		return (this->nHeight == co.nHeight && this->nIndex == co.nIndex);
	}
	bool operator !=(const CRegID& co) const {
		return (this->nHeight != co.nHeight || this->nIndex != co.nIndex);
	}
	static bool IsSimpleRegIdStr(const string & str);
	static bool IsRegIdStr(const string & str);
	static bool GetKeyID(const string & str,CKeyID &keyId);
	CRegID(const vector<unsigned char> &vIn) ;
    bool IsEmpty()const{return (nHeight == 0 && nIndex == 0);};
	CRegID(uint32_t nHeight = 0, uint16_t nIndex = 0);
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


class CID {
private:
	vector_unsigned_char vchData;
public:
	const vector_unsigned_char &GetID() {
		return vchData;
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




enum TxType {
	REWARD_TX = 1,    //!< reward tx
	REG_ACCT_TX = 2,  //!< tx that used to register account
	COMMON_TX = 3,    //!< transfer money from one account to another
	CONTRACT_TX = 4,  //!< contract tx
	FREEZE_TX = 5,    //!< freeze tx
	REG_SCRIPT_TX = 6,//!< register script or modify authorization
	NULL_TX,          //!< NULL_TX
};

enum RegScriptType {
	SCRIPT_ID = 0,     //!< SCRIPT_ID
	SCRIPT_CONTENT = 1,//!< SCRIPT_CONTENT
	NULL_TYPE,         //!< NULL_TYPE
};

class CNetAuthorizate {
public:
	CNetAuthorizate()
	{
		nAuthorizeTime = 0;
		nUserDefine.clear();
		nMaxMoneyPerTime = 0;
		nMaxMoneyTotal = 0;
		nMaxMoneyPerDay = 0;
	}

	CNetAuthorizate(uint32_t nauthorizetime, vector<unsigned char> nuserdefine, uint64_t nmaxmoneypertime, uint64_t nmaxmoneytotal,
			uint64_t nmaxmoneyperday);

	uint32_t GetAuthorizeTime() const {
		return nAuthorizeTime;
	}
	const vector<unsigned char> &GetUserData() const {
			return nUserDefine;
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
	void SetUserData(const vector<unsigned char> &data) {
		nUserDefine = data;
		}
	void SetMaxMoneyTotal(uint64_t nMoney) {
		nMaxMoneyTotal = nMoney;
	}
	void SetMaxMoneyPerDay(uint64_t nMoney) {
		nMaxMoneyPerDay = nMoney;
	}

	bool IsValid() {
		if(nUserDefine.size() > 128) {
			return false;
		}
		return true;
	}

	bool IsNull() {
		return (nAuthorizeTime == 0);
	}

	IMPLEMENT_SERIALIZE
	(
		READWRITE(VARINT(nAuthorizeTime));
		READWRITE(nUserDefine);
		READWRITE(VARINT(nMaxMoneyPerTime));
		READWRITE(VARINT(nMaxMoneyTotal));
		READWRITE(VARINT(nMaxMoneyPerDay));
	)
	Object ToJosnObj() const;
	string ToString(bool bFlag) const;
protected:
	uint32_t nAuthorizeTime;
	vector<unsigned char> nUserDefine;
	uint64_t nMaxMoneyPerTime;
	uint64_t nMaxMoneyTotal;
	uint64_t nMaxMoneyPerDay;
};

class CAuthorizate :public CNetAuthorizate{
public:
	CAuthorizate(CNetAuthorizate te) ;
	CAuthorizate() {
		nLastOperHeight = 0;
		nCurMaxMoneyPerDay = 0;
	}

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
	unsigned int GetSerializeSize(int nType, int nVersion) const;

	template<typename Stream>
	void Serialize(Stream& s, int nType, int nVersion) const {
		CSerActionSerialize ser_action;
		unsigned int nSerSize = 0;
		vector<unsigned char> vData;
		vData.clear();
		if (nAuthorizeTime > 0) {
			CDataStream ds(SER_DISK, CLIENT_VERSION);
			ds << VARINT(nAuthorizeTime);
			ds << nUserDefine;
			ds << VARINT(nMaxMoneyPerTime);
			ds << VARINT(nMaxMoneyTotal);
			ds << VARINT(nMaxMoneyPerDay);
			ds << VARINT(nLastOperHeight);
			ds << VARINT(nCurMaxMoneyPerDay);
			vData.insert(vData.end(), ds.begin(), ds.end());
		}
		{
			(nSerSize += ::SerReadWrite(s, (vData), nType, nVersion, ser_action));
		}
	}
	template<typename Stream>
	void Unserialize(Stream& s, int nType, int nVersion) {
		CSerActionUnserialize ser_action;
		unsigned int nSerSize = 0;
		vector<unsigned char> vData;
		vData.clear();
		{
			(nSerSize += ::SerReadWrite(s, (vData), nType, nVersion, ser_action));
		}
		if (!vData.empty()) {
			CDataStream ds(vData, SER_DISK, CLIENT_VERSION);
			ds >> VARINT(nAuthorizeTime);
			ds >> nUserDefine;
			ds >> VARINT(nMaxMoneyPerTime);
			ds >> VARINT(nMaxMoneyTotal);
			ds >> VARINT(nMaxMoneyPerDay);
			ds >> VARINT(nLastOperHeight);
			ds >> VARINT(nCurMaxMoneyPerDay);
		}
	}
	Object ToJosnObj() const;
	string ToString(bool bFlag) const;

private:
	uint32_t nLastOperHeight;
	uint64_t nCurMaxMoneyPerDay;
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
			nVersion(CURRENT_VERSION), nTxType(COMMON_TX) {
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

	virtual bool GetAddress(std::set<CKeyID> &vAddr, CAccountViewCache &view) = 0;

	virtual bool IsValidHeight(int nCurHeight, int nTxCacheHeight) const = 0;

	bool IsCoinBase() {
		return (nTxType == REWARD_TX);
	}

	virtual bool UpdateAccount(int nIndex, CAccountViewCache &view, CValidationState &state, CTxUndo &txundo,
			int nHeight, CTransactionDBCache &txCache, CScriptDBViewCache &scriptCache) = 0;

	virtual bool UndoUpdateAccount(int nIndex, CAccountViewCache &view, CValidationState &state, CTxUndo &txundo,
			int nHeight, CTransactionDBCache &txCache, CScriptDBViewCache &scriptCache) = 0;

	virtual bool CheckTransction(CValidationState &state, CAccountViewCache &view) = 0;

};

class CRegisterAccountTx: public CBaseTransaction {

public:
	mutable CUserID userId;      //pubkey
	mutable CUserID minerId;     //Miner pubkey
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
		llFees = 0;
		nValidHeight = 0;
	}

	~CRegisterAccountTx() {
	}

//	unsigned int GetSerializeSize(int nType, int nVersion) const {
//		CSerActionGetSerializeSize ser_action;
//		const bool fGetSize = true;
//		const bool fWrite = false;
//		const bool fRead = false;
//		unsigned int nSerSize = 0;
//		ser_streamplaceholder s;
//		s.nType = nType;
//		s.nVersion = nVersion;
//		{
//			(nSerSize += ::SerReadWrite(s, (REF(WrapVarInt(REF(this->nVersion)))), nType, nVersion, ser_action));
//			nVersion = this->nVersion;
//			CID id(userId);
//			(nSerSize += ::SerReadWrite(s, (id), nType, nVersion, ser_action));
//			CID mMinerid(minerId);
//			(nSerSize += ::SerReadWrite(s, (mMinerid), nType, nVersion, ser_action));
//			if (fRead) {
//				userId = id.GetUserId();
//				minerId = mMinerid.GetUserId();
//			}
//			(nSerSize += ::SerReadWrite(s, (REF(WrapVarInt(REF(llFees)))), nType, nVersion, ser_action));
//			(nSerSize += ::SerReadWrite(s, (REF(WrapVarInt(REF(nValidHeight)))), nType, nVersion, ser_action));
//			(nSerSize += ::SerReadWrite(s, (signature), nType, nVersion, ser_action));
//		}
//		return nSerSize;
//	}
//	template<typename Stream>
//	void Serialize(Stream& s, int nType, int nVersion) const {
//		CSerActionSerialize ser_action;
//		const bool fGetSize = false;
//		const bool fWrite = true;
//		const bool fRead = false;
//		unsigned int nSerSize = 0;
//		{
//			(nSerSize += ::SerReadWrite(s, (REF(WrapVarInt(REF(this->nVersion)))), nType, nVersion, ser_action));
//			nVersion = this->nVersion;
//			CID id(userId);
//			(nSerSize += ::SerReadWrite(s, (id), nType, nVersion, ser_action));
//			CID mMinerid(minerId);
//			(nSerSize += ::SerReadWrite(s, (mMinerid), nType, nVersion, ser_action));
//			if (fRead) {
//				userId = id.GetUserId();
//				minerId = mMinerid.GetUserId();
//			}
//			(nSerSize += ::SerReadWrite(s, (REF(WrapVarInt(REF(llFees)))), nType, nVersion, ser_action));
//			(nSerSize += ::SerReadWrite(s, (REF(WrapVarInt(REF(nValidHeight)))), nType, nVersion, ser_action));
//			(nSerSize += ::SerReadWrite(s, (signature), nType, nVersion, ser_action));
//		}
//	}
//	template<typename Stream>
//	void Unserialize(Stream& s, int nType, int nVersion) {
//		CSerActionUnserialize ser_action;
//		const bool fGetSize = false;
//		const bool fWrite = false;
//		const bool fRead = true;
//		unsigned int nSerSize = 0;
//		{
//			(nSerSize += ::SerReadWrite(s, (REF(WrapVarInt(REF(this->nVersion)))), nType, nVersion, ser_action));
//			nVersion = this->nVersion;
//			CID id(userId);
//			(nSerSize += ::SerReadWrite(s, (id), nType, nVersion, ser_action));
//			CID mMinerid(minerId);
//			(nSerSize += ::SerReadWrite(s, (mMinerid), nType, nVersion, ser_action));
//			if (fRead) {
//				userId = id.GetUserId();
//				minerId = mMinerid.GetUserId();
//			}
//			(nSerSize += ::SerReadWrite(s, (REF(WrapVarInt(REF(llFees)))), nType, nVersion, ser_action));
//			(nSerSize += ::SerReadWrite(s, (REF(WrapVarInt(REF(nValidHeight)))), nType, nVersion, ser_action));
//			(nSerSize += ::SerReadWrite(s, (signature), nType, nVersion, ser_action));
//		}
//	}
	IMPLEMENT_SERIALIZE
	(
		READWRITE(VARINT(this->nVersion));
		nVersion = this->nVersion;
		CID id(userId);
		READWRITE(id);
		CID mMinerid(minerId);
		READWRITE(mMinerid);
		if(fRead) {
			userId = id.GetUserId();
			minerId = mMinerid.GetUserId();
		}
		READWRITE(VARINT(llFees));
		READWRITE(VARINT(nValidHeight));
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
		CID id(userId);
		CID id2(minerId);
		ss << VARINT(nVersion) << nTxType << id << id2 << VARINT(llFees) << VARINT(nValidHeight);
		return ss.GetHash();
	}

	std::shared_ptr<CBaseTransaction> GetNewInstance() {
		return make_shared<CRegisterAccountTx>(this);
	}

	bool GetAddress(set<CKeyID> &vAddr, CAccountViewCache &view);

	bool IsValidHeight(int nCurHeight, int nTxCacheHeight) const;

	string ToString(CAccountViewCache &view) const;

	bool UpdateAccount(int nIndex, CAccountViewCache &view, CValidationState &state, CTxUndo &txundo, int nHeight,
			CTransactionDBCache &txCache, CScriptDBViewCache &scriptCache);

	bool UndoUpdateAccount(int nIndex, CAccountViewCache &view, CValidationState &state, CTxUndo &txundo, int nHeight,
			CTransactionDBCache &txCache, CScriptDBViewCache &scriptCache);

	bool CheckTransction(CValidationState &state, CAccountViewCache &view);
};

class CTransaction: public CBaseTransaction {

public:
	mutable CUserID srcUserId;    //regid
	uint64_t llFees;
	uint64_t llValues;
	mutable CUserID desUserId;    //regid or keyid
	int nValidHeight;
	vector_unsigned_char signature;
public:

	CTransaction(const CBaseTransaction *pBaseTx) {
		assert(COMMON_TX == pBaseTx->nTxType);
		*this = *(CTransaction *) pBaseTx;
	}

	CTransaction() {
		signature.clear();
		llValues = 0;
		llFees = 0;
		nValidHeight = 0;
		nTxType = COMMON_TX;
	}

	~CTransaction() {

	}

	IMPLEMENT_SERIALIZE
	(
			READWRITE(VARINT(this->nVersion));
			nVersion = this->nVersion;
			CID srcId(srcUserId);
			READWRITE(srcId);
			READWRITE(VARINT(llFees));
			READWRITE(VARINT(llValues));
			CID desId(desUserId);
			READWRITE(desId);
			READWRITE(VARINT(nValidHeight));
			READWRITE(signature);
			if(fRead) {
				srcUserId = srcId.GetUserId();
				desUserId = desId.GetUserId();
			}
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
		CID srcId(srcUserId);
		CID desId(desUserId);
		ss << srcId << VARINT(llFees) << VARINT(llValues) << desId << VARINT(nValidHeight);
		return ss.GetHash();
	}

	std::shared_ptr<CBaseTransaction> GetNewInstance() {
		return make_shared<CTransaction>(this);
	}

	bool GetAddress(set<CKeyID> &vAddr, CAccountViewCache &view);

	bool IsValidHeight(int nCurHeight, int nTxCacheHeight) const;

	string ToString(CAccountViewCache &view) const;

	bool UpdateAccount(int nIndex, CAccountViewCache &view, CValidationState &state, CTxUndo &txundo, int nHeight,
			CTransactionDBCache &txCache, CScriptDBViewCache &scriptCache);

	bool UndoUpdateAccount(int nIndex, CAccountViewCache &view, CValidationState &state, CTxUndo &txundo, int nHeight,
			CTransactionDBCache &txCache, CScriptDBViewCache &scriptCache);

	bool CheckTransction(CValidationState &state, CAccountViewCache &view);
};

class CContractTransaction : public CBaseTransaction {
public:
	mutable CUserID scriptRegId;                    //regid
	mutable vector<CUserID> vAccountRegId;
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
			READWRITE(VARINT(this->nVersion));
			nVersion = this->nVersion;
			CID scriptId(scriptRegId);
			READWRITE(scriptId);
			vector<CID> vAcctId;
			if(fRead) {
				scriptRegId = scriptId.GetUserId();
				READWRITE(vAcctId);
				for(auto &acctId : vAcctId) {
					CUserID userId = acctId.GetUserId();
					vAccountRegId.push_back(userId);
				}
			} else {
				for(auto &acctRegId : vAccountRegId) {
					vAcctId.push_back(CID(acctRegId));
				}
				READWRITE(vAcctId);
			}
			READWRITE(VARINT(llFees));
			READWRITE(vContract);
			READWRITE(VARINT(nValidHeight));
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

	uint256 SignatureHash() const;

	std::shared_ptr<CBaseTransaction> GetNewInstance() {
		return make_shared<CContractTransaction>(this);
	}

	string ToString(CAccountViewCache &view) const;

	bool GetAddress(set<CKeyID> &vAddr, CAccountViewCache &view);

	bool IsValidHeight(int nCurHeight, int nTxCacheHeight) const;

	const vector_unsigned_char& GetvContract() {
		return vContract;
	}

	bool UpdateAccount(int nIndex, CAccountViewCache &view, CValidationState &state, CTxUndo &txundo, int nHeight,
			CTransactionDBCache &txCache, CScriptDBViewCache &scriptCache);

	bool UndoUpdateAccount(int nIndex, CAccountViewCache &view, CValidationState &state, CTxUndo &txundo, int nHeight,
			CTransactionDBCache &txCache, CScriptDBViewCache &scriptCache);

	bool CheckTransction(CValidationState &state, CAccountViewCache &view);

};

class CFreezeTransaction: public CBaseTransaction {

public:
	mutable CUserID regAccountId;      //regid
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
		nValidHeight = 0;
		nUnfreezeHeight = 0;
	}

	~CFreezeTransaction() {

	}

	uint256 GetHash() const {
		return SerializeHash(*this);
	}

	IMPLEMENT_SERIALIZE
	(
		    READWRITE(VARINT(this->nVersion));
			nVersion = this->nVersion;
			CID regId(regAccountId);
			READWRITE(regId);
			if(fRead) {
				regAccountId = regId.GetUserId();
			}
			READWRITE(VARINT(llFees));
			READWRITE(VARINT(llFreezeFunds));
			READWRITE(VARINT(nValidHeight));
			READWRITE(VARINT(nUnfreezeHeight));
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
		CID regId(regAccountId);
		ss <<VARINT(nVersion) << nTxType << regId << VARINT(llFees) << VARINT(llFreezeFunds) << VARINT(nValidHeight) << VARINT(nUnfreezeHeight);
		return ss.GetHash();
	}

	string ToString(CAccountViewCache &view) const;

	bool GetAddress(set<CKeyID> &vAddr, CAccountViewCache &view);

	bool IsValidHeight(int nCurHeight, int nTxCacheHeight) const;

	bool UpdateAccount(int nIndex, CAccountViewCache &view, CValidationState &state, CTxUndo &txundo, int nHeight,
			CTransactionDBCache &txCache, CScriptDBViewCache &scriptCache);

	bool UndoUpdateAccount(int nIndex, CAccountViewCache &view, CValidationState &state, CTxUndo &txundo, int nHeight,
			CTransactionDBCache &txCache, CScriptDBViewCache &scriptCache);

	bool CheckTransction(CValidationState &state, CAccountViewCache &view);
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

	uint256 GetHash() const {
		return SerializeHash(*this);
	}

	std::shared_ptr<CBaseTransaction> GetNewInstance() {
		return make_shared<CRewardTransaction>(this);
	}

	uint256 SignatureHash() const {
		CHashWriter ss(SER_GETHASH, 0);
		CID accId(account);
		ss <<VARINT(nVersion) << nTxType<< accId << VARINT(rewardValue) << VARINT(nHeight);
		return ss.GetHash();
	}

	uint64_t GetFee() const {
		return 0;
	}

	double GetPriority() const {
		return 0.0f;
	}

	string ToString(CAccountViewCache &view) const;

	bool GetAddress(set<CKeyID> &vAddr, CAccountViewCache &view);

	bool IsValidHeight(int nCurHeight, int nTxCacheHeight) const {
		return true;
	}

	bool UpdateAccount(int nIndex, CAccountViewCache &view, CValidationState &state, CTxUndo &txundo, int nHeight,
			CTransactionDBCache &txCache, CScriptDBViewCache &scriptCache);

	bool UndoUpdateAccount(int nIndex, CAccountViewCache &view, CValidationState &state, CTxUndo &txundo, int nHeight,
			CTransactionDBCache &txCache, CScriptDBViewCache &scriptCache);

	bool CheckTransction(CValidationState &state, CAccountViewCache &view);
};

#define SCRIPT_ID_SIZE (6)

class CRegistScriptTx: public CBaseTransaction {

public:
	mutable CUserID regAccountId;         //regid
	vector_unsigned_char script;          //regid or new script content
	uint64_t llFees;
	int nValidHeight;
	CNetAuthorizate aAuthorizate;
	vector_unsigned_char signature;
public:
	CRegistScriptTx(const CBaseTransaction *pBaseTx) {
		assert(REG_SCRIPT_TX == pBaseTx->nTxType);
		*this = *(CRegistScriptTx*) pBaseTx;
	}

	CRegistScriptTx() {
		nTxType = REG_SCRIPT_TX;
		llFees = 0;
		nValidHeight = 0;
	}

	~CRegistScriptTx() {
	}

	IMPLEMENT_SERIALIZE
	(
		READWRITE(VARINT(this->nVersion));
		nVersion = this->nVersion;
		CID regAcctId(regAccountId);
		READWRITE(regAcctId);
		if(fRead) {
			regAccountId = regAcctId.GetUserId();
		}
		READWRITE(script);
		READWRITE(VARINT(llFees));
		READWRITE(VARINT(nValidHeight));
		READWRITE(aAuthorizate);
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
		CID regAccId(regAccountId);
		ss << regAccId << script << VARINT(llFees) << VARINT(nValidHeight) << aAuthorizate;
		return ss.GetHash();
	}

	uint64_t GetFee() const {
		return llFees;
	}

	double GetPriority() const {
		return llFees / GetSerializeSize(SER_NETWORK, PROTOCOL_VERSION);
	}

	string ToString(CAccountViewCache &view) const;

	bool GetAddress(set<CKeyID> &vAddr, CAccountViewCache &view);

	bool IsValidHeight(int nCurHeight, int nTxCacheHeight) const;

	bool UpdateAccount(int nIndex, CAccountViewCache &view, CValidationState &state, CTxUndo &txundo, int nHeight,
			CTransactionDBCache &txCache, CScriptDBViewCache &scriptCache);

	bool UndoUpdateAccount(int nIndex, CAccountViewCache &view, CValidationState &state, CTxUndo &txundo, int nHeight,
			CTransactionDBCache &txCache, CScriptDBViewCache &scriptCache);

	bool CheckTransction(CValidationState &state, CAccountViewCache &view);
};

/**
 * brief:	kinds of fund type
 */
enum FundType {
	FREEDOM = 1,	    //!< FREEDOM
	REWARD_FUND,     	//!< REWARD_FUND
	FREEDOM_FUND,    	//!< FREEDOM_FUND
	FREEZD_FUND,     	//!< FREEZD_FUND
	SELF_FREEZD_FUND,	//!< SELF_FREEZD_FUND
	NULL_FUNDTYPE,   	//!< NULL_FUNDTYPE
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
	unsigned char nFundType;		//!< fund type
	vector_unsigned_char scriptID;	//!< hash of the tx which create the fund
	uint64_t value;					//!< amount of money
	int nHeight;					//!< time-out height
public:
	CFund() {
		nFundType = 0;
		value = 0;
		nHeight = 0;
	}
	CFund(uint64_t _value) {
		nFundType = 0;
		value = _value;
		nHeight = 0;
	}
	CFund(unsigned char _type, uint64_t _value, int _Height,const vector_unsigned_char& _scriptID = vector_unsigned_char()) {
		nFundType = _type;
		value = _value;
		nHeight = _Height;
		if (!_scriptID.empty())
			scriptID = _scriptID;
	}
	CFund(const CFund &fund) {
		nFundType = fund.nFundType;
		value = fund.value;
		nHeight = fund.nHeight;
		scriptID = fund.scriptID;
	}
	CFund & operator =(const CFund &fund) {
		if (this == &fund) {
			return *this;
		}
		this->nFundType = fund.nFundType;
		this->value = fund.value;
		this->nHeight = fund.nHeight;
		this->scriptID = fund.scriptID;
		return *this;
	}
	~CFund() {
	}
	Object ToJosnObj() const;
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
		if (fa.scriptID != fb.scriptID)
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
			READWRITE(scriptID);
			READWRITE(value);
			READWRITE(nHeight);

	)

	string ToString() const;
};

enum AccountOper {
	ADD_FUND = 1, 		//!< add operate
	MINUS_FUND = 2, 	//!< minus operate
	NULL_OPER,			//!< invalid
};


class CScriptDBOperLog {
public:
	vector<unsigned char> vKey;
	vector<unsigned char> vValue;

	CScriptDBOperLog (const vector<unsigned char> &vKeyIn, const vector<unsigned char> &vValueIn) {
		vKey = vKeyIn;
		vValue = vValueIn;
	}

	CScriptDBOperLog() {

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


class COperFund {
public:
	unsigned char operType;  		//!<1:ADD_VALUE 2:MINUS_VALUE
	unsigned char bAuthorizated;
	vector<CFund> vFund;

	IMPLEMENT_SERIALIZE
	(
			READWRITE(operType);
			READWRITE(vFund);
	)
public:
	COperFund() {
		operType = NULL_OPER;
		bAuthorizated = false;
		vFund.clear();
	}

	COperFund(unsigned char nType, const vector<CFund>& vOperFund,bool _bAuthorizated = false) {
		operType = nType;
		vFund = vOperFund;
		bAuthorizated = _bAuthorizated;
	}

	COperFund(unsigned char nType, const CFund& fund,bool _bAuthorizated) {
		operType = nType;
		vFund.push_back(fund);
		bAuthorizated = _bAuthorizated;
	}

	string ToString() const;

};

class CAuthorizateLog {
public:
	CAuthorizateLog() {
		bValid = false;
		nLastOperHeight = 0;
		nLastCurMaxMoneyPerDay = 0;
		nLastMaxMoneyTotal = 0;
		scriptID.clear();
	}

	CAuthorizateLog(int nHeight,uint64_t nMoneyPerDay, uint64_t nTotalMoney, bool _bValid,const vector_unsigned_char& _scriptID) {
		bValid = _bValid;
		nLastOperHeight = nHeight;
		nLastCurMaxMoneyPerDay = nMoneyPerDay;
		nLastMaxMoneyTotal = nTotalMoney;
		scriptID = _scriptID;
	}

	int GetLastOperHeight() const{
		return nLastOperHeight;
	}
	uint64_t GetLastCurMaxMoneyPerDay() const {
		return nLastCurMaxMoneyPerDay;
	}
	uint64_t GetLastMaxMoneyTotal() const {
		return nLastMaxMoneyTotal;
	}
	const vector_unsigned_char& GetScriptID() const {
		return scriptID;
	}
	void SetLastOperHeight(int nHeight)  {
		assert(nHeight>=0);
		nLastOperHeight = nHeight;
	}
	void SetScriptID(const vector_unsigned_char& _scriptID) {
		scriptID = _scriptID;
	}
	bool IsLogValid() const{
		return bValid;
	}

	string ToString() const;
	void SetNULL() {
		bValid = false;
		nLastOperHeight = 0;
		nLastCurMaxMoneyPerDay = 0;
		nLastMaxMoneyTotal = 0;
		scriptID.clear();
	}

	unsigned int GetSerializeSize(int nType, int nVersion) const {
		CSerActionGetSerializeSize ser_action;
		unsigned int nSerSize = 0;
		ser_streamplaceholder s;
		s.nType = nType;
		vector<unsigned char> vData;
		vData.clear();
		if(bValid) {
			CDataStream ds(SER_DISK, CLIENT_VERSION);
			ds << bValid;
			ds << VARINT(nLastOperHeight);
			ds << VARINT(nLastCurMaxMoneyPerDay);
			ds << VARINT(nLastMaxMoneyTotal);
			ds << scriptID;
			vData.insert(vData.end(), ds.begin(), ds.end());
		}
		s.nVersion = nVersion;
		{
			(nSerSize += ::SerReadWrite(s, (vData), nType, nVersion, ser_action));
		}
		return nSerSize;
	}
	template<typename Stream>
	void Serialize(Stream& s, int nType, int nVersion) const {
		CSerActionSerialize ser_action;
		unsigned int nSerSize = 0;
		vector<unsigned char> vData;
		vData.clear();
		if(bValid) {
			CDataStream ds(SER_DISK, CLIENT_VERSION);
			ds << bValid;
			ds << VARINT(nLastOperHeight);
			ds << VARINT(nLastCurMaxMoneyPerDay);
			ds << VARINT(nLastMaxMoneyTotal);
			ds << scriptID;
			vData.insert(vData.end(), ds.begin(), ds.end());
		}
		{
			(nSerSize += ::SerReadWrite(s, (bValid), nType, nVersion, ser_action));
		}
	}
	template<typename Stream>
	void Unserialize(Stream& s, int nType, int nVersion) {
		CSerActionUnserialize ser_action;
		unsigned int nSerSize = 0;
		vector<unsigned char> vData;
		vData.clear();
		{
			(nSerSize += ::SerReadWrite(s, (vData), nType, nVersion, ser_action));
		}
		if (!vData.empty()) {
			CDataStream ds(vData, SER_DISK, CLIENT_VERSION);
			ds >> bValid;
			ds >> VARINT(nLastOperHeight);
			ds >> VARINT(nLastCurMaxMoneyPerDay);
			ds >> VARINT(nLastMaxMoneyTotal);
			ds >> scriptID;
		}
	}

private:
	bool bValid;
	int nLastOperHeight;
	uint64_t nLastCurMaxMoneyPerDay;
	uint64_t nLastMaxMoneyTotal;
	vector_unsigned_char scriptID;
};

class CAccountOperLog {
public:

	mutable CKeyID keyID;
	mutable vector<COperFund> vOperFund;
	mutable CAuthorizateLog   authorLog;
	IMPLEMENT_SERIALIZE
	(
		vector<unsigned char> vData;
		if(fWrite || fGetSize) {
			if(keyID == uint160(0)) {
				vData.clear();
			}
			else {
				CDataStream ds(SER_DISK, CLIENT_VERSION);
				ds << keyID;
				ds << vOperFund;
				ds << authorLog;
				vData.insert(vData.end(), ds.begin(), ds.end());
			}
		}
		READWRITE(vData);
		if(fRead) {
			if(!vData.empty()) {
				CDataStream ds(vData, SER_DISK, CLIENT_VERSION);
				ds >> keyID;
				ds >> vOperFund;
				ds >> authorLog;
			}
		}

	)
public:
	void InsertOperateLog(const COperFund& op) {
		vOperFund.push_back(op);
	}

	void InsertAuthorLog(const CAuthorizateLog& log) {
		authorLog = log;
	}

	string ToString() const;

	void SetNULL() {
		keyID = uint160(0);
		vOperFund.clear();
	}

};

class CTxUndo {
public:
	uint256 txHash;
	vector<CAccountOperLog> vAccountOperLog;
	vector<CScriptDBOperLog> vScriptOperLog;
	IMPLEMENT_SERIALIZE
	(
		READWRITE(txHash);
		READWRITE(vAccountOperLog);
		READWRITE(vScriptOperLog);
	)

public:
	bool GetAccountOperLog(const CKeyID &keyId, CAccountOperLog &accountOperLog);
	string ToString() const;
};

class CAccount {
public:
	CRegID regID;
	CKeyID keyID;											//!< keyID of the account
	CPubKey PublicKey;										//!< public key of the account
	CPubKey MinerPKey;									    //!< public key of the account for miner
	uint64_t llValues;										//!< freedom money which coinage greater than 30 days
	vector<CFund> vRewardFund;								//!< reward money
	vector<CFund> vFreedomFund;								//!< freedom money
	vector<CFund> vFreeze;									//!< freezed money
	vector<CFund> vSelfFreeze;								//!< self-freeze money
	map<vector_unsigned_char,CAuthorizate> mapAuthorizate;	//!< Key:scriptID,value :CAuthorizate
	CAccountOperLog accountOperLog;							//!< record operlog, write at undoinfo

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
	bool OperateAccount(OperType type, const CFund &fund, int nHeight = 0,
			const vector_unsigned_char* pscriptID = NULL,bool bCheckAuthorized = false );

	/**
	 * @brief:	test whether  can minus money  from the account by the script
	 * @param nMoney:	the amount of money to minus
	 * @param nHeight:	the height that block connect into the chain
	 * @param scriptID:
	 * @return if we can minus the money then return ture,otherwise return false
	 */
	bool IsAuthorized(uint64_t nMoney,int nHeight,const vector_unsigned_char& scriptID);

	/**
	 * @brief get user defined data in authorizate class by scriptID
	 * @param scriptID
	 * @param vData user defined data
	 * @return true if success,otherwise false
	 */
	bool GetUserData(const vector_unsigned_char& scriptID,vector<unsigned char> & vData);
public:
	CAccount(CKeyID &keyId, CPubKey &pubKey) :
			keyID(keyId), PublicKey(pubKey) {
		llValues = 0;
		MinerPKey =  CPubKey();
		accountOperLog.keyID = keyID;
		vFreedomFund.clear();
		vSelfFreeze.clear();
	}
	CAccount() :
			keyID(uint160(0)), llValues(0) {
		PublicKey = CPubKey();
		MinerPKey =  CPubKey();
		accountOperLog.keyID = keyID;
		vFreedomFund.clear();
		vSelfFreeze.clear();
	}
	std::shared_ptr<CAccount> GetNewInstance() {
		return make_shared<CAccount>(*this);
	}
	uint64_t GetInterest() const {
		/**
		 * @todo  what this   by ranger.shi
		 */
		uint64_t rest = 0;

		return rest;
	}
	bool IsRegister() const {
		return (PublicKey.IsFullyValid() && PublicKey.GetKeyID() == keyID);
	}
	bool SetRegId(const CRegID &regID){this->regID = regID;return true;};
	bool GetRegId(CRegID &regID)const {regID = this->regID;return regID.IsEmpty();};
	uint64_t GetRewardAmount(int nCurHeight);
	uint64_t GetSripteFreezeAmount(int nCurHeight);
	uint64_t GetSelfFreezeAmount(int nCurHeight);
	uint64_t GetRawBalance(int nCurHeight);
	uint256 BuildMerkleTree(int prevBlockHeight) const;
	void ClearAccPos(uint256 hash, int prevBlockHeight, int nIntervalPos);
	uint64_t GetAccountPos(int prevBlockHeight) const;
	string ToString() const;
	Object ToJosnObj() const;
	bool IsEmptyValue() const {
		return !(llValues > 0 || !vFreedomFund.empty() || !vFreeze.empty() || !vSelfFreeze.empty());
	}
	bool CompactAccount(int nCurHeight);
	void AddToFreedom(const CFund &fund,bool bWriteLog = true);
	void AddToFreeze(const CFund &fund,bool bWriteLog = true);
	void AddToSelfFreeze(const CFund &fund,bool bWriteLog = true);

	bool UndoOperateAccount(const CAccountOperLog & accountOperLog);
	bool FindFund(const vector<CFund>& vFund, const vector_unsigned_char &scriptID,CFund&fund);

	IMPLEMENT_SERIALIZE
	(
			READWRITE(regID);
			READWRITE(keyID);
			READWRITE(PublicKey);
			READWRITE(MinerPKey);
			READWRITE(llValues);
			READWRITE(vRewardFund);
			READWRITE(vFreedomFund);
			READWRITE(vFreeze);
			READWRITE(vSelfFreeze);
			READWRITE(mapAuthorizate);
	)

private:
	bool MergerFund(vector<CFund> &vFund, int nCurHeight);
	void WriteOperLog(AccountOper emOperType, const CFund &fund,bool bAuthorizated = false);
	void WriteOperLog(const COperFund &operLog);
	bool IsFundValid(OperType type, const CFund &fund, int nHeight, const vector_unsigned_char* pscriptID = NULL,
			bool bCheckAuthorized = false);

	bool MinusFreezed(const CFund& fund);
	bool MinusFree(const CFund &fund,bool bAuthorizated);
	bool MinusSelf(const CFund &fund,bool bAuthorizated);
	bool IsMoneyOverflow(uint64_t nAddMoney);
	void UpdateAuthority(int nHeight,uint64_t nMoney, const vector_unsigned_char& scriptID);
	void UndoAuthorityOnDay(uint64_t nUndoMoney,const CAuthorizateLog& log);
	void UndoAuthorityOverDay(const CAuthorizateLog& log);
	uint64_t GetVecMoney(const vector<CFund>& vFund);
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
	else if (nTxType == COMMON_TX) {
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
