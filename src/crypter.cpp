// Copyright (c) 2009-2013 The Dacrs developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "crypter.h"

//#include "script.h"
#include "util.h"
#include <string>
#include <vector>
#include <boost/foreach.hpp>
#include <openssl/aes.h>
#include <openssl/evp.h>

bool CCrypter::SetKeyFromPassphrase(const SecureString& strKeyData, const vector<unsigned char>& vchSalt,
		const unsigned int unRounds, const unsigned int unDerivationMethod) {
	if (unRounds < 1 || vchSalt.size() != g_kWalletCryptoSaltSize) {
		return false;
	}
	int i = 0;
	if (unDerivationMethod == 0) {
		i = EVP_BytesToKey(EVP_aes_256_cbc(), EVP_sha512(), &vchSalt[0], (unsigned char *) &strKeyData[0],
						strKeyData.size(), unRounds, m_chKey, m_chIV);
	}
	if (i != (int) g_kWalletCryptoKeySize) {
		OPENSSL_cleanse(m_chKey, sizeof(m_chKey));
		OPENSSL_cleanse(m_chIV, sizeof(m_chIV));
		return false;
	}
	m_bKeySet = true;

	return true;
}

bool CCrypter::SetKey(const CKeyingMaterial& chNewKey, const vector<unsigned char>& chNewIV) {
	if (chNewKey.size() != g_kWalletCryptoKeySize || chNewIV.size() != g_kWalletCryptoKeySize) {
		return false;
	}
	memcpy(&m_chKey[0], &chNewKey[0], sizeof m_chKey);
	memcpy(&m_chIV[0], &chNewIV[0], sizeof m_chIV);
	m_bKeySet = true;

	return true;
}

bool CCrypter::Encrypt(const CKeyingMaterial& vchPlaintext, vector<unsigned char> &vchCiphertext) {
	if (!m_bKeySet) {
		return false;
	}
	// max ciphertext len for a n bytes of plaintext is
	// n + AES_BLOCK_SIZE - 1 bytes
	int nLen = vchPlaintext.size();
	int nCLen = nLen + AES_BLOCK_SIZE, nFLen = 0;
	vchCiphertext = vector<unsigned char>(nCLen);

	EVP_CIPHER_CTX sEvpCipherCtx;

	bool bOk = true;

	EVP_CIPHER_CTX_init(&sEvpCipherCtx);
	if (bOk) {
		bOk = EVP_EncryptInit_ex(&sEvpCipherCtx, EVP_aes_256_cbc(), NULL, m_chKey, m_chIV);
	}
	if (bOk) {
		bOk = EVP_EncryptUpdate(&sEvpCipherCtx, &vchCiphertext[0], &nCLen, &vchPlaintext[0], nLen);
	}
	if (bOk) {
		bOk = EVP_EncryptFinal_ex(&sEvpCipherCtx, (&vchCiphertext[0]) + nCLen, &nFLen);
	}

	EVP_CIPHER_CTX_cleanup(&sEvpCipherCtx);

	if (!bOk) {
		return false;
	}
	vchCiphertext.resize(nCLen + nFLen);

	return true;
}

bool CCrypter::Decrypt(const vector<unsigned char>& vchCiphertext, CKeyingMaterial& vchPlaintext) {
	if (!m_bKeySet) {
		return false;
	}
	// plaintext will always be equal to or lesser than length of ciphertext
	int nLen = vchCiphertext.size();
	int nPLen = nLen, nFLen = 0;

	vchPlaintext = CKeyingMaterial(nPLen);
	EVP_CIPHER_CTX sEvpCipherCtx;
	bool bOk = true;

	EVP_CIPHER_CTX_init(&sEvpCipherCtx);
	if (bOk) {
		bOk = EVP_DecryptInit_ex(&sEvpCipherCtx, EVP_aes_256_cbc(), NULL, m_chKey, m_chIV);
	}
	if (bOk) {
		bOk = EVP_DecryptUpdate(&sEvpCipherCtx, &vchPlaintext[0], &nPLen, &vchCiphertext[0], nLen);
	}
	if (bOk) {
		bOk = EVP_DecryptFinal_ex(&sEvpCipherCtx, (&vchPlaintext[0]) + nPLen, &nFLen);
	}

	EVP_CIPHER_CTX_cleanup(&sEvpCipherCtx);

	if (!bOk) {
		return false;
	}
	vchPlaintext.resize(nPLen + nFLen);

	return true;
}

bool EncryptSecret(const CKeyingMaterial& vMasterKey, const CKeyingMaterial &vPlaintext, const uint256& cIV,
		vector<unsigned char> &vchCiphertext) {
	CCrypter cKeyCrypter;
	vector<unsigned char> vchIV(g_kWalletCryptoKeySize);
	memcpy(&vchIV[0], &cIV, g_kWalletCryptoKeySize);
	if (!cKeyCrypter.SetKey(vMasterKey, vchIV)) {
		return false;
	}

	return cKeyCrypter.Encrypt(*((const CKeyingMaterial*) &vPlaintext), vchCiphertext);
}

bool DecryptSecret(const CKeyingMaterial& vMasterKey, const vector<unsigned char>& vchCiphertext, const uint256& cIV,
		CKeyingMaterial& vPlaintext) {
	CCrypter cKeyCrypter;
	vector<unsigned char> vchIV(g_kWalletCryptoKeySize);
	memcpy(&vchIV[0], &cIV, g_kWalletCryptoKeySize);
	if (!cKeyCrypter.SetKey(vMasterKey, vchIV)) {
		return false;
	}

	return cKeyCrypter.Decrypt(vchCiphertext, *((CKeyingMaterial*) &vPlaintext));
}

bool CCryptoKeyStore::SetCrypted() {
	LOCK(cs_KeyStore);
	if (m_bUseCrypto) {
		return true;
	}
	if (IsContainMainKey()) {
		return false;
	}
	m_bUseCrypto = true;

	return true;
}

bool CCryptoKeyStore::Lock() {
	if (!SetCrypted()) {
		return false;
	}
	{
		LOCK(cs_KeyStore);
		m_vMasterKey.clear();
	}
	g_pUIInterface->NotifyMessage("Lock");
	NotifyStatusChanged(this);

	return true;
}

bool CCryptoKeyStore::Unlock(const CKeyingMaterial& vMasterKeyIn) {
	{
		LOCK(cs_KeyStore);
		if (!SetCrypted()) {
			return false;
		}
		CryptedKeyMap::const_iterator mi = m_mapCryptedKeys.begin();
		for (; mi != m_mapCryptedKeys.end(); ++mi) {
			const CPubKey &cPubKey = (*mi).second.first;
			const vector<unsigned char> &vchCryptedSecret = (*mi).second.second;
			CKeyingMaterial vSecret;
			if (!DecryptSecret(vMasterKeyIn, vchCryptedSecret, cPubKey.GetHash(), vSecret)) {
				return false;
			}
			if (vSecret.size() != 32) {
				return false;
			}
			CKey cKey;
			cKey.Set(vSecret.begin(), vSecret.end(), cPubKey.IsCompressed());
			if (cKey.GetPubKey() == cPubKey) {
				break;
			}

			return false;
		}
		m_vMasterKey = vMasterKeyIn;
	}
	g_pUIInterface->NotifyMessage("UnLock");
	NotifyStatusChanged(this);

	return true;
}

bool CCryptoKeyStore::AddKeyCombi(const CKeyID & cKeyId, const CKeyCombi &cKeyCombi) {
	{
		LOCK(cs_KeyStore);

		if (!IsCrypted())
			return CBasicKeyStore::AddKeyCombi(cKeyId, cKeyCombi);

		if (IsLocked())
			return false;

		CKey mainKey;
		cKeyCombi.GetCKey(mainKey, false);
		CKeyCombi newkeyCombi = cKeyCombi;
		newkeyCombi.CleanMainKey();
		CBasicKeyStore::AddKeyCombi(cKeyId, cKeyCombi);

		vector<unsigned char> vchCryptedSecret;
		CKeyingMaterial vchSecret(mainKey.begin(), mainKey.end());
		CPubKey pubKey;
		pubKey = mainKey.GetPubKey();
		if (!EncryptSecret(m_vMasterKey, vchSecret, pubKey.GetHash(), vchCryptedSecret))
			return false;

		if (!AddCryptedKey(pubKey, vchCryptedSecret))
			return false;
	}

	return true;
}

bool CCryptoKeyStore::AddCryptedKey(const CPubKey &cPubKey, const vector<unsigned char> &vchCryptedSecret) {
	{
		LOCK(cs_KeyStore);
		if (!SetCrypted())
			return false;
		m_mapCryptedKeys[cPubKey.GetKeyID()] = make_pair(cPubKey, vchCryptedSecret);
	}

	return true;
}

bool CCryptoKeyStore::GetKey(const CKeyID &cAddress, CKey& ckeyOut, bool bIsMine) const {
	{
		LOCK(cs_KeyStore);
		if (bIsMine) {
			return CBasicKeyStore::GetKey(cAddress, ckeyOut, bIsMine);
		} else {
			if (!IsCrypted()) {
				return CBasicKeyStore::GetKey(cAddress, ckeyOut);
			}
			CryptedKeyMap::const_iterator mi = m_mapCryptedKeys.find(cAddress);
			if (mi != m_mapCryptedKeys.end()) {
				const CPubKey &cPubKey = (*mi).second.first;
				const vector<unsigned char> &vchCryptedSecret = (*mi).second.second;
				CKeyingMaterial vSecret;
				if (!DecryptSecret(m_vMasterKey, vchCryptedSecret, cPubKey.GetHash(), vSecret)) {
					return false;
				}
				if (vSecret.size() != 32) {
					return false;
				}
				ckeyOut.Set(vSecret.begin(), vSecret.end(), cPubKey.IsCompressed());
				return true;
			}
		}
	}

	return false;
}

bool CCryptoKeyStore::GetPubKey(const CKeyID &cAddress, CPubKey& cPubKeyOut, bool bIsMine) const {
	{
		LOCK(cs_KeyStore);
		if (bIsMine) {
			return CKeyStore::GetPubKey(cAddress, cPubKeyOut, bIsMine);
		} else {
			if (!IsCrypted()) {
				return CKeyStore::GetPubKey(cAddress, cPubKeyOut, bIsMine);
			}
			CryptedKeyMap::const_iterator mi = m_mapCryptedKeys.find(cAddress);
			if (mi != m_mapCryptedKeys.end()) {
				cPubKeyOut = (*mi).second.first;
				return true;
			}
		}
	}

	return false;
}

bool CCryptoKeyStore::GetKeyCombi(const CKeyID & cAddress, CKeyCombi & cKeyCombiOut) const {
	CBasicKeyStore::GetKeyCombi(cAddress, cKeyCombiOut);
	if (!IsCrypted())
		return true;
	CKey keyOut;
	if (!IsLocked()) {
		if (!GetKey(cAddress, keyOut))
			return false;
		cKeyCombiOut.SetMainKey(keyOut);
	}

	return true;
}

bool CCryptoKeyStore::EncryptKeys(CKeyingMaterial& vMasterKeyIn) {
	{
		LOCK(cs_KeyStore);
		if (!m_mapCryptedKeys.empty() || IsCrypted()) {
			return false;
		}
		m_bUseCrypto = true;
		for (auto& mKey : mapKeys) {
			CKey cMainKey;
			mKey.second.GetCKey(cMainKey, false);
			CPubKey cPubKey = cMainKey.GetPubKey();
			CKeyingMaterial vSecret(cMainKey.begin(), cMainKey.end());
			vector<unsigned char> vchCryptedSecret;
			if (!EncryptSecret(vMasterKeyIn, vSecret, cPubKey.GetHash(), vchCryptedSecret)) {
				return false;
			}
			if (!AddCryptedKey(cPubKey, vchCryptedSecret)) {
				return false;
			}

			mKey.second.CleanMainKey();
		}
	}

	return true;
}
