// Copyright (c) 2012-2013 The Bitcoin Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "bloom.h"

#include "base58.h"
#include "key.h"
#include "main.h"
#include "serialize.h"
#include "uint256.h"
#include "util.h"

#include <vector>

#include <boost/test/unit_test.hpp>

using namespace std;
using namespace boost::tuples;

BOOST_AUTO_TEST_SUITE(bloom_tests)

BOOST_AUTO_TEST_CASE(bloom_create_insert_serialize) {
	CBloomFilter cFilter(3, 0.01, 0, EM_BLOOM_UPDATE_ALL);

	cFilter.insert(ParseHex("99108ad8ed9bb6274d3980bab5a85c048f0950c8"));
	BOOST_CHECK_MESSAGE(cFilter.contains(ParseHex("99108ad8ed9bb6274d3980bab5a85c048f0950c8")),"BloomFilter doesn't contain just-inserted object!");
	// One bit different in first byte
	BOOST_CHECK_MESSAGE(!cFilter.contains(ParseHex("19108ad8ed9bb6274d3980bab5a85c048f0950c8")),"BloomFilter contains something it shouldn't!");

	cFilter.insert(ParseHex("b5a2c786d9ef4658287ced5914b37a1b4aa32eee"));
	BOOST_CHECK_MESSAGE(cFilter.contains(ParseHex("b5a2c786d9ef4658287ced5914b37a1b4aa32eee")),"BloomFilter doesn't contain just-inserted object (2)!");

	cFilter.insert(ParseHex("b9300670b4c5366e95b2699e8b18bc75e5f729c5"));
	BOOST_CHECK_MESSAGE(cFilter.contains(ParseHex("b9300670b4c5366e95b2699e8b18bc75e5f729c5")),"BloomFilter doesn't contain just-inserted object (3)!");

	CDataStream cStream(SER_NETWORK, g_sProtocolVersion);
	cFilter.Serialize(cStream, SER_NETWORK, g_sProtocolVersion);

	vector<unsigned char> vch = ParseHex("03614e9b050000000000000001");
	vector<char> vchExpected(vch.size());

	for (unsigned int i = 0; i < vch.size(); i++) {
		vchExpected[i] = (char) vch[i];
	}
	BOOST_CHECK_EQUAL_COLLECTIONS(cStream.begin(), cStream.end(), vchExpected.begin(), vchExpected.end());
}

BOOST_AUTO_TEST_CASE(bloom_create_insert_serialize_with_tweak) {
	// Same test as bloom_create_insert_serialize, but we add a nTweak of 100
	CBloomFilter cFilter(3, 0.01, 2147483649, EM_BLOOM_UPDATE_ALL);

	cFilter.insert(ParseHex("99108ad8ed9bb6274d3980bab5a85c048f0950c8"));
	BOOST_CHECK_MESSAGE( cFilter.contains(ParseHex("99108ad8ed9bb6274d3980bab5a85c048f0950c8")), "BloomFilter doesn't contain just-inserted object!");
	// One bit different in first byte
	BOOST_CHECK_MESSAGE(!cFilter.contains(ParseHex("19108ad8ed9bb6274d3980bab5a85c048f0950c8")), "BloomFilter contains something it shouldn't!");

	cFilter.insert(ParseHex("b5a2c786d9ef4658287ced5914b37a1b4aa32eee"));
	BOOST_CHECK_MESSAGE(cFilter.contains(ParseHex("b5a2c786d9ef4658287ced5914b37a1b4aa32eee")), "BloomFilter doesn't contain just-inserted object (2)!");

	cFilter.insert(ParseHex("b9300670b4c5366e95b2699e8b18bc75e5f729c5"));
	BOOST_CHECK_MESSAGE(cFilter.contains(ParseHex("b9300670b4c5366e95b2699e8b18bc75e5f729c5")), "BloomFilter doesn't contain just-inserted object (3)!");

	CDataStream cStream(SER_NETWORK, g_sProtocolVersion);
	cFilter.Serialize(cStream, SER_NETWORK, g_sProtocolVersion);

	vector<unsigned char> vch = ParseHex("03ce4299050000000100008001");
	vector<char> vchExpected(vch.size());

	for (unsigned int i = 0; i < vch.size(); i++) {
		vchExpected[i] = (char) vch[i];
	}
	BOOST_CHECK_EQUAL_COLLECTIONS(cStream.begin(), cStream.end(), vchExpected.begin(), vchExpected.end());
}

BOOST_AUTO_TEST_CASE(bloom_create_insert_key) {
	if (SysCfg().emNetWork::EM_TESTNET == SysCfg().NetworkID()
			|| SysCfg().emNetWork::EM_REGTEST == SysCfg().NetworkID()) {
		return;
	}
	string strSecret = string("5Kg1gnAjaLfKiwhhPpGS3QfRg2m6awQvaj98JCZBZQ5SuS2F15C");
	CDacrsSecret cVchSecret;
	BOOST_CHECK(cVchSecret.SetString(strSecret));

	CKey cKey = cVchSecret.GetKey();
	CPubKey cPubkey = cKey.GetPubKey();
	vector<unsigned char> vuchPubKey(cPubkey.begin(), cPubkey.end());

	CBloomFilter cFilter(2, 0.001, 0, EM_BLOOM_UPDATE_ALL);
	cFilter.insert(vuchPubKey);
	uint160 cHash = cPubkey.GetKeyID();
	cFilter.insert(vector<unsigned char>(cHash.begin(), cHash.end()));

	CDataStream cStream(SER_NETWORK, g_sProtocolVersion);
	cFilter.Serialize(cStream, SER_NETWORK, g_sProtocolVersion);

	vector<unsigned char> vuch = ParseHex("038fc16b080000000000000001");
	vector<char> vchExpected(vuch.size());

	for (unsigned int i = 0; i < vuch.size(); i++) {
		vchExpected[i] = (char) vuch[i];
	}
	BOOST_CHECK_EQUAL_COLLECTIONS(cStream.begin(), cStream.end(), vchExpected.begin(), vchExpected.end());
}

BOOST_AUTO_TEST_CASE(merkle_block_1) {
	// Random real cBlock (34ef2048a4c15e5a76e55af5cdbac0f0bd008a51ca68d233d7cb39158cb21933)
	// With 9 txes
	CBlock cBlock;
	
	CDataStream cStream(ParseHex("020000004d0449f6a390f734568e3ad4bead7b4abd8cdfe48839\
ce8151b50ef2ec3f0debcd37f5d3b75a2f58ef560459bf11c29070eee1c2\
64b9cffc3c436b7a8a621a9976746c2e752750315464708766f359a6b69e\
c40e3f843ff2b56fd0cb6b0400002754325882661c1bb701000066040000\
00000000000000001b010000473045022100b2a5e1ad39ae2eeaafbff026\
ea510362ac5e6cc253e36f969679ebc8b4e2f28102203aa8aa2bb33328ba\
ed172124f9274e8bf61d7b62b175625d27f878b5adc1287603010202000d\
82dbebaf2087660302876502000d140031f37de36553a23438846f342ac3\
bd765fe1f5cd109399febadda9000046304402206aaa631b6dca6e004bc8\
536acfdc75008fa93e132e53f499b2ecfdcaa7ec1fa602200c6d0fcabad4\
47b81f513eabec6cfc04294bf65576317665563ba20a766e950a03028765\
02000d140031f37de36553a23438846f342ac3bd765fe1f5cd108181d8ad\
8fef0000473045022100fc89572a8152d9294b6dd1c2834882e04bb62134\
6af41ea41c6a2e6107df545a02207b640c21116b01878bc1b6b58b94ec3a\
c38bd52715bd94bc7505c28ffdb4b0d0"), SER_NETWORK, g_sProtocolVersion);
	//CDataStream cStream(ParseHex("02000000a547b2124f7994309037326067ac4adb83983991903f6f9cecdb01f268dc44e5c8c2aff3b51ce1ea9e8072b008fd442fff52b811ef13fc91b52888e6a374324a7cee3055ffff032027000000020000008cad02000000000064000000473045022100a318df2a7fb2271ea522531646b3390bdddb287c1eaa7673be1f5bd38a7d59c502205b3380331f9a6679981541a9efb4e03acef1da1d6eb20ada11e959b4ebd255fa09010102000285d9d7e53402030101020002020001aede845882f6d3b08f0000463044022055d74ad09628743039adf3dd88443e03cfc4b8a237e1d7d960cd2be7dd0bb852022075bc9742b6a34e4c1bae0dceb1da8ade41fb786a71fd03886d8cea1b01a4d599030101020003020002aede845882f6d3b08f00004630440220698b580c9ca3f539514d5d99878990025b30d2a12df94166d85155907fd00d4a02206e42597a43be85be650276b581fd68c8115846b6a97c184a8475ee3e4390c1ed030101020002020003aede845882f6d3b08f00004730450221008aa75bb1fe1c418385cf59d5b04cf567523d58827b1884f3656744e45db4db2e02203fa3e5d53015905d74dd9eb72c1775cdae4356104d246253156d6f46619bf8b0040101020001020101aede845882f6d3b08f000115463044022064f1366f8c22304ccf04146004165cae16339dfc4de94f18f0a5e64033303293022010a20a359a5e1cc33fb661b6ad0af5d8ba63afdde4d50096196dab029019926d040101020003020101aede845882f6d3b08f000114473045022100b8c6c07c0cf5697ed08a2a68416083b81233e5f51732e23858cd6c6c1f78d77d0220151fad458b52d2be3b853057f900648262bc196f508d73a520de1863e0460ea0040101020001020101aede845882f6d3b08f000115473045022100fdd6c7942aeb2446d5dcb40dfda72df98bd934a175c0229617b2b759a972272f02205f02eabaa6b5709951cdc479fee91ae14c27c1d3a3a1981b684a120884c35810040101020002020101aede845882f6d3b08f000115473045022100df977ad23884494fc6953d33473b09e54bb8355993de159e37c4dc5e5e84de8d02200f9af6a53b1272e4ea8c7f67b82be36fbbcc46142ed28201868a8c212f2a3468040101020001020101aede845882f6d3b08f000115473045022100d187fa43e7bfbde412a6a08ff0c5fe4f4673ccc1066be4cd2e37c0f6f0ec30f80220572013d85a6eeeba3fb18836c2ae410f7ca0a2317bc8d436f9a33a2ed423e94a"), SER_NETWORK, g_sProtocolVersion);
	cStream >> cBlock;

	CBloomFilter cFilter(10, 0.000001, 0, EM_BLOOM_UPDATE_ALL);
	// Match the last transaction
	//cFilter.insert(uint256S("0xfbadf1682488087592fb8f48c2b3cb22a23273409d64bbb3e35469c6e9125da3"));
	cFilter.insert(uint256S("0xb09135750f39e4ccb81c39ace2820f8eabecf0ee8e4138427091130ffe45c13c"));
	CMerkleBlock cMerkleBlock(cBlock, cFilter);
	BOOST_CHECK(cMerkleBlock.m_cBlockHeader.GetHash() == cBlock.GetHash());

	BOOST_CHECK(cMerkleBlock.m_vMatchedTxn.size() == 1);
	pair<unsigned int, uint256> pair = cMerkleBlock.m_vMatchedTxn[0];

	BOOST_CHECK(cMerkleBlock.m_vMatchedTxn[0].second == uint256S("0xb09135750f39e4ccb81c39ace2820f8eabecf0ee8e4138427091130ffe45c13c"));
	BOOST_CHECK(cMerkleBlock.m_vMatchedTxn[0].first == 1);

	vector<uint256> vcMatched;
//	cout<<"cBlock.GetHashMerkleRoot()="<<cBlock.GetHashMerkleRoot().ToString()<<endl;
//	cout<<"cMerkleBlock.m_cTxNum.ExtractMatches(vcMatched)="<<cMerkleBlock.m_cTxNum.ExtractMatches(vcMatched).ToString()<<endl;
	BOOST_CHECK(cMerkleBlock.m_cTxNum.ExtractMatches(vcMatched) == cBlock.GetHashMerkleRoot());
	BOOST_CHECK(vcMatched.size() == cMerkleBlock.m_vMatchedTxn.size());
	for (unsigned int i = 0; i < vcMatched.size(); i++)
	BOOST_CHECK(vcMatched[i] == cMerkleBlock.m_vMatchedTxn[i].second);

	// Also match the 8th transaction
	cFilter.insert(uint256S("0x9eeed094d4dc4327ae5af54ba92b991ae545840d2fa882eb90c0f62d0885f5b9"));
	cMerkleBlock = CMerkleBlock(cBlock, cFilter);
	BOOST_CHECK(cMerkleBlock.m_cBlockHeader.GetHash() == cBlock.GetHash());

	BOOST_CHECK(cMerkleBlock.m_vMatchedTxn.size() == 2);

	BOOST_CHECK(cMerkleBlock.m_vMatchedTxn[0] == pair);

	BOOST_CHECK(cMerkleBlock.m_vMatchedTxn[1].second == uint256S("0x9eeed094d4dc4327ae5af54ba92b991ae545840d2fa882eb90c0f62d0885f5b9"));
	BOOST_CHECK(cMerkleBlock.m_vMatchedTxn[1].first == 2);

	BOOST_CHECK(cMerkleBlock.m_cTxNum.ExtractMatches(vcMatched) == cBlock.GetHashMerkleRoot());
	BOOST_CHECK(vcMatched.size() == cMerkleBlock.m_vMatchedTxn.size());
	for (unsigned int i = 0; i < vcMatched.size(); i++) {
		BOOST_CHECK(vcMatched[i] == cMerkleBlock.m_vMatchedTxn[i].second);
	}
}

BOOST_AUTO_TEST_CASE(merkle_block_2) {
	// Random real cBlock (c6c28d39d04e8ed6b92850fab5f214490e62697890eab9f1b393a8805da397f6)
	// With 4 txes
	CBlock cBlock;
	CDataStream cStream(ParseHex("020000004d0449f6a390f734568e3ad4bead7b4abd8cdfe48839\
ce8151b50ef2ec3f0debcd37f5d3b75a2f58ef560459bf11c29070eee1c2\
64b9cffc3c436b7a8a621a9976746c2e752750315464708766f359a6b69e\
c40e3f843ff2b56fd0cb6b0400002754325882661c1bb701000066040000\
00000000000000001b010000473045022100b2a5e1ad39ae2eeaafbff026\
ea510362ac5e6cc253e36f969679ebc8b4e2f28102203aa8aa2bb33328ba\
ed172124f9274e8bf61d7b62b175625d27f878b5adc1287603010202000d\
82dbebaf2087660302876502000d140031f37de36553a23438846f342ac3\
bd765fe1f5cd109399febadda9000046304402206aaa631b6dca6e004bc8\
536acfdc75008fa93e132e53f499b2ecfdcaa7ec1fa602200c6d0fcabad4\
47b81f513eabec6cfc04294bf65576317665563ba20a766e950a03028765\
02000d140031f37de36553a23438846f342ac3bd765fe1f5cd108181d8ad\
8fef0000473045022100fc89572a8152d9294b6dd1c2834882e04bb62134\
6af41ea41c6a2e6107df545a02207b640c21116b01878bc1b6b58b94ec3a\
c38bd52715bd94bc7505c28ffdb4b0d0"), SER_NETWORK, g_sProtocolVersion);
	//CDataStream cStream(ParseHex("02000000a627a8a255cc122abad79817386d3a1ecab5caef1fe73fbe9f8bf610f41df63da93aab3927e409d196362c3cf71c9f8dbcdb33ebc05b94d2e9d3dfd17d99871dd8ef3055ffff03200300000002000000fc6c000000000000640000004730450221009b3c1fad42208f41e55173d24e3384360252d4b59f08394060b81d003d13212d022059ce8e3846e14580ed9c26bc341e91eaaec7d9e9cf2ff2abb305a5c72f6b74f304010102000283ebeeb33102030101020002020003af80fd0f84a68eb3fd0000473045022100ad99ad8c3d81919c9c7e51efc58c8ef12f6dca4c27d5e2dcfe9625a36e9cf26902204ea3d6e44f9de31dbbfb63c1b561402615be161f39cb4ea06ff86343778db569030101020002020001af80fd0f84a68eb3fd0000473045022100c39ea9696091192b51008825e2a591ed5f12243564d1c2bbf0f012b8fdc8ac45022078a1ff57c2ec3c6b029eb69b9a37b2cb66fa7e6a92dc108f747105db7df11fc1040101020001020101af80fd0f84a68eb3fd000114463044022040e1a998778ff56cb2d7477e6ee5f001af09674c766d6cb1f0d72db2623543f40220504e24fa48d82acb28c7253d365038776c9b4209d428a83a7c37da2a23957960"), SER_NETWORK, g_sProtocolVersion);
	cStream >> cBlock;

	CBloomFilter cFilter(10, 0.000001, 0, EM_BLOOM_UPDATE_ALL);
	// Match the first transaction
	cFilter.insert(uint256S("0xed525c5920c8f353a1d6a080f5f6e1c3bb12fa5100efe8cce44ed6f04682b61b"));

	CMerkleBlock cMerkleBlock(cBlock, cFilter);
	BOOST_CHECK(cMerkleBlock.m_cBlockHeader.GetHash() == cBlock.GetHash());

	BOOST_CHECK(cMerkleBlock.m_vMatchedTxn.size() == 1);

	BOOST_CHECK(cMerkleBlock.m_vMatchedTxn[0].second == uint256S("0xed525c5920c8f353a1d6a080f5f6e1c3bb12fa5100efe8cce44ed6f04682b61b"));
	BOOST_CHECK(cMerkleBlock.m_vMatchedTxn[0].first == 0);

	vector<uint256> vcMatched;
	BOOST_CHECK(cMerkleBlock.m_cTxNum.ExtractMatches(vcMatched) == cBlock.GetHashMerkleRoot());
	BOOST_CHECK(vcMatched.size() == cMerkleBlock.m_vMatchedTxn.size());
	for (unsigned int i = 0; i < vcMatched.size(); i++) {
		BOOST_CHECK(vcMatched[i] == cMerkleBlock.m_vMatchedTxn[i].second);
	}
}

BOOST_AUTO_TEST_CASE(merkle_block_2_with_update_none) {
	// Random real cBlock (c6c28d39d04e8ed6b92850fab5f214490e62697890eab9f1b393a8805da397f6)
	// With 4 txes
	CBlock cBlock;
	CDataStream cStream(ParseHex("020000004d0449f6a390f734568e3ad4bead7b4abd8cdfe48839\
ce8151b50ef2ec3f0debcd37f5d3b75a2f58ef560459bf11c29070eee1c2\
64b9cffc3c436b7a8a621a9976746c2e752750315464708766f359a6b69e\
c40e3f843ff2b56fd0cb6b0400002754325882661c1bb701000066040000\
00000000000000001b010000473045022100b2a5e1ad39ae2eeaafbff026\
ea510362ac5e6cc253e36f969679ebc8b4e2f28102203aa8aa2bb33328ba\
ed172124f9274e8bf61d7b62b175625d27f878b5adc1287603010202000d\
82dbebaf2087660302876502000d140031f37de36553a23438846f342ac3\
bd765fe1f5cd109399febadda9000046304402206aaa631b6dca6e004bc8\
536acfdc75008fa93e132e53f499b2ecfdcaa7ec1fa602200c6d0fcabad4\
47b81f513eabec6cfc04294bf65576317665563ba20a766e950a03028765\
02000d140031f37de36553a23438846f342ac3bd765fe1f5cd108181d8ad\
8fef0000473045022100fc89572a8152d9294b6dd1c2834882e04bb62134\
6af41ea41c6a2e6107df545a02207b640c21116b01878bc1b6b58b94ec3a\
c38bd52715bd94bc7505c28ffdb4b0d0"), SER_NETWORK, g_sProtocolVersion);
	//CDataStream cStream(ParseHex("02000000a627a8a255cc122abad79817386d3a1ecab5caef1fe73fbe9f8bf610f41df63da93aab3927e409d196362c3cf71c9f8dbcdb33ebc05b94d2e9d3dfd17d99871dd8ef3055ffff03200300000002000000fc6c000000000000640000004730450221009b3c1fad42208f41e55173d24e3384360252d4b59f08394060b81d003d13212d022059ce8e3846e14580ed9c26bc341e91eaaec7d9e9cf2ff2abb305a5c72f6b74f304010102000283ebeeb33102030101020002020003af80fd0f84a68eb3fd0000473045022100ad99ad8c3d81919c9c7e51efc58c8ef12f6dca4c27d5e2dcfe9625a36e9cf26902204ea3d6e44f9de31dbbfb63c1b561402615be161f39cb4ea06ff86343778db569030101020002020001af80fd0f84a68eb3fd0000473045022100c39ea9696091192b51008825e2a591ed5f12243564d1c2bbf0f012b8fdc8ac45022078a1ff57c2ec3c6b029eb69b9a37b2cb66fa7e6a92dc108f747105db7df11fc1040101020001020101af80fd0f84a68eb3fd000114463044022040e1a998778ff56cb2d7477e6ee5f001af09674c766d6cb1f0d72db2623543f40220504e24fa48d82acb28c7253d365038776c9b4209d428a83a7c37da2a23957960"), SER_NETWORK, g_sProtocolVersion);
	cStream >> cBlock;

	CBloomFilter cFilter(10, 0.000001, 0, EM_BLOOM_UPDATE_NONE);
	// Match the first transaction
	cFilter.insert(uint256S("0xed525c5920c8f353a1d6a080f5f6e1c3bb12fa5100efe8cce44ed6f04682b61b"));

	CMerkleBlock cMerkleBlock(cBlock, cFilter);
	BOOST_CHECK(cMerkleBlock.m_cBlockHeader.GetHash() == cBlock.GetHash());

	BOOST_CHECK(cMerkleBlock.m_vMatchedTxn.size() == 1);

	BOOST_CHECK(cMerkleBlock.m_vMatchedTxn[0].second == uint256S("0xed525c5920c8f353a1d6a080f5f6e1c3bb12fa5100efe8cce44ed6f04682b61b"));
	BOOST_CHECK(cMerkleBlock.m_vMatchedTxn[0].first == 0);

	vector<uint256> vcMatched;
	BOOST_CHECK(cMerkleBlock.m_cTxNum.ExtractMatches(vcMatched) == cBlock.GetHashMerkleRoot());
	BOOST_CHECK(vcMatched.size() == cMerkleBlock.m_vMatchedTxn.size());
	for (unsigned int i = 0; i < vcMatched.size(); i++) {
		BOOST_CHECK(vcMatched[i] == cMerkleBlock.m_vMatchedTxn[i].second);
	}

}

BOOST_AUTO_TEST_CASE(merkle_block_3_and_serialize) {
	// Random real cBlock (cd0529322ed0393491966d4adb13141775dcc5a1408f495ede95e14b41185262)
	// With one tx
	CBlock cBlock;
	CDataStream cStream(ParseHex("0200000060203f9559534438ca3e15b94a9b0d304d3b62ad264a\
dc87ac62a30b8753712a175f70e4113da4ecc3df18f55458acee8c3d4f0a\
cc47a2416ece8189c22d343c9f9d3321322c3a5f275d6532250d49405406\
8fa74eb9fcc6b05696ddb4060000c154325839ec181b2f0200006c040000\
0000000000000000940000004630440220363ceed0b99a6fd24c80f7d88a\
5f437a81bc92de454d50e6a8af65984c850de702203693ebc10c3c92d027\
e7ac79057100ccb3a6b5f30f18a197e4993cd40cd520c801010202000782\
dbea9300876c"), SER_NETWORK, g_sProtocolVersion);//CDataStream cStream(ParseHex("02000000f3e278295c17a6eb153dbde3340ab5dc727f5e23fc9da7c26e5b7c21537a7564c75bbd39d41d1e41ca981b25d980ee45b123e77ff3d462ea8d6c4f8b18c024a96df13055ffff03206b0000000100000000000000000000006400000046304402206771d65c08f3b6ab44bdf3ddda58909ac08272feb85757a65f9185457f919a9f022033c55dc76a43561e25f04ff3cb7a0e086879f6b08e7c44cc82318165aad2770901010102000582dbea930001"), SER_NETWORK, g_sProtocolVersion);
	cStream >> cBlock;

	CBloomFilter cFilter(10, 0.000001, 0, EM_BLOOM_UPDATE_ALL);
	// Match the only transaction
	cFilter.insert(uint256S("0x3c342dc28981ce6e41a247cc0a4f3d8ceeac5854f518dfc3eca43d11e4705f17"));

	CMerkleBlock cMerkleBlock(cBlock, cFilter);
	BOOST_CHECK(cMerkleBlock.m_cBlockHeader.GetHash() == cBlock.GetHash());

	BOOST_CHECK(cMerkleBlock.m_vMatchedTxn.size() == 1);

	BOOST_CHECK(cMerkleBlock.m_vMatchedTxn[0].second == uint256S("0x3c342dc28981ce6e41a247cc0a4f3d8ceeac5854f518dfc3eca43d11e4705f17"));
	BOOST_CHECK(cMerkleBlock.m_vMatchedTxn[0].first == 0);

	vector<uint256> vcMatched;
	BOOST_CHECK(cMerkleBlock.m_cTxNum.ExtractMatches(vcMatched) == cBlock.GetHashMerkleRoot());
	BOOST_CHECK(vcMatched.size() == cMerkleBlock.m_vMatchedTxn.size());
	for (unsigned int i = 0; i < vcMatched.size(); i++)
	BOOST_CHECK(vcMatched[i] == cMerkleBlock.m_vMatchedTxn[i].second);

	CDataStream merkleStream(SER_NETWORK, g_sProtocolVersion);
	merkleStream << cMerkleBlock;
//	cout << "cMerkleBlock:" << HexStr(merkleStream) << endl;
	//Hex: BlockHeader + txnum + txhash + bits
	vector<unsigned char> vuch = ParseHex("0200000060203f9559534438ca3e15b94a9b0d304d3b62ad264adc87ac62a30b8753712a175f70e4113da4ecc3df18f55458acee8c3d4f0acc47a2416ece8189c22d343c9f9d3321322c3a5f275d6532250d494054068fa74eb9fcc6b05696ddb4060000c154325839ec181b2f0200006c0400000000000000000000940000004630440220363ceed0b99a6fd24c80f7d88a5f437a81bc92de454d50e6a8af65984c850de702203693ebc10c3c92d027e7ac79057100ccb3a6b5f30f18a197e4993cd40cd520c80100000001175f70e4113da4ecc3df18f55458acee8c3d4f0acc47a2416ece8189c22d343c0101");//ParseHex("02000000f3e278295c17a6eb153dbde3340ab5dc727f5e23fc9da7c26e5b7c21537a7564c75bbd39d41d1e41ca981b25d980ee45b123e77ff3d462ea8d6c4f8b18c024a96df13055ffff03206b0000000100000000000000000000006400000046304402206771d65c08f3b6ab44bdf3ddda58909ac08272feb85757a65f9185457f919a9f022033c55dc76a43561e25f04ff3cb7a0e086879f6b08e7c44cc82318165aad277090100000001c75bbd39d41d1e41ca981b25d980ee45b123e77ff3d462ea8d6c4f8b18c024a90101");

	vector<char> vchExpected(vuch.size());

	for (unsigned int i = 0; i < vuch.size(); i++) {
		vchExpected[i] = (char) vuch[i];
	}
	BOOST_CHECK_EQUAL_COLLECTIONS(vchExpected.begin(), vchExpected.end(), merkleStream.begin(), merkleStream.end());
}

BOOST_AUTO_TEST_CASE(merkle_block_4) {
	// Random real cBlock (6ea6fda338a47a0c57fd3bce48a7c40d3b106c7506b9b7e44005683b89ceda7d)
	// With 7 txes
	CBlock cBlock;
	CDataStream cStream(ParseHex("020000004d0449f6a390f734568e3ad4bead7b4abd8cdfe48839\
			ce8151b50ef2ec3f0debcd37f5d3b75a2f58ef560459bf11c29070eee1c2\
			64b9cffc3c436b7a8a621a9976746c2e752750315464708766f359a6b69e\
			c40e3f843ff2b56fd0cb6b0400002754325882661c1bb701000066040000\
			00000000000000001b010000473045022100b2a5e1ad39ae2eeaafbff026\
			ea510362ac5e6cc253e36f969679ebc8b4e2f28102203aa8aa2bb33328ba\
			ed172124f9274e8bf61d7b62b175625d27f878b5adc1287603010202000d\
			82dbebaf2087660302876502000d140031f37de36553a23438846f342ac3\
			bd765fe1f5cd109399febadda9000046304402206aaa631b6dca6e004bc8\
			536acfdc75008fa93e132e53f499b2ecfdcaa7ec1fa602200c6d0fcabad4\
			47b81f513eabec6cfc04294bf65576317665563ba20a766e950a03028765\
			02000d140031f37de36553a23438846f342ac3bd765fe1f5cd108181d8ad\
			8fef0000473045022100fc89572a8152d9294b6dd1c2834882e04bb62134\
			6af41ea41c6a2e6107df545a02207b640c21116b01878bc1b6b58b94ec3a\
			c38bd52715bd94bc7505c28ffdb4b0d0"), SER_NETWORK, g_sProtocolVersion);//CDataStream cStream(ParseHex("02000000c3d210686483d5f545c8fcc61a860279bf91a8fa2de6454aab67a8530c42d19845a683bc371eeda2f490a92c2fbcbd2ef4c99b88cd2671501be46b58ff92911d4af33055ffff0320120000000200000020fd00000000000064000000473045022100c065e91fb6a92a79d6fea4a4fdabbca754a454e8342b4bf423e72cc2c56ddd1502201724cec1d874e664782fb476751f34a182b015a13f71963310019d8ef3c94c8007010102000384fb9b830602030101020003020002aef2bb3181ffe8a2d1000046304402202e215f62bbd8c3b168e4c0595e2163c10be5cd74126b38c486686f65843c78f802206714b5f79ddee89e4f61d599d69b1409abe3baa4992688181b975a1a05d6a595030101020003020002aef2bb3181ffe8a2d100004630440220314ffcc1436f2f336308b95aaafcd6d765a421bcbe2b40e1927eb0d6f01e7bad022061a3cf83321193a14437643e307d434c81dde6126e88b59e1d3c7af66b6add9c030101020002020003aef2bb3181ffe8a2d10000463044022061e81046a0d3eca340d5697cf84fe202e61102f3d652e9a9b9c9126389cabb43022001e7c6a4fe645e13790d0a01049a185ada136d32f9292024e48fee2bdb9b8bd6030101020002020001aef2bb3181ffe8a2d10000473045022100a6ab6eecf603c9dfb27c0300cbff3b508e1c5fc67bc3149d09c433ed46d515b7022027c4c748cda207ea8c4bd767f0094e74ab5b6c75830b893d16b175f3001e2a26040101020003020101aef2bb3181ffe8a2d10001144630440220550ad6c28ae23b3312cf1b7581050c32f506144d08282e569253ba37c4d40691022059fa8f7814e732ccdca38ea4ece062d3da4cb76046391b1f83d411663e06f7b8040101020001020101aef2bb3181ffe8a2d100011547304502210092515704d7bde78db386ed843b07d1ff38a969774188ff6484c90798b4dbf23a0220580c4bcf57fd7616299edaedf7cea5368491747ce1e271e5e4704c69c92a7dc7"), SER_NETWORK, g_sProtocolVersion);
	cStream >> cBlock;

	CBloomFilter cFilter(10, 0.000001, 0, EM_BLOOM_UPDATE_ALL);
	// Match the last transaction
	cFilter.insert(uint256S("0x9eeed094d4dc4327ae5af54ba92b991ae545840d2fa882eb90c0f62d0885f5b9"));

	CMerkleBlock cMerkleBlock(cBlock, cFilter);
	BOOST_CHECK(cMerkleBlock.m_cBlockHeader.GetHash() == cBlock.GetHash());

	BOOST_CHECK(cMerkleBlock.m_vMatchedTxn.size() == 1);
	pair<unsigned int, uint256> pair = cMerkleBlock.m_vMatchedTxn[0];

	BOOST_CHECK(cMerkleBlock.m_vMatchedTxn[0].second == uint256S("0x9eeed094d4dc4327ae5af54ba92b991ae545840d2fa882eb90c0f62d0885f5b9"));
	BOOST_CHECK(cMerkleBlock.m_vMatchedTxn[0].first == 2);

	vector<uint256> vcMatched;
	BOOST_CHECK(cMerkleBlock.m_cTxNum.ExtractMatches(vcMatched) == cBlock.GetHashMerkleRoot());
	BOOST_CHECK(vcMatched.size() == cMerkleBlock.m_vMatchedTxn.size());
	for (unsigned int i = 0; i < vcMatched.size(); i++)
	BOOST_CHECK(vcMatched[i] == cMerkleBlock.m_vMatchedTxn[i].second);

	// Also match the 4th transaction
	cFilter.insert(uint256S("0xb09135750f39e4ccb81c39ace2820f8eabecf0ee8e4138427091130ffe45c13c"));
	cMerkleBlock = CMerkleBlock(cBlock, cFilter);
	BOOST_CHECK(cMerkleBlock.m_cBlockHeader.GetHash() == cBlock.GetHash());

	BOOST_CHECK(cMerkleBlock.m_vMatchedTxn.size() == 2);

	BOOST_CHECK(cMerkleBlock.m_vMatchedTxn[0].second == uint256S("0xb09135750f39e4ccb81c39ace2820f8eabecf0ee8e4138427091130ffe45c13c"));
	BOOST_CHECK(cMerkleBlock.m_vMatchedTxn[0].first == 1);

	BOOST_CHECK(cMerkleBlock.m_vMatchedTxn[1] == pair);

	BOOST_CHECK(cMerkleBlock.m_cTxNum.ExtractMatches(vcMatched) == cBlock.GetHashMerkleRoot());
	BOOST_CHECK(vcMatched.size() == cMerkleBlock.m_vMatchedTxn.size());
	for (unsigned int i = 0; i < vcMatched.size(); i++) {
		BOOST_CHECK(vcMatched[i] == cMerkleBlock.m_vMatchedTxn[i].second);
	}
}

BOOST_AUTO_TEST_SUITE_END()
