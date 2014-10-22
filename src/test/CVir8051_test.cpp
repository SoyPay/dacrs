#include <boost/test/unit_test.hpp>
#include <stdint.h>
#include <vector>
#include <map>
#include "VmScript/CVir8051.h"
#include "VmScript/TestMcu.h"
#include <iostream>

#include <boost/foreach.hpp>

using namespace std;

CVir8051 *pgoble = NULL;

typedef string (CTestMcu::*pfun)(int space);

struct __Map {
	int index;
	string fname;
	pfun fun;
};

BOOST_AUTO_TEST_SUITE(cvir8051_tests)

#if 0

BOOST_AUTO_TEST_CASE(cvir8051_test2)
{
	std::vector<unsigned char> m_ROM;
	char *filepath = "D:\\C51\\Debug\\Exe\\CPLUS1.bin";
	assert(filepath);
	FILE *m_file = fopen(filepath, "rb");
	if (m_file != NULL) {
		fseek(m_file, 0, SEEK_SET);
		fseek(m_file, 0, SEEK_END);
		int len = ftell(m_file);
		fseek(m_file, 0, SEEK_SET);
		char *buffer = (char*) malloc(len);
		memset(buffer, 0, len);
		fread(buffer, 1, len + 1, m_file);
		m_ROM.insert(m_ROM.begin(), buffer, buffer + len);
		if (buffer != NULL) {
			free(buffer);
			buffer = NULL;
		}
		if (m_file != NULL) {
			fclose(m_file);
			m_file = NULL;
		}
	}

	string reslut = "";
	vector<unsigned char> InputData;
	CVir8051 mcu(m_ROM,InputData);
	pgoble = &mcu;
	CTestMcu test(&mcu);

	struct __Map map[] = {
		{	0, "NOPTest", &CTestMcu::NOPTest}, //
		{	1, "LJMPTest", &CTestMcu::LJMPTest}, //
		{	2, "RR_ATest", &CTestMcu::RR_ATest}, //
		{	3, "INC_ATest", &CTestMcu::INC_ATest}, //
		{	4, "INC_DirectTest", &CTestMcu::INC_DirectTest}, //
		{	5, "INC_Ri_1Test", &CTestMcu::INC_Ri_1Test}, //
		{	6, "INC_RnTest", &CTestMcu::INC_RnTest}, //
		{	7, "JBC_Bit_RelTest", &CTestMcu::JBC_Bit_RelTest}, //
		{	8, "ACALL_Addr11Test", &CTestMcu::ACALL_Addr11Test}, //
		{	9, "LCALL_Addr16Test", &CTestMcu::LCALL_Addr16Test}, //
		{	10, "RRC_ATest", &CTestMcu::RRC_ATest}, //
		{	11, "DEC_ATest", &CTestMcu::DEC_ATest}, //
		{	12, "DEC_DirectTest", &CTestMcu::DEC_DirectTest}, //
		{	13, "DEC_Ri_1Test", &CTestMcu::DEC_Ri_1Test}, //
		{	14, "DEC_RnTest", &CTestMcu::DEC_RnTest}, //
		{	15, "JB_Bit_RelTest", &CTestMcu::JB_Bit_RelTest}, //
		{	16, "RETTest", &CTestMcu::RETTest}, //
		{	17, "RL_ATest", &CTestMcu::RL_ATest}, //
		{	18, "ADD_A_DataTest", &CTestMcu::ADD_A_DataTest}, //
		{	19, "ADD_A_DirectTest", &CTestMcu::ADD_A_DirectTest}, //
		{	20, "ADD_A_Ri_1Test", &CTestMcu::ADD_A_Ri_1Test}, //
		{	21, "ADD_A_RnTest", &CTestMcu::ADD_A_RnTest}, //
		{	22, "JNB_Bit_RelTest", &CTestMcu::JNB_Bit_RelTest}, //
		{	23, "RETITest", &CTestMcu::RETITest}, //
		{	24, "RLC_ATest", &CTestMcu::RLC_ATest}, //
		{	25, "ADDC_A_DataTest", &CTestMcu::ADDC_A_DataTest}, //
		{	26, "ADDC_A_DirectTest", &CTestMcu::ADDC_A_DirectTest}, //
		{	27, "ADDC_A_RnTest", &CTestMcu::ADDC_A_RnTest}, //
		{	28, "ADDC_A_RiTest", &CTestMcu::ADDC_A_RiTest}, //
		{	29, "JC_RelTest", &CTestMcu::JC_RelTest}, //
		{	30, "ORL_Direct_ATest", &CTestMcu::ORL_Direct_ATest}, //
		{	31, "ORL_Direct_DataTest", &CTestMcu::ORL_Direct_DataTest}, //
		{	32, "ORL_A_Data_And_DirectTest", &CTestMcu::ORL_A_Data_And_DirectTest}, //
		{	33, "ORL_A_Rn_1Test", &CTestMcu::ORL_A_Rn_1Test}, //
		{	34, "ORL_A_Ri_1Test", &CTestMcu::ORL_A_Ri_1Test}, //
		{	35, "ANL_Direct_ATest", &CTestMcu::ANL_Direct_ATest}, //
		{	36, "ANL_Direct_DataTest", &CTestMcu::ANL_Direct_DataTest}, //
		{	37, "ANL_A_Data_And_DirectTest", &CTestMcu::ANL_A_Data_And_DirectTest}, //
		{	38, "ANL_A_Rn_1Test", &CTestMcu::ANL_A_Rn_1Test}, //
		{	39, "ANL_A_Ri_1Test", &CTestMcu::ANL_A_Ri_1Test}, //
		{	40, "JZ_RelTest", &CTestMcu::JZ_RelTest}, //
		{	41, "XRL_Direct_ATest", &CTestMcu::XRL_Direct_ATest}, //
		{	42, "XRL_Direct_DataTest", &CTestMcu::XRL_Direct_DataTest}, //
		{	43, "XRL_A_Data_And_DirectTest", &CTestMcu::XRL_A_Data_And_DirectTest}, //
		{	44, "XRL_A_Rn_1Test", &CTestMcu::XRL_A_Rn_1Test}, //
		{	45, "XRL_A_Ri_1Test", &CTestMcu::XRL_A_Ri_1Test}, //
		{	46, "JNZ_RelTest", &CTestMcu::JNZ_RelTest}, //
		{	47, "ORL_C_DirectTest", &CTestMcu::ORL_C_DirectTest}, //
		{	48, "JMP_A_DPTRTest", &CTestMcu::JMP_A_DPTRTest}, //
		{	49, "MOV_A_DataTest", &CTestMcu::MOV_A_DataTest}, //
		{	50, "MOV_Direct_DataTest", &CTestMcu::MOV_Direct_DataTest}, //
		{	51, "MOV_R0_1_DataTest", &CTestMcu::MOV_Ri_DataTest}, //
		{	52, "SJMP_RelTest", &CTestMcu::SJMP_RelTest}, //
		{	53, "ANL_C_BitTest", &CTestMcu::ANL_C_BitTest}, //
		{	54, "MOVC_A_PCTest", &CTestMcu::MOVC_A_PCTest}, //
		{	55, "DIV_ABTest", &CTestMcu::DIV_ABTest}, //
		{	56, "MOV_Direct_DirectTest", &CTestMcu::MOV_Direct_DirectTest}, //
		{	57, "MOV_Direct_Rn_1Test", &CTestMcu::MOV_Direct_Rn_1Test}, //
		{	58, "MOV_Rn_ATest", &CTestMcu::MOV_Rn_ATest}, //
		{	59, "MOV_Direct_ATest", &CTestMcu::MOV_Direct_ATest}, //
		{	60, "CPL_ATest", &CTestMcu::CPL_ATest}, ///
		{	61, "MOVX_Ri_1_ATest", &CTestMcu::MOVX_Ri_1_ATest}, //
		{	62, "MOVX_DPTR_ATest", &CTestMcu::MOVX_DPTR_ATest}, //
		{	63, "MOV_A_Rn_1Test", &CTestMcu::MOV_A_Rn_1Test}, //
		{	64, "MOV_A_DirectTest", &CTestMcu::MOV_A_DirectTest}, //
		{	65, "CLR_ATest", &CTestMcu::CLR_ATest}, //
		{	66, "MOVX_A_Ri_1Test", &CTestMcu::MOVX_A_Ri_1Test}, //
		{	67, "MOVX_A_DPTRTest", &CTestMcu::MOVX_A_DPTRTest}, //
		{	68, "DJNZ_Rn_RelRTest", &CTestMcu::DJNZ_Rn_RelRTest}, //
		{	69, "XCHD_A_Ri_1Test", &CTestMcu::XCHD_A_Ri_1Test}, //
		{	70, "DJNZ_Direct_RelTest", &CTestMcu::DJNZ_Direct_RelTest}, //
		{	71, "DA_ATest", &CTestMcu::DA_ATest}, //
		{	72, "SETB_CTest", &CTestMcu::SETB_CTest}, //
		{	73, "SETB_BitTest", &CTestMcu::SETB_BitTest}, //
		{	74, "POP_DirectTest", &CTestMcu::POP_DirectTest}, //
		{	75, "XCH_A_RnTest", &CTestMcu::XCH_A_RnTest}, //
		{	76, "XCH_A_DirectTest", &CTestMcu::XCH_A_DirectTest}, //
		{	77, "SWAP_ATest", &CTestMcu::SWAP_ATest}, //
		{	78, "CLR_CTest", &CTestMcu::CLR_CTest}, //
		{	79, "CLR_BitTest", &CTestMcu::CLR_BitTest}, //
		{	80, "PUSH_DirectTest", &CTestMcu::PUSH_DirectTest}, //
		{	81, "MOV_Direct_Rn_Test", &CTestMcu::MOV_Direct_Rn_Test}, //
		{	82, "MOV_Direct_Ri_Test", &CTestMcu::MOV_Direct_Ri_Test}, //
		{	83, "MOV_DPTR_Data_Test", &CTestMcu::MOV_DPTR_Data_Test}, //
		{	84, "MOV_Bit_C_Test", &CTestMcu::MOV_Bit_C_Test}, //
		{	85, "MOV_C_Bit_Test", &CTestMcu::MOV_C_Bit_Test}, //
		{	86, "MOVC_A_DPTR_Test", &CTestMcu::MOVC_A_DPTR_Test}, //
		{	87, "SUBB_A_Data_Test", &CTestMcu::SUBB_A_Data_Test}, //
		{	88, "SUBB_A_RnTest", &CTestMcu::SUBB_A_RnTest}, //
		{	89, "SUBB_A_RiTest", &CTestMcu::SUBB_A_RiTest}, //
		{	90, "SUBB_A_DirectTest", &CTestMcu::SUBB_A_DirectTest}, //
		{	91, "MOV_Rn_Direct_Test", &CTestMcu::MOV_Rn_Direct_Test}, //
		{	92, "MOV_Ri_Direct_Test", &CTestMcu::MOV_Ri_Direct_Test}, //
		{	93, "CJNE_Rn_Data_Rel_Test", &CTestMcu::CJNE_Rn_Data_Rel_Test}, //
		{	94, "CJNE_Ri_Data_Rel_Test", &CTestMcu::CJNE_Ri_Data_Rel_Test}, //
		{	95, "CJNE_A_Direct_Rel_Test", &CTestMcu::CJNE_A_Direct_Rel_Test}, //
		{	96, "CJNE_A_Data_Rel_Test", &CTestMcu::CJNE_A_Data_Rel_Test}, //
		{	97, "CPL_C_Test", &CTestMcu::CPL_C_Test}, //
		{	98, "CPL_Bit_Test", &CTestMcu::CPL_Bit_Test}, //
		{	99, "ACALL_Addr11_Test", &CTestMcu::ACALL_Addr11_Test}, //
		{	100, "ANL_C_Bit_1_Test", &CTestMcu::ANL_C_Bit_1_Test}, //
		{	101, "MUL_AB_Test", &CTestMcu::MUL_AB_Test}, //
		{	102, "INC_DPTR_Test", &CTestMcu::INC_DPTR_Test}, //
		{	103, "AJMP_Addr11_Test", &CTestMcu::AJMP_Addr11_Test}, //
		{	104, "ORL_C_Bit_Test", &CTestMcu::ORL_C_Bit_Test}, //
		{	105, "MOV_Rn_DataTest", &CTestMcu::MOV_Rn_DataTest}, //
		{	106, "MOV_A_Ri_1Test", &CTestMcu::MOV_A_Ri_1Test}, //
		{	107, "AJMPTest", &CTestMcu::AJMPTest}, //
		{	108, "MOV_Ri_ATest", &CTestMcu::MOV_Ri_ATest}, //
		{	109, "JNC_RelTest", &CTestMcu::JNC_RelTest}, //
		{	110, "XCH_A_RiTest", &CTestMcu::XCH_A_RiTest}}; //

	int start = 0;int end = 0;int space = 10000;
//	if (argc < 2) {
//		start = 0;
//		end = 0;
//	} else {
//		start = atoi(argv[1]);
//		end = atoi(argv[2]);
//	}
//
//	if (argc == 4)
//		space = atoi(argv[3]);

	Assert(end < sizeof(map) / sizeof(map[0]));
	end = (end) ? (end) : (sizeof(map) / sizeof(map[0]));

	for (int index = start; index < end; index++) {
		string ret = (test.*(map[index].fun))(space);
		BOOST_CHECK_EQUAL(ret,"OK");
		if (ret != "OK") {
			char temp[512];
			sprintf(temp, "No.%d-", map[index].index);
			reslut += (string(temp) + "ERROR:function->" + map[index].fname + " info:\r\n" + ret + "\r\n");
		}

	}

	if (reslut == "") {
		BOOST_TEST_MESSAGE("No Error");
	}

//	printf("test %d-%d result:\r\n%s", start, end, reslut.c_str());
//
//	printf("the untested cmd list:\r\n");
	BOOST_CHECK_EQUAL(test.PrintCMDUntested(),"OK");
//	test.PrintCMDUntested();

}

#else
BOOST_AUTO_TEST_CASE(xxxx) {
	BOOST_ERROR("ERROR:THE SUITE NEED TO MODIFY!");
}
#endif

BOOST_AUTO_TEST_SUITE_END()
