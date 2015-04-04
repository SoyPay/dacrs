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
	CDataStream stream(ParseHex("020000007d1c32e1ed287be080e18c291a2f07396bcf51272afddea282bf1c62e86733813f609ebc37297b2df8f7cbf1536b2dbe1a6ea0b58ea21778c748b222248005e3790d1e55ffff0020290000000200000046304402207e9866eb724e4d6064297512e76628488697b4be56e59fff6db9147ab153775502204caf46bd8990d50491064d57eb1e593fc4a67278a9aa1a2e6e0e7f06ae858389090101020001858bd85002030101020003020001e0ca1a84b98ea3a3000046304402200ebf881f64a27d47b46bd88b0a441bf8180f15b671a8f69db43bcc6f526d634602205b6aee4f0c0ab291ac8be29908bfca944cba306dfbe50256b1b3dbc723b6a923030101020002020003e0ca1a84b98ea3a300004630440220102f9fd34e6a883caa4f925d440a627b00d95c17d87e9ba09e0a88aa0ed9d06702200bb3adb035433d4c9873206091b735850cdb094b7b0743b592e5d2c5f2267e37030101020003020002e0ca1a84b98ea3a300004630440220285441802609391fde53ce99eccfb98d64f73e9c7fb931f4f0164afdf440191402205b7b13be2a1eae3d831fed4f6711a7dbc9c541ba39a00a3695b80c3081c0a3a9040101020001020101e0ca1a84b98ea3a3000115463044022010d855ebbadea3d4a24e18bac513307abbbf7efa69750d306fec9498f5f137dd0220265e8fb633160f721d40090f48df384eb6833e76d8a30390d85adc846427729b030101020001020002e0ca1a84b98ea3a30000473045022100b513f344982bf08e7ab15ad3132fcc093e30d51a616937f7fe1548dba0c4e41402200f1fdc562cae6fbd818aecd726776f87d38a7e478ee1e14de881fb1b3216dee9030101020002020003e0ca1a84b98ea3a30000473045022100fe1af1025cc3bff6b79067ec95db1dfeb4c9b8d7c95db522b338037ca304d1a002202bcb2735574effa652affebe371e4cc34fb376efc4d9e2022a6b9a6740d78be4040101020001020101e0ca1a84b98ea3a3000114473045022100fe1c912a54ddb0c5bf7dadd3508366bc66f116c81e13eab9146c54bea11aed2f022004dcdb409016de9c375ebfc7b46307bd6d107d3311243ca6e41a0d744fc84bae040101020001020101e0ca1a84b98ea3a3000114473045022100a59e59261e402a29940e4fd35368cdced9f22e1d50102a3c31e43c24e8ab7fcf022009454eaa65035c66dabd8c1cd8ae8952e0dbb6f90d4a41be1804bfbca23573e2"), SER_NETWORK, PROTOCOL_VERSION);
	stream >> block;

	CBloomFilter filter(10, 0.000001, 0, BLOOM_UPDATE_ALL);
	// Match the last transaction
	filter.insert(uint256("0x86e3f1833e17acea40f1ab8bae3321ea688a0c5634efc9c35edd137425d625d5"));

	CMerkleBlock merkleBlock(block, filter);
	BOOST_CHECK(merkleBlock.header.GetHash() == block.GetHash());

	BOOST_CHECK(merkleBlock.vMatchedTxn.size() == 1);
	pair<unsigned int, uint256> pair = merkleBlock.vMatchedTxn[0];

	BOOST_CHECK(merkleBlock.vMatchedTxn[0].second == uint256("0x86e3f1833e17acea40f1ab8bae3321ea688a0c5634efc9c35edd137425d625d5"));
	BOOST_CHECK(merkleBlock.vMatchedTxn[0].first == 8);

	vector<uint256> vMatched;
	BOOST_CHECK(merkleBlock.txn.ExtractMatches(vMatched) == block.hashMerkleRoot);
	BOOST_CHECK(vMatched.size() == merkleBlock.vMatchedTxn.size());
	for (unsigned int i = 0; i < vMatched.size(); i++)
	BOOST_CHECK(vMatched[i] == merkleBlock.vMatchedTxn[i].second);

	// Also match the 8th transaction
	filter.insert(uint256("0xdce095e6944ed383dfefe0a53e97c644dc7955036d1797f7055873ee386e0891"));
	merkleBlock = CMerkleBlock(block, filter);
	BOOST_CHECK(merkleBlock.header.GetHash() == block.GetHash());

	BOOST_CHECK(merkleBlock.vMatchedTxn.size() == 2);

	BOOST_CHECK(merkleBlock.vMatchedTxn[1] == pair);

	BOOST_CHECK(merkleBlock.vMatchedTxn[0].second == uint256("0xdce095e6944ed383dfefe0a53e97c644dc7955036d1797f7055873ee386e0891"));
	BOOST_CHECK(merkleBlock.vMatchedTxn[0].first == 7);

	BOOST_CHECK(merkleBlock.txn.ExtractMatches(vMatched) == block.hashMerkleRoot);
	BOOST_CHECK(vMatched.size() == merkleBlock.vMatchedTxn.size());
	for (unsigned int i = 0; i < vMatched.size(); i++)
	BOOST_CHECK(vMatched[i] == merkleBlock.vMatchedTxn[i].second);
}

BOOST_AUTO_TEST_CASE(merkle_block_2)
{
	// Random real block (a0a180107a21c5bcfb4cd92a5f0cefcb0aaa8af64d92687aa5d90023c0c1e211)
	// With 4 txes
	CBlock block;
	CDataStream stream(ParseHex("02000000de2ca3a3d65e0188beac69a01307af604a03dcbf374d478bcd7e5ab19d7a9dc5a61478d1c6b796b4b4ed6ae347608360f956ad8dbd277bfa5d225284633a4a436c2d1e55ffff00201f0000000200000046304402205e3cea36409bc900effe92f71fc158740dc3bd5f4263dfa1c5478078b00dd85a02201f1b17a0dfe0a666d0ab9556161fa57702c310a975ec10ba92bc1bbe7119011404010102000180b7bf1802030101020002020001bcbf0883dbfdacaf0000463044022029a6f879e1130692bfa5b8f43f15a404a1e625b74e373de6ae42e4129935e0640220289ec13b41fc07f3ffbd7e1b04ffd6d4a2c0d2b031bf057db5c98ea1b93a5bd7040101020002020101bcbf0883dbfdacaf000115463044022074a008a65d1bd2ddf28049e6da3be22682fdd99450ac5df66b0ca7ba2576f0dd0220207054d07407fc9bb83c232e65d5676b2a76607f0595be8511177b1dfd21b8ed040101020003020101bcbf0883dbfdacaf000115473045022100b76c0a8caf9f5d91ca5f83927e444c43269b7d58b18889351a3cc1bee5cc3b59022057028c527a0f21098806a924db64d2d76f48f27fcc271c40af3e621608a0d159"), SER_NETWORK, PROTOCOL_VERSION);
	stream >> block;

	CBloomFilter filter(10, 0.000001, 0, BLOOM_UPDATE_ALL);
	// Match the first transaction
	filter.insert(uint256("0x364bb7c1643262ede7d080478de9e0837bdbbc7d1df84a1ff5acc2c2bd570e56"));

	CMerkleBlock merkleBlock(block, filter);
	BOOST_CHECK(merkleBlock.header.GetHash() == block.GetHash());

	BOOST_CHECK(merkleBlock.vMatchedTxn.size() == 1);

	BOOST_CHECK(merkleBlock.vMatchedTxn[0].second == uint256("0x364bb7c1643262ede7d080478de9e0837bdbbc7d1df84a1ff5acc2c2bd570e56"));
	BOOST_CHECK(merkleBlock.vMatchedTxn[0].first == 0);

	vector<uint256> vMatched;
	BOOST_CHECK(merkleBlock.txn.ExtractMatches(vMatched) == block.hashMerkleRoot);
	BOOST_CHECK(vMatched.size() == merkleBlock.vMatchedTxn.size());
	for (unsigned int i = 0; i < vMatched.size(); i++)
	BOOST_CHECK(vMatched[i] == merkleBlock.vMatchedTxn[i].second);
}

BOOST_AUTO_TEST_CASE(merkle_block_2_with_update_none)
{
	// Random real block (a0a180107a21c5bcfb4cd92a5f0cefcb0aaa8af64d92687aa5d90023c0c1e211)
	// With 4 txes
	CBlock block;
	CDataStream stream(ParseHex("02000000de2ca3a3d65e0188beac69a01307af604a03dcbf374d478bcd7e5ab19d7a9dc5a61478d1c6b796b4b4ed6ae347608360f956ad8dbd277bfa5d225284633a4a436c2d1e55ffff00201f0000000200000046304402205e3cea36409bc900effe92f71fc158740dc3bd5f4263dfa1c5478078b00dd85a02201f1b17a0dfe0a666d0ab9556161fa57702c310a975ec10ba92bc1bbe7119011404010102000180b7bf1802030101020002020001bcbf0883dbfdacaf0000463044022029a6f879e1130692bfa5b8f43f15a404a1e625b74e373de6ae42e4129935e0640220289ec13b41fc07f3ffbd7e1b04ffd6d4a2c0d2b031bf057db5c98ea1b93a5bd7040101020002020101bcbf0883dbfdacaf000115463044022074a008a65d1bd2ddf28049e6da3be22682fdd99450ac5df66b0ca7ba2576f0dd0220207054d07407fc9bb83c232e65d5676b2a76607f0595be8511177b1dfd21b8ed040101020003020101bcbf0883dbfdacaf000115473045022100b76c0a8caf9f5d91ca5f83927e444c43269b7d58b18889351a3cc1bee5cc3b59022057028c527a0f21098806a924db64d2d76f48f27fcc271c40af3e621608a0d159"), SER_NETWORK, PROTOCOL_VERSION);
	stream >> block;

	CBloomFilter filter(10, 0.000001, 0, BLOOM_UPDATE_NONE);
	// Match the first transaction
	filter.insert(uint256("0x364bb7c1643262ede7d080478de9e0837bdbbc7d1df84a1ff5acc2c2bd570e56"));

	CMerkleBlock merkleBlock(block, filter);
	BOOST_CHECK(merkleBlock.header.GetHash() == block.GetHash());

	BOOST_CHECK(merkleBlock.vMatchedTxn.size() == 1);

	BOOST_CHECK(merkleBlock.vMatchedTxn[0].second == uint256("0x364bb7c1643262ede7d080478de9e0837bdbbc7d1df84a1ff5acc2c2bd570e56"));
	BOOST_CHECK(merkleBlock.vMatchedTxn[0].first == 0);

	vector<uint256> vMatched;
	BOOST_CHECK(merkleBlock.txn.ExtractMatches(vMatched) == block.hashMerkleRoot);
	BOOST_CHECK(vMatched.size() == merkleBlock.vMatchedTxn.size());
	for (unsigned int i = 0; i < vMatched.size(); i++)
	BOOST_CHECK(vMatched[i] == merkleBlock.vMatchedTxn[i].second);

}

BOOST_AUTO_TEST_CASE(merkle_block_3_and_serialize)
{
	// Random real block (70f9df86777443d549585f49c73cdf13e50888144e0247ffe5ad70910e311ef0)
	// With one tx
	CBlock block;
	CDataStream stream(ParseHex("02000000f9591069fd054522564ebc65bbc1ccfee3dab4e90fa5d16ee03545d19d8677356499ff5cab2440927f4193510c35ec49eca699eafb106509627c4820c4ffc99a7d341e55ffff0020c600000001000000473045022100f4f278c12e83e271bea95037f9318aeb9b6d7c5309d0bf7296f00d112b36566d022056a27c5a0c0e9a1065415c299ca916e37bf68c03dc0a4a181e2960bdb24eb0490101010200050001"), SER_NETWORK, PROTOCOL_VERSION);
	stream >> block;

	CBloomFilter filter(10, 0.000001, 0, BLOOM_UPDATE_ALL);
	// Match the only transaction
	filter.insert(uint256("0x9ac9ffc420487c62096510fbea99a6ec49ec350c5193417f924024ab5cff9964"));

	CMerkleBlock merkleBlock(block, filter);
	BOOST_CHECK(merkleBlock.header.GetHash() == block.GetHash());

	BOOST_CHECK(merkleBlock.vMatchedTxn.size() == 1);

	BOOST_CHECK(merkleBlock.vMatchedTxn[0].second == uint256("0x9ac9ffc420487c62096510fbea99a6ec49ec350c5193417f924024ab5cff9964"));
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
	vector<unsigned char> vch = ParseHex("02000000f9591069fd054522564ebc65bbc1ccfee3dab4e90fa5d16ee03545d19d8677356499ff5cab2440927f4193510c35ec49eca699eafb106509627c4820c4ffc99a7d341e55ffff0020c600000001000000473045022100f4f278c12e83e271bea95037f9318aeb9b6d7c5309d0bf7296f00d112b36566d022056a27c5a0c0e9a1065415c299ca916e37bf68c03dc0a4a181e2960bdb24eb04901000000016499ff5cab2440927f4193510c35ec49eca699eafb106509627c4820c4ffc99a0101");
	vector<char> expected(vch.size());

	for (unsigned int i = 0; i < vch.size(); i++)
	expected[i] = (char)vch[i];

	BOOST_CHECK_EQUAL_COLLECTIONS(expected.begin(), expected.end(), merkleStream.begin(), merkleStream.end());
}

BOOST_AUTO_TEST_CASE(merkle_block_4)
{
	// Random real block (759f9028f01d180014eb38182f74cba5f3d9c57063b13681dc6b93ac8d375238)
	// With 7 txes
	CBlock block;
	CDataStream stream(ParseHex("02000000b8c4851f791f1d3788faae7180c835d2e389fb2b058d75cc8a42f1138cd7f4ea6e1a45770891165e4043e84680cce0ddc399d5a63c5df6193276e20ed1eb0eb3133a1e55ffff00204801000002000000473045022100f4af57e63946677d7e5b5d03332e2760ba6a8b4fdba3e3b63d348763d22bfd090220247eaf1f3d92889c1fdb2bd21b40e843f98e77661a1637e0559fc56c51d2ddb107010102000182e5812602030101020003020002cfff3182acc2a8c1000046304402205c6e23b6c5f906042c658b8952c6dc2b0960e211c60f993d8621720b5f3f8cd90220656ff949b7046066fa2c441feba6959dfa125e5a6f731583a779fdbc4514f9b8040101020002020101cfff3182acc2a8c1000114463044022046f0466d1e1f5dda49b32149cf92d5484106ccb2ab56e9dde8a86dd703159a2c022014bd41e28f1d2dcfff1aa03fccc6805b96568e9c66c224fd70d4e9138b433fc4040101020003020101cfff3182acc2a8c100011547304502210091cece55cd2b272ffb5cd892a6eb1a0f486511ff4c32955bea3284329c13a88f0220520a0acbc0e558ffc929cfdae36166fdaa8ae4c1c4329465df177e65f1d52b86040101020001020101cfff3182acc2a8c1000115473045022100caca261c7cef31182bf1caa132d9e642893c16e7bd31c753abec0a6c484bc33202200ff5024ccda560a7684d09e754d2f686dab1bcf62bb02b9c2244c6b38e220c42040101020002020101cfff3182acc2a8c1000115473045022100ebd3f0e69182133cf8f01e4af20367392c6f7d395416f22df9655956b902d3bf022078ddc45f9cc55c68f3ab4c760aaedf142023a0a337db4f506a6d212eae0f2871040101020003020101cfff3182acc2a8c1000115473045022100eb69925f9988db66fddcb61b14cfefb43ab75ad8cd9cabab32453665cdfd71e502204c9b9cd2da528cf75c8e0b1d8c42be5a8ae7b9bcb152e728648d5f645bd9d8ac"), SER_NETWORK, PROTOCOL_VERSION);
	stream >> block;

	CBloomFilter filter(10, 0.000001, 0, BLOOM_UPDATE_ALL);
	// Match the last transaction
	filter.insert(uint256("0x4a7b0b53f9865c4de0763ebc1bafde22f73cac0dc0eb0de28dc8409da923cdf5"));

	CMerkleBlock merkleBlock(block, filter);
	BOOST_CHECK(merkleBlock.header.GetHash() == block.GetHash());

	BOOST_CHECK(merkleBlock.vMatchedTxn.size() == 1);
	pair<unsigned int, uint256> pair = merkleBlock.vMatchedTxn[0];

	BOOST_CHECK(merkleBlock.vMatchedTxn[0].second == uint256("0x4a7b0b53f9865c4de0763ebc1bafde22f73cac0dc0eb0de28dc8409da923cdf5"));
	BOOST_CHECK(merkleBlock.vMatchedTxn[0].first == 6);

	vector<uint256> vMatched;
	BOOST_CHECK(merkleBlock.txn.ExtractMatches(vMatched) == block.hashMerkleRoot);
	BOOST_CHECK(vMatched.size() == merkleBlock.vMatchedTxn.size());
	for (unsigned int i = 0; i < vMatched.size(); i++)
	BOOST_CHECK(vMatched[i] == merkleBlock.vMatchedTxn[i].second);

	// Also match the 4th transaction
	filter.insert(uint256("0x8086423e5963aad399452b708e43eec785aa74d716bf1465d68de5d2b5c716c3"));
	merkleBlock = CMerkleBlock(block, filter);
	BOOST_CHECK(merkleBlock.header.GetHash() == block.GetHash());

	BOOST_CHECK(merkleBlock.vMatchedTxn.size() == 2);

	BOOST_CHECK(merkleBlock.vMatchedTxn[0].second == uint256("0x8086423e5963aad399452b708e43eec785aa74d716bf1465d68de5d2b5c716c3"));
	BOOST_CHECK(merkleBlock.vMatchedTxn[0].first == 3);

	BOOST_CHECK(merkleBlock.vMatchedTxn[1] == pair);

	BOOST_CHECK(merkleBlock.txn.ExtractMatches(vMatched) == block.hashMerkleRoot);
	BOOST_CHECK(vMatched.size() == merkleBlock.vMatchedTxn.size());
	for (unsigned int i = 0; i < vMatched.size(); i++)
	BOOST_CHECK(vMatched[i] == merkleBlock.vMatchedTxn[i].second);
}

BOOST_AUTO_TEST_SUITE_END()
