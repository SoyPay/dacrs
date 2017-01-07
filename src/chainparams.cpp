// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Dacrs developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "chainparams.h"

#include "assert.h"
#include "core.h"
#include "protocol.h"
#include "util.h"
#include "key.h"
#include "tx.h"
#include "main.h"

#include <boost/assign/list_of.hpp>
#include <boost/algorithm/string/case_conv.hpp> // for to_lower()
#include <boost/algorithm/string/predicate.hpp> // for startswith() and endswith()
#include <boost/filesystem.hpp>
using namespace boost::assign;
using namespace std;

map<string, string> CBaseParams::m_mapArgs;
map<string, vector<string> > CBaseParams::m_mapMultiArgs;



//	{
//        "addr" : "dsjkLDFfhenmx2JkFMdtJ22TYDvSGgmJem",
//        "RegID" : "0-9",
//        "RegID2" : "000000000900"
//    },
//    {
//        "addr" : "dsGb9GyDGYnnHdjSvRfYbj9ox2zPbtgtpo",
//        "RegID" : "0-5",
//        "RegID2" : "000000000500"
//    },
//    {
//        "addr" : "dejEcGCsBkwsZaiUH1hgMapjbJqPdPNV9U",
//        "RegID" : "0-3",
//        "RegID2" : "000000000300"
//    },
//    {
//        "addr" : "e21rEzVwkPFQYfgxcg7xLp7DKeYrW4Fpoz",
//        "RegID" : "0-8",
//        "RegID2" : "000000000800"
//    },
//    {
//        "addr" : "dd936HZcwj9dQkefHPqZpxzUuKZZ2QEsbN",
//        "RegID" : "0-7",
//        "RegID2" : "000000000700"
//    },
//    {
//        "addr" : "dkoEsWuW3aoKaGduFbmjVDbvhmjxFnSbyL",
//        "RegID" : "0-4",
//        "RegID2" : "000000000400"
//    },
//    {
//        "addr" : "ddEaChh3846J6xLkeyNaXwo6tMMZdHUTx6",
//        "RegID" : " ",
//        "RegID2" : "000000000000"
//    },
//    {
//        "addr" : "dggsWmQ7jH46dgtA5dEZ9bhFSAK1LASALw",
//        "RegID" : "0-1",
//        "RegID2" : "000000000100"
//    },
//    {
//        "addr" : "dps9hqUmBAVGVg7ijLGPcD9CJz9HHiTw6H",
//        "RegID" : "0-10",
//        "RegID2" : "000000000a00"
//    },
//    {
//        "addr" : "dkJwhBs2P2SjbQWt5Bz6vzjqUhXTymvsGr",
//        "RegID" : " ",
//        "RegID2" : "000000000000"
//    },
//    {
//        "addr" : "doym966kgNUKr2M9P7CmjJeZdddqvoU5RZ",
//        "RegID" : "0-6",
//        "RegID2" : "000000000600"
//    },
//    {
//        "addr" : "dtKsuK9HUvLLHtBQL8Psk5fUnTLTFC83GS",
//        "RegID" : "0-2",
//        "RegID2" : "000000000200"
//    }
//
// testnet network
//
vector<string> g_vstrIntPubKeyMainNet = {
		"0388a07c89727f9065703100e94c00ce82bda6987215a88abee65db9b37f52f9e0",
		"03fdfda984690ff2b10f27ccc38b90634ca101a27621c1a9dcec7a2f33fa0282d6"
};

vector<string> g_vstrInitPubKeyTestNet = {
		"0388a07c89727f9065703100e94c00ce82bda6987215a88abee65db9b37f52f9e0",
		"0360328964121d4625ca827c3e111f99fb005d5b30455d2609ac0ac3bbe17df601"
};

vector<string> g_vstrInitPubkeyRegTest = {
		"03d308757fc1f8efd69f2da329db560cd7d3cba951eb09786c375cf1709f9165ba",
		"024126ccf4b5f6463a3f874f234b77d02e9f5c2057c6c382160dc17c7f9ba2b333",
		"0221b571330617821e8c508416b90988e81e8dc8623576b8f6e942797e9f381111",
		"02e5c2fbea1055139e3d46621ef49aede5eb3ca1629fc3520986c5aba203706e74",
		"0295c3d0f4a913fd8dccf7a0782ae59fae282057169214a136b3722d5299248683",
		"03a616686dd872e301eb317a1ff4b53530d69492e8a39f6468afbd018043623166",
		"02ac35dabc82e297697e9774a2a3f9b1ddd5dc536da0c9bba7f5e95eb525aa4706",
		"0324da799ffc36177c9d401a81b6a1b4d90e7553ac4165fb263bbb8b60940a2b1d",
		"0311ca277a0f3880eefed349727cc954354c4e539b8128c79f18a87c6f2b979186",
		"02be4a840cf29bcbf84e3b7a0243adff77a32b9d76083ff254c1110734ea6b5792",
		"029bbfd75711ac071b361e133679d21c1cc6314a6c3fd71e91880956b91e640948",

		"02a3e7998d3f8dd6dedfc6bf06e46adf98041bf4ca8e9af103d62f85c6f0a0a9bf",
		"024ee551da4a0ca765f21a2c9d33ad61826f36eba8912ad80e8d5bf75c397f3ee2",
		"03e6cda0f68b8028a74bcbbde1a164a022dad8956eabaa61bd40935d06c6fb55a4",
		"03ec7e8f2521cc5d88be7e603c35ba9d4ecda09abe8f96a28514956c16ac7d9019",
		"03e7f41495038767d7eb522938de5bc4556ef2e1075432245b9fc0e348c0516baf",
		"03a61b40def5330abbb9f7491b11d586283b5fd2fb2e5cd546b0265f21e9cf9356",
		"025a1c65b72c72569559edf54491dcf45e7ed0299886ee3eaa3cf5d5765b5b606b",
		"035a998c0adb99003c0552ed44ac3f46e80f542b2d26cb94dd3356027b1c844b99",
		"03f61c32ccc409ce5b55844f7cd2b62c52a8fe3d790efbc0644140cda33547aa67",
		"03ae28a4100145a4c354338c727a54800dc540069fa2f5fd5d4a1c80b4a35a1762"
};

unsigned int g_unPnSeed[] = {
		0xa78a2879, 0xb5af0bc6, 0x2f4f4a70, 0x30e65cb6, 0x7ae82879, 0x680ec48b, 0x7F1E4A70,
		0x8868D772, 0xCD382879, 0xD239397B, 0x51C41978, 0x73B4C48B, 0x73EF1A78
};

class CMainParams: public CBaseParams {
 public:
	CMainParams() {

// The message start string is designed to be unlikely to occur in normal data.
// The characters are rarely used upper ASCII, not valid as UTF-8, and produce
// a large 4-byte int at any alignment.
		m_pchMessageStart[0] = 0xff;
		m_pchMessageStart[1] = 0xfe;
		m_pchMessageStart[2] = 0x1d;
		m_pchMessageStart[3] = 0x20;
		m_vchAlertPubKey = ParseHex("02d99681b6287b3765dfbb930e6caa10d1f8ac19e02b88f52362ce6eb43c0ec71e");
		m_nDefaultPort = 8668;
		m_nRPCPort = 18332;
		m_nUIPort = 4246;
		m_strDataDir = "main";
		m_bnProofOfStakeLimit = ~arith_uint256(0) >> 10;        //00 3f ff ff
		m_nSubsidyHalvingInterval = 210000;

		assert(CreateGenesisRewardTx(genesis.vptx, g_vstrIntPubKeyMainNet));
		genesis.SetHashPrevBlock(uint256());
		genesis.SetHashMerkleRoot(genesis.BuildMerkleTree());
		genesis.SetHashPos(uint256());
		genesis.SetVersion(1);
		genesis.SetTime(1436538491);
		genesis.SetBits(0x1f3fffff);        //00 3f ff
		genesis.SetNonce(888);
		genesis.SetFuelRate(INIT_FUEL_RATES);
		genesis.SetHeight(0);
		genesis.ClearSignature();
		m_cHashGenesisBlock = genesis.GetHash();
		m_strPublicKey = "02d99681b6287b3765dfbb930e6caa10d1f8ac19e02b88f52362ce6eb43c0ec71e";
//		{
//			cout << "main hashGenesisBlock:\r\n" << hashGenesisBlock.ToString() << endl;
//			cout << "main hashMerkleRoot:\r\n" << genesis.hashMerkleRoot.ToString() << endl;
//		}
		assert(m_cHashGenesisBlock == uint256S("0xd9ffedf0475c7734e1ea1a7aa1a05361825e77869371962ee6a3ec515e2e2c3d"));
		assert(
				genesis.GetHashMerkleRoot()
						== uint256S("0x362155f5bb005be0523c7247cf1b901bd6f3567d105bd5defca28d221c90d1ef"));

//      vSeeds.push_back(CDNSSeedData("soypay.org.cn", "seed_cn_0.dspay.org"));
//      vSeeds.push_back(CDNSSeedData("soypay.org.us", "seed_us_0.dspay.org"));

		m_vchBase58Prefixes[EM_PUBKEY_ADDRESS] = {0x1f};
		m_vchBase58Prefixes[EM_SCRIPT_ADDRESS] = {5};
		m_vchBase58Prefixes[EM_SECRET_KEY] = {128};
		m_vchBase58Prefixes[EM_EXT_PUBLIC_KEY] = {0x04,0x88,0xB2,0x1E};
		m_vchBase58Prefixes[EM_EXT_SECRET_KEY] = {0x04,0x88,0xAD,0xE4};

		// Convert the pnSeeds array into usable address objects.
		for (unsigned int i = 0; i < ARRAYLEN(g_unPnSeed); i++) {
			// It'll only connect to one or two seed nodes because once it connects,
			// it'll get a pile of addresses with newer timestamps.
			// Seed nodes are given a random 'last seen time' of between one and two
			// weeks ago.
			const int64_t nOneWeek = 7 * 24 * 60 * 60;
			struct in_addr ip;
			memcpy(&ip, &g_unPnSeed[i], sizeof(ip));
			CAddress addr(CService(ip, GetDefaultPort()));
			addr.m_ullTime = GetTime() - GetRand(nOneWeek) - nOneWeek;
			vFixedSeeds.push_back(addr);
		}
	}

	virtual const CBlock& GenesisBlock() const {
		return genesis;
	}
	virtual emNetWork NetworkID() const {
		return CBaseParams::EM_MAIN;
	}
	virtual bool InitalConfig() {
		return CBaseParams::InitalConfig();
	}
	virtual int GetBlockMaxNonce() const {
		return 1000;
	}
	virtual const vector<CAddress>& FixedSeeds() const {
		return vFixedSeeds;
	}
	virtual bool IsInFixedSeeds(CAddress &addr) {
		vector<CAddress>::iterator iterAddr = find(vFixedSeeds.begin(), vFixedSeeds.end(), addr);
		return iterAddr != vFixedSeeds.end();
	}

protected:
	CBlock genesis;
	vector<CAddress> vFixedSeeds;
};
//static CMainParams mainParams;

//
// Testnet (v3)
//
class CTestNetParams: public CMainParams {
public:
	CTestNetParams() {
		// The message start string is designed to be unlikely to occur in normal data.
		// The characters are rarely used upper ASCII, not valid as UTF-8, and produce
		// a large 4-byte int at any alignment.
        m_pchMessageStart[0] = 0xfe;
        m_pchMessageStart[1] = 0x2d;
        m_pchMessageStart[2] = 0x1c;
        m_pchMessageStart[3] = 0x0d;
		m_vchAlertPubKey =	ParseHex("036e15523feb9e329b4fdf53c227fc89ea45a1a36342e7e38fad7fe6e3777243af");
		m_nDefaultPort = 18668;
		m_nRPCPort = 18383;
		m_nUIPort = 4264;
		m_strDataDir = "testnet";
		m_strPublicKey = "036e15523feb9e329b4fdf53c227fc89ea45a1a36342e7e38fad7fe6e3777243af";
		// Modify the testnet genesis block so the timestamp is valid for a later start.
		genesis.SetTime(1436598023);
		genesis.SetNonce(888);
		genesis.vptx.clear();
		assert(CreateGenesisRewardTx(genesis.vptx, g_vstrInitPubKeyTestNet));
		genesis.SetHashMerkleRoot(genesis.BuildMerkleTree());
		m_cHashGenesisBlock = genesis.GetHash();
		for(auto & item : vFixedSeeds)
			item.SetPort(GetDefaultPort());

//		{
//			cout << "testnet hashGenesisBlock:\r\n" << hashGenesisBlock.ToString() << endl;
//		}
		assert(m_cHashGenesisBlock == uint256S("0xc6f81a98e9de1ac7da65a8b2bbd937f1d49aaedc7f8f2f0517c13f099df1ed49"));
//		vSeeds.clear();
//		vSeeds.push_back(CDNSSeedData("Dacrs.petertodd.org", "testnet-seed.Dacrs.petertodd.org"));
//		vSeeds.push_back(CDNSSeedData("bluematt.me", "testnet-seed.bluematt.me"));

//		base58Prefixes[PUBKEY_ADDRESS] = {111};
		m_vchBase58Prefixes[EM_PUBKEY_ADDRESS] = {0x5b};
		m_vchBase58Prefixes[EM_SCRIPT_ADDRESS] = {196};
		m_vchBase58Prefixes[EM_SECRET_KEY]     = {239};
		m_vchBase58Prefixes[EM_EXT_PUBLIC_KEY] = {0x04,0x35,0x87,0xCF};
		m_vchBase58Prefixes[EM_EXT_SECRET_KEY] = {0x04,0x35,0x83,0x94};
	}
	virtual emNetWork NetworkID() const {return CBaseParams::EM_TESTNET;}
	virtual bool InitalConfig()
	{
		CMainParams::InitalConfig();
		m_bServer = true;
		return true;
	}
	virtual int GetBlockMaxNonce() const
	{
		return 1000;
	}
};
//static CTestNetParams testNetParams;

//
// Regression test
//
class CRegTestParams: public CTestNetParams {
public:
	CRegTestParams() {
		m_pchMessageStart[0] = 0xfc;
		m_pchMessageStart[1] = 0x1d;
		m_pchMessageStart[2] = 0x2d;
		m_pchMessageStart[3] = 0x3d;
		m_nSubsidyHalvingInterval = 150;
		m_bnProofOfStakeLimit = ~arith_uint256(0) >> 6;     //target:00000011 11111111 11111111
		genesis.SetTime(1421808634);
		genesis.SetBits(0x2003ffff);
		genesis.SetNonce(888);
		genesis.vptx.clear();
		assert(CreateGenesisRewardTx(genesis.vptx, g_vstrInitPubkeyRegTest));
		genesis.SetHashMerkleRoot(genesis.BuildMerkleTree());
		m_cHashGenesisBlock = genesis.GetHash();
		m_nDefaultPort = 18666;
		m_llTargetSpacing = 20;
		m_llTargetTimespan = 30 * 20;
		m_strDataDir = "regtest";
//		{
//			CBigNum bnTarget;
//			bnTarget.SetCompact(genesis.nBits);
//			cout << "regtest bnTarget:" << bnTarget.getuint256().GetHex() << endl;
//			cout << "regtest hashGenesisBlock:\r\n" << hashGenesisBlock.ToString() << endl;
//			cout << "regtest hashMerkleRoot:\r\n" << genesis.hashMerkleRoot.ToString() << endl;
//		}
		assert(m_cHashGenesisBlock == uint256S("0x18d876339c5014803507332d0ae509862b4da382253b7696605e58963592723f"));

		vFixedSeeds.clear();
		m_vSeeds.clear();  // Regtest mode doesn't have any DNS seeds.
	}

	virtual bool RequireRPCPassword() const {
		return false;
	}
	virtual emNetWork NetworkID() const {
		return CBaseParams::EM_REGTEST;
	}
	virtual bool InitalConfig() {
		CTestNetParams::InitalConfig();
		m_bServer = true;
		return true;
	}
};
//static CRegTestParams regTestParams;

//static CBaseParams *pCurrentParams = &mainParams;

//CBaseParams &Params() {
//	return *pCurrentParams;
//}

//void SelectParams(CBaseParams::Network network) {
//	switch (network) {
//	case CBaseParams::MAIN:
//		pCurrentParams = &mainParams;
//		break;
//	case CBaseParams::TESTNET:
//		pCurrentParams = &testNetParams;
//		break;
//	case CBaseParams::REGTEST:
//		pCurrentParams = &regTestParams;
//		break;
//	default:
//		assert(false && "Unimplemented network");
//		return;
//	}
//}

//bool SelectParamsFromCommandLine() {
//	bool fRegTest = GetBoolArg("-regtest", false);
//	bool fTestNet = GetBoolArg("-testnet", false);
//
//	if (fTestNet && fRegTest) {
//		return false;
//	}
//
//	if (fRegTest) {
//		SelectParams(CBaseParams::REGTEST);
//	} else if (fTestNet) {
//		SelectParams(CBaseParams::TESTNET);
//	} else {
//		SelectParams(CBaseParams::MAIN);
//	}
//	return true;
//}

/********************************************************************************/
const vector<string> &CBaseParams::GetMultiArgs(const string& strArg) {
	return m_mapMultiArgs[strArg];
}
int CBaseParams::GetArgsSize() {
	return m_mapArgs.size();
}
int CBaseParams::GetMultiArgsSize() {
	return m_mapMultiArgs.size();
}

string CBaseParams::GetArg(const string& strArg, const string& strDefault) {
	if (m_mapArgs.count(strArg)) {
		return m_mapArgs[strArg];
	}
	return strDefault;
}

int64_t CBaseParams::GetArg(const string& strArg, int64_t llDefault) {
	if (m_mapArgs.count(strArg)) {
		return atoi64(m_mapArgs[strArg]);
	}
	return llDefault;
}

bool CBaseParams::GetBoolArg(const string& strArg, bool bDefault) {
	if (m_mapArgs.count(strArg)) {
		if (m_mapArgs[strArg].empty()) {
			return true;
		}
		return (atoi(m_mapArgs[strArg]) != 0);
	}
	return bDefault;
}

bool CBaseParams::SoftSetArg(const string& strArg, const string& strValue) {
	if (m_mapArgs.count(strArg)) {
		return false;
	}
	m_mapArgs[strArg] = strValue;
	return true;
}

bool CBaseParams::SoftSetArgCover(const string& strArg, const string& strValue) {
	m_mapArgs[strArg] = strValue;
	return true;
}

void CBaseParams::EraseArg(const string& strArgKey) {
	m_mapArgs.erase(strArgKey);
}

bool CBaseParams::SoftSetBoolArg(const string& strArg, bool bValue) {
	if (bValue) {
		return SoftSetArg(strArg, string("1"));
	} else {
		return SoftSetArg(strArg, string("0"));
	}
}

bool CBaseParams::IsArgCount(const string& strArg) {
	if (m_mapArgs.count(strArg)) {
		return true;
	}
	return false;
}

CBaseParams &SysCfg() {
	static shared_ptr<CBaseParams> pParams;

	if (pParams.get() == NULL) {
		bool bRegTest = CBaseParams::GetBoolArg("-regtest", false);
		bool bTestNet = CBaseParams::GetBoolArg("-testnet", false);
		if (bTestNet && bRegTest) {
			fprintf(stderr, "Error: Invalid combination of -regtest and -testnet.\n");
//			assert(0);
		}

		if (bRegTest) {
			//LogPrint("spark", "In Reg Test Net\n");
			pParams = std::make_shared<CRegTestParams>();
		} else if (bTestNet) {
			//LogPrint("spark", "In Test Net\n");
			pParams = std::make_shared<CTestNetParams>();
		} else {
			//LogPrint("spark", "In Main Net\n");
			pParams = std::make_shared<CMainParams>();
		}

	}
	assert(pParams != NULL);
	return *pParams.get();
}

//write for test code
const CBaseParams &SysParamsMain() {
	static std::shared_ptr<CBaseParams> pParams;
	pParams = std::make_shared<CMainParams>();
	assert(pParams != NULL);
	return *pParams.get();
}

//write for test code
const CBaseParams &SysParamsTest() {
	static std::shared_ptr<CBaseParams> pParams;
	pParams = std::make_shared<CTestNetParams>();
	assert(pParams != NULL);
	return *pParams.get();
}

//write for test code
const CBaseParams &SysParamsReg() {
	static std::shared_ptr<CBaseParams> pParams;
	pParams = std::make_shared<CRegTestParams>();
	assert(pParams != NULL);
	return *pParams.get();
}

static void InterpretNegativeSetting(string name, map<string, string>& mapSettingsRet) {
	// interpret -nofoo as -foo=0 (and -nofoo=0 as -foo=1) as long as -foo not set
	if (name.find("-no") == 0) {
		string positive("-");
		positive.append(name.begin() + 3, name.end());
		if (mapSettingsRet.count(positive) == 0) {
			bool value = !SysCfg().GetBoolArg(name, false);
			mapSettingsRet[positive] = (value ? "1" : "0");
		}
	}
}

void CBaseParams::ParseParameters(int argc, const char* const argv[]) {
	m_mapArgs.clear();
	m_mapMultiArgs.clear();
	for (int i = 1; i < argc; i++) {
		string str(argv[i]);
		string strValue;
		size_t is_index = str.find('=');
		if (is_index != string::npos) {
			strValue = str.substr(is_index + 1);
			str = str.substr(0, is_index);
		}
#ifdef WIN32
		boost::to_lower(str);
		if (boost::algorithm::starts_with(str, "/")) {
			str = "-" + str.substr(1);
		}
#endif
		if (str[0] != '-') {
			break;
		}
		m_mapArgs[str] = strValue;
		m_mapMultiArgs[str].push_back(strValue);
	}

	// New 0.6 features:
//	BOOST_FOREACH(const PAIRTYPE(string,string)& entry, m_mapArgs) {
	for (auto& entry : m_mapArgs) {
		string name = entry.first;

		//  interpret --foo as -foo (as long as both are not set)
		if (name.find("--") == 0) {
			string singleDash(name.begin() + 1, name.end());
			if (m_mapArgs.count(singleDash) == 0) {
				m_mapArgs[singleDash] = entry.second;
			}
			name = singleDash;
		}

		// interpret -nofoo as -foo=0 (and -nofoo=0 as -foo=1) as long as -foo not set
		InterpretNegativeSetting(name, m_mapArgs);
	}
#if 0
	for(const auto& tmp:m_mapArgs) {
		printf("key:%s - value:%s\n", tmp.first.c_str(), tmp.second.c_str());
	}
#endif
}

bool CBaseParams::CreateGenesisRewardTx(vector<std::shared_ptr<CBaseTransaction> > &vRewardTx, const vector<string> &vInitPubKey) {
	int nLength = vInitPubKey.size();
	for (int i = 0; i < nLength; ++i) {
		int64_t money(0);
		if( i > 0) {
			money = 1000000000 * COIN;
		}
		shared_ptr<CRewardTransaction> pRewardTx = std::make_shared<CRewardTransaction>(ParseHex(vInitPubKey[i].c_str()), money, 0);
		pRewardTx->m_nVersion = g_sTxVersion1;
		if (pRewardTx.get())
			vRewardTx.push_back(pRewardTx);
		else
			return false;
	}
	return true;

};

bool CBaseParams::IntialParams(int argc, const char* const argv[]) {
	ParseParameters(argc, argv);
	if (!boost::filesystem::is_directory(GetDataDir(false))) {
		fprintf(stderr, "Error: Specified data directory \"%s\" does not exist.\n", CBaseParams::m_mapArgs["-datadir"].c_str());
		return false;
	}
	try {
		ReadConfigFile(CBaseParams::m_mapArgs, CBaseParams::m_mapMultiArgs);
	} catch (exception &e) {
		fprintf(stderr, "Error reading configuration file: %s\n", e.what());
		return false;
	}
	return true;
}

int64_t CBaseParams::GetTxFee() const{
     return m_llpaytxfee;
}
int64_t CBaseParams::SetDeflautTxFee(int64_t llFee)const{
	m_llpaytxfee = llFee;

	return llFee;
}

CBaseParams::CBaseParams() {
	m_bImporting 			= false;
	m_bReindex 				= false;
	m_bBenchmark 			= false;
	m_bTxIndex 				= false;
	m_nIntervalPos 			= 1;
	m_nLogmaxsize 			= 100 * 1024 * 1024;//100M
	m_nTxCacheHeight 		= 500;
	m_llTimeBestReceived 	= 0;
	m_unScriptCheckThreads 	= 0;
	m_llViewCacheSize 		= 2000000;
	m_llTargetSpacing 		= 60;
	m_llTargetTimespan 		= 30 * 60;
	m_nSubsidyHalvingInterval = 0;
	m_llpaytxfee 			= 10000;
	m_nDefaultPort 			= 0;
	m_bPrintToConsole		= 0;
	m_bPrintToToFile 		= 0;
	m_bLogTimestamps 		= 0;
	m_bLogPrintFileLine 	= 0;
	m_bDebug 				= 0;
	m_bDebugAll				= 0 ;
	m_bServer 				= 0 ;
	m_nRPCPort 				= 0;
	m_bOutPut 				= false;
	m_nUIPort 				= 0;
	m_bAddressToTx			= false;
}
