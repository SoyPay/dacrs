// Copyright (c) 2009-2013 The Dacrs developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef DACRS_CRYPTER_H_
#define DACRS_CRYPTER_H_

#include "allocators.h"
#include "serialize.h"
#include "keystore.h"

class uint256;

const unsigned int g_kWalletCryptoKeySize	 = 32;
const unsigned int g_kWalletCryptoSaltSize 	 = 8;

/*
Private key encryption is done based on a CMasterKey,
which holds a salt and random encryption key.

CMasterKeys are encrypted using AES-256-CBC using a key
derived using derivation method nDerivationMethod
(0 == EVP_sha512()) and derivation iterations nDeriveIterations.
vchOtherDerivationParameters is provided for alternative algorithms
which may require more parameters (such as scrypt).

Wallet Private Keys are then encrypted using AES-256-CBC
with the double-sha256 of the public key as the IV, and the
master key's key as the encryption key (see keystore.[ch]).
*/

/** Master key for wallet encryption */
class CMasterKey {
 public:
	IMPLEMENT_SERIALIZE
	(
			READWRITE(m_vchCryptedKey);
			READWRITE(m_vchSalt);
			READWRITE(m_unDerivationMethod);
			READWRITE(m_unDeriveIterations);
			READWRITE(m_vchOtherDerivationParameters);
	)

	CMasterKey() {
		// 25000 rounds is just under 0.1 seconds on a 1.86 GHz Pentium M
		// ie slightly lower than the lowest hardware we need bother supporting
		m_unDeriveIterations = 25000;
		m_unDerivationMethod = 0;
		m_vchOtherDerivationParameters = vector<unsigned char>(0);
	}

	bool SetNull() {
		m_vchCryptedKey.clear();
		m_vchSalt.clear();
		m_vchOtherDerivationParameters.clear();
		m_unDeriveIterations = 25000;
		m_unDerivationMethod = 0;

		return true;
	}

	vector<unsigned char> m_vchCryptedKey;
	vector<unsigned char> m_vchSalt;
	// 0 = EVP_sha512()
	// 1 = scrypt()
	unsigned int m_unDerivationMethod;
	unsigned int m_unDeriveIterations;
	// Use this for more parameters to key derivation,
	// such as the various parameters to scrypt
	vector<unsigned char> m_vchOtherDerivationParameters;
};

typedef vector<unsigned char, secure_allocator<unsigned char> > CKeyingMaterial;

/** Encryption/decryption context with key information */
class CCrypter {
 public:
	bool SetKeyFromPassphrase(const SecureString &strKeyData, const vector<unsigned char>& vchSalt,
			const unsigned int unRounds, const unsigned int unDerivationMethod);
	bool Encrypt(const CKeyingMaterial& vchPlaintext, vector<unsigned char> &vchCiphertext);
	bool Decrypt(const vector<unsigned char>& vchCiphertext, CKeyingMaterial& vchPlaintext);
	bool SetKey(const CKeyingMaterial& vchNewKey, const vector<unsigned char>& vchNewIV);

	void CleanKey() {
		OPENSSL_cleanse(m_chKey, sizeof(m_chKey));
		OPENSSL_cleanse(m_chIV, sizeof(m_chIV));
		m_bKeySet = false;
	}

	CCrypter() {
		m_bKeySet = false;

		// Try to keep the key data out of swap (and be a bit over-careful to keep the IV that we don't even use out of swap)
		// Note that this does nothing about suspend-to-disk (which will put all our key data on disk)
		// Note as well that at no point in this program is any attempt made to prevent stealing of keys by reading the memory of the running process.
		LockedPageManager::Instance().LockRange(&m_chKey[0], sizeof m_chKey);
		LockedPageManager::Instance().LockRange(&m_chIV[0], sizeof m_chIV);
	}

	~CCrypter() {
		CleanKey();
		LockedPageManager::Instance().UnlockRange(&m_chKey[0], sizeof m_chKey);
		LockedPageManager::Instance().UnlockRange(&m_chIV[0], sizeof m_chIV);
	}

 private:
	unsigned char m_chKey[g_kWalletCryptoKeySize];
	unsigned char m_chIV[g_kWalletCryptoKeySize];
	bool m_bKeySet;
};

bool EncryptSecret(const CKeyingMaterial& vMasterKey, const CKeyingMaterial &vPlaintext, const uint256& cIV,
		vector<unsigned char> &vchCiphertext);
bool DecryptSecret(const CKeyingMaterial& vMasterKey, const vector<unsigned char>& vchCiphertext, const uint256& cIV,
		CKeyingMaterial& vPlaintext);

/** Keystore which keeps the private keys encrypted.
 * It derives from the basic key store, which is used if no encryption is active.
 */
class CCryptoKeyStore: public CBasicKeyStore {
 public:
	CCryptoKeyStore() :
		m_bUseCrypto(false) {
	}

	bool IsCrypted() const {
		return m_bUseCrypto;
	}

	bool IsLocked() const {
		if (!IsCrypted()) {
			return false;
		}
		bool bResult;
		{
			LOCK(cs_KeyStore);
			bResult = m_vMasterKey.empty();
		}
		return bResult;
	}

	bool Lock();

	bool IsEmpty() {
		return m_mapCryptedKeys.empty() && mapKeys.empty();
	}

	virtual bool AddCryptedKey(const CPubKey &cPubKey, const vector<unsigned char> &vchCryptedSecret);
	bool AddKeyCombi(const CKeyID & cKeyId, const CKeyCombi &cKeyCombi);

	bool HaveKey(const CKeyID &cAddress) const {
		{
			LOCK(cs_KeyStore);
			if (!IsCrypted()) {
				return CBasicKeyStore::HaveKey(cAddress);
			}
			return m_mapCryptedKeys.count(cAddress) > 0;
		}
		return false;
	}

	bool GetKey(const CKeyID &cAddress, CKey& ckeyOut, bool bIsMine = false) const;
	bool GetPubKey(const CKeyID &cAddress, CPubKey& cPubKeyOut, bool bIsMine = false) const;

	void GetKeys(set<CKeyID> &setAddress, bool bFlag = false) const {
		if (!IsCrypted()) {
			CBasicKeyStore::GetKeys(setAddress, bFlag);
			return;
		}
		setAddress.clear();
		CryptedKeyMap::const_iterator mi = m_mapCryptedKeys.begin();
		while (mi != m_mapCryptedKeys.end()) {
			if (!bFlag) {
				setAddress.insert((*mi).first);
			} else {
				CKeyCombi keyCombi;
				if (GetKeyCombi((*mi).first, keyCombi)) {
					if (keyCombi.IsContainMinerKey() || keyCombi.IsContainMainKey()) {
						//only return satisfied mining address
						setAddress.insert((*mi).first);
					}
				}
			}
			mi++;
		}
	}

	bool GetKeyCombi(const CKeyID & cAddress, CKeyCombi & cKeyCombiOut) const;
	/* Wallet status (encrypted, locked) changed.
	 * Note: Called without locks held.
	 */
	boost::signals2::signal<void(CCryptoKeyStore* wallet)> NotifyStatusChanged;

 protected:
	bool SetCrypted();
	// will encrypt previously unencrypted keys
	bool EncryptKeys(CKeyingMaterial& vMasterKeyIn);
	bool Unlock(const CKeyingMaterial& vMasterKeyIn);

 private:
	CryptedKeyMap m_mapCryptedKeys;
	CKeyingMaterial m_vMasterKey;
	// if fUseCrypto is true, mainKey in mapKeys must be empty
	// if fUseCrypto is false, vMasterKey must be empty
	bool m_bUseCrypto;
};

#endif
