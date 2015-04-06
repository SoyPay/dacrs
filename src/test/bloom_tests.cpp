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

BOOST_AUTO_TEST_CASE(bloom_create_insert_serialize)
{
	CBloomFilter filter(3, 0.01, 0, BLOOM_UPDATE_ALL);

	filter.insert(ParseHex("99108ad8ed9bb6274d3980bab5a85c048f0950c8"));
	BOOST_CHECK_MESSAGE( filter.contains(ParseHex("99108ad8ed9bb6274d3980bab5a85c048f0950c8")), "BloomFilter doesn't contain just-inserted object!");
	// One bit different in first byte
	BOOST_CHECK_MESSAGE(!filter.contains(ParseHex("19108ad8ed9bb6274d3980bab5a85c048f0950c8")), "BloomFilter contains something it shouldn't!");

	filter.insert(ParseHex("b5a2c786d9ef4658287ced5914b37a1b4aa32eee"));
	BOOST_CHECK_MESSAGE(filter.contains(ParseHex("b5a2c786d9ef4658287ced5914b37a1b4aa32eee")), "BloomFilter doesn't contain just-inserted object (2)!");

	filter.insert(ParseHex("b9300670b4c5366e95b2699e8b18bc75e5f729c5"));
	BOOST_CHECK_MESSAGE(filter.contains(ParseHex("b9300670b4c5366e95b2699e8b18bc75e5f729c5")), "BloomFilter doesn't contain just-inserted object (3)!");

	CDataStream stream(SER_NETWORK, PROTOCOL_VERSION);
	filter.Serialize(stream, SER_NETWORK, PROTOCOL_VERSION);

	vector<unsigned char> vch = ParseHex("03614e9b050000000000000001");
	vector<char> expected(vch.size());

	for (unsigned int i = 0; i < vch.size(); i++)
	expected[i] = (char)vch[i];

	BOOST_CHECK_EQUAL_COLLECTIONS(stream.begin(), stream.end(), expected.begin(), expected.end());
}

BOOST_AUTO_TEST_CASE(bloom_create_insert_serialize_with_tweak)
{
	// Same test as bloom_create_insert_serialize, but we add a nTweak of 100
	CBloomFilter filter(3, 0.01, 2147483649, BLOOM_UPDATE_ALL);

	filter.insert(ParseHex("99108ad8ed9bb6274d3980bab5a85c048f0950c8"));
	BOOST_CHECK_MESSAGE( filter.contains(ParseHex("99108ad8ed9bb6274d3980bab5a85c048f0950c8")), "BloomFilter doesn't contain just-inserted object!");
	// One bit different in first byte
	BOOST_CHECK_MESSAGE(!filter.contains(ParseHex("19108ad8ed9bb6274d3980bab5a85c048f0950c8")), "BloomFilter contains something it shouldn't!");

	filter.insert(ParseHex("b5a2c786d9ef4658287ced5914b37a1b4aa32eee"));
	BOOST_CHECK_MESSAGE(filter.contains(ParseHex("b5a2c786d9ef4658287ced5914b37a1b4aa32eee")), "BloomFilter doesn't contain just-inserted object (2)!");

	filter.insert(ParseHex("b9300670b4c5366e95b2699e8b18bc75e5f729c5"));
	BOOST_CHECK_MESSAGE(filter.contains(ParseHex("b9300670b4c5366e95b2699e8b18bc75e5f729c5")), "BloomFilter doesn't contain just-inserted object (3)!");

	CDataStream stream(SER_NETWORK, PROTOCOL_VERSION);
	filter.Serialize(stream, SER_NETWORK, PROTOCOL_VERSION);

	vector<unsigned char> vch = ParseHex("03ce4299050000000100008001");
	vector<char> expected(vch.size());

	for (unsigned int i = 0; i < vch.size(); i++)
	expected[i] = (char)vch[i];

	BOOST_CHECK_EQUAL_COLLECTIONS(stream.begin(), stream.end(), expected.begin(), expected.end());
}

BOOST_AUTO_TEST_CASE(bloom_create_insert_key)
{

	if (SysCfg().Network::TESTNET == SysCfg().NetworkID() || SysCfg().Network::REGTEST == SysCfg().NetworkID()) {
		return ;
	}
	string strSecret = string("5Kg1gnAjaLfKiwhhPpGS3QfRg2m6awQvaj98JCZBZQ5SuS2F15C");
	CDacrsSecret vchSecret;
	BOOST_CHECK(vchSecret.SetString(strSecret));

	CKey key = vchSecret.GetKey();
	CPubKey pubkey = key.GetPubKey();
	vector<unsigned char> vchPubKey(pubkey.begin(), pubkey.end());

	CBloomFilter filter(2, 0.001, 0, BLOOM_UPDATE_ALL);
	filter.insert(vchPubKey);
	uint160 hash = pubkey.GetKeyID();
	filter.insert(vector<unsigned char>(hash.begin(), hash.end()));

	CDataStream stream(SER_NETWORK, PROTOCOL_VERSION);
	filter.Serialize(stream, SER_NETWORK, PROTOCOL_VERSION);

	vector<unsigned char> vch = ParseHex("038fc16b080000000000000001");
	vector<char> expected(vch.size());

	for (unsigned int i = 0; i < vch.size(); i++)
	expected[i] = (char)vch[i];

	BOOST_CHECK_EQUAL_COLLECTIONS(stream.begin(), stream.end(), expected.begin(), expected.end());
}

BOOST_AUTO_TEST_CASE(merkle_block_1)
{
	// Random real block (acb4dc2222b086d868952603157870f956b77e244ec3e30056e37ada11ff227d)
	// With 9 txes
	CBlock block;
	CDataStream stream(ParseHex("02000000f66e95fc82be7c7910cdc9ae31262939bc1a9749898c76ec40e7da16703a2c6ae5c836467afa8edc41a8f2ba2afb80fdd40338ab0371edd379a03bcba84bf34911e02155ffff00203f010000020000009cda010000000000463044022018d84723b76de1d7581efa400ec08fb8be7e8ac864dd5e591a49b00bb82d7a18022012a502114bd5df91a01179918f84aec69b81abb0d51c095ad81eef397b0be1a209010102000185dc9aef7c02030101020003020002af86812382d9fbddb5000046304402205f7bb40d58fa7bce100813b208ed174696b6b4695dafaf6ec698629e5e68b84c022067b44fb3d58e48a2db2110324cc27601466c7e53b5ec850fad8a8c49ce823f00030101020002020001af86812382d9fbddb5000046304402201cdb5778c4f58c20ca86438587612196c7cba54e98ab422b71fdb6b52f2af91902202a1f1c1d3b2a30d5f5b0cc3337f3996250a8f882a7917db847b1052ae6aff1bd030101020003020002af86812382d9fbddb500004730450221009b2edd6209504b79d8b0203e4f8f8c82084f02275258d7d06472e24520d6a0bc0220778a64487d338e673607c36f9b3d0791f009db305d96dbeeb9a7866aea2f46e8030101020003020001af86812382d9fbddb50000473045022100b27e0b72c87a9b0acdd13ffb50346b9441ac41dd27531bcfbf633186e918095c022014c530d4bf5b84c54624e787c0741a7bf8063ad166a389875110435eea5d04ef040101020002020101af86812382d9fbddb500011446304402205447d56ea83b5b10ca0a9864e35aed4a1a65e9978c93b43fc7e43da5a65bd88f02204d761814dd7d60bd6b06e4f90c413233925b57e5527485a9c3d886429c429ee0040101020001020101af86812382d9fbddb500011446304402204e5c33a1eb0334bc5befce082eb31aba8e32dbed2f7352d88cd9944176f3c38f02203a056348f4f2c687fff97693d8e4028f0273225f16aaccd9767a8450cf5e4475040101020002020101af86812382d9fbddb5000114473045022100925855032f8a8f38a4e3b2f3e5308ee7fda097d667013e2e087a4994b5a68886022028e35278f689fe454b86cdbe697f760a272013a5adb7cca33eb5709bd43290b9040101020003020101af86812382d9fbddb5000115473045022100b7d0183bb00b19782f2d7c090e562651b21747512b171ea4f8d9cf17c2004f3102202fbd70b54c33706bceba7f3e21eb083790213778eb68c8883db9e159bb817e30"), SER_NETWORK, PROTOCOL_VERSION);
	stream >> block;

	CBloomFilter filter(10, 0.000001, 0, BLOOM_UPDATE_ALL);
	// Match the last transaction
	filter.insert(uint256("0xf5c623e598d265859a259a2e4340cd9bea8812143cf4c3a49e20b1e50151079b"));

	CMerkleBlock merkleBlock(block, filter);
	BOOST_CHECK(merkleBlock.header.GetHash() == block.GetHash());

	BOOST_CHECK(merkleBlock.vMatchedTxn.size() == 1);
	pair<unsigned int, uint256> pair = merkleBlock.vMatchedTxn[0];

	BOOST_CHECK(merkleBlock.vMatchedTxn[0].second == uint256("0xf5c623e598d265859a259a2e4340cd9bea8812143cf4c3a49e20b1e50151079b"));
	BOOST_CHECK(merkleBlock.vMatchedTxn[0].first == 8);

	vector<uint256> vMatched;
	BOOST_CHECK(merkleBlock.txn.ExtractMatches(vMatched) == block.hashMerkleRoot);
	BOOST_CHECK(vMatched.size() == merkleBlock.vMatchedTxn.size());
	for (unsigned int i = 0; i < vMatched.size(); i++)
	BOOST_CHECK(vMatched[i] == merkleBlock.vMatchedTxn[i].second);

	// Also match the 8th transaction
	filter.insert(uint256("0x16948b899a92456f866450b03f3be75495c9360c64039f928901474d78be0042"));
	merkleBlock = CMerkleBlock(block, filter);
	BOOST_CHECK(merkleBlock.header.GetHash() == block.GetHash());

	BOOST_CHECK(merkleBlock.vMatchedTxn.size() == 2);

	BOOST_CHECK(merkleBlock.vMatchedTxn[1] == pair);

	BOOST_CHECK(merkleBlock.vMatchedTxn[0].second == uint256("0x16948b899a92456f866450b03f3be75495c9360c64039f928901474d78be0042"));
	BOOST_CHECK(merkleBlock.vMatchedTxn[0].first == 7);

	BOOST_CHECK(merkleBlock.txn.ExtractMatches(vMatched) == block.hashMerkleRoot);
	BOOST_CHECK(vMatched.size() == merkleBlock.vMatchedTxn.size());
	for (unsigned int i = 0; i < vMatched.size(); i++)
	BOOST_CHECK(vMatched[i] == merkleBlock.vMatchedTxn[i].second);
}

BOOST_AUTO_TEST_CASE(merkle_block_2)
{
	// Random real block (38563067ac8cc2d493f7ccd5d98cfd1924977f9a38ff764533b53064f9e083d0)
	// With 4 txes
	CBlock block;
	CDataStream stream(ParseHex("02000000be931dda19c392b943163316f81c049cddd56a9864d906c4df051f4d7dd60e18444e4fa0f33aa511bb437eba6623e3d5d15f5680767df10266c650ce9e311ec12cf12155ffff00207600000002000000e0dd00000000000046304402200569f888925e3979db9447b7a089025714cd3bd454292c2eeacf94aaaee72b3002202622788ae61a8eab2b473e09168aef83aa03237a8b846448d610b6021ffb0d3704010102000383ec998b2502030101020002020003af8fe55783ccdaa89d000047304502210097508a7cbd1fa1cba18c8a658b9c85f334404441dd5835d63053a049a52763ef02207a79e16b4b3f9825ba8a0921887f31f702e8127f7f287e922712c9c968f3f18f040101020001020101af8fe55783ccdaa89d00011446304402207ba01a740c544fc943cfb674a7b2dad0797a2e4cba9bc486cea83b3aff8f7a150220208551c9a6ea1b60cbd0514405b8e194261fa5b6c9f843242fd9ac1ea8a5df4c040101020001020101af8fe55783ccdaa89d00011446304402206b040ea8307a0c9d9dec17f5659705555968b7c6423d83ee3cc3b680c74920550220458f40bf7c2e2b4170d888b68226113fb12e405abe88175089c68476946f9df3"), SER_NETWORK, PROTOCOL_VERSION);
	stream >> block;

	CBloomFilter filter(10, 0.000001, 0, BLOOM_UPDATE_ALL);
	// Match the first transaction
	filter.insert(uint256("0xa99366c0309871904c2f7b1c4a66ec39338d9f3468b25b556d76b88f7896d7e9"));

	CMerkleBlock merkleBlock(block, filter);
	BOOST_CHECK(merkleBlock.header.GetHash() == block.GetHash());

	BOOST_CHECK(merkleBlock.vMatchedTxn.size() == 1);

	BOOST_CHECK(merkleBlock.vMatchedTxn[0].second == uint256("0xa99366c0309871904c2f7b1c4a66ec39338d9f3468b25b556d76b88f7896d7e9"));
	BOOST_CHECK(merkleBlock.vMatchedTxn[0].first == 0);

	vector<uint256> vMatched;
	BOOST_CHECK(merkleBlock.txn.ExtractMatches(vMatched) == block.hashMerkleRoot);
	BOOST_CHECK(vMatched.size() == merkleBlock.vMatchedTxn.size());
	for (unsigned int i = 0; i < vMatched.size(); i++)
	BOOST_CHECK(vMatched[i] == merkleBlock.vMatchedTxn[i].second);
}

BOOST_AUTO_TEST_CASE(merkle_block_2_with_update_none)
{
	// Random real block (38563067ac8cc2d493f7ccd5d98cfd1924977f9a38ff764533b53064f9e083d0)
	// With 4 txes
	CBlock block;
	CDataStream stream(ParseHex("02000000be931dda19c392b943163316f81c049cddd56a9864d906c4df051f4d7dd60e18444e4fa0f33aa511bb437eba6623e3d5d15f5680767df10266c650ce9e311ec12cf12155ffff00207600000002000000e0dd00000000000046304402200569f888925e3979db9447b7a089025714cd3bd454292c2eeacf94aaaee72b3002202622788ae61a8eab2b473e09168aef83aa03237a8b846448d610b6021ffb0d3704010102000383ec998b2502030101020002020003af8fe55783ccdaa89d000047304502210097508a7cbd1fa1cba18c8a658b9c85f334404441dd5835d63053a049a52763ef02207a79e16b4b3f9825ba8a0921887f31f702e8127f7f287e922712c9c968f3f18f040101020001020101af8fe55783ccdaa89d00011446304402207ba01a740c544fc943cfb674a7b2dad0797a2e4cba9bc486cea83b3aff8f7a150220208551c9a6ea1b60cbd0514405b8e194261fa5b6c9f843242fd9ac1ea8a5df4c040101020001020101af8fe55783ccdaa89d00011446304402206b040ea8307a0c9d9dec17f5659705555968b7c6423d83ee3cc3b680c74920550220458f40bf7c2e2b4170d888b68226113fb12e405abe88175089c68476946f9df3"), SER_NETWORK, PROTOCOL_VERSION);
	stream >> block;

	CBloomFilter filter(10, 0.000001, 0, BLOOM_UPDATE_NONE);
	// Match the first transaction
	filter.insert(uint256("0xa99366c0309871904c2f7b1c4a66ec39338d9f3468b25b556d76b88f7896d7e9"));

	CMerkleBlock merkleBlock(block, filter);
	BOOST_CHECK(merkleBlock.header.GetHash() == block.GetHash());

	BOOST_CHECK(merkleBlock.vMatchedTxn.size() == 1);

	BOOST_CHECK(merkleBlock.vMatchedTxn[0].second == uint256("0xa99366c0309871904c2f7b1c4a66ec39338d9f3468b25b556d76b88f7896d7e9"));
	BOOST_CHECK(merkleBlock.vMatchedTxn[0].first == 0);

	vector<uint256> vMatched;
	BOOST_CHECK(merkleBlock.txn.ExtractMatches(vMatched) == block.hashMerkleRoot);
	BOOST_CHECK(vMatched.size() == merkleBlock.vMatchedTxn.size());
	for (unsigned int i = 0; i < vMatched.size(); i++)
	BOOST_CHECK(vMatched[i] == merkleBlock.vMatchedTxn[i].second);

}

BOOST_AUTO_TEST_CASE(merkle_block_3_and_serialize)
{
	// Random real block (3b731216a5febdc5db28fca904d6a84779f72c2734427acded3100a33d90c2e6)
	// With one tx
	CBlock block;
	CDataStream stream(ParseHex("02000000d58456a8e4322f13a2204d00d058ca8fc50e5f720f54ad19945df543f928890fc75bbd39d41d1e41ca981b25d980ee45b123e77ff3d462ea8d6c4f8b18c024a921f32155ffff0020b5010000010000000000000000000000473045022100dae9bb9b9cd8f50bfba6240565833ab57124b9d4d013283826a722a77fa2bae7022010b76cc4539d443eff29cf00036a80c9d146656d95fd408068a9d70cf84f6a0801010102000582dbea930001"), SER_NETWORK, PROTOCOL_VERSION);
	stream >> block;

	CBloomFilter filter(10, 0.000001, 0, BLOOM_UPDATE_ALL);
	// Match the only transaction
	filter.insert(uint256("0xa924c0188b4f6c8dea62d4f37fe723b145ee80d9251b98ca411e1dd439bd5bc7"));

	CMerkleBlock merkleBlock(block, filter);
	BOOST_CHECK(merkleBlock.header.GetHash() == block.GetHash());

	BOOST_CHECK(merkleBlock.vMatchedTxn.size() == 1);

	BOOST_CHECK(merkleBlock.vMatchedTxn[0].second == uint256("0xa924c0188b4f6c8dea62d4f37fe723b145ee80d9251b98ca411e1dd439bd5bc7"));
	BOOST_CHECK(merkleBlock.vMatchedTxn[0].first == 0);

	vector<uint256> vMatched;
	BOOST_CHECK(merkleBlock.txn.ExtractMatches(vMatched) == block.hashMerkleRoot);
	BOOST_CHECK(vMatched.size() == merkleBlock.vMatchedTxn.size());
	for (unsigned int i = 0; i < vMatched.size(); i++)
	BOOST_CHECK(vMatched[i] == merkleBlock.vMatchedTxn[i].second);

	CDataStream merkleStream(SER_NETWORK, PROTOCOL_VERSION);
	merkleStream << merkleBlock;
//	cout << "MerkleBlock:" << HexStr(merkleStream) << endl;
	//Hex: BlockHeader + txnum + txhash + bits
	vector<unsigned char> vch = ParseHex("02000000d58456a8e4322f13a2204d00d058ca8fc50e5f720f54ad19945df543f928890fc75bbd39d41d1e41ca981b25d980ee45b123e77ff3d462ea8d6c4f8b18c024a921f32155ffff0020b5010000010000000000000000000000473045022100dae9bb9b9cd8f50bfba6240565833ab57124b9d4d013283826a722a77fa2bae7022010b76cc4539d443eff29cf00036a80c9d146656d95fd408068a9d70cf84f6a080100000001c75bbd39d41d1e41ca981b25d980ee45b123e77ff3d462ea8d6c4f8b18c024a90101");
	vector<char> expected(vch.size());

	for (unsigned int i = 0; i < vch.size(); i++)
	expected[i] = (char)vch[i];

	BOOST_CHECK_EQUAL_COLLECTIONS(expected.begin(), expected.end(), merkleStream.begin(), merkleStream.end());
}

BOOST_AUTO_TEST_CASE(merkle_block_4)
{
	// Random real block (8cf825067f5d6943d466ed516cdfa57faa7584f558c3429a3414a156e88e9c6c)
	// With 7 txes
	CBlock block;
	CDataStream stream(ParseHex("020000002e1ca7238b4718c20a1002fcf740aab32717010c3a4da836183ef219664198a13be6d414deb6b145a89b58966d9d28642bb86faa5f2ebce45188b1c56718f80525f52155ffff00205e01000002000000981b010000000000463044022053f0fcdf9d0f190ad504b6710bbac522b7d3c7a69484cf19bb3341657d3cd94b022075976f4c1f10ad0fe055a0f3c049a3b0180cab44c6f8fce3e4671bfe74379f2b07010102000284fb98fb0002030101020001020003aef2994483f8a5a7c7000046304402201c33b63fec74e92fb24cb29d84b29bc3a82db1c1c549579067884ffdfcea11d0022034b158ab37d35166d8673b2bba169bfe8e36af9f3ee7d5cc1cee801b914b3aa0030101020003020002aef2994483f8a5a7c70000463044022078029d2280cf72abfe13e1d8a39f1b69886dcb9339d8ec21f64bf69de99e286302206e055f44837c0ac5b9c696947e11b7f23b23b194c61fe1351f2d215f96a8388a030101020003020001aef2994483f8a5a7c70000463044022008e0bd6fcab9a7716333de28cc1d3fef782060d04cda4928c9455e4b1f0990d8022048dc58b3bb5f17bfa7b924de44c80ead73d3ccf818d3adf34e5e3de0a090b619030101020002020003aef2994483f8a5a7c70000473045022100a5a98d87e0e15c5967ebad40fee12d639ac5384a0af70b0df1cb440b943573c40220467985d6c09bc0894d672c3d67d92f740eb7648c5100b150d028c63def766d52040101020001020101aef2994483f8a5a7c700011546304402201cdc5850bd48137641a6725d2ac32cb1648060c2dfc2021cc318e070d7a41d910220430eb3574ce6a09f9ccb20078a778d7661698edb0ecc94a2649c1e4d88ca24d6040101020003020101aef2994483f8a5a7c7000115473045022100b079bced1e8c2673a29b2ec9d305466ee47c5b1d3fdc549b602b85644c3a053b022044382e23991da38df31082395fff3f3fd5b6ab74a2799a2794a6feb2c6d4de5a;"), SER_NETWORK, PROTOCOL_VERSION);
	stream >> block;

	CBloomFilter filter(10, 0.000001, 0, BLOOM_UPDATE_ALL);
	// Match the last transaction
	filter.insert(uint256("0x07d18b6c69073af75353b6ccecb01fcc1eb5ae035d2c4bb1b3e672b2e1d6aa57"));

	CMerkleBlock merkleBlock(block, filter);
	BOOST_CHECK(merkleBlock.header.GetHash() == block.GetHash());

	BOOST_CHECK(merkleBlock.vMatchedTxn.size() == 1);
	pair<unsigned int, uint256> pair = merkleBlock.vMatchedTxn[0];

	BOOST_CHECK(merkleBlock.vMatchedTxn[0].second == uint256("0x07d18b6c69073af75353b6ccecb01fcc1eb5ae035d2c4bb1b3e672b2e1d6aa57"));
	BOOST_CHECK(merkleBlock.vMatchedTxn[0].first == 6);

	vector<uint256> vMatched;
	BOOST_CHECK(merkleBlock.txn.ExtractMatches(vMatched) == block.hashMerkleRoot);
	BOOST_CHECK(vMatched.size() == merkleBlock.vMatchedTxn.size());
	for (unsigned int i = 0; i < vMatched.size(); i++)
	BOOST_CHECK(vMatched[i] == merkleBlock.vMatchedTxn[i].second);

	// Also match the 4th transaction
	filter.insert(uint256("0x1056bc1fbf369870f0725e1ae1d1bcbab5a1bcbb412fa4bbf8173234ee6a3c3b"));
	merkleBlock = CMerkleBlock(block, filter);
	BOOST_CHECK(merkleBlock.header.GetHash() == block.GetHash());

	BOOST_CHECK(merkleBlock.vMatchedTxn.size() == 2);

	BOOST_CHECK(merkleBlock.vMatchedTxn[0].second == uint256("0x1056bc1fbf369870f0725e1ae1d1bcbab5a1bcbb412fa4bbf8173234ee6a3c3b"));
	BOOST_CHECK(merkleBlock.vMatchedTxn[0].first == 3);

	BOOST_CHECK(merkleBlock.vMatchedTxn[1] == pair);

	BOOST_CHECK(merkleBlock.txn.ExtractMatches(vMatched) == block.hashMerkleRoot);
	BOOST_CHECK(vMatched.size() == merkleBlock.vMatchedTxn.size());
	for (unsigned int i = 0; i < vMatched.size(); i++)
	BOOST_CHECK(vMatched[i] == merkleBlock.vMatchedTxn[i].second);
}

BOOST_AUTO_TEST_SUITE_END()
