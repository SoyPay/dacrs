// Copyright (c) 2012-2013 The Bitcoin Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

//
// Unit tests for canonical signatures
//

//#include "script.h"
#include "util.h"
#include "data/sig_noncanonical.json.h"
#include "data/sig_canonical.json.h"

#include <boost/foreach.hpp>
#include <boost/test/unit_test.hpp>
#include "json/json_spirit_writer_template.h"
#include <openssl/ecdsa.h>

using namespace std;
using namespace json_spirit;

// In script_tests.cpp
extern Array read_json(const std::string& jsondata);

BOOST_AUTO_TEST_SUITE(canonical_tests)

// OpenSSL-based test for canonical signature (without test for hashtype byte)
bool static IsCanonicalSignature_OpenSSL_inner(const std::vector<unsigned char>& vuchSig) {
	if (vuchSig.size() == 0) {
		return false;
	}
	const unsigned char *puchInput = &vuchSig[0];
	ECDSA_SIG *pSig = NULL;
	d2i_ECDSA_SIG(&pSig, &puchInput, vuchSig.size());
	if (pSig == NULL) {
		return false;
	}
	unsigned char arruchBuf[256];
	unsigned char *puchBuf = arruchBuf;
	unsigned int unLen = i2d_ECDSA_SIG(pSig, NULL);
	if (unLen != vuchSig.size()) {
		ECDSA_SIG_free(pSig);
		return false;
	}
	unLen = i2d_ECDSA_SIG(pSig, &puchBuf);
	ECDSA_SIG_free(pSig);
	return (memcmp(&vuchSig[0], &arruchBuf[0], unLen) == 0);
}

// OpenSSL-based test for canonical signature
bool static IsCanonicalSignature_OpenSSL(const std::vector<unsigned char> &vuchSignature) {
	if (vuchSignature.size() < 1) {
		return false;
	}
	if (vuchSignature.size() > 127) {
		return false;
	}
	if (vuchSignature[vuchSignature.size() - 1] & 0x7C) {
		return false;
	}

	std::vector<unsigned char> vuchSig(vuchSignature);
	vuchSig.pop_back();
	if (!IsCanonicalSignature_OpenSSL_inner(vuchSig)) {
		return false;
	}
	return true;
}

//BOOST_AUTO_TEST_CASE(script_canon)
//{
//    Array tests = read_json(std::string(json_tests::sig_canonical, json_tests::sig_canonical + sizeof(json_tests::sig_canonical)));
//
//    for (auto &tv : tests) {
//        string test = tv.get_str();
//        if (IsHex(test)) {
//            std::vector<unsigned char> sig = ParseHex(test);
//            BOOST_CHECK_MESSAGE(IsCanonicalSignature(sig, SCRIPT_VERIFY_STRICTENC), test);
//            BOOST_CHECK_MESSAGE(IsCanonicalSignature_OpenSSL(sig), test);
//        }
//    }
//}

//BOOST_AUTO_TEST_CASE(script_noncanon)
//{
//    Array tests = read_json(std::string(json_tests::sig_noncanonical, json_tests::sig_noncanonical + sizeof(json_tests::sig_noncanonical)));
//
//    for (auto &tv : tests) {
//        string test = tv.get_str();
//        if (IsHex(test)) {
//            std::vector<unsigned char> sig = ParseHex(test);
//            BOOST_CHECK_MESSAGE(!IsCanonicalSignature(sig, SCRIPT_VERIFY_STRICTENC), test);
//            BOOST_CHECK_MESSAGE(!IsCanonicalSignature_OpenSSL(sig), test);
//        }
//    }
//}

BOOST_AUTO_TEST_SUITE_END()
