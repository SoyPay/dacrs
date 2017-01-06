#ifndef DACRS_PTEST_RED_PACKET_TESTS_H_
#define DACRS_PTEST_RED_PACKET_TESTS_H_

#include "cycle_test_base.h"
#include <stdlib.h>
#include <time.h>
#include "main.h"
#include "miner.h"
#include "uint256.h"
#include "util.h"
#include <boost/foreach.hpp>
#include <boost/test/unit_test.hpp>

#include "json/json_spirit_writer_template.h"
#include "./rpc/rpcclient.h"
#include "json/json_spirit_reader_template.h"
#include "json/json_spirit_reader.h"
#include "json/json_spirit_writer.h"
#include "json/json_spirit_value.h"
#include "json/json_spirit_stream_reader.h"
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
using namespace std;
using namespace boost;
using namespace json_spirit;

class CRedPacketTest: public CycleTestBase {
 public:
	CRedPacketTest();
	~CRedPacketTest() {
	}
	;
	virtual emTEST_STATE Run();
	bool RegistScript();
	bool WaitRegistScript();
	bool WithDraw();
	bool SendRedPacketTx();
	bool AcceptRedPacketTx();
	bool WaitTxConfirmedPackage(string m_strTxHash);
	void Initialize();
	bool SendSpecailRedPacketTx();
	bool AcceptSpecailRedPacketTx();

 private:
	int m_nNum;
	int m_nStep;
	string m_strTxHash;
	string m_strAppRegId;
	string m_strRedHash;
	string m_strAppAddr;
	uint64_t m_ullSpecailmM;
	string m_srtRchangeAddr;
};



#endif

