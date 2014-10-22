/*
 * TestMcu.h
 *
 *  Created on: Aug 6, 2014
 *      Author: ranger.shi
 */

#ifndef TESTMCU_H_
#define TESTMCU_H_
#include <boost/format.hpp>

#include "CVir8051.h"
#include <stdio.h>
#include <string.h>
#include <iostream>
using namespace boost;

class CTestMcu {
	CVir8051 *pCVir8051;

public:
	static bool IsOdd(unsigned char un);
	string AJMPTest(int space);
	string NOPTest(int space);
	string LJMPTest(int space);
	string RR_ATest(int space);
	string INC_ATest(int space);
	string INC_DirectTest(int space);
	string INC_Ri_1Test(int space);
	string INC_RnTest(int space);
	string JBC_Bit_RelTest(int space);
	string ACALL_Addr11Test(int space);
	string LCALL_Addr16Test(int space);
	string RRC_ATest(int space);
	string DEC_ATest(int space);
	string DEC_DirectTest(int space);
	string DEC_Ri_1Test(int space);
	string DEC_RnTest(int space);
	string JB_Bit_RelTest(int space);
	string RETTest(int space);
	string RL_ATest(int space);
	string ADD_A_DataTest(int space);
	string ADD_A_DirectTest(int space);
	string ADD_A_Ri_1Test(int space);
	string ADD_A_RnTest(int space);
	string JNB_Bit_RelTest(int space);
	string RETITest(int space);
	string RLC_ATest(int space);
	string ADDC_A_DataTest(int space);
	string ADDC_A_DirectTest(int space);
	string ADDC_A_RnTest(int space);
	string ADDC_A_RiTest(int space);
	string JC_RelTest(int space);
	string ORL_Direct_ATest(int space);
	string ORL_Direct_DataTest(int space);
	string ORL_A_Data_And_DirectTest(int space);
	string ORL_A_Rn_1Test(int space);
	string ORL_A_Ri_1Test(int space);
	string ANL_Direct_ATest(int space);
	string ANL_Direct_DataTest(int space);
	string ANL_A_Data_And_DirectTest(int space);
	string ANL_A_Rn_1Test(int space);
	string ANL_A_Ri_1Test(int space);
	string JZ_RelTest(int space);
	string XRL_Direct_ATest(int space);
	string XRL_Direct_DataTest(int space);
	string XRL_A_Data_And_DirectTest(int space);
	string XRL_A_Rn_1Test(int space);
	string XRL_A_Ri_1Test(int space);
	string JNZ_RelTest(int space);
	string ORL_C_DirectTest(int space);
	string JMP_A_DPTRTest(int space);
	string MOV_A_DataTest(int space);
	string MOV_Direct_DataTest(int space);
	string MOV_Ri_DataTest(int space);
	string MOV_Rn_DataTest(int space);
	string SJMP_RelTest(int space);
	string ANL_C_BitTest(int space);
	string MOVC_A_PCTest(int space);
	string DIV_ABTest(int space);
	string MOV_Direct_DirectTest(int space);
	string MOV_Direct_Rn_1Test(int space);

	string MOV_Rn_ATest(int space);
	string MOV_Direct_ATest(int space);
	string CPL_ATest(int space);
	string MOVX_Ri_1_ATest(int space);
	string MOVX_DPTR_ATest(int space);
	string MOV_A_Rn_1Test(int space);
	string MOV_A_Ri_1Test(int space);
	string MOV_A_DirectTest(int space);
	string CLR_ATest(int space);
	string MOVX_A_Ri_1Test(int space);
	string MOVX_A_DPTRTest(int space);
	string DJNZ_Rn_RelRTest(int space);
	string XCHD_A_Ri_1Test(int space);
	string DJNZ_Direct_RelTest(int space);
	string DA_ATest(int space);
	string SETB_CTest(int space);
	string SETB_BitTest(int space);
	string POP_DirectTest(int space);
	string XCH_A_RnTest(int space);
	string XCH_A_DirectTest(int space);
	string SWAP_ATest(int space);
	string CLR_CTest(int space);
	string CLR_BitTest(int space);
	string PUSH_DirectTest(int space);

//86spark
	string MOV_Direct_Rn_Test(int space);
	string MOV_Direct_Ri_Test(int space);
	string MOV_DPTR_Data_Test(int space);
	string MOV_Bit_C_Test(int space);
	string MOV_C_Bit_Test(int space);
	string MOVC_A_DPTR_Test(int space);
	string SUBB_A_Data_Test(int space);
	string SUBB_A_RnTest(int space);
	string SUBB_A_RiTest(int space);
	string SUBB_A_DirectTest(int space);
	string MOV_Rn_Direct_Test(int space);
	string MOV_Ri_Direct_Test(int space);
	string CJNE_Rn_Data_Rel_Test(int space);
	string CJNE_Ri_Data_Rel_Test(int space);
	string CJNE_A_Direct_Rel_Test(int space);
	string CJNE_A_Data_Rel_Test(int space);
	string CPL_C_Test(int space);
	string CPL_Bit_Test(int space);
	string ACALL_Addr11_Test(int space);
	string ANL_C_Bit_1_Test(int space);
	string MUL_AB_Test(int space);
	string INC_DPTR_Test(int space);
	string AJMP_Addr11_Test(int space);
	string ORL_C_Bit_Test(int space);
	string MOV_Ri_ATest(int space);
	string XCH_A_RiTest(int space);
	string JNC_RelTest(int space);
	void TestRun();


	string PrintCMDUntested(void);
	CTestMcu(CVir8051 *pCVir8051);

	virtual ~CTestMcu();
};

#endif /* TESTMCU_H_ */
