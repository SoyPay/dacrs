/*
 * TestMcu.cpp
 *
 *  Created on: Aug 6, 2014
 *      Author: ranger.shi
 */
#include "util.h"
#include "TestMcu.h"
typedef unsigned char byte;

int noprint(...)
{
	return 1;
}

#define LogPrint(a,...) noprint(__VA_ARGS__)

string CTestMcu::MOV_Direct_Rn_1Test(int space) {
	return "OK";
}

string CTestMcu::MOV_Direct_DirectTest(int space) {
	int PC = 0;
	byte code = 0x85;
	byte expect = 0;
	int saddr = 0, daddr = 0;

	auto checkresult = [&]()
	{
		if ((pCVir8051->GetRamData(daddr) != expect) || (pCVir8051->Sys.PC != (PC + 3)))
		return false;
		return true;
	};

	auto errstring = [&] () {
		char temp[1024];
		sprintf(temp,"runtime info: PC:%x expect:%x result:%x\r\n", PC, expect, (byte) (saddr + 1));
		return "error :" + string(temp)+ pCVir8051->ToStr();
	};

	auto updatecpu = [&]()
	{
		pCVir8051->InitalReg();
		pCVir8051->SetRamData((byte)saddr, expect);
		pCVir8051->Sys.PC = PC;
		pCVir8051->m_ExeFile[PC] = code;
		pCVir8051->m_ExeFile[PC + 1] = (byte)saddr;
		pCVir8051->m_ExeFile[PC + 2] = (byte)daddr;
	};

	for (PC = 0; PC < CVir8051::MAX_ROM - 2; PC = PC + space) {
		if (PC % 1000 == 0) {
			LogPrint("INFO", "code = %x PC = %d\n", code, PC);
		}
		for (saddr = 0; saddr <= 0XFF; saddr++) {
			for (daddr = 0; daddr <= 0XFF; daddr++) {
				expect = (byte) (saddr + 1);
				updatecpu();
				TestRun();
				if (!checkresult()) {
					return errstring();
				}
			}
		}
	}
	return "OK";
}
string CTestMcu::DIV_ABTest(int space) {
	int PC = 0, a = 0, b = 0;
	byte code = 0x84;

	auto checkresult = [&]()
	{
		if(b == 0) {
			if (pCVir8051->Sys.psw().ov != 1) {
				return false;
			}
		}
		else {
			if(pCVir8051->Sys.PC != (PC +1))
			return false;
			if(((pCVir8051->Sys.a() != (byte)a/(byte)b)))
			return false;
			if((pCVir8051->Sys.b() != (byte)a%(byte)b))
			return false;
			if(pCVir8051->Sys.psw().ov != 0)
			return false;
			if(IsOdd(pCVir8051->Sys.a()) != pCVir8051->Sys.psw().p)
			return false;
		}
		return true;
	};

	auto errstring = [&] () {
		char temp[1024];
		sprintf(temp,"runtime info: PC:%x a:%x b:%x\r\n", PC, (byte)a, (byte)b);
		return "error :" + string(temp)+ pCVir8051->ToStr();
	};

	auto updatecpu = [&]()
	{
		pCVir8051->InitalReg();
		pCVir8051->Sys.PC = PC;
		memcpy(&pCVir8051->m_ExeFile[PC], &code, sizeof(code));
		pCVir8051->Sys.a = (byte) a;
		pCVir8051->Sys.b = (byte) b;
	};

	for (PC = 0; PC < CVir8051::MAX_ROM; PC = PC + space) {

		if (PC % 1000 == 0) {
			LogPrint("INFO", "code = %x PC = %d\n", code, PC);
		}
		for (a = 0; a <= 0XFF; a++) {
			for (b = 0; b <= 0XFF; b++) {
				updatecpu();
				TestRun();
				if (!checkresult()) {
					return errstring();
				}
			}
		}
	}
	return "OK";

}
string CTestMcu::MOVC_A_PCTest(int space) {
	int PC = 0;
	byte code = 0x83, expect = 0, data = 0;
	int a = 0;

	auto checkresult = [&]()
	{
		if(pCVir8051->Sys.PC != (PC + 1))
		return false;
		if(pCVir8051->Sys.a() != expect)
		return false;
		if(IsOdd(pCVir8051->Sys.a()) != pCVir8051->Sys.psw().p)
		return false;
		return true;
	};

	auto errstring = [&] () {
		char temp[1024];
		sprintf(temp,"runtime info: PC:%x a:%x expect:%x data:%x\r\n", PC, (byte)a, expect, data);
		return "error :" + string(temp)+ pCVir8051->ToStr();
	};

	auto updatecpu = [&]()
	{
		pCVir8051->InitalReg();
		pCVir8051->Sys.PC = PC;
		pCVir8051->Sys.a = (byte)a;
		pCVir8051->m_ExeFile[PC] = code;
		pCVir8051->m_ExeFile[PC + 1 + a] = data;
	};

	for (PC = 0; PC < CVir8051::MAX_ROM; PC = PC + space) {
		if (PC % 1000 == 0) {
			LogPrint("INFO", "code = %x PC = %d\n", code, PC);
		}
		for (a = 0; a <= 0XFF; a++) {
			data = rand() % 256;
			expect = (1 + a + PC > CVir8051::MAX_ROM) ? (0) : (data);
			updatecpu();
			TestRun();
			if (!checkresult()) {
				return errstring();
			}
		}
	}
	return "OK";
}
string CTestMcu::ANL_C_BitTest(int space) {
	int PC = 0, addr = 0;
	byte code = 0x82;
	bool bflag = false, cflag = false, expect = false;

	auto checkresult = [&]()
	{
		if(pCVir8051->Sys.PC != (PC + 2))
		return false;
		if(pCVir8051->Sys.psw().cy != expect)
		return false;
		if(addr>= 0xe0 &&addr <= 0xe7) {
			if(IsOdd(pCVir8051->Sys.a()) != pCVir8051->Sys.psw().p)
			return false;
		}
		return true;
	};

	auto errstring = [&] () {
		char temp[1024];
		sprintf(temp,"runtime info: PC:%x cflag:%x bflag:%x expect:%x\r\n", PC, cflag, bflag, expect);
		return "error :" + string(temp)+ pCVir8051->ToStr();
	};

	auto updatecpu = [&]()
	{
		pCVir8051->InitalReg();
		pCVir8051->Sys.PC = PC;
		pCVir8051->m_ExeFile[PC] = code;
		pCVir8051->m_ExeFile[PC + 1] = (byte)addr;
		pCVir8051->Sys.psw().cy = cflag;
		if (bflag) {
			pCVir8051->SetBitFlag((byte)addr);
		} else {
			pCVir8051->ClrBitFlag((byte)addr);
		}
	};

	for (PC = 0; PC <= CVir8051::MAX_ROM - 2; PC = PC + space) {
		if (PC % 1000 == 0) {
			LogPrint("INFO", "code = %x PC = %d\n", code, PC);
		}
		for (addr = 0; addr <= 0XFF; addr++) {
			bflag = rand() % 2;
			cflag = rand() % 2;
			expect = (addr == 0xd7) ? (bflag & bflag) : (cflag & bflag); // because 0xd7 is the Sys.psw().cy falg;
			updatecpu();
			TestRun();
			if (!checkresult()) {
				return errstring();
			}
		}
	}
	return "OK";
}
string CTestMcu::SJMP_RelTest(int space) {
	int PC = 0, expect = 0;
	byte code = 0x80;
	int raddr = 0;

	auto checkresult = [&]()
	{
		if(pCVir8051->Sys.PC != expect)
		return false;
		return true;
	};

	auto errstring = [&] () {
		char temp[1024];
		sprintf(temp,"runtime info: PC:%x raddr:%x expect:%x\r\n", PC, raddr, expect);
		return "error :" + string(temp)+ pCVir8051->ToStr();
	};

	auto updatecpu = [&]()
	{
		pCVir8051->InitalReg();
		pCVir8051->Sys.PC = PC;
		pCVir8051->m_ExeFile[PC] = code;
		pCVir8051->m_ExeFile[PC + 1] = (byte)raddr;
	};

	for (PC = 0; PC < CVir8051::MAX_ROM - 2; PC = PC + space) {
		if (PC % 1000 == 0) {
			LogPrint("INFO", "code = %x PC = %d\n", code, PC);
		}
		for (raddr = -128; raddr <= 127; raddr++) {
			expect = raddr + PC + 2;
			if (expect >= CVir8051::MAX_ROM || expect < 0) {
				continue;
			}
			updatecpu();
			TestRun();
			if (!checkresult()) {
				return errstring();
			}
		}
	}
	return "OK";
}
string CTestMcu::MOV_Ri_DataTest(int space) {
	int PC = 0, ri = 0;
	byte code = 0x76, idata = 0;

	auto checkresult = [&]()
	{
		if(pCVir8051->GetRamDataAt(ri) != idata) {
			return false;
		}
		if(pCVir8051->Sys.PC != PC + 2)
		return false;
		return true;
	};

	auto errstring = [&] () {
		char temp[1024];
		sprintf(temp,"runtime info: PC:%x idata:%x ri:%x\r\n", PC, idata, ri);
		return "error :" + string(temp)+ pCVir8051->ToStr();
	};

	auto updatecpu = [&]()
	{
		pCVir8051->InitalReg();
		if(code == 0x76)
		{
			pCVir8051->Rges.R0 = ri;
		}
		else if(code == 0x77)
		{
			pCVir8051->Rges.R1 = ri;
		}
		pCVir8051->Sys.PC = PC;
		pCVir8051->m_ExeFile[PC] = code;
		pCVir8051->m_ExeFile[PC + 1] = idata;
	};

	for (code = 0x76; code <= 0x77; code++) {
		for (PC = 0; PC < CVir8051::MAX_ROM - 1; PC = PC + space) {
			if (PC % 1000 == 0) {
				LogPrint("INFO", "code = %x PC = %d\n", code, PC);
			}
			for (ri = 8; ri <= CVir8051::MAX_IN_RAM; ri++) {
				idata = rand() % 256;
				updatecpu();
				TestRun();
				if (!checkresult()) {
					return errstring();
				}
			}
		}
	}

	return "OK";
}

string CTestMcu::MOV_Rn_DataTest(int space) {
	int PC = 0, idata = 0;
	byte code = 0x76;

	auto checkresult = [&]()
	{
		byte ret = 0;
		switch (code) {
			case 0x78:
			ret = pCVir8051->Rges.R0();
			break;
			case 0x79:
			ret = pCVir8051->Rges.R1();
			break;
			case 0x7a:
			ret = pCVir8051->Rges.R2();
			break;
			case 0x7b:
			ret = pCVir8051->Rges.R3();
			break;
			case 0x7c:
			ret = pCVir8051->Rges.R4();
			break;
			case 0x7d:
			ret = pCVir8051->Rges.R5();
			break;
			case 0x7e:
			ret = pCVir8051->Rges.R6();
			break;
			case 0x7f:
			ret = pCVir8051->Rges.R7();
			break;
			default:
			ret = 0;
			break;
		}
		if(ret != idata)
		return false;
		if(pCVir8051->Sys.PC != PC + 2)
		return false;
		return true;
	};

	auto errstring = [&] () {
		char temp[1024];
		sprintf(temp,"runtime info: PC:%x idata:%x\r\n", PC, idata);
		return "error :" + string(temp)+ pCVir8051->ToStr();
	};

	auto updatecpu = [&]()
	{
		pCVir8051->InitalReg();
		pCVir8051->Sys.PC = PC;
		pCVir8051->m_ExeFile[PC] = code;
		pCVir8051->m_ExeFile[PC + 1] = idata;
	};

	for (code = 0x78; code <= 0x7f; code++) {
		for (PC = 0; PC < CVir8051::MAX_ROM - 1; PC = PC + space) {
			if (PC % 1000 == 0) {
				LogPrint("INFO", "code = %x PC = %d\n", code, PC);
			}
			for (idata = 0; idata <= 0xff; idata++) {
				updatecpu();
				TestRun();
				if (!checkresult()) {
					return errstring();
				}
			}
		}
	}

	return "OK";
}

string CTestMcu::MOV_Direct_DataTest(int space) {
	int PC = 0, daddr = 0, idata = 0;
	byte code = 0x75;

	auto checkresult = [&]()
	{
		if(pCVir8051->Sys.PC != (PC + 3))
		return false;
		if(pCVir8051->GetRamData(daddr) != idata)
		return false;
		return true;
	};

	auto errstring = [&] () {
		char temp[1024];
		sprintf(temp,"runtime info: PC:%x daddr:%x idata:%x\r\n", PC, daddr, idata);
		return "error :" + string(temp)+ pCVir8051->ToStr();
	};

	auto updatecpu = [&]()
	{
		pCVir8051->InitalReg();
		pCVir8051->Sys.PC = PC;
		pCVir8051->m_ExeFile[PC] = code;
		pCVir8051->m_ExeFile[PC + 1] = daddr;
		pCVir8051->m_ExeFile[PC + 2] = idata;
	};

	for (PC = 0; PC <= CVir8051::MAX_ROM - 3; PC = PC + space) {
		if (PC % 1000 == 0) {
			LogPrint("INFO", "code = %x PC = %d\n", code, PC);
		}
		for (daddr = 7; daddr <= CVir8051::MAX_IN_RAM; daddr++) {
			for (idata = 0; idata <= 0XFF; idata++) {
				updatecpu();
				TestRun();
				if (!checkresult()) {
					return errstring();
				}
			}
		}
	}
	return "OK";

}
string CTestMcu::MOV_A_DataTest(int space) {
	int PC = 0, idata = 0;
	byte code = 0x74;

	auto checkresult = [&]()
	{
		if(pCVir8051->Sys.PC != (PC + 2))
		return false;
		if(pCVir8051->Sys.a() != idata)
		return false;
		if(IsOdd(pCVir8051->Sys.a()) != pCVir8051->Sys.psw().p)
		return false;
		return true;
	};

	auto errstring = [&] () {
		char temp[1024];
		sprintf(temp,"runtime info: PC:%x idata:%x\r\n", PC, idata);
		return "error :" + string(temp)+ pCVir8051->ToStr();
	};

	auto updatecpu = [&]()
	{
		pCVir8051->InitalReg();
		pCVir8051->Sys.PC = PC;
		pCVir8051->m_ExeFile[PC] = code;
		pCVir8051->m_ExeFile[PC + 1] = idata;
	};

	for (PC = 0; PC <= CVir8051::MAX_ROM - 3; PC = PC + space) {
		if (PC % 1000 == 0) {
			LogPrint("INFO", "code = %x PC = %d\n", code, PC);
		}
		for (idata = 0; idata <= 0XFF; idata++) {
			updatecpu();
			TestRun();
			if (!checkresult()) {
				return errstring();
			}
		}
	}
	return "OK";
}
string CTestMcu::JMP_A_DPTRTest(int space) {
	int PC = 0, dptr = 0, expect = 0, a = 0;
	byte code = 0x73;

	auto checkresult = [&]()
	{
		if(pCVir8051->Sys.PC != expect)
		return false;
		if(IsOdd(pCVir8051->Sys.a()) != pCVir8051->Sys.psw().p)
		return false;
		return true;
	};

	auto errstring = [&] () {
		char temp[1024];
		sprintf(temp,"runtime info: PC:%x expect:%x\r\n", PC, expect);
		return "error :" + string(temp)+ pCVir8051->ToStr();
	};

	auto updatecpu = [&]()
	{
		pCVir8051->InitalReg();
		pCVir8051->Sys.PC = PC;
		pCVir8051->Sys.dptr = dptr;
		pCVir8051->Sys.a = a;
		pCVir8051->m_ExeFile[PC] = code;
	};

	for (PC = 0; PC <= CVir8051::MAX_ROM - 1; PC = PC + space) {
		if (PC % 1000 == 0) {
			LogPrint("INFO", "code = %x PC = %d\n", code, PC);
		}
		for (dptr = 0; dptr <= CVir8051::MAX_ROM; dptr++) {
			for (a = 0; a <= 0xFF; a++) {
				if (a + dptr > CVir8051::MAX_ROM) {
					continue;
				}
				expect = a + dptr;
				updatecpu();
				TestRun();
				if (!checkresult()) {
					return errstring();
				}
			}

		}
	}
	return "OK";
}
string CTestMcu::ORL_C_DirectTest(int space) {
	int PC = 0, baddr = 0;
	byte code = 0x72;
	bool bflag = false, cflag = false, expect = false;

	auto checkresult = [&]()
	{
		if(pCVir8051->Sys.PC != (PC + 2))
		return false;
		if(pCVir8051->Sys.psw().cy != expect)
		return false;
		if(baddr>= 0xe0 && baddr <= 0xe7) {
			if(IsOdd(pCVir8051->Sys.a()) != pCVir8051->Sys.psw().p)
			return false;
		}
		return true;
	};

	auto errstring = [&] () {
		char temp[1024];
		sprintf(temp,"runtime info: PC:%x expect:%x bflag:%x cflag:%x baddr:%x\r\n", PC, expect, bflag, cflag, baddr);
		return "error :" + string(temp)+ pCVir8051->ToStr();
	};

	auto updatecpu = [&]()
	{
		pCVir8051->InitalReg();
		pCVir8051->Sys.PC = PC;
		pCVir8051->Sys.psw().cy = cflag;
		if (bflag) {
			pCVir8051->SetBitFlag(baddr);
		} else {
			pCVir8051->ClrBitFlag(baddr);
		}
		pCVir8051->m_ExeFile[PC] = code;
		pCVir8051->m_ExeFile[PC + 1] = baddr;
	};

	for (PC = 0; PC <= CVir8051::MAX_ROM - 2; PC = PC + space) {
		if (PC % 1000 == 0) {
			LogPrint("INFO", "code = %x PC = %d\n", code, PC);
		}
		for (baddr = 0; baddr <= 0XFF; baddr++) {
			if (baddr == 0xd7) {
				continue; // because 0xd7 is the Sys.psw().cy falg;
			}
			bflag = rand() % 2;
			cflag = rand() % 2;
			expect = bflag | cflag;
			updatecpu();
			TestRun();
			if (!checkresult()) {
				return errstring();
			}
		}
	}
	return "OK";
}
string CTestMcu::JNZ_RelTest(int space) {
	int PC = 0, expect = 0;
	byte code = 0x70, a = 0;
	int raddr = 0;

	auto checkresult = [&]()
	{
		if(pCVir8051->Sys.PC != expect)
		return false;
		return true;
	};

	auto errstring = [&] () {
		char temp[1024];
		sprintf(temp,"runtime info: PC:%x expect:%x a:%x raddr:%x\r\n", PC, expect, a, raddr);
		return "error :" + string(temp)+ pCVir8051->ToStr();
	};

	auto updatecpu = [&]()
	{
		pCVir8051->InitalReg();
		pCVir8051->Sys.PC = PC;
		pCVir8051->Sys.a = a;
		pCVir8051->m_ExeFile[PC] = code;
		pCVir8051->m_ExeFile[PC + 1] = raddr;
	};

	for (PC = 0; PC <= CVir8051::MAX_ROM - 2; PC = PC + space) {
		if (PC % 1000 == 0) {
			LogPrint("INFO", "code = %x PC = %d\n", code, PC);
		}
		for (raddr = -128; raddr <= 127; raddr++) {
			a = rand() % 2;
			expect = (a) ? (PC + 2 + raddr) : (PC + 2);
			if (expect > CVir8051::MAX_ROM || expect < 0) {
				continue;
			}
			updatecpu();
			TestRun();
			if (!checkresult()) {
				return errstring();
			}
		}
	}
	return "OK";
}

string CTestMcu::XRL_A_Rn_1Test(int space) {
	byte code = 0;
	int PC = 0;
	int rr = 0, aa = 0;
	byte expect = 0;
	auto checkresult = [&]()
	{
		if(IsOdd(pCVir8051->Sys.a()) != pCVir8051->Sys.psw().p)
		return false;
		if ((pCVir8051->Sys.a() != expect) || (pCVir8051->Sys.PC != (PC + 1)))
		return false;
		return true;
	};

	auto errstring = [&] () {
		char temp[1024];
		sprintf(temp,"runtime info: PC:%x expect:%x aa:%x rr:%x\r\n", PC, expect, aa, rr);
		return "error :" + string(temp)+ pCVir8051->ToStr();
	};

	auto updatecpu = [&]()
	{
		pCVir8051->InitalReg();
		switch (code) {
			case 0x68:
			pCVir8051->Rges.R0 = rr;
			break;
			case 0x69:
			pCVir8051->Rges.R1 = rr;
			break;
			case 0x6a:
			pCVir8051->Rges.R2 = rr;
			break;
			case 0x6b:
			pCVir8051->Rges.R3 = rr;
			break;
			case 0x6c:
			pCVir8051->Rges.R4 = rr;
			break;
			case 0x6d:
			pCVir8051->Rges.R5 = rr;
			break;
			case 0x6e:
			pCVir8051->Rges.R6 = rr;
			break;
			case 0x6f:
			pCVir8051->Rges.R7 = rr;
			break;
			default:
			break;
		}
		pCVir8051->Sys.PC = PC;
		pCVir8051->Sys.a = aa;
		pCVir8051->m_ExeFile[PC] = code;
	};

	for (code = 0x68; code <= 0x6f; code++) {
		for (PC = 0; PC <= CVir8051::MAX_ROM - 1; PC = PC + space) {
			if (PC % 1000 == 0) {
				LogPrint("INFO", "code = %x PC = %d\n", code, PC);
			}
			for (rr = 0; rr <= 0xff; rr++) {
				for (aa = 0; aa <= 0xff; aa++) {
					expect = aa ^ rr;
					updatecpu();
					TestRun();
					if (!checkresult()) {
						return errstring();
					}
				}
			}
		}
	}
	return "OK";
}
string CTestMcu::XRL_A_Ri_1Test(int space) {
	byte code = 0;
	int PC = 0, rr = 0, aa = 0;
	byte data = 0;
	byte expect = 0;

	auto updatecpu = [&]() {
		pCVir8051->InitalReg();
		switch (code) {
			case 0x66:
			pCVir8051->Rges.R0 = rr;
			break;
			case 0x67:
			pCVir8051->Rges.R1 = rr;
			break;
			default:
			break;
		}
		pCVir8051->SetRamDataAt(rr, data);
		pCVir8051->Sys.PC = PC;
		pCVir8051->Sys.a = aa;
		pCVir8051->m_ExeFile[PC] = code;
	};

	auto checkresult = [&]()
	{
		if(IsOdd(pCVir8051->Sys.a()) != pCVir8051->Sys.psw().p)
		return false;
		if ((pCVir8051->Sys.a() != expect) || (pCVir8051->Sys.PC != (PC + 1)))
		return false;
		return true;
	};

	auto errstring = [&] () {
		char temp[1024];
		sprintf(temp,"runtime info: PC:%x expect:%x rr:%x aa:%x data:%x\r\n", PC, expect, rr, aa, data);
		return "error :" + string(temp)+ pCVir8051->ToStr();
	};

	for (code = 0x66; code <= 0x67; code++) {
		for (PC = 0; PC <= CVir8051::MAX_ROM - 1; PC = PC + space) {
			if (PC % 1000 == 0) {
				LogPrint("INFO", "code = %x PC = %d\n", code, PC);
			}
			for (rr = 8; rr <= CVir8051::MAX_IN_RAM; rr++) {
				for (aa = 0; aa <= 0xff; aa++) {
					data = rand() % 256;
					expect = data ^ aa;
					updatecpu();
					TestRun();
					if (!checkresult()) {
						return errstring();
					}
				}
			}
		}
	}
	return "OK";
}
string CTestMcu::XRL_A_Data_And_DirectTest(int space) {
	byte code;
	int PC = 0, aa = 0, daddr = 0;
	byte expect = 0, ddata = 0, idata = 0;

	auto updatecpu = [&]() {
		pCVir8051->InitalReg();
		switch (code) {
			case 0x64:
			pCVir8051->m_ExeFile[PC + 1] = idata;
			break;
			case 0x65:
			pCVir8051->SetRamData(daddr, ddata);
			pCVir8051->m_ExeFile[PC + 1] = daddr;
			break;
			default:
			break;
		}
		pCVir8051->Sys.PC = PC;
		pCVir8051->Sys.a = aa;
		pCVir8051->m_ExeFile[PC] = code;
	};

	auto checkresult = [&]()
	{
		if(IsOdd(pCVir8051->Sys.a()) != pCVir8051->Sys.psw().p)
		return false;
		if ((pCVir8051->Sys.a() != expect) || (pCVir8051->Sys.PC != (PC + 2)))
		return false;
		return true;
	};

	auto errstring = [&] () {
		char temp[1024];
		sprintf(temp,"runtime info: PC:%x expect:%x aa:%x ddata:%x idata:%x daddr:%x\r\n",
				PC, expect, aa, ddata, idata, daddr);
		return "error :" + string(temp)+ pCVir8051->ToStr();
	};

	code = 0x65;
	for (PC = 0; PC <= CVir8051::MAX_ROM - 2; PC = PC + space) {
		if (PC % 1000 == 0) {
			LogPrint("INFO", "code = %x PC = %d\n", code, PC);
		}
		for (daddr = 8; daddr <= CVir8051::MAX_IN_RAM; daddr++) {
			for (aa = 0; aa <= 0xff; aa++) {
				if (daddr == 0xe0 || daddr == 0xd0) {
					continue; //modify the psw and a
				}
				ddata = rand() % 256;
				expect = aa ^ ddata;
				updatecpu();
				TestRun();
				if (!checkresult()) {
					return errstring();
				}
			}
		}
	}

	code = 0x64;
	for (PC = 0; PC <= CVir8051::MAX_ROM - 2; PC = PC + space) {
		if (PC % 1000 == 0) {
			LogPrint("INFO", "code = %x PC = %d\n", code, PC);
		}
		for (aa = 0; aa <= 0xff; aa++) {
			idata = rand() % 256;
			expect = aa ^ idata;
			updatecpu();
			TestRun();
			if (!checkresult()) {
				return errstring();
			}
		}
	}
	return "OK";
}
string CTestMcu::XRL_Direct_DataTest(int space) {
	byte code = 0x63;
	int PC = 0, daddr = 0, idata = 0;
	byte expect = 0;
	byte ddata = 0;

	auto checkresult = [&]()
	{
		if ((pCVir8051->GetRamData(daddr) != expect) || (pCVir8051->Sys.PC != (PC + 3)))
		return false;
		return true;
	};

	auto errstring = [&] () {
		char temp[1024];
		sprintf(temp,"runtime info: PC:%x expect:%x daddr:%x idata:%x ddata:%x\r\n", PC, expect, daddr, idata, ddata);
		return "error :" + string(temp)+ pCVir8051->ToStr();
	};

	auto updatecpu = [&]() {
		pCVir8051->InitalReg();
		pCVir8051->SetRamData(daddr, ddata);
		pCVir8051->Sys.PC = PC;
		pCVir8051->m_ExeFile[PC] = code;
		pCVir8051->m_ExeFile[PC + 1] = daddr;
		pCVir8051->m_ExeFile[PC + 2] = idata;
	};

	for (PC = 0; PC <= CVir8051::MAX_ROM - 3; PC = PC + space) {
		if (PC % 1000 == 0) {
			LogPrint("INFO", "code = %x PC = %d\n", code, PC);
		}
		for (daddr = 8; daddr <= CVir8051::MAX_IN_RAM; daddr++) {
			for (idata = 0; idata <= 0xff; idata++) {
				ddata = rand() % 256;
				expect = idata ^ ddata;
				updatecpu();
				TestRun();
				if (!checkresult()) {
					return errstring();
				}
			}
		}
	}
	return "OK";
}
string CTestMcu::XRL_Direct_ATest(int space) {
	byte code = 0x62;
	int PC = 0, daddr = 0, aa = 0;
	byte expect = 0;
	byte ddata = 0;

	auto checkresult = [&]()
	{
		if(IsOdd(pCVir8051->Sys.a()) != pCVir8051->Sys.psw().p)
		return false;
		if ((pCVir8051->GetRamData(daddr) != expect) || (pCVir8051->Sys.PC != (PC + 2)))
		return false;
		return true;
	};

	auto errstring = [&] () {
		char temp[1024];
		sprintf(temp,"runtime info: PC:%x expect:%x\r\n", PC, expect);
		return "error :" + string(temp)+ pCVir8051->ToStr();
	};

	auto updatecpu = [&]() {
		pCVir8051->InitalReg();
		pCVir8051->SetRamData(daddr, ddata);
		pCVir8051->Sys.PC = PC;
		pCVir8051->Sys.a = aa;
		pCVir8051->m_ExeFile[PC] = code;
		pCVir8051->m_ExeFile[PC + 1] = daddr;
	};

	for (PC = 0; PC <= CVir8051::MAX_ROM - 2; PC = PC + space) {
		if (PC % 1000 == 0) {
			LogPrint("INFO", "code = %x PC = %d\n", code, PC);
		}
		for (daddr = 8; daddr <= CVir8051::MAX_IN_RAM; daddr++) {
			for (aa = 0; aa <= 0xff; aa++) {
				if (daddr == 0xd0 || daddr == 0xe0) {
					continue; //modify the psw and a
				}
				ddata = rand() % 256;
				expect = aa ^ ddata;
				updatecpu();
				TestRun();
				if (!checkresult()) {
					return errstring();
				}
			}
		}
	}
	return "OK";
}
string CTestMcu::JZ_RelTest(int space) {
	int expect = 0;
	int PC = 0;
	int raddr = 0;
	byte aa = 0, code = 0x60;

	auto checkresult = [&]()
	{
		if(IsOdd(pCVir8051->Sys.a()) != pCVir8051->Sys.psw().p)
		return false;
		if (pCVir8051->Sys.PC != expect)
		return false;
		return true;
	};

	auto errstring = [&] () {
		char temp[1024];
		sprintf(temp,"runtime info: PC:%x expect:%x\r\n", PC, expect);
		return "error :" + string(temp)+ pCVir8051->ToStr();
	};

	auto updatecpu = [&]() {
		pCVir8051->InitalReg();
		pCVir8051->Sys.PC = PC;
		pCVir8051->Sys.a = aa;
		pCVir8051->m_ExeFile[PC] = code;
		pCVir8051->m_ExeFile[PC + 1] = raddr;
	};

	for (PC = 0; PC <= CVir8051::MAX_ROM - 2; PC = PC + space) {
		if (PC % 1000 == 0) {
			LogPrint("INFO", "code = %x PC = %d\n", code, PC);
		}
		for (raddr = -128; raddr <= 127; raddr++) {
			aa = rand() % 2;
			expect = (aa) ? (PC + 2) : (raddr + PC + 2);
			if (expect > CVir8051::MAX_ROM || expect < 0) {
				continue;
			}
			updatecpu();
			TestRun();
			if (!checkresult()) {
				return errstring();
			}
		}
	}
	return "OK";
}
string CTestMcu::ANL_A_Rn_1Test(int space) {
	byte code = 0;
	int PC = 0, rr = 0, aa = 0;
	byte expect = 0;

	auto checkresult = [&]()
	{
		if(IsOdd(pCVir8051->Sys.a()) != pCVir8051->Sys.psw().p)
		return false;
		if ((pCVir8051->Sys.a() != expect) || (pCVir8051->Sys.PC != (PC + 1)))
		return false;
		return true;
	};

	auto errstring = [&] () {
		char temp[1024];
		sprintf(temp,"runtime info: PC:%x expect:%x aa:%x rr:%x\r\n", PC, expect, aa, rr);
		return "error :" + string(temp)+ pCVir8051->ToStr();
	};

	auto updatecpu = [&]() {
		pCVir8051->InitalReg();
		switch (code) {
			case 0x58:
			pCVir8051->Rges.R0 = rr;
			break;
			case 0x59:
			pCVir8051->Rges.R1 = rr;
			break;
			case 0x5a:
			pCVir8051->Rges.R2 = rr;
			break;
			case 0x5b:
			pCVir8051->Rges.R3 = rr;
			break;
			case 0x5c:
			pCVir8051->Rges.R4 = rr;
			break;
			case 0x5d:
			pCVir8051->Rges.R5 = rr;
			break;
			case 0x5e:
			pCVir8051->Rges.R6 = rr;
			break;
			case 0x5f:
			pCVir8051->Rges.R7 = rr;
			break;
			default:
			break;
		}
		pCVir8051->Sys.PC = PC;
		pCVir8051->Sys.a = aa;
		pCVir8051->m_ExeFile[PC] = code;
	};

	for (code = 0x58; code <= 0x5f; code++) {
		for (PC = 0; PC <= CVir8051::MAX_ROM; PC = PC + space) {
			if (PC % 1000 == 0) {
				LogPrint("INFO", "code = %x PC = %d\n", code, PC);
			}
			for (rr = 0; rr <= 0xff; rr++) {
				for (aa = 0; aa <= 0xff; aa++) {
					expect = aa & rr;
					updatecpu();
					TestRun();
					if (!checkresult()) {
						return errstring();
					}
				}
			}
		}
	}
	return "OK";
}

string CTestMcu::ANL_A_Ri_1Test(int space) {
	byte code = 0;
	int PC = 0, rr = 0, aa = 0;
	byte ridata = 0;
	byte expect = 0;

	auto checkresult = [&]()
	{
		if(IsOdd(pCVir8051->Sys.a()) != pCVir8051->Sys.psw().p)
		return false;
		if ((pCVir8051->Sys.a() != expect) || (pCVir8051->Sys.PC != (PC + 1)))
		return false;
		return true;
	};

	auto errstring = [&] () {
		char temp[1024];
		sprintf(temp,"runtime info: PC:%x expect:%x aa:%x rr:%x ridata:%x\r\n", PC, expect, aa, rr, ridata);
		return "error :" + string(temp)+ pCVir8051->ToStr();
	};

	auto updatecpu = [&]() {
		pCVir8051->InitalReg();
		switch (code) {
			case 0x56:
			pCVir8051->Rges.R0 = rr;
			break;
			case 0x57:
			pCVir8051->Rges.R1 = rr;
			break;
			default:
			break;
		}
		pCVir8051->SetRamDataAt(rr, ridata);
		pCVir8051->Sys.PC = PC;
		pCVir8051->Sys.a = aa;
		pCVir8051->m_ExeFile[PC] = code;
	};

	for (code = 0x56; code <= 0x57; code++) {
		for (PC = 0; PC <= CVir8051::MAX_ROM - 1; PC = PC + space) {
			if (PC % 1000 == 0) {
				LogPrint("INFO", "code = %x PC = %d\n", code, PC);
			}
			for (rr = 8; rr <= CVir8051::MAX_IN_RAM; rr++) {
				for (aa = 0; aa <= 0xff; aa++) {
					ridata = rand() % 256;
					expect = ridata & aa;
					updatecpu();
					TestRun();
					if (!checkresult()) {
						return errstring();
					}
				}
			}
		}
	}
	return "OK";
}
string CTestMcu::ANL_A_Data_And_DirectTest(int space) {
	byte code;
	int PC = 0, daddr = 0, aa = 0, idata = 0;
	byte expect = 0, ddata = 0;

	auto checkresult = [&]()
	{
		if(IsOdd(pCVir8051->Sys.a()) != pCVir8051->Sys.psw().p)
		return false;
		if ((pCVir8051->Sys.a() != expect) || (pCVir8051->Sys.PC != (PC + 2)))
		return false;
		return true;
	};

	auto errstring = [&] () {
		char temp[1024];
		sprintf(temp,"runtime info: PC:%x expect:%x\r\n", PC, expect);
		return "error :" + string(temp)+ pCVir8051->ToStr();
	};

	auto updatecpu = [&]() {
		pCVir8051->InitalReg();
		switch (code) {
			case 0x54:
			pCVir8051->m_ExeFile[PC + 1] = idata;
			break;
			case 0x55:
			pCVir8051->SetRamData(daddr, ddata);
			pCVir8051->m_ExeFile[PC + 1] = daddr;
			break;
			default:
			break;
		}
		pCVir8051->Sys.PC = PC;
		pCVir8051->Sys.a = aa;
		pCVir8051->m_ExeFile[PC] = code;
	};

	code = 0x55;
	for (PC = 0; PC <= CVir8051::MAX_ROM - 2; PC = PC + space) {
		if (PC % 1000 == 0) {
			LogPrint("INFO", "code = %x PC = %d\n", code, PC);
		}
		for (daddr = 8; daddr <= CVir8051::MAX_IN_RAM; daddr++) {
			for (aa = 0; aa <= 0xff; aa++) {
				if (daddr == 0xe0 || daddr == 0xd0) {
					continue; //modify the psw and a
				}
				ddata = rand() % 256;
				expect = aa & ddata;
				updatecpu();
				TestRun();
				if (!checkresult()) {
					return errstring();
				}
			}
		}
	}

	code = 0x54;
	for (PC = 0; PC <= CVir8051::MAX_ROM - 2; PC = PC + space) {
		if (PC % 1000 == 0) {
			LogPrint("INFO", "code = %x PC = %d\n", code, PC);
		}
		for (idata = 0; idata <= 0xff; idata++) {
			for (aa = 0; aa <= 0xff; aa++) {
				expect = aa & idata;
				updatecpu();
				TestRun();
				if (!checkresult()) {
					return errstring();
				}
			}
		}
	}
	return "OK";
}
string CTestMcu::ANL_Direct_DataTest(int space) {
	byte code;
	int PC = 0, daddr = 0, idata = 0;
	byte expect;
	byte ddata = 0;

	auto checkresult = [&]()
	{
		if ((pCVir8051->GetRamData(daddr) != expect) || (pCVir8051->Sys.PC != (PC + 3)))
		return false;
		return true;
	};

	auto errstring = [&] () {
		char temp[1024];
		sprintf(temp,"runtime info: PC:%x expect:%x\r\n", PC, expect);
		return "error :" + string(temp)+ pCVir8051->ToStr();
	};

	auto updatecpu = [&]() {
		pCVir8051->InitalReg();
		pCVir8051->SetRamData(daddr, ddata);
		pCVir8051->Sys.PC = PC;
		pCVir8051->m_ExeFile[PC] = code;
		pCVir8051->m_ExeFile[PC + 1] = daddr;
		pCVir8051->m_ExeFile[PC + 2] = idata;
	};

	code = 0x53;
	for (PC = 0; PC <= CVir8051::MAX_ROM - 3; PC = PC + space) {
		if (PC % 1000 == 0) {
			LogPrint("INFO", "code = %x PC = %d\n", code, PC);
		}
		for (daddr = 8; daddr <= CVir8051::MAX_IN_RAM; daddr++) {
			for (idata = 0; idata <= 0xff; idata++) {
				ddata = rand() % 256;
				expect = ddata & idata;
				updatecpu();
				TestRun();
				if (!checkresult()) {
					return errstring();
				}
			}
		}
	}
	return "OK";
}
string CTestMcu::ANL_Direct_ATest(int space) {
	byte code;
	int PC = 0, daddr = 0, aa = 0;
	byte expect;
	byte ddata = 0;

	auto checkresult = [&]()
	{
		if ((pCVir8051->GetRamData(daddr) != expect) || (pCVir8051->Sys.PC != (PC + 2)))
		return false;
		return true;
	};

	auto errstring = [&] () {
		char temp[1024];
		sprintf(temp,"runtime info: PC:%x expect:%x daddr:%x\r\n", PC, expect, daddr);
		return "error :" + string(temp)+ pCVir8051->ToStr();
	};

	auto updatecpu = [&]() {
		pCVir8051->InitalReg();
		pCVir8051->SetRamData(daddr, ddata);
		pCVir8051->Sys.PC = PC;
		pCVir8051->Sys.a = aa;
		pCVir8051->m_ExeFile[PC] = code;
		pCVir8051->m_ExeFile[PC + 1] = daddr;
	};

	code = 0x52;
	for (PC = 0; PC <= CVir8051::MAX_ROM - 2; PC = PC + space) {
		if (PC % 1000 == 0) {
			LogPrint("INFO", "code = %x PC = %d\n", code, PC);
		}
		for (daddr = 8; daddr <= CVir8051::MAX_IN_RAM; daddr++) {
			for (aa = 0; aa <= 0xff; aa++) {
				if (daddr == 0xd0 || daddr == 0xe0) {
					continue; //modify the a and psw
				}
				ddata = rand() % 256;
				expect = aa & ddata;
				updatecpu();
				TestRun();
				if (!checkresult()) {
					return errstring();
				}
			}
		}
	}
	return "OK";
}
string CTestMcu::ORL_A_Rn_1Test(int space) {
	byte code;
	int PC;
	int rr = 0, aa = 0;
	byte expect;

	auto checkresult = [&]()
	{
		if(IsOdd(pCVir8051->Sys.a()) != pCVir8051->Sys.psw().p)
		return false;
		if ((pCVir8051->Sys.a() != expect) || (pCVir8051->Sys.PC != (PC + 1)))
		return false;
		return true;
	};

	auto errstring = [&] () {
		char temp[1024];
		sprintf(temp,"runtime info: PC:%x expect:%x\r\n", PC, expect);
		return "error :" + string(temp)+ pCVir8051->ToStr();
	};

	auto updatecpu = [&]() {
		pCVir8051->InitalReg();
		switch (code) {
			case 0x48:
			pCVir8051->Rges.R0 = rr;
			break;
			case 0x49:
			pCVir8051->Rges.R1 = rr;
			break;
			case 0x4a:
			pCVir8051->Rges.R2 = rr;
			break;
			case 0x4b:
			pCVir8051->Rges.R3 = rr;
			break;
			case 0x4c:
			pCVir8051->Rges.R4 = rr;
			break;
			case 0x4d:
			pCVir8051->Rges.R5 = rr;
			break;
			case 0x4e:
			pCVir8051->Rges.R6 = rr;
			break;
			case 0x4f:
			pCVir8051->Rges.R7 = rr;
			break;
			default:
			break;
		}
		pCVir8051->Sys.PC = PC;
		pCVir8051->Sys.a = aa;
		pCVir8051->m_ExeFile[PC] = code;
	};

	for (code = 0x48; code <= 0x4f; code++) {
		for (PC = 0; PC <= CVir8051::MAX_ROM - 1; PC = PC + space) {
			if (PC % 1000 == 0) {
				LogPrint("INFO", "code = %x PC = %d\n", code, PC);
			}
			for (rr = 0; rr <= 0xff; rr++) {
				for (aa = 0; aa <= 0xff; aa++) {
					expect = aa | rr;
					updatecpu();
					TestRun();
					if (!checkresult()) {
						return errstring();
					}
				}
			}
		}
	}
	return "OK";
}
string CTestMcu::ORL_A_Ri_1Test(int space) {
	byte code;
	int PC = 0, rr = 0, aa = 0;
	byte ridata = 0;
	byte expect;

	auto checkresult = [&]()
	{
		if(IsOdd(pCVir8051->Sys.a()) != pCVir8051->Sys.psw().p)
		return false;
		if ((pCVir8051->Sys.a() != expect) || (pCVir8051->Sys.PC != (PC + 1)))
		return false;
		return true;
	};

	auto errstring = [&] () {
		char temp[1024];
		sprintf(temp,"runtime info: PC:%x expect:%x\r\n", PC, expect);
		return "error :" + string(temp)+ pCVir8051->ToStr();
	};

	auto updatecpu = [&]() {
		pCVir8051->InitalReg();
		switch (code) {
			case 0x46:
			pCVir8051->Rges.R0 = rr;
			break;
			case 0x47:
			pCVir8051->Rges.R1 = rr;
			break;
			default:
			break;
		}
		pCVir8051->SetRamDataAt(rr, ridata);
		pCVir8051->Sys.PC = PC;
		pCVir8051->Sys.a = aa;
		pCVir8051->m_ExeFile[PC] = code;
	};

	for (code = 0x46; code <= 0x47; code++) {
		for (PC = 0; PC <= CVir8051::MAX_ROM - 1; PC = PC + space) {
			if (PC % 1000 == 0) {
				LogPrint("INFO", "code = %x PC = %d\n", code, PC);
			}
			for (rr = 8; rr <= CVir8051::MAX_IN_RAM; rr++) {
				for (aa = 0; aa <= 0xff; aa++) {
					ridata = rand() % 256;
					expect = ridata | aa;
					updatecpu();
					TestRun();
					if (!checkresult()) {
						return errstring();
					}
				}
			}
		}
	}
	return "OK";
}
string CTestMcu::ORL_A_Data_And_DirectTest(int space) {
	byte code;
	int PC = 0, aa = 0, daddr = 0, idata = 0;
	byte expect = 0, ddata = 0;

	auto checkresult = [&]()
	{
		if(IsOdd(pCVir8051->Sys.a()) != pCVir8051->Sys.psw().p)
		return false;
		if ((pCVir8051->Sys.a() != expect) || (pCVir8051->Sys.PC != (PC + 2)))
		return false;
		return true;
	};

	auto errstring = [&] () {
		char temp[1024];
		sprintf(temp,"runtime info: PC:%x expect:%x daddr:%x\r\n", PC, expect, daddr);
		return "error :" + string(temp)+ pCVir8051->ToStr();
	};

	auto updatecpu = [&]() {
		pCVir8051->InitalReg();
		switch (code) {
			case 0x44:
			pCVir8051->m_ExeFile[PC + 1] = idata;
			break;
			case 0x45:
			pCVir8051->SetRamData(daddr, ddata);
			pCVir8051->m_ExeFile[PC + 1] = daddr;
			break;
			default:
			break;
		}
		pCVir8051->Sys.PC = PC;
		pCVir8051->Sys.a = aa;
		pCVir8051->m_ExeFile[PC] = code;
	};

	code = 0x45;
	for (PC = 0; PC <= CVir8051::MAX_ROM - 2; PC = PC + space) {
		if (PC % 1000 == 0) {
			LogPrint("INFO", "code = %x PC = %d\n", code, PC);
		}
		for (daddr = 8; daddr <= CVir8051::MAX_IN_RAM; daddr++) {
			for (aa = 0; aa <= 0xff; aa++) {
				if (daddr == 0xe0 || daddr == 0xd0) {
					continue; //modify the cy and a
				}
				ddata = rand() % 256;
				expect = aa | ddata;
				updatecpu();
				TestRun();
				if (!checkresult()) {
					return errstring();
				}
			}
		}
	}

	code = 0x44;
	for (PC = 0; PC <= CVir8051::MAX_ROM - 2; PC = PC + space) {
		if (PC % 1000 == 0) {
			LogPrint("INFO", "code = %x PC = %d\n", code, PC);
		}
		for (idata = 0; idata <= 0xff; idata++) {
			for (aa = 0; aa <= 0xff; aa++) {
				expect = aa | idata;
				updatecpu();
				TestRun();
				if (!checkresult()) {
					return errstring();
				}
			}
		}
	}
	return "OK";
}
string CTestMcu::ORL_Direct_DataTest(int space) {
	byte code;
	int PC = 0, daddr = 0, idata = 0;
	byte expect;
	byte ddata = 0;

	auto checkresult = [&]()
	{
		if ((pCVir8051->GetRamData(daddr) != expect) || (pCVir8051->Sys.PC != (PC + 3)))
		return false;
		return true;
	};

	auto errstring = [&] () {
		char temp[1024];
		sprintf(temp,"runtime info: PC:%x expect:%x\r\n", PC, expect);
		return "error :" + string(temp)+ pCVir8051->ToStr();
	};

	auto updatecpu = [&]() {
		pCVir8051->InitalReg();
		pCVir8051->SetRamData(daddr, ddata);
		pCVir8051->Sys.PC = PC;
		pCVir8051->m_ExeFile[PC] = code;
		pCVir8051->m_ExeFile[PC + 1] = daddr;
		pCVir8051->m_ExeFile[PC + 2] = idata;
	};

	code = 0x43;
	for (PC = 0; PC <= CVir8051::MAX_ROM - 3; PC = PC + space) {
		if (PC % 1000 == 0) {
			LogPrint("INFO", "code = %x PC = %d\n", code, PC);
		}
		for (daddr = 8; daddr <= CVir8051::MAX_IN_RAM; daddr++) {
			for (idata = 0; idata <= 0xff; idata++) {
				ddata = rand() % 256;
				expect = idata | ddata;
				updatecpu();
				TestRun();
				if (!checkresult()) {
					return errstring();
				}
			}
		}
	}
	return "OK";
}
string CTestMcu::ORL_Direct_ATest(int space) {
	byte code;
	int PC = 0, daddr = 0, aa = 0;
	byte expect;
	byte ddata = 0;

	auto checkresult = [&]()
	{
		if ((pCVir8051->GetRamData(daddr) != expect) || (pCVir8051->Sys.PC != (PC + 2)))
		return false;
		return true;
	};

	auto errstring = [&] () {
		char temp[1024];
		sprintf(temp,"runtime info: PC:%x expect:%x daddr:%x aa:%x\r\n", PC, expect, daddr, aa);
		return "error :" + string(temp)+ pCVir8051->ToStr();
	};

	auto updatecpu = [&]() {
		pCVir8051->InitalReg();
		pCVir8051->SetRamData(daddr, ddata);
		pCVir8051->Sys.PC = PC;
		pCVir8051->Sys.a = aa;
		pCVir8051->m_ExeFile[PC] = code;
		pCVir8051->m_ExeFile[PC + 1] = daddr;
	};

	code = 0x42;
	for (PC = 0; PC <= CVir8051::MAX_ROM - 2; PC = PC + space) {
		if (PC % 1000 == 0) {
			LogPrint("INFO", "code = %x PC = %d\n", code, PC);
		}
		for (daddr = 8; daddr <= CVir8051::MAX_IN_RAM; daddr++) {
			for (aa = 0; aa <= 0xff; aa++) {
				if (daddr == 0xe0 || daddr == 0xd0) {
					continue; //modify the psw and a
				}
				ddata = rand() % 256;
				expect = aa | ddata;
				updatecpu();
				TestRun();
				if (!checkresult()) {
					return errstring();
				}
			}
		}
	}
	return "OK";
}
string CTestMcu::JC_RelTest(int space) {
	byte code = 0x40, cc = 0;
	int raddr = 0;
	int PC = 0, expect = 0;

	auto checkresult = [&]()
	{
		if (pCVir8051->Sys.PC != expect)
		return false;
		return true;
	};

	auto errstring = [&] () {
		char temp[1024];
		sprintf(temp,"runtime info: PC:%x expect:%x\r\n", PC, expect);
		return "error :" + string(temp)+ pCVir8051->ToStr();
	};

	auto updatecpu = [&]() {
		pCVir8051->InitalReg();
		pCVir8051->Sys.PC = PC;
		pCVir8051->Sys.psw().cy = cc;
		pCVir8051->m_ExeFile[PC] = code;
		pCVir8051->m_ExeFile[PC + 1] = raddr;
	};

	for (PC = 0; PC <= CVir8051::MAX_ROM - 2; PC = PC + space) {
		if (PC % 1000 == 0) {
			LogPrint("INFO", "code = %x PC = %d\n", code, PC);
		}
		for (raddr = -128; raddr <= 127; raddr++) {
			cc = rand() % 2;
			expect = (cc) ? (raddr + PC + 2) : (PC + 2);
			if (expect > CVir8051::MAX_ROM || expect < 0) {
				continue;
			}
			updatecpu();
			TestRun();
			if (!checkresult()) {
				return errstring();
			}
		}
	}
	return "OK";
}
string CTestMcu::ADDC_A_RnTest(int space) {
	byte code;
	int PC;
	int expect = 0, aa = 0, rr = 0;
	byte cc = 0;

	auto checkresult = [&]()
	{
		byte xx6 = (((aa & 0x7f) + (rr & 0x7f) + cc) > 0x7f) ? (1) : (0);
		byte xx7 = (expect > 0xff)?(1):(0);
		if(IsOdd(pCVir8051->Sys.a()) != pCVir8051->Sys.psw().p)
		return false;
		if(pCVir8051->Sys.psw().cy != ((expect > 0xff)?(1):(0)))
		return false;
		if(pCVir8051->Sys.psw().ac != ((((aa&0x0f) + (rr&0x0f) + cc) > 0x0f)?(1):(0)))
		return false;
		if(pCVir8051->Sys.psw().ov != ((xx7 ^ xx6)?(1):(0)))
		return false;
		if ((pCVir8051->Sys.a() != (byte)expect) || (pCVir8051->Sys.PC != (PC + 1)))
		return false;
		return true;
	};

	auto errstring = [&] () {
		char temp[1024];
		byte xx6 = (((aa & 0x7f) + (rr & 0x7f) + cc) > 0x7f) ? (1) : (0);
		byte xx7 = (expect > 0xff)?(1):(0);
		sprintf(temp,"runtime info: PC:%x expect:%x aa:%x rr:%x cc:%x xx6:%x xx7:%x\r\n", PC, expect, aa, rr, cc,
				xx6, xx7);
		return "error :" + string(temp)+ pCVir8051->ToStr();
	};

	auto updatecpu = [&]() {
		pCVir8051->InitalReg();
		switch (code) {
			case 0x38:
			pCVir8051->Rges.R0 = rr;
			break;
			case 0x39:
			pCVir8051->Rges.R1 = rr;
			break;
			case 0x3a:
			pCVir8051->Rges.R2 = rr;
			break;
			case 0x3b:
			pCVir8051->Rges.R3 = rr;
			break;
			case 0x3c:
			pCVir8051->Rges.R4 = rr;
			break;
			case 0x3d:
			pCVir8051->Rges.R5 = rr;
			break;
			case 0x3e:
			pCVir8051->Rges.R6 = rr;
			break;
			case 0x3f:
			pCVir8051->Rges.R7 = rr;
			break;
			default:
			break;
		}
		pCVir8051->Sys.PC = PC;
		pCVir8051->Sys.a = aa;
		pCVir8051->Sys.psw().cy = cc;
		pCVir8051->m_ExeFile[PC] = code;
	};

	for (code = 0x38; code <= 0x3f; code++) {
		for (PC = 0; PC <= CVir8051::MAX_ROM - 1; PC = PC + space) {
			if (PC % 1000 == 0) {
				LogPrint("INFO", "code = %x PC = %d\n", code, PC);
			}
			for (rr = 0; rr <= 0xff; rr++) {
				for (aa = 0; aa <= 0xff; aa++) {
					cc = rand() % 2;
					expect = aa + rr + cc;
					updatecpu();
					TestRun();
					if (!checkresult()) {
						return errstring();
					}
				}
			}
		}
	}
	return "OK";
}
string CTestMcu::ADDC_A_RiTest(int space) {
	byte code;
	int PC = 0, rr = 0, aa = 0;
	int expect;
	int cc;
	byte ridata = 0;

	auto checkresult = [&]()
	{
		byte xx6 = (((aa & 0x7f) + (ridata & 0x7f) + cc) > 0x7f) ? (1) : (0);
		byte xx7 = (expect > 0xff)?(1):(0);
		if(IsOdd(pCVir8051->Sys.a()) != pCVir8051->Sys.psw().p)
		{
			LogPrint("INFO","1\n");
			return false;
		}

		if(pCVir8051->Sys.psw().cy != ((expect > 0xff)?(1):(0)))
		{
			LogPrint("INFO","2\n");
			return false;
		}
		if(pCVir8051->Sys.psw().ac != ((((aa&0x0f) + (ridata&0x0f) + cc) > 0x0f)?(1):(0)))
		{
			LogPrint("INFO","3\n");
			return false;
		}
		if(pCVir8051->Sys.psw().ov != ((xx7 ^ xx6)?(1):(0)))
		{
			LogPrint("INFO","4\n");
			return false;
		}
		if ((pCVir8051->Sys.a() != (byte)expect) || (pCVir8051->Sys.PC != (PC + 1)))
		{
			LogPrint("INFO","5\n");
			return false;
		}
		return true;
	};

	auto errstring = [&] () {
		char temp[1024];
		byte xx6 = (((aa & 0x7f) + (ridata & 0x7f) + cc) > 0x7f) ? (1) : (0);
		byte xx7 = (expect > 0xff)?(1):(0);
		sprintf(temp,"runtime info: PC:%x expect:%x aa:%x rr:%x cc:%x xx6:%x xx7:%x 00:%x\r\n", PC, expect, aa, rr, cc,
				xx6, xx7, pCVir8051->GetRamDataAt(0));
		return "error :" + string(temp)+ pCVir8051->ToStr();
	};

	auto updatecpu = [&]() {
		pCVir8051->InitalReg();
		switch (code) {
			case 0x36:
			pCVir8051->Rges.R0 = rr;
			break;
			case 0x37:
			pCVir8051->Rges.R1 = rr;
			break;
			default:
			break;
		}
		pCVir8051->SetRamDataAt(rr, ridata);
		pCVir8051->Sys.PC = PC;
		pCVir8051->Sys.a = aa;
		pCVir8051->Sys.psw().cy = cc;
		pCVir8051->m_ExeFile[PC] = code;
	};

	for (code = 0x36; code <= 0x37; code++) {
		for (PC = 0; PC <= CVir8051::MAX_ROM - 1; PC = PC + space) {
			if (PC % 1000 == 0) {
				LogPrint("INFO", "code = %x PC = %d\n", code, PC);
			}
			for (rr = 8; rr <= CVir8051::MAX_IN_RAM; rr++) {
				for (aa = 0; aa <= 0xff; aa++) {
					ridata = rand() % 256;
					cc = rand() % 2;
					expect = aa + ridata + cc;
					updatecpu();
					TestRun();
					if (!checkresult()) {
						return errstring();
					}
				}
			}
		}
	}
	return "OK";
}
string CTestMcu::ADDC_A_DirectTest(int space) {
	byte code;
	int PC;
	int expect = 0, addr = 0, aa = 0;
	byte data;
	int cc;

	auto checkresult = [&]()
	{
		byte xx6 = (((aa & 0x7f) + (data & 0x7f) + cc) > 0x7f) ? (1) : (0);
		byte xx7 = (expect > 0xff)?(1):(0);
		if(IsOdd(pCVir8051->Sys.a()) != pCVir8051->Sys.psw().p)
		{
			LogPrint("INFO","1\n");
			return false;
		}
		if(pCVir8051->Sys.psw().cy != ((expect > 0xff)?(1):(0)))
		{
			LogPrint("INFO","2\n");
			return false;
		}
		if(pCVir8051->Sys.psw().ac != ((((aa&0x0f) + (data&0x0f) + cc) > 0x0f)?(1):(0)))
		{
			LogPrint("INFO","3\n");
			return false;
		}
		if(pCVir8051->Sys.psw().ov != ((xx7 ^ xx6)?(1):(0)))
		{
			LogPrint("INFO","4\n");
			return false;
		}
		if ((pCVir8051->Sys.a() != (byte)expect) || (pCVir8051->Sys.PC != (PC + 2)))
		{
			LogPrint("INFO","5\n");
			return false;
		}
		return true;
	};

	auto errstring = [&] () {
		char temp[1024];
		byte xx6 = (((aa & 0x7f) + (data & 0x7f) + cc) > 0x7f) ? (1) : (0);
		byte xx7 = (expect > 0xff)?(1):(0);
		sprintf(temp,"runtime info: PC:%x expect:%x aa:%x data:%x cc:%x xx6:%x xx7:%x addr:%x\r\n",
				PC, expect, aa, data, cc, xx6, xx7, addr);
		return "error :" + string(temp)+ pCVir8051->ToStr();
	};

	auto updatecpu = [&]() {
		pCVir8051->InitalReg();
		pCVir8051->SetRamData(addr, data);
		pCVir8051->Sys.PC = PC;
		pCVir8051->Sys.a = aa;
		pCVir8051->Sys.psw().cy = cc;
		pCVir8051->m_ExeFile[PC] = code;
		pCVir8051->m_ExeFile[PC + 1] = addr;
	};

	code = 0x35;
	for (PC = 0; PC <= CVir8051::MAX_ROM - 2; PC = PC + space) {
		if (PC % 1000 == 0) {
			LogPrint("INFO", "code = %x PC = %d\n", code, PC);
		}
		for (addr = 8; addr <= CVir8051::MAX_IN_RAM; addr++) {
			for (aa = 0; aa <= 0xff; aa++) {
				if (addr == 0xd0 || addr == 0xe0) {
					continue; //modify the psw and a
				}
				data = rand() % 256;
				cc = rand() % 2;
				expect = aa + data + cc;
				updatecpu();
				TestRun();
				if (!checkresult()) {
					return errstring();
				}
			}
		}
	}
	return "OK";
}
string CTestMcu::ADDC_A_DataTest(int space) {
	byte code;
	int PC;
	int expect = 0, data = 0, aa = 0;
	int cc;

	auto checkresult = [&]()
	{
		byte xx6 = (((aa & 0x7f) + (data & 0x7f) + cc) > 0x7f) ? (1) : (0);
		byte xx7 = (expect > 0xff)?(1):(0);
		if(IsOdd(pCVir8051->Sys.a()) != pCVir8051->Sys.psw().p)
		return false;
		if(pCVir8051->Sys.psw().cy != ((expect > 0xff)?(1):(0)))
		return false;
		if(pCVir8051->Sys.psw().ac != ((((aa&0x0f) + (data&0x0f) + cc) > 0x0f)?(1):(0)))
		return false;
		if(pCVir8051->Sys.psw().ov != ((xx7 ^ xx6)?(1):(0)))
		return false;
		if ((pCVir8051->Sys.a() != (byte)expect) || (pCVir8051->Sys.PC != (PC + 2)))
		return false;
		return true;
	};

	auto errstring = [&] () {
		char temp[1024];
		byte xx6 = (((aa & 0x7f) + (data & 0x7f) + cc) > 0x7f) ? (1) : (0);
		byte xx7 = (expect > 0xff)?(1):(0);
		sprintf(temp,"runtime info: PC:%x expect:%x aa:%x data:%x cc:%x xx6:%x xx7:%x\r\n", PC, expect, aa, data, cc,
				xx6, xx7);
		return "error :" + string(temp)+ pCVir8051->ToStr();
	};

	auto updatecpu = [&]() {
		pCVir8051->InitalReg();
		pCVir8051->Sys.PC = PC;
		pCVir8051->Sys.a = aa;
		pCVir8051->m_ExeFile[PC] = code;
		pCVir8051->m_ExeFile[PC + 1] = data;
		pCVir8051->Sys.psw().cy = cc;
	};

	code = 0x34;
	for (PC = 0; PC <= CVir8051::MAX_ROM - 2; PC = PC + space) {
		if (PC % 1000 == 0) {
			LogPrint("INFO", "code = %x PC = %d\n", code, PC);
		}
		for (data = 0; data <= 0xff; data++) {
			for (aa = 0; aa <= 0xff; aa++) {
				cc = rand() % 2;
				expect = aa + data + cc;
				updatecpu();
				TestRun();
				if (!checkresult()) {
					return errstring();
				}
			}
		}
	}
	return "OK";
}
string CTestMcu::RLC_ATest(int space) {
	byte code;
	int PC;
	int aa;
	int cc;

	auto checkresult = [&]()
	{
		if(IsOdd(pCVir8051->Sys.a()) != pCVir8051->Sys.psw().p)
		return false;
		if(pCVir8051->Sys.a() != (byte)((aa << 1)|cc))
		return false;
		if(pCVir8051->Sys.psw().cy != aa >> 7)
		return false;
		if (pCVir8051->Sys.PC != (PC + 1))
		return false;
		return true;
	};

	auto errstring = [&] () {
		char temp[1024];
		sprintf(temp,"runtime info: PC:%x aa:%x cc:%x\r\n", PC, aa, cc);
		return "error :" + string(temp)+ pCVir8051->ToStr();
	};

	auto updatecpu = [&]() {
		pCVir8051->InitalReg();
		pCVir8051->Sys.PC = PC;
		pCVir8051->Sys.a = aa;
		pCVir8051->Sys.psw().cy = cc;
		pCVir8051->m_ExeFile[PC] = code;
	};

	code = 0x33;
	for (PC = 0; PC <= CVir8051::MAX_ROM - 1; PC = PC + space) {
		if (PC % 1000 == 0) {
			LogPrint("INFO", "code = %x PC = %d\n", code, PC);
		}
		for (aa = 0; aa <= 0xff; aa++) {
			cc = rand() % 2;
			updatecpu();
			TestRun();
			if (!checkresult()) {
				return errstring();
			}
		}
	}
	return "OK";
}
string CTestMcu::RETITest(int space) {
	return "OK";
}
string CTestMcu::JNB_Bit_RelTest(int space) {
	byte code = 0x30, bb = 0;
	int raddr = 0, baddr = 0;
	int PC = 0, expect = 0;

	auto checkresult = [&]()
	{
		if(pCVir8051->Sys.PC != expect)
		return false;
		return true;
	};

	auto errstring = [&] () {
		char temp[1024];
		sprintf(temp,"runtime info: PC:%x expect:%x bb:%x baddr:%x raddr:%x\r\n", PC, expect, bb, baddr, raddr);
		return "error :" + string(temp)+ pCVir8051->ToStr();
	};

	auto updatecpu = [&]() {
		pCVir8051->InitalReg();
		if (bb) {
			pCVir8051->SetBitFlag(baddr);
		} else {
			pCVir8051->ClrBitFlag(baddr);
		}
		pCVir8051->Sys.PC = PC;
		pCVir8051->m_ExeFile[PC] = code;
		pCVir8051->m_ExeFile[PC + 1] = baddr;
		pCVir8051->m_ExeFile[PC + 2] = raddr;
	};

	for (PC = 0; PC <= CVir8051::MAX_ROM - 3; PC = PC + space) {
		if (PC % 1000 == 0) {
			LogPrint("INFO", "code = %x PC = %d\n", code, PC);
		}
		for (baddr = 0; baddr <= 0xff; baddr++) {
			for (raddr = -128; raddr <= 127; raddr++) {
				bb = rand() % 2;
				expect = (bb) ? (PC + 3) : (raddr + PC + 3);
				if (expect >= CVir8051::MAX_ROM || expect < 0) {
					continue;
				}
				updatecpu();
				TestRun();
				if (!checkresult()) {
					return errstring();
				}
			}
		}
	}
	return "OK";
}
string CTestMcu::ADD_A_RnTest(int space) {
	byte code;
	int PC;
	int expect = 0, rr = 0, aa = 0;

	auto checkresult = [&]()
	{
		byte xx6 = (((aa & 0x7f) + (rr & 0x7f)) > 0x7f) ? (1) : (0);
		byte xx7 = (expect > 0xff)?(1):(0);
		if(IsOdd(pCVir8051->Sys.a()) != pCVir8051->Sys.psw().p)
		return false;
		if(pCVir8051->Sys.psw().cy != ((expect > 0xff)?(1):(0)))
		return false;
		if(pCVir8051->Sys.psw().ac != ((((aa&0x0f) + (rr&0x0f)) > 0x0f)?(1):(0)))
		return false;
		if(pCVir8051->Sys.psw().ov != ((xx7 ^ xx6)?(1):(0)))
		return false;
		if ((pCVir8051->Sys.a() != (byte)expect) || (pCVir8051->Sys.PC != (PC + 1)))
		return false;
		return true;
	};

	auto errstring = [&] () {
		char temp[1024];
		byte xx6 = (((aa & 0x7f) + (rr & 0x7f)) > 0x7f) ? (1) : (0);
		byte xx7 = (expect > 0xff)?(1):(0);
		sprintf(temp,"runtime info: PC:%x expect:%x aa:%x rr:%x xx6:%x xx7:%x\r\n", PC, expect, aa, rr,
				xx6, xx7);
		return "error :" + string(temp)+ pCVir8051->ToStr();
	};

	auto updatecpu = [&]() {
		pCVir8051->InitalReg();
		switch (code) {
			case 0x28:
			pCVir8051->Rges.R0 = rr;
			break;
			case 0x29:
			pCVir8051->Rges.R1 = rr;
			break;
			case 0x2a:
			pCVir8051->Rges.R2 = rr;
			break;
			case 0x2b:
			pCVir8051->Rges.R3 = rr;
			break;
			case 0x2c:
			pCVir8051->Rges.R4 = rr;
			break;
			case 0x2d:
			pCVir8051->Rges.R5 = rr;
			break;
			case 0x2e:
			pCVir8051->Rges.R6 = rr;
			break;
			case 0x2f:
			pCVir8051->Rges.R7 = rr;
			break;
			default:
			break;
		}
		pCVir8051->Sys.PC = PC;
		pCVir8051->Sys.a = aa;
		pCVir8051->m_ExeFile[PC] = code;
	};

	for (code = 0x28; code <= 0x2f; code++) {
		for (PC = 0; PC <= CVir8051::MAX_ROM - 1; PC = PC + space) {
			if (PC % 1000 == 0) {
				LogPrint("INFO", "code = %x PC = %d\n", code, PC);
			}
			for (rr = 0; rr <= 0xff; rr++) {
				for (aa = 0; aa <= 0xff; aa++) {
					expect = aa + rr;
					updatecpu();
					TestRun();
					if (!checkresult()) {
						return errstring();
					}
				}
			}
		}
	}
	return "OK";
}
string CTestMcu::ADD_A_Ri_1Test(int space) {
	byte code;
	int PC;
	int expect = 0, aa = 0, rr = 0;
	byte data;

	auto checkresult = [&]()
	{
		byte xx6 = (((aa & 0x7f) + (data & 0x7f)) > 0x7f) ? (1) : (0);
		byte xx7 = (expect > 0xff)?(1):(0);
		if(IsOdd(pCVir8051->Sys.a()) != pCVir8051->Sys.psw().p)
		{
			LogPrint("INFO","1\n");
			return false;
		}

		if(pCVir8051->Sys.psw().cy != ((expect > 0xff)?(1):(0)))
		{
			LogPrint("INFO","2\n");
			return false;
		}
		if(pCVir8051->Sys.psw().ac != ((((aa&0x0f) + (data&0x0f)) > 0x0f)?(1):(0)))
		{
			LogPrint("INFO","3\n");
			return false;
		}
		if(pCVir8051->Sys.psw().ov != ((xx7 ^ xx6)?(1):(0)))
		{
			LogPrint("INFO","4\n");
			return false;
		}
		if ((pCVir8051->Sys.a() != (byte)expect) || (pCVir8051->Sys.PC != (PC + 1)))
		{
			LogPrint("INFO","5\n");
			return false;
		}
		return true;
	};

	auto errstring = [&] () {
		char temp[1024];
		byte xx6 = (((aa & 0x7f) + (data & 0x7f)) > 0x7f) ? (1) : (0);
		byte xx7 = (expect > 0xff)?(1):(0);
		sprintf(temp,"runtime info: PC:%x expect:%x aa:%x rr:%x xx6:%x xx7:%x data:%x\r\n", PC, expect, aa, rr,
				xx6, xx7, data);
		return "error :" + string(temp)+ pCVir8051->ToStr();
	};

	auto updatecpu = [&]() {
		pCVir8051->InitalReg();
		switch (code) {
			case 0x26:
			pCVir8051->Rges.R0 = rr;
			break;
			case 0x27:
			pCVir8051->Rges.R1 = rr;
			break;
			default:
			break;
		}
		pCVir8051->SetRamDataAt(rr, data);
		pCVir8051->Sys.PC = PC;
		pCVir8051->Sys.a = aa;
		pCVir8051->m_ExeFile[PC] = code;
	};

	for (code = 0x26; code <= 0x27; code++) {
		for (PC = 0; PC <= CVir8051::MAX_ROM - 1; PC = PC + space) {
			if (PC % 1000 == 0) {
				LogPrint("INFO", "code = %x PC = %d\n", code, PC);
			}
			for (rr = 8; rr <= CVir8051::MAX_IN_RAM; rr++) {
				for (aa = 0; aa <= 0xff; aa++) {
					data = rand() % 256;
					expect = aa + data;
					updatecpu();
					TestRun();
					if (!checkresult()) {
						return errstring();
					}
				}
			}
		}
	}
	return "OK";
}
string CTestMcu::ADD_A_DirectTest(int space) {
	byte code;
	int PC;
	int expect = 0, aa = 0, addr = 0;
	byte data;

	auto checkresult = [&]()
	{
		byte xx6 = (((aa & 0x7f) + (data & 0x7f)) > 0x7f) ? (1) : (0);
		byte xx7 = (expect > 0xff)?(1):(0);
		if(IsOdd(pCVir8051->Sys.a()) != pCVir8051->Sys.psw().p)
		return false;
		if(pCVir8051->Sys.psw().cy != ((expect > 0xff)?(1):(0)))
		return false;
		if(pCVir8051->Sys.psw().ac != ((((aa&0x0f) + (data&0x0f)) > 0x0f)?(1):(0)))
		return false;
		if(pCVir8051->Sys.psw().ov != ((xx7 ^ xx6)?(1):(0)))
		return false;
		if ((pCVir8051->Sys.a() != (byte)expect) || (pCVir8051->Sys.PC != (PC + 2)))
		return false;
		return true;
	};

	auto errstring = [&] () {
		char temp[1024];
		byte xx6 = (((aa & 0x7f) + (data & 0x7f)) > 0x7f) ? (1) : (0);
		byte xx7 = (expect > 0xff)?(1):(0);
		sprintf(temp,"runtime info: PC:%x expect:%x aa:%x data:%x xx6:%x xx7:%x\r\n", PC, expect, aa, data,
				xx6, xx7);
		return "error :" + string(temp)+ pCVir8051->ToStr();
	};

	auto updatecpu = [&] () {
		pCVir8051->InitalReg();
		pCVir8051->SetRamData(addr, data);
		pCVir8051->Sys.PC = PC;
		pCVir8051->Sys.a = aa;
		pCVir8051->m_ExeFile[PC] = code;
		pCVir8051->m_ExeFile[PC + 1] = addr;
	};

	code = 0x25;
	for (PC = 0; PC <= CVir8051::MAX_ROM - 2; PC = PC + space) {
		if (PC % 1000 == 0) {
			LogPrint("INFO", "code = %x PC = %d\n", code, PC);
		}
		for (addr = 8; addr <= CVir8051::MAX_IN_RAM; addr++) {
			for (aa = 0; aa <= 0xff; aa++) {
				if (addr == 0xe0 || addr == 0xd0) {
					continue; //modify the psw and a
				}
				data = rand() % 256;
				expect = aa + data;
				updatecpu();
				TestRun();
				if (!checkresult()) {
					return errstring();
				}
			}
		}
	}
	return "OK";
}
string CTestMcu::ADD_A_DataTest(int space) {
	byte code;
	int PC;
	int expect = 0, aa = 0, data = 0;

	auto checkresult = [&]()
	{
		byte xx6 = (((aa & 0x7f) + (data & 0x7f)) > 0x7f) ? (1) : (0);
		byte xx7 = (expect > 0xff)?(1):(0);
		if(IsOdd(pCVir8051->Sys.a()) != pCVir8051->Sys.psw().p)
		return false;
		if(pCVir8051->Sys.psw().cy != ((expect > 0xff)?(1):(0)))
		return false;
		if(pCVir8051->Sys.psw().ac != ((((aa&0x0f) + (data&0x0f)) > 0x0f)?(1):(0)))
		return false;
		if(pCVir8051->Sys.psw().ov != ((xx7 ^ xx6)?(1):(0)))
		return false;
		if ((pCVir8051->Sys.a() != (byte)expect) || (pCVir8051->Sys.PC != (PC + 2)))
		return false;
		return true;
	};

	auto errstring = [&] () {
		char temp[1024];
		byte xx6 = (((aa & 0x7f) + (data & 0x7f)) > 0x7f) ? (1) : (0);
		byte xx7 = (expect > 0xff)?(1):(0);
		sprintf(temp,"runtime info: PC:%x expect:%x aa:%x data:%x xx6:%x xx7:%x\r\n", PC, expect, aa, data,
				xx6, xx7);
		return "error :" + string(temp)+ pCVir8051->ToStr();
	};

	auto updatecpu = [&] () {
		pCVir8051->InitalReg();
		pCVir8051->Sys.PC = PC;
		pCVir8051->Sys.a = aa;
		pCVir8051->m_ExeFile[PC] = code;
		pCVir8051->m_ExeFile[PC + 1] = data;
	};

	code = 0x24;
	for (PC = 0; PC <= CVir8051::MAX_ROM - 2; PC = PC + space) {
		if (PC % 1000 == 0) {
			LogPrint("INFO", "code = %x PC = %d\n", code, PC);
		}
		for (data = 0; data <= 0xff; data++) {
			for (aa = 0; aa <= 0xff; aa++) {
				expect = aa + data;
				updatecpu();
				TestRun();
				if (!checkresult()) {
					return errstring();
				}
			}
		}
	}
	return "OK";
}
string CTestMcu::RL_ATest(int space) {
	int aa = 0, PC = 0;
	byte code = 0x23, expect = 0;

	auto checkresult = [&]()
	{
		if(IsOdd(pCVir8051->Sys.a()) != pCVir8051->Sys.psw().p)
		return false;
		if ((pCVir8051->Sys.a() != (byte)expect) || (pCVir8051->Sys.PC != (PC + 1)))
		return false;
		return true;
	};

	auto errstring = [&] () {
		char temp[1024];
		sprintf(temp,"runtime info: PC:%x aa:%x expect:%x\r\n", PC, aa, expect);
		return "error :" + string(temp)+ pCVir8051->ToStr();
	};

	auto updatecpu = [&] () {
		pCVir8051->InitalReg();
		pCVir8051->Sys.PC = PC;
		pCVir8051->Sys.a = aa;
		pCVir8051->m_ExeFile[PC] = code;
	};

	for (PC = 0; PC <= CVir8051::MAX_ROM - 1; PC = PC + space) {
		if (PC % 1000 == 0) {
			LogPrint("INFO", "code = %x PC = %d\n", code, PC);
		}
		for (aa = 0; aa <= 0xFF; aa++) {
			expect = (byte) (((aa & 0x7f) << 1) | ((aa & 0x80) >> 7));
			updatecpu();
			TestRun();
			if (!checkresult()) {
				return errstring();
			}
		}
	}
	return "OK";
}
string CTestMcu::RETTest(int space) {
	int PC = 0, address = 0;
	int expectPC;
	byte expectSP;
	byte Value1, Value2, code = 0x22;

	auto errstring = [&] () {
		char temp[1024];
		sprintf(temp,"runtime info: expect-pc:%x expect-sp:%x Value1:%x Value2:%x\r\n",
				expectPC, expectSP, Value1, Value2);
		return "error :" + string(temp)+ pCVir8051->ToStr();
	};

	auto checkresult = [&]()
	{
		if (pCVir8051->Sys.PC != expectPC || pCVir8051->Sys.sp() != expectSP)
		return false;
		return true;
	};

	auto updatecpu = [&] () {
		pCVir8051->InitalReg();
		pCVir8051->Sys.PC = PC;
		pCVir8051->Sys.sp = address;
		pCVir8051->m_ExeFile[PC] = code;
		pCVir8051->SetRamDataAt(address, Value1);
		pCVir8051->SetRamDataAt(address - 1, Value2);
	};

	for (PC = 0; PC <= CVir8051::MAX_ROM; PC = PC + space) {
		if (PC % 1000 == 0) {
			LogPrint("INFO", "code = %x PC = %d\n", code, PC);
		}
		for (address = 0x00; address <= CVir8051::MAX_IN_RAM; address++) {
			Value1 = rand() % 0xff;
			Value2 = rand() % 0xff;
			expectPC = (int) ((Value1 << 8) | (Value2));
			if (address < 2) {
				continue;
			}
			expectSP = address - 2;
			updatecpu();
			TestRun();
			if (!checkresult()) {
				return errstring();
			}

		}
	}
	return "OK";
}
string CTestMcu::JB_Bit_RelTest(int space) {
	int PC = 0, expect = 0, baddr = 0;
	byte code = 0x20, bb = 0;
	int raddr = 0;

	auto errstring = [&] () {
		char temp[1024];
		sprintf(temp,"runtime info: PC:%x bb:%x baddr:%x raddr:%x expect:%x\r\n",
				PC, bb, baddr, raddr, expect);
		return "error :" + string(temp)+ pCVir8051->ToStr();
	};

	auto checkresult = [&]()
	{
		if (pCVir8051->Sys.PC != expect)
		return false;
		return true;
	};

	auto updatecpu = [&] () {
		pCVir8051->InitalReg();
		if (bb) {
			pCVir8051->SetBitFlag(baddr);
		} else {
			pCVir8051->ClrBitFlag(baddr);
		}
		pCVir8051->Sys.PC = PC;
		pCVir8051->m_ExeFile[PC] = code;
		pCVir8051->m_ExeFile[PC + 1] = baddr;
		pCVir8051->m_ExeFile[PC + 2] = raddr;
	};

	for (PC = 0; PC <= CVir8051::MAX_ROM - 3; PC = PC + space) {
		if (PC % 1000 == 0) {
			LogPrint("INFO", "code = %x PC = %d\n", code, PC);
		}
		for (baddr = 0; baddr <= 0xff; baddr++) {
			for (raddr = -128; raddr <= 127; raddr++) {
				bb = rand() % 2;
				expect = (bb) ? (PC + 3 + raddr) : (PC + 3);
				if (expect > CVir8051::MAX_ROM || expect < 0) {
					continue;
				}
				updatecpu();
				TestRun();
				if (!checkresult()) {
					return errstring();
				}
			}
		}
	}
	return "OK";
}
string CTestMcu::DEC_RnTest(int space) {
	byte code;
	int PC;
	int rr;
	byte expect;

	auto errstring = [&] () {
		char temp[1024];
		sprintf(temp,"runtime info: PC:%x expect:%x\r\n", PC, expect);
		return "error :" + string(temp)+ pCVir8051->ToStr();
	};

	auto checkresult = [&]()
	{
		byte ret = 0;
		switch (code) {
			case 0x18:
			ret = pCVir8051->Rges.R0();
			break;
			case 0x19:
			ret = pCVir8051->Rges.R1();
			break;
			case 0x1a:
			ret = pCVir8051->Rges.R2();
			break;
			case 0x1b:
			ret = pCVir8051->Rges.R3();
			break;
			case 0x1c:
			ret = pCVir8051->Rges.R4();
			break;
			case 0x1d:
			ret = pCVir8051->Rges.R5();
			break;
			case 0x1e:
			ret = pCVir8051->Rges.R6();
			break;
			case 0x1f:
			ret = pCVir8051->Rges.R7();
			break;
			default:
			ret = 0;
			break;
		}
		if(ret != expect)
		return false;
		if (pCVir8051->Sys.PC != PC + 1)
		return false;
		return true;
	};

	auto updatecpu = [&] () {
		pCVir8051->InitalReg();
		switch (code) {
			case 0x18:
			pCVir8051->Rges.R0 = rr;
			break;
			case 0x19:
			pCVir8051->Rges.R1 = rr;
			break;
			case 0x1a:
			pCVir8051->Rges.R2 = rr;
			break;
			case 0x1b:
			pCVir8051->Rges.R3 = rr;
			break;
			case 0x1c:
			pCVir8051->Rges.R4 = rr;
			break;
			case 0x1d:
			pCVir8051->Rges.R5 = rr;
			break;
			case 0x1e:
			pCVir8051->Rges.R6 = rr;
			break;
			case 0x1f:
			pCVir8051->Rges.R7 = rr;
			break;
			default:
			break;
		}
		pCVir8051->Sys.PC = PC;
		pCVir8051->m_ExeFile[PC] = code;
	};

	for (code = 0x18; code <= 0x1f; code++) {
		for (PC = 0; PC <= CVir8051::MAX_ROM - 1; PC = PC + space) {
			if (PC % 1000 == 0) {
				LogPrint("INFO", "code = %x PC = %d\n", code, PC);
			}
			for (rr = 0; rr <= 0xff; rr++) {
				expect = (rr == 0) ? (0xff) : (rr - 1);
				updatecpu();
				TestRun();
				if (!checkresult()) {
					return errstring();
				}
			}
		}
	}
	return "OK";
}
string CTestMcu::DEC_Ri_1Test(int space) {
	byte code;
	int PC;
	int rr;
	byte expect, ridata = 0;

	auto errstring = [&] () {
		char temp[1024];
		sprintf(temp,"runtime info: PC:%x expect:%x\r\n", PC, expect);
		return "error :" + string(temp)+ pCVir8051->ToStr();
	};

	auto updatecpu = [&] ()
	{
		pCVir8051->InitalReg();
		switch (code) {
			case 0x16:
			pCVir8051->Rges.R0 = rr;
			break;
			case 0x17:
			pCVir8051->Rges.R1 = rr;
			break;
			default:
			break;
		}
		pCVir8051->SetRamDataAt(rr, ridata);
		pCVir8051->Sys.PC = PC;
		pCVir8051->m_ExeFile[PC] = code;
	};

	auto checkresult = [&]()
	{
		if ((pCVir8051->GetRamDataAt(rr) != expect) || (pCVir8051->Sys.PC != (PC + 1)))
		return false;
		return true;
	};

	for (code = 0x16; code <= 0x17; code++) {
		for (PC = 0; PC <= CVir8051::MAX_ROM - 1; PC = PC + space) {
			if (PC % 1000 == 0) {
				LogPrint("INFO", "code = %x PC = %d\n", code, PC);
			}
			for (rr = 8; rr <= CVir8051::MAX_IN_RAM; rr++) {
				ridata = rand() % 256;
				expect = (ridata == 0) ? (0xff) : (ridata - 1);
				updatecpu();
				TestRun();
				if (!checkresult()) {
					return errstring();
				}
			}
		}
	}
	return "OK";
}
string CTestMcu::DEC_DirectTest(int space) {
	byte code;
	int PC = 0, daddr = 0;
	byte expect = 0, ddata = 0;

	auto checkresult = [&]()
	{
		if ((pCVir8051->GetRamData(daddr) != expect) || (pCVir8051->Sys.PC != (PC + 2)))
		{
			LogPrint("INFO","addr.data = %x\n", pCVir8051->GetRamData(daddr));
			return false;
		}

		return true;
	};

	auto errstring = [&] () {
		char temp[1024];
		sprintf(temp,"runtime info: PC:%x expect:%x\r\n", PC, expect);
		return "error :" + string(temp)+ pCVir8051->ToStr();
	};

	auto updatecpu = [&] () {
		pCVir8051->InitalReg();
		pCVir8051->SetRamData(daddr, ddata);
		pCVir8051->Sys.PC = PC;
		pCVir8051->m_ExeFile[PC] = code;
		pCVir8051->m_ExeFile[PC + 1] = daddr;
	};

	code = 0x15;
	for (PC = 0; PC <= CVir8051::MAX_ROM - 2; PC = PC + space) {
		if (PC % 1000 == 0) {
			LogPrint("INFO", "code = %x PC = %d\n", code, PC);
		}
		for (daddr = 8; daddr <= CVir8051::MAX_IN_RAM; daddr++) {
			ddata = rand() % 256;
			expect = (ddata == 0x00) ? (0xff) : (ddata - 1);
			updatecpu();
			TestRun();
			if (!checkresult()) {
				return errstring();
			}
		}
	}
	return "OK";
}
string CTestMcu::DEC_ATest(int space) {
	byte code;
	int PC = 0, aa = 0;
	byte expect = 0;

	auto checkresult = [&]()
	{
		if(IsOdd(pCVir8051->Sys.a()) != pCVir8051->Sys.psw().p)
		return false;
		if ((pCVir8051->Sys.a() != expect) || (pCVir8051->Sys.PC != (PC + 1)))
		return false;
		return true;
	};

	auto errstring = [&] () {
		char temp[1024];
		sprintf(temp,"runtime info: PC:%x expect:%x\r\n", PC, expect);
		return "error :" + string(temp)+ pCVir8051->ToStr();
	};

	auto updatecpu = [&] () {
		pCVir8051->InitalReg();
		pCVir8051->Sys.PC = PC;
		pCVir8051->Sys.a = aa;
		pCVir8051->m_ExeFile[PC] = code;
	};

	code = 0x14;
	for (PC = 0; PC <= CVir8051::MAX_ROM - 1; PC = PC + space) {
		if (PC % 1000 == 0) {
			LogPrint("INFO", "code = %x PC = %d\n", code, PC);
		}
		for (aa = 0; aa <= 0xff; aa++) {
			expect = (aa == 0x00) ? (0xff) : (aa - 1);
			updatecpu();
			TestRun();
			if (!checkresult()) {
				return errstring();
			}
		}
	}
	return "OK";
}
string CTestMcu::RRC_ATest(int space) {
	byte code = 0x13, cc = 0, expect = 0, expectcc = 0;
	int PC = 0, aa = 0;

	auto checkresult = [&]()
	{
		if(IsOdd(pCVir8051->Sys.a()) != pCVir8051->Sys.psw().p)
		return false;
		if ((pCVir8051->Sys.a() != expect) || (pCVir8051->Sys.PC != (PC + 1)))
		return false;
		if(pCVir8051->Sys.psw().cy != expectcc)
		return false;
		return true;
	};

	auto errstring = [&] () {
		char temp[1024];
		sprintf(temp,"runtime info: PC:%x aa:%x expect:%x expectcc:%x\r\n", PC, aa, expect, expectcc);
		return "error :" + string(temp)+ pCVir8051->ToStr();
	};

	auto updatecpu = [&] () {
		pCVir8051->InitalReg();
		pCVir8051->Sys.PC = PC;
		pCVir8051->Sys.a = aa;
		pCVir8051->Sys.psw().cy = cc;
		pCVir8051->m_ExeFile[PC] = code;
	};

	for (PC = 0; PC <= CVir8051::MAX_ROM - 1; PC = PC + space) {
		if (PC % 1000 == 0) {
			LogPrint("INFO", "code = %x PC = %d\n", code, PC);
		}
		for (aa = 0; aa <= 0xFF; aa++) {
			cc = rand() % 2;
			expect = (byte) (((aa & 0xfe) >> 1) | ((cc & 0x01) << 7));
			expectcc = aa & 0x01;
			updatecpu();
			TestRun();
			if (!checkresult()) {
				return errstring();
			}
		}
	}
	return "OK";
}
string CTestMcu::LCALL_Addr16Test(int space) {
	byte code = 0x12, SP = 0, expectSP = 0, spdata1 = 0, spdata2 = 0;
	int PC = 0, addr = 0, expectPC = 0;

	auto checkresult = [&]()
	{
		if(pCVir8051->Sys.sp() != expectSP)
		return false;
		if(pCVir8051->Sys.PC != expectPC)
		return false;
		if(pCVir8051->GetRamDataAt(expectSP - 1) != spdata1)
		return false;
		if(pCVir8051->GetRamDataAt(expectSP) != spdata2)
		return false;
		return true;
	};

	auto errstring = [&] () {
		char temp[1024];
		sprintf(temp,"runtime info: PC:%x expectPC:%x expectSP:%x addr:%x SP:%x spdata1:%x spdata2:%x\r\n",
				PC, expectPC, expectSP, addr, SP, spdata1, spdata2);
		return "error :" + string(temp)+ pCVir8051->ToStr();
	};

	auto updatecpu = [&] () {
		pCVir8051->InitalReg();
		pCVir8051->Sys.PC = PC;
		pCVir8051->Sys.sp = SP;
		pCVir8051->m_ExeFile[PC] = code;
		pCVir8051->m_ExeFile[PC + 1] = (byte)(addr >> 8);
		pCVir8051->m_ExeFile[PC + 2] = (byte)(addr & 0xff);
	};

	for (PC = 0; PC <= CVir8051::MAX_ROM - 3; PC = PC + space) {
		if (PC % 1000 == 0) {
			LogPrint("INFO", "code = %x PC = %d\n", code, PC);
		}
		for (addr = 0; addr <= 0xffff; addr++) {
			for (SP = 8; SP <= pCVir8051->MAX_IN_RAM - 2; SP++) {
				expectPC = PC + 3;
				expectSP = SP + 1;
				spdata1 = (byte) (expectPC & 0xff);
				expectSP = expectSP + 1;
				spdata2 = (byte) (expectPC >> 8);
				expectPC = addr;
				updatecpu();
				TestRun();
				if (!checkresult()) {
					return errstring();
				}
			}
		}
	}
	return "OK";
}
string CTestMcu::ACALL_Addr11Test(int space) {
	int PC = 0, addr = 0, expectPC = 0;
	byte code = 0, SP = 0, expectSP = 0, spdata1 = 0, spdata2 = 0;

	auto checkresult = [&]()
	{
		if(pCVir8051->Sys.sp() != expectSP)
		return false;
		if(pCVir8051->Sys.PC != expectPC)
		return false;
		if(pCVir8051->GetRamDataAt(expectSP - 1) != spdata1)
		return false;
		if(pCVir8051->GetRamDataAt(expectSP) != spdata2)
		return false;
		return true;
	};

	auto errstring = [&] () {
		char temp[1024];
		sprintf(temp,"runtime info: PC:%x expectPC:%x expectSP:%x addr:%x SP:%x spdata1:%x spdata2:%x\r\n",
				PC, expectPC, expectSP, addr, SP, spdata1, spdata2);
		return "error :" + string(temp)+ pCVir8051->ToStr();
	};

	auto updatecpu = [&] () {
		pCVir8051->InitalReg();
		pCVir8051->Sys.PC = PC;
		pCVir8051->Sys.sp = SP;
		pCVir8051->m_ExeFile[PC] = code;
		pCVir8051->m_ExeFile[PC + 1] = (byte)(addr & 0xff);
	};

	for (PC = 0; PC <= CVir8051::MAX_ROM - 2; PC = PC + space) {
		if (PC % 1000 == 0) {
			LogPrint("INFO", "code = %x PC = %d\n", code, PC);
		}
		for (addr = 0; addr <= 0x07ff; addr++) {
			for (SP = 8; SP <= pCVir8051->MAX_IN_RAM - 2; SP++) {
				code = (byte) (0x11 | ((addr >> 8) << 5));
				expectPC = PC + 2;
				expectSP = SP + 1;
				spdata1 = (byte) (expectPC & 0xff);
				expectSP = expectSP + 1;
				spdata2 = (byte) (expectPC >> 8);
				expectPC = (int) ((expectPC & 0xf800) | (addr & 0x07ff));
				updatecpu();
				TestRun();
				if (!checkresult()) {
					return errstring();
				}
			}
		}
	}
	return "OK";
}
string CTestMcu::JBC_Bit_RelTest(int space) {
	int PC = 0, expect = 0;
	byte code = 0x10, bb = 0;
	int raddr = 0, baddr = 0;

	auto errstring = [&] () {
		char temp[1024];
		sprintf(temp,"runtime info: PC:%x bb:%x baddr:%x raddr:%x expect:%x\r\n",
				PC, bb, baddr, raddr, expect);
		return "error :" + string(temp)+ pCVir8051->ToStr();
	};

	auto checkresult = [&]()
	{
		if (pCVir8051->Sys.PC != expect)
		{
			LogPrint("INFO","1\n");
			return false;
		}
		if(pCVir8051->GetBitFlag(baddr) != 0)
		{
			LogPrint("INFO","2\n");
			return false;
		}
		return true;
	};

	auto updatecpu = [&] () {
		pCVir8051->InitalReg();
		if (bb) {
			pCVir8051->SetBitFlag(baddr);
		} else {
			pCVir8051->ClrBitFlag(baddr);
		}
		pCVir8051->Sys.PC = PC;
		pCVir8051->m_ExeFile[PC] = code;
		pCVir8051->m_ExeFile[PC + 1] = baddr;
		pCVir8051->m_ExeFile[PC + 2] = raddr;
	};

	for (PC = 0; PC <= CVir8051::MAX_ROM - 3; PC = PC + space) {
		if (PC % 1000 == 0) {
			LogPrint("INFO", "code = %x PC = %d\n", code, PC);
		}
		for (baddr = 0; baddr <= 0xff; baddr++) {
			for (raddr = -128; raddr <= 127; raddr++) {
				bb = rand() % 2;
				expect = (bb) ? (PC + 3 + raddr) : (PC + 3);
				if (expect > CVir8051::MAX_ROM || expect < 0) {
					continue;
				}
				updatecpu();
				TestRun();
				if (!checkresult()) {
					return errstring();
				}
			}
		}
	}
	return "OK";
}
string CTestMcu::INC_RnTest(int space) {
	byte code;
	int PC;
	int rr;
	byte expect;

	auto errstring = [&] () {
		char temp[1024];
		sprintf(temp,"runtime info: PC:%x expect:%x\r\n", PC, expect);
		return "error :" + string(temp)+ pCVir8051->ToStr();
	};

	auto checkresult = [&]()
	{
		byte ret = 0;
		switch (code) {
			case 0x08:
			ret = pCVir8051->Rges.R0();
			break;
			case 0x09:
			ret = pCVir8051->Rges.R1();
			break;
			case 0x0a:
			ret = pCVir8051->Rges.R2();
			break;
			case 0x0b:
			ret = pCVir8051->Rges.R3();
			break;
			case 0x0c:
			ret = pCVir8051->Rges.R4();
			break;
			case 0x0d:
			ret = pCVir8051->Rges.R5();
			break;
			case 0x0e:
			ret = pCVir8051->Rges.R6();
			break;
			case 0x0f:
			ret = pCVir8051->Rges.R7();
			break;
			default:
			ret = 0;
			break;
		}
		if(ret != expect)
		return false;
		if (pCVir8051->Sys.PC != PC + 1)
		return false;
		return true;
	};

	auto updatecpu = [&] () {
		pCVir8051->InitalReg();
		switch (code) {
			case 0x08:
			pCVir8051->Rges.R0 = rr;
			break;
			case 0x09:
			pCVir8051->Rges.R1 = rr;
			break;
			case 0x0a:
			pCVir8051->Rges.R2 = rr;
			break;
			case 0x0b:
			pCVir8051->Rges.R3 = rr;
			break;
			case 0x0c:
			pCVir8051->Rges.R4 = rr;
			break;
			case 0x0d:
			pCVir8051->Rges.R5 = rr;
			break;
			case 0x0e:
			pCVir8051->Rges.R6 = rr;
			break;
			case 0x0f:
			pCVir8051->Rges.R7 = rr;
			break;
			default:
			break;
		}
		pCVir8051->Sys.PC = PC;
		pCVir8051->m_ExeFile[PC] = code;
	};

	for (code = 0x08; code <= 0x0f; code++) {
		for (PC = 0; PC <= CVir8051::MAX_ROM - 1; PC = PC + space) {
			if (PC % 1000 == 0) {
				LogPrint("INFO", "code = %x PC = %d\n", code, PC);
			}
			for (rr = 0; rr <= 0xff; rr++) {
				expect = (rr == 0xff) ? (0) : (rr + 1);
				updatecpu();
				TestRun();
				if (!checkresult()) {
					return errstring();
				}
			}
		}
	}
	return "OK";
}
string CTestMcu::INC_Ri_1Test(int space) {
	byte code;
	int PC;
	int rr;
	byte expect, ridata = 0;

	auto errstring = [&] () {
		char temp[1024];
		sprintf(temp,"runtime info: PC:%x expect:%x\r\n", PC, expect);
		return "error :" + string(temp)+ pCVir8051->ToStr();
	};

	auto updatecpu = [&] ()
	{
		pCVir8051->InitalReg();
		switch (code) {
			case 0x06:
			pCVir8051->Rges.R0 = rr;
			break;
			case 0x07:
			pCVir8051->Rges.R1 = rr;
			break;
			default:
			break;
		}
		pCVir8051->SetRamDataAt(rr, ridata);
		pCVir8051->Sys.PC = PC;
		pCVir8051->m_ExeFile[PC] = code;
	};

	auto checkresult = [&]()
	{
		if ((pCVir8051->GetRamDataAt(rr) != expect) || (pCVir8051->Sys.PC != (PC + 1)))
		return false;
		return true;
	};

	for (code = 0x06; code <= 0x07; code++) {
		for (PC = 0; PC <= CVir8051::MAX_ROM - 1; PC = PC + space) {
			if (PC % 1000 == 0) {
				LogPrint("INFO", "code = %x PC = %d\n", code, PC);
			}
			for (rr = 8; rr <= CVir8051::MAX_IN_RAM; rr++) {
				ridata = rand() % 256;
				expect = (ridata == 0xff) ? (0) : (ridata + 1);
				updatecpu();
				TestRun();
				if (!checkresult()) {
					return errstring();
				}
			}
		}
	}
	return "OK";
}
string CTestMcu::INC_DirectTest(int space) {
	byte code;
	int PC = 0, daddr = 0;
	byte expect = 0, ddata = 0;

	auto checkresult = [&]()
	{
		if ((pCVir8051->GetRamData(daddr) != expect) || (pCVir8051->Sys.PC != (PC + 2)))
		{
			LogPrint("INFO","addr.data = %x\n", pCVir8051->GetRamData(daddr));
			return false;
		}

		return true;
	};

	auto errstring = [&] () {
		char temp[1024];
		sprintf(temp,"runtime info: PC:%x expect:%x\r\n", PC, expect);
		return "error :" + string(temp)+ pCVir8051->ToStr();
	};

	auto updatecpu = [&] () {
		pCVir8051->InitalReg();
		pCVir8051->SetRamData(daddr, ddata);
		pCVir8051->Sys.PC = PC;
		pCVir8051->m_ExeFile[PC] = code;
		pCVir8051->m_ExeFile[PC + 1] = daddr;
	};

	code = 0x05;
	for (PC = 0; PC <= CVir8051::MAX_ROM - 2; PC = PC + space) {
		if (PC % 1000 == 0) {
			LogPrint("INFO", "code = %x PC = %d\n", code, PC);
		}
		for (daddr = 8; daddr <= CVir8051::MAX_IN_RAM; daddr++) {
			ddata = rand() % 256;
			expect = (ddata == 0xff) ? (0) : (ddata + 1);
			updatecpu();
			TestRun();
			if (!checkresult()) {
				return errstring();
			}
		}
	}
	return "OK";
}
string CTestMcu::INC_ATest(int space) {
	byte code;
	int PC = 0, aa = 0;
	byte expect = 0;

	auto checkresult = [&]()
	{
		if(IsOdd(pCVir8051->Sys.a()) != pCVir8051->Sys.psw().p)
		return false;
		if ((pCVir8051->Sys.a() != expect) || (pCVir8051->Sys.PC != (PC + 1)))
		return false;
		return true;
	};

	auto errstring = [&] () {
		char temp[1024];
		sprintf(temp,"runtime info: PC:%x expect:%x\r\n", PC, expect);
		return "error :" + string(temp)+ pCVir8051->ToStr();
	};

	auto updatecpu = [&] () {
		pCVir8051->InitalReg();
		pCVir8051->Sys.PC = PC;
		pCVir8051->Sys.a = aa;
		pCVir8051->m_ExeFile[PC] = code;
	};

	code = 0x04;
	for (PC = 0; PC <= CVir8051::MAX_ROM - 1; PC = PC + space) {
		if (PC % 1000 == 0) {
			LogPrint("INFO", "code = %x PC = %d\n", code, PC);
		}
		for (aa = 0; aa <= 0xff; aa++) {
			expect = (aa == 0xff) ? (0) : (aa + 1);
			updatecpu();
			TestRun();
			if (!checkresult()) {
				return errstring();
			}
		}
	}
	return "OK";
}
string CTestMcu::RR_ATest(int space) {
	byte code = 0x03, expect = 0;
	int PC = 0, aa = 0;

	auto checkresult = [&]()
	{
		if(IsOdd(pCVir8051->Sys.a()) != pCVir8051->Sys.psw().p)
		return false;
		if ((pCVir8051->Sys.a() != expect) || (pCVir8051->Sys.PC != (PC + 1)))
		return false;
		return true;
	};

	auto errstring = [&] () {
		char temp[1024];
		sprintf(temp,"runtime info: PC:%x aa:%x expect:%x\r\n", PC, aa, expect);
		return "error :" + string(temp)+ pCVir8051->ToStr();
	};

	auto updatecpu = [&] () {
		pCVir8051->InitalReg();
		pCVir8051->Sys.PC = PC;
		pCVir8051->Sys.a = aa;
		pCVir8051->m_ExeFile[PC] = code;
	};

	for (PC = 0; PC <= CVir8051::MAX_ROM - 1; PC = PC + space) {
		if (PC % 1000 == 0) {
			LogPrint("INFO", "code = %x PC = %d\n", code, PC);
		}
		for (aa = 0; aa <= 0xFF; aa++) {
			expect = (byte) (((aa & 0xfe) >> 1) | ((aa & 0x01) << 7));
			updatecpu();
			TestRun();
			if (!checkresult()) {
				return errstring();
			}
		}
	}
	return "OK";
}
string CTestMcu::LJMPTest(int space) {
	byte code = 0x02;
	int PC = 0, addr = 0;

	auto checkresult = [&]()
	{
		if (pCVir8051->Sys.PC != addr)
		return false;
		return true;
	};

	auto errstring = [&] () {
		char temp[1024];
		sprintf(temp,"runtime info: PC:%x addr:%x\r\n", PC, addr);
		return "error :" + string(temp)+ pCVir8051->ToStr();
	};

	auto updatecpu = [&] () {
		pCVir8051->InitalReg();
		pCVir8051->Sys.PC = PC;
		pCVir8051->m_ExeFile[PC] = code;
		pCVir8051->m_ExeFile[PC + 1] = (byte)(addr>>8);
		pCVir8051->m_ExeFile[PC + 2] = (byte)(addr&0xff);
	};

	for (PC = 0; PC <= CVir8051::MAX_ROM - 3; PC = PC + space) {
		if (PC % 1000 == 0) {
			LogPrint("INFO", "code = %x PC = %d\n", code, PC);
		}
		for (addr = 0; addr <= CVir8051::MAX_ROM; addr++) {
			updatecpu();
			TestRun();
			if (!checkresult()) {
				return errstring();
			}
		}
	}
	return "OK";
}
string CTestMcu::NOPTest(int space) {
	int PC = 0;
	byte code = 0;

	auto checkresult = [&]()
	{
		if (pCVir8051->Sys.PC != (PC + 1))
		return false;
		return true;
	};

	auto errstring = [&] () {
		char temp[1024];
		sprintf(temp,"runtime info: PC:%x\r\n", PC);
		return "error :" + string(temp)+ pCVir8051->ToStr();
	};

	auto updatecpu = [&] () {
		pCVir8051->InitalReg();
		pCVir8051->Sys.PC = PC;
		pCVir8051->m_ExeFile[PC] = code;
	};

	for (PC = 0; PC <= CVir8051::MAX_ROM - 1; PC = PC + space) {
		if (PC % 1000 == 0) {
			LogPrint("INFO", "code = %x PC = %d\n", code, PC);
		}
		updatecpu();
		TestRun();
		if (!checkresult()) {
			return errstring();
		}
	}
	return "OK";

}
string CTestMcu::AJMPTest(int space) {
	int PC = 0, addr = 0, expectPC = 0;
	byte code = 0;

	auto checkresult = [&]()
	{
		if (pCVir8051->Sys.PC != expectPC)
		return false;
		return true;
	};

	auto errstring = [&] () {
		char temp[1024];
		sprintf(temp,"runtime info: PC:%x expectPC:%x\r\n", PC, expectPC);
		return "error :" + string(temp)+ pCVir8051->ToStr();
	};

	auto updatecpu = [&] () {
		pCVir8051->InitalReg();
		pCVir8051->Sys.PC = PC;
		pCVir8051->m_ExeFile[PC] = code;
		pCVir8051->m_ExeFile[PC + 1] = (byte)(addr & 0xff);
	};

	for (PC = 0; PC <= CVir8051::MAX_ROM - 2; PC = PC + space) {
		for (addr = 0; addr <= 0x07ff; addr++) {
			code = (byte) (0x01 | ((addr >> 8) << 5));
			expectPC = PC + 2;
			expectPC = (int) ((expectPC & 0xf800) | (addr & 0x07ff));
			updatecpu();
			TestRun();
			if (!checkresult()) {
				return errstring();
			}
		}
	}
	return "OK";
}

//bess
string CTestMcu::MOV_Rn_ATest(int space) {
	byte expect = 0;
	int code = 0, PC = 0, aa = 0;

	auto checkresult = [&]()
	{
		byte ret = 0;
		switch (code) {
			case 0xf8:
			ret = pCVir8051->Rges.R0();
			break;
			case 0xf9:
			ret = pCVir8051->Rges.R1();
			break;
			case 0xfa:
			ret = pCVir8051->Rges.R2();
			break;
			case 0xfb:
			ret = pCVir8051->Rges.R3();
			break;
			case 0xfc:
			ret = pCVir8051->Rges.R4();
			break;
			case 0xfd:
			ret = pCVir8051->Rges.R5();
			break;
			case 0xfe:
			ret = pCVir8051->Rges.R6();
			break;
			case 0xff:
			ret = pCVir8051->Rges.R7();
			break;
			default:
			ret = 0;
			break;
		}
		if ((ret != expect) || (pCVir8051->Sys.PC != (PC + 1)))
		return false;
		return true;
	};

	auto errstring = [&] () {
		char temp[1024];
		sprintf(temp,"runtime info: PC:%x expect:%x code:%x\r\n", PC, expect, code);
		return "error :" + string(temp)+ pCVir8051->ToStr();
	};

	auto updatecpu = [&] () {
		pCVir8051->InitalReg();
		pCVir8051->Sys.PC = PC;
		pCVir8051->Sys.a = aa;
		pCVir8051->m_ExeFile[PC] = code;
	};

	for (code = 0xf8; code <= 0xff; code++) {
		for (PC = 0; PC <= CVir8051::MAX_ROM - 1; PC = PC + space) {
			if (PC % 1000 == 0) {
				LogPrint("INFO", "code = %x PC = %d\n", code, PC);
			}
			for (aa = 0; aa <= 0xff; aa++) {
				expect = aa;
				updatecpu();
				TestRun();
				if (!checkresult()) {
					return errstring();
				}
			}
		}
	}
	return "OK";
}
string CTestMcu::MOV_Direct_ATest(int space) {
	int PC = 0, daddr = 0, aa = 0;
	byte code = 0xf5;

	auto checkresult = [&]()
	{
		if(pCVir8051->Sys.PC != (PC + 2))
		return false;
		if(pCVir8051->GetRamData(daddr) != aa)
		return false;
		return true;
	};

	auto errstring = [&] () {
		char temp[1024];
		sprintf(temp,"runtime info: PC:%x daddr:%x\r\n", PC, daddr);
		return "error :" + string(temp)+ pCVir8051->ToStr();
	};

	auto updatecpu = [&]()
	{
		pCVir8051->InitalReg();
		pCVir8051->Sys.PC = PC;
		pCVir8051->Sys.a = aa;
		pCVir8051->m_ExeFile[PC] = code;
		pCVir8051->m_ExeFile[PC + 1] = daddr;
	};

	for (PC = 0; PC <= CVir8051::MAX_ROM - 2; PC = PC + space) {
		if (PC % 1000 == 0) {
			LogPrint("INFO", "code = %x PC = %d\n", code, PC);
		}
		for (daddr = 0; daddr <= 0Xff; daddr++) {
			for (aa = 0; aa <= 0xff; aa++) {
				updatecpu();
				TestRun();
				if (!checkresult()) {
					return errstring();
				}
			}
		}
	}
	return "OK";
}
string CTestMcu::CPL_ATest(int space) {
	int PC = 0, aa = 0;
	byte code = 0xf4, expect = 0;

	auto checkresult = [&]()
	{
		if(pCVir8051->Sys.PC != (PC + 1))
		return false;
		if(pCVir8051->Sys.a() != expect)
		return false;
		if(IsOdd(pCVir8051->Sys.a()) != pCVir8051->Sys.psw().p)
		return false;
		return true;
	};

	auto errstring = [&] () {
		char temp[1024];
		sprintf(temp,"runtime info: PC:%x daddr:%x\r\n", PC, expect);
		return "error :" + string(temp)+ pCVir8051->ToStr();
	};

	auto updatecpu = [&]()
	{
		pCVir8051->InitalReg();
		pCVir8051->Sys.PC = PC;
		pCVir8051->Sys.a = aa;
		pCVir8051->m_ExeFile[PC] = code;
	};

	for (PC = 0; PC <= CVir8051::MAX_ROM - 1; PC = PC + space) {
		if (PC % 1000 == 0) {
			LogPrint("INFO", "code = %x PC = %d\n", code, PC);
		}
		for (aa = 0; aa <= 0xff; aa++) {
			expect = ~aa;
			updatecpu();
			TestRun();
			if (!checkresult()) {
				return errstring();
			}
		}
	}
	return "OK";
}
string CTestMcu::MOVX_Ri_1_ATest(int space) {
	int PC = 0, aa = 0, ri = 0;
	byte code = 0xf2, expect = 0;

	auto checkresult = [&]()
	{
		if(pCVir8051->Sys.PC != (PC + 1))
		return false;
		if(pCVir8051->m_ExRam[ri] != expect)
		return false;
		if(IsOdd(pCVir8051->Sys.a()) != pCVir8051->Sys.psw().p)
		return false;
		return true;
	};

	auto errstring = [&] () {
		char temp[1024];
		sprintf(temp,"runtime info: PC:%x daddr:%x\r\n", PC, expect);
		return "error :" + string(temp)+ pCVir8051->ToStr();
	};

	auto updatecpu = [&]()
	{
		pCVir8051->InitalReg();
		if(code == 0xf2) {
			pCVir8051->Rges.R0 = ri;
		}
		else if(code == 0xf3) {
			pCVir8051->Rges.R1 = ri;
		}
		pCVir8051->Sys.PC = PC;
		pCVir8051->Sys.a = aa;
		pCVir8051->m_ExeFile[PC] = code;
	};

	for (code = 0xf2; code <= 0xf3; code++) {
		for (PC = 0; PC <= CVir8051::MAX_ROM - 1; PC = PC + space) {
			if (PC % 1000 == 0) {
				LogPrint("INFO", "code = %x PC = %d\n", code, PC);
			}
			for (ri = 0; ri <= 0xff; ri++) {
				for (aa = 0; aa <= 0xff; aa++) {
					expect = aa;
					updatecpu();
					TestRun();
					if (!checkresult()) {
						return errstring();
					}
				}
			}
		}
	}
	return "OK";
}
string CTestMcu::MOVX_DPTR_ATest(int space) {
	int PC = 0, dptr = 0, aa = 0;
	byte code = 0xf0, expect = 0;

	auto checkresult = [&]()
	{
		if(pCVir8051->Sys.PC != (PC + 1))
		return false;
		if(pCVir8051->m_ExRam[dptr] != expect)
		return false;
		if(IsOdd(pCVir8051->Sys.a()) != pCVir8051->Sys.psw().p)
		return false;
		return true;
	};

	auto errstring = [&] () {
		char temp[1024];
		sprintf(temp,"runtime info: PC:%x daddr:%x\r\n", PC, expect);
		return "error :" + string(temp)+ pCVir8051->ToStr();
	};

	auto updatecpu = [&]()
	{
		pCVir8051->InitalReg();
		pCVir8051->Sys.dptr = dptr;
		pCVir8051->Sys.PC = PC;
		pCVir8051->Sys.a = aa;
		pCVir8051->m_ExeFile[PC] = code;
	};

	for (PC = 0; PC <= CVir8051::MAX_ROM - 1; PC = PC + space) {
		if (PC % 1000 == 0) {
			LogPrint("INFO", "code = %x PC = %d\n", code, PC);
		}
		for (dptr = 0; dptr <= 0xffff; dptr++) {
			for (aa = 0; aa <= 0xff; aa++) {
				expect = aa;
				updatecpu();
				TestRun();
				if (!checkresult()) {
					return errstring();
				}
			}
		}
	}
	return "OK";
}
string CTestMcu::MOV_A_Rn_1Test(int space) {
	byte code = 0, expect = 0;
	int PC = 0, rr = 0;

	auto checkresult = [&]()
	{
		if (pCVir8051->Sys.PC != (PC + 1))
		return false;
		if(pCVir8051->Sys.a() != expect)
		return false;
		if(IsOdd(pCVir8051->Sys.a()) != pCVir8051->Sys.psw().p)
		return false;
		return true;
	};

	auto errstring = [&] () {
		char temp[1024];
		sprintf(temp,"runtime info: PC:%x expect:%x\r\n", PC, expect);
		return "error :" + string(temp)+ pCVir8051->ToStr();
	};

	auto updatecpu = [&] () {
		pCVir8051->InitalReg();
		switch (code) {
			case 0xe8:
			pCVir8051->Rges.R0 = rr;
			break;
			case 0xe9:
			pCVir8051->Rges.R1 = rr;
			break;
			case 0xea:
			pCVir8051->Rges.R2 = rr;
			break;
			case 0xeb:
			pCVir8051->Rges.R3 = rr;
			break;
			case 0xec:
			pCVir8051->Rges.R4 = rr;
			break;
			case 0xed:
			pCVir8051->Rges.R5 = rr;
			break;
			case 0xee:
			pCVir8051->Rges.R6 = rr;
			break;
			case 0xef:
			pCVir8051->Rges.R7 = rr;;
			break;
			default:
			break;
		}
		pCVir8051->Sys.PC = PC;
		pCVir8051->m_ExeFile[PC] = code;
	};

	for (code = 0xe8; code <= 0xef; code++) {
		for (PC = 0; PC <= CVir8051::MAX_ROM - 1; PC = PC + space) {
			if (PC % 1000 == 0) {
				LogPrint("INFO", "code = %x PC = %d\n", code, PC);
			}
			for (rr = 0; rr <= 0xff; rr++) {
				expect = rr;
				updatecpu();
				TestRun();
				if (!checkresult()) {
					return errstring();
				}
			}
		}
	}
	return "OK";
}

string CTestMcu::MOV_A_Ri_1Test(int space) {
	byte code = 0, expect = 0, ridata = 0;
	int PC = 0, ri = 0;

	auto checkresult = [&]()
	{
		if (pCVir8051->Sys.PC != (PC + 1))
		return false;
		if(pCVir8051->Sys.a() != expect)
		return false;
		if(IsOdd(pCVir8051->Sys.a()) != pCVir8051->Sys.psw().p)
		return false;
		return true;
	};

	auto errstring = [&] () {
		char temp[1024];
		sprintf(temp,"runtime info: PC:%x expect:%x\r\n", PC, expect);
		return "error :" + string(temp)+ pCVir8051->ToStr();
	};

	auto updatecpu = [&] () {
		pCVir8051->InitalReg();
		switch (code) {
			case 0xe6:
			pCVir8051->Rges.R0 = ri;
			break;
			case 0xe7:
			pCVir8051->Rges.R1 = ri;
			break;
			default:
			break;
		}
		pCVir8051->SetRamDataAt(ri, ridata);
		pCVir8051->Sys.PC = PC;
		pCVir8051->m_ExeFile[PC] = code;
	};

	for (code = 0xe6; code <= 0xe7; code++) {
		for (PC = 0; PC <= CVir8051::MAX_ROM - 1; PC = PC + space) {
			if (PC % 1000 == 0) {
				LogPrint("INFO", "code = %x PC = %d\n", code, PC);
			}
			for (ri = 8; ri <= 0xff; ri++) {
				ridata = rand() % 256;
				expect = ridata;
				updatecpu();
				TestRun();
				if (!checkresult()) {
					return errstring();
				}
			}
		}
	}
	return "OK";
}

string CTestMcu::MOV_A_DirectTest(int space) {
	byte code = 0xe5, expect = 0, ddata = 0;
	int PC = 0, daddr = 0;

	auto checkresult = [&]()
	{
		if (pCVir8051->Sys.PC != (PC + 2))
		return false;
		if(pCVir8051->Sys.a() != expect)
		return false;
		if(IsOdd(pCVir8051->Sys.a()) != pCVir8051->Sys.psw().p)
		return false;
		return true;
	};

	auto errstring = [&] () {
		char temp[1024];
		sprintf(temp,"runtime info: PC:%x expect:%x\r\n", PC, expect);
		return "error :" + string(temp)+ pCVir8051->ToStr();
	};

	auto updatecpu = [&] () {
		pCVir8051->InitalReg();
		pCVir8051->SetRamData(daddr, ddata);
		pCVir8051->Sys.PC = PC;
		pCVir8051->m_ExeFile[PC] = code;
		pCVir8051->m_ExeFile[PC + 1] = daddr;
	};

	for (PC = 0; PC <= CVir8051::MAX_ROM - 2; PC = PC + space) {
		if (PC % 1000 == 0) {
			LogPrint("INFO", "code = %x PC = %d\n", code, PC);
		}
		for (daddr = 8; daddr <= 0xff; daddr++) {
			ddata = rand() % 256;
			expect = ddata;
			updatecpu();
			TestRun();
			if (!checkresult()) {
				return errstring();
			}
		}
	}
	return "OK";
}
string CTestMcu::CLR_ATest(int space) {
	int PC = 0, aa = 0;
	byte code = 0xe4, expect = 0;

	auto checkresult = [&]()
	{
		if(pCVir8051->Sys.PC != (PC + 1))
		return false;
		if(pCVir8051->Sys.a() != expect)
		return false;
		if(IsOdd(pCVir8051->Sys.a()) != pCVir8051->Sys.psw().p)
		return false;
		return true;
	};

	auto errstring = [&] () {
		char temp[1024];
		sprintf(temp,"runtime info: PC:%x daddr:%x\r\n", PC, expect);
		return "error :" + string(temp)+ pCVir8051->ToStr();
	};

	auto updatecpu = [&]()
	{
		pCVir8051->InitalReg();
		pCVir8051->Sys.PC = PC;
		pCVir8051->Sys.a = aa;
		pCVir8051->m_ExeFile[PC] = code;
	};

	for (PC = 0; PC <= CVir8051::MAX_ROM - 1; PC = PC + space) {
		if (PC % 1000 == 0) {
			LogPrint("INFO", "code = %x PC = %d\n", code, PC);
		}
		for (aa = 0; aa <= 0xff; aa++) {
			expect = 0;
			updatecpu();
			TestRun();
			if (!checkresult()) {
				return errstring();
			}
		}
	}
	return "OK";
}
string CTestMcu::MOVX_A_Ri_1Test(int space) {
	int PC = 0, ri = 0;
	byte code = 0xe2, expect = 0, ridata = 0;

	auto checkresult = [&]()
	{
		if(pCVir8051->Sys.PC != (PC + 1))
		return false;
		if(pCVir8051->Sys.a() != expect)
		return false;
		if(IsOdd(pCVir8051->Sys.a()) != pCVir8051->Sys.psw().p)
		return false;
		return true;
	};

	auto errstring = [&] () {
		char temp[1024];
		sprintf(temp,"runtime info: PC:%x expect:%x\r\n", PC, expect);
		return "error :" + string(temp)+ pCVir8051->ToStr();
	};

	auto updatecpu = [&]()
	{
		pCVir8051->InitalReg();
		if(code == 0xe2) {
			pCVir8051->Rges.R0 = ri;
		}
		else if(code == 0xe3) {
			pCVir8051->Rges.R1 = ri;
		}
		pCVir8051->m_ExRam[ri] = ridata;
		pCVir8051->Sys.PC = PC;
		pCVir8051->m_ExeFile[PC] = code;
	};

	for (code = 0xe2; code <= 0xe3; code++) {
		for (PC = 0; PC <= CVir8051::MAX_ROM - 1; PC = PC + space) {
			if (PC % 1000 == 0) {
				LogPrint("INFO", "code = %x PC = %d\n", code, PC);
			}
			for (ri = 0; ri <= 0xff; ri++) {
				ridata = rand() % 256;
				expect = ridata;
				updatecpu();
				TestRun();
				if (!checkresult()) {
					return errstring();
				}
			}
		}
	}
	return "OK";
}
string CTestMcu::MOVX_A_DPTRTest(int space) {
	int PC = 0, dptr = 0;
	byte code = 0xe0, expect = 0, dptrdata = 0;

	auto checkresult = [&]()
	{
		if(pCVir8051->Sys.PC != (PC + 1))
		return false;
		if(pCVir8051->Sys.a() != expect)
		return false;
		if(IsOdd(pCVir8051->Sys.a()) != pCVir8051->Sys.psw().p)
		return false;
		return true;
	};

	auto errstring = [&] () {
		char temp[1024];
		sprintf(temp,"runtime info: PC:%x expect:%x\r\n", PC, expect);
		return "error :" + string(temp)+ pCVir8051->ToStr();
	};

	auto updatecpu = [&]()
	{
		pCVir8051->InitalReg();
		pCVir8051->m_ExRam[dptr] = dptrdata;
		pCVir8051->Sys.PC = PC;
		pCVir8051->Sys.dptr = dptr;
		pCVir8051->m_ExeFile[PC] = code;
	};

	for (PC = 0; PC <= CVir8051::MAX_ROM - 1; PC = PC + space) {
		if (PC % 1000 == 0) {
			LogPrint("code = %x PC = %d\n", code, PC);
		}
		for (dptr = 0; dptr <= 0xffff; dptr++) {
			dptrdata = rand() % 256;
			expect = dptrdata;
			updatecpu();
			TestRun();
			if (!checkresult()) {
				return errstring();
			}
		}
	}
	return "OK";
}
string CTestMcu::DJNZ_Rn_RelRTest(int space) {
	int PC = 0, expectPC = 0, rn = 0;
	byte code = 0xd8, expectRn = 0;
	int raddr = 0;

	auto errstring = [&] () {
		char temp[1024];
		sprintf(temp,"runtime info: PC:%x expectPC:%x rn:%x raddr:%x expectRn:%x\r\n",
				PC, expectPC, rn, raddr, expectRn);
		return "error :" + string(temp)+ pCVir8051->ToStr();
	};

	auto checkresult = [&]()
	{
		byte ret = 0;
		switch (code) {
			case 0xd8:
			ret = pCVir8051->Rges.R0();
			break;
			case 0xd9:
			ret = pCVir8051->Rges.R1();
			break;
			case 0xda:
			ret = pCVir8051->Rges.R2();
			break;
			case 0xdb:
			ret = pCVir8051->Rges.R3();
			break;
			case 0xdc:
			ret = pCVir8051->Rges.R4();
			break;
			case 0xdd:
			ret = pCVir8051->Rges.R5();
			break;
			case 0xde:
			ret = pCVir8051->Rges.R6();
			break;
			case 0xdf:
			ret = pCVir8051->Rges.R7();
			break;
			default:
			ret = 0;
			break;
		}
		if(ret != expectRn)
		return false;
		if (pCVir8051->Sys.PC != expectPC)
		return false;
		return true;
	};

	auto updatecpu = [&] () {
		pCVir8051->InitalReg();
		switch (code) {
			case 0xd8:
			pCVir8051->Rges.R0 = rn;
			break;
			case 0xd9:
			pCVir8051->Rges.R1 = rn;
			break;
			case 0xda:
			pCVir8051->Rges.R2 = rn;
			break;
			case 0xdb:
			pCVir8051->Rges.R3 = rn;
			break;
			case 0xdc:
			pCVir8051->Rges.R4 = rn;
			break;
			case 0xdd:
			pCVir8051->Rges.R5 = rn;
			break;
			case 0xde:
			pCVir8051->Rges.R6 = rn;
			break;
			case 0xdf:
			pCVir8051->Rges.R7 = rn;
			break;
			default:
			break;
		}
		pCVir8051->Sys.PC = PC;
		pCVir8051->m_ExeFile[PC] = code;
		pCVir8051->m_ExeFile[PC + 1] = raddr;
	};

	for (code = 0xd8; code <= 0xdf; code++) {
		for (PC = 0; PC <= CVir8051::MAX_ROM - 2; PC = PC + space) {
			if (PC % 1000 == 0) {
				LogPrint("code = %x PC = %d\n", code, PC);
			}
			for (rn = 0; rn <= 0xff; rn++) {
				for (raddr = -128; raddr <= 127; raddr++) {
					expectRn = rn - 1;
					expectPC = (expectRn) ? (PC + 2 + raddr) : (PC + 2);
					if (expectPC > CVir8051::MAX_ROM || expectPC < 0) {
						continue;
					}
					updatecpu();
					TestRun();
					if (!checkresult()) {
						return errstring();
					}
				}
			}
		}
	}
	return "OK";
}
string CTestMcu::XCHD_A_Ri_1Test(int space) {
	byte code = 0;
	int rr = 0, aa = 0, PC = 0;
	byte ridata = 0;
	byte expectaa = 0, expectri = 0;

	auto checkresult = [&]()
	{
		if(pCVir8051->GetRamDataAt(rr) != expectri)
		return false;
		if(IsOdd(pCVir8051->Sys.a()) != pCVir8051->Sys.psw().p)
		return false;
		if (pCVir8051->Sys.a() != expectaa)
		return false;
		if(pCVir8051->Sys.PC != (PC + 1))
		return false;
		return true;
	};

	auto errstring = [&] () {
		char temp[1024];
		sprintf(temp,"runtime info: PC:%x expectaa:%x aa:%x rr:%x expectri:%x ridata:%x\r\n",
				PC, expectaa, aa, rr, expectri, ridata);
		return "error :" + string(temp)+ pCVir8051->ToStr();
	};

	auto updatecpu = [&]() {
		pCVir8051->InitalReg();
		switch (code) {
			case 0xd6:
			pCVir8051->Rges.R0 = rr;
			break;
			case 0xd7:
			pCVir8051->Rges.R1 = rr;
			break;
			default:
			break;
		}
		pCVir8051->SetRamDataAt(rr, ridata);
		pCVir8051->Sys.PC = PC;
		pCVir8051->Sys.a = aa;
		pCVir8051->m_ExeFile[PC] = code;
	};

	for (code = 0xd6; code <= 0xd7; code++) {
		for (PC = 0; PC <= CVir8051::MAX_ROM - 1; PC = PC + space) {
			if (PC % 1000 == 0) {
				LogPrint("code = %x PC = %d\n", code, PC);
			}
			for (rr = 8; rr <= CVir8051::MAX_IN_RAM; rr++) {
				for (aa = 0; aa <= 0xff; aa++) {
					ridata = rand() % 256;
					expectaa = (byte) ((aa & 0xf0) | (ridata & 0x0f));
					expectri = (byte) ((ridata & 0xf0) | (aa & 0x0f));
					updatecpu();
					TestRun();
					if (!checkresult()) {
						return errstring();
					}
				}
			}
		}
	}
	return "OK";
}
string CTestMcu::DJNZ_Direct_RelTest(int space) {
	int PC = 0, expectPC = 0, daddr = 0;
	byte code = 0xd5, ddata = 0, expectdata = 0;
	int raddr = 0;

	auto errstring = [&] () {
		char temp[1024];
		sprintf(temp,"runtime info: PC:%x expectPC:%x expectdata:%x raddr:%x daddr:%x\r\n",
				PC, expectPC, expectdata, raddr, daddr);
		return "error :" + string(temp)+ pCVir8051->ToStr();
	};

	auto checkresult = [&]()
	{
		if (pCVir8051->Sys.PC != expectPC)
		return false;
		if(pCVir8051->GetRamData(daddr) != expectdata)
		return false;
		return true;
	};

	auto updatecpu = [&] () {
		pCVir8051->InitalReg();
		pCVir8051->SetRamData(daddr, ddata);
		pCVir8051->Sys.PC = PC;
		pCVir8051->m_ExeFile[PC] = code;
		pCVir8051->m_ExeFile[PC + 1] = daddr;
		pCVir8051->m_ExeFile[PC + 2] = raddr;
	};

	for (PC = 0; PC <= CVir8051::MAX_ROM - 3; PC = PC + space) {
		if (PC % 1000 == 0) {
			LogPrint("code = %x PC = %d\n", code, PC);
		}
		for (daddr = 0; daddr <= 0xff; daddr++) {
			for (raddr = -128; raddr <= 127; raddr++) {
				ddata = rand() % 256;
				expectdata = ddata - 1;
				expectPC = (expectdata) ? (PC + 3 + raddr) : (PC + 3);
				if (expectPC > CVir8051::MAX_ROM || expectPC < 0) {
					continue;
				}
				updatecpu();
				TestRun();
				if (!checkresult()) {
					return errstring();
				}
			}
		}
	}
	return "OK";
}
string CTestMcu::DA_ATest(int space) {
	int PC = 0, aa = 0;
	byte code = 0xd4, expect = 0, ac = 0, c = 0;

	auto checkresult = [&]()
	{
		if(pCVir8051->Sys.PC != (PC + 1))
		return false;
		if(pCVir8051->Sys.a() != expect)
		return false;
		if(IsOdd(pCVir8051->Sys.a()) != pCVir8051->Sys.psw().p)
		return false;
		return true;
	};

	auto errstring = [&] () {
		char temp[1024];
		sprintf(temp,"runtime info: PC:%x daddr:%x\r\n", PC, expect);
		return "error :" + string(temp)+ pCVir8051->ToStr();
	};

	auto updatecpu = [&]()
	{
		pCVir8051->InitalReg();
		pCVir8051->Sys.PC = PC;
		pCVir8051->Sys.a = aa;
		pCVir8051->Sys.psw().cy = c;
		pCVir8051->Sys.psw().ac = ac;
		pCVir8051->m_ExeFile[PC] = code;
	};

	for (PC = 0; PC <= CVir8051::MAX_ROM - 1; PC = PC + space) {
		if (PC % 1000 == 0) {
			LogPrint("code = %x PC = %d\n", code, PC);
		}
		for (aa = 0; aa <= 0xff; aa++) {
			ac = rand() % 2;
			c = rand() % 2;
			byte al = 0, ah = 0;
			al = aa & 0x0f;
			ah = (aa >> 4) & 0x0f;
			if (al > 9 || ac == 1) {
				al = al + 6;
			}

			if (ah > 9 || c == 1) {
				ah = ah + 6;
			}

			expect = (byte) ((al & 0x0f) | ((ah & 0x0f) << 4));
			updatecpu();
			TestRun();
			if (!checkresult()) {
				return errstring();
			}
		}
	}
	return "OK";
}
string CTestMcu::SETB_CTest(int space) {
	int PC = 0;
	byte code = 0xd3, cc = 0, expect = 0;

	auto checkresult = [&]()
	{
		if(pCVir8051->Sys.PC != (PC + 1))
		return false;
		if(pCVir8051->Sys.psw().cy != expect)
		return false;
		return true;
	};

	auto errstring = [&] () {
		char temp[1024];
		sprintf(temp,"runtime info: PC:%x daddr:%x\r\n", PC, expect);
		return "error :" + string(temp)+ pCVir8051->ToStr();
	};

	auto updatecpu = [&]()
	{
		pCVir8051->InitalReg();
		pCVir8051->Sys.PC = PC;
		pCVir8051->Sys.psw().cy = cc;
		pCVir8051->m_ExeFile[PC] = code;
	};

	for (PC = 0; PC <= CVir8051::MAX_ROM - 1; PC = PC + space) {
		if (PC % 1000 == 0) {
			LogPrint("code = %x PC = %d\n", code, PC);
		}
		for (cc = 0; cc <= 0x01; cc++) {
			expect = 1;
			updatecpu();
			TestRun();
			if (!checkresult()) {
				return errstring();
			}
		}
	}
	return "OK";
}
string CTestMcu::SETB_BitTest(int space) {
	int PC = 0, baddr = 0;
	byte code = 0xd2, bb = 0, expect = 0;

	auto checkresult = [&]()
	{
		if(pCVir8051->Sys.PC != (PC + 2))
		return false;
		if(pCVir8051->GetBitFlag(baddr) != expect)
		return false;
		return true;
	};

	auto errstring = [&] () {
		char temp[1024];
		sprintf(temp,"runtime info: PC:%x daddr:%x\r\n", PC, expect);
		return "error :" + string(temp)+ pCVir8051->ToStr();
	};

	auto updatecpu = [&]()
	{
		pCVir8051->InitalReg();
		if(bb) {
			pCVir8051->SetBitFlag(baddr);
		}
		else {
			pCVir8051->ClrBitFlag(baddr);
		}
		pCVir8051->Sys.PC = PC;
		pCVir8051->m_ExeFile[PC] = code;
		pCVir8051->m_ExeFile[PC + 1] = baddr;
	};

	for (PC = 0; PC <= CVir8051::MAX_ROM - 1; PC = PC + space) {
		if (PC % 1000 == 0) {
			LogPrint("code = %x PC = %d\n", code, PC);
		}
		for (baddr = 0; baddr <= 0xff; baddr++) {
			bb = rand() % 2;
			expect = 1;
			updatecpu();
			TestRun();
			if (!checkresult()) {
				return errstring();
			}
		}
	}
	return "OK";
}
string CTestMcu::POP_DirectTest(int space) {
	int PC = 0, daddr = 0, SP = 0;
	byte code = 0xd0, expect = 0, spdata = 0;

	auto checkresult = [&]()
	{
		if(pCVir8051->Sys.PC != (PC + 2))
		return false;
		if(pCVir8051->Sys.sp() != (byte)(SP - 1))
		return false;
		if(pCVir8051->GetRamData(daddr) != expect)
		return false;
		return true;
	};

	auto errstring = [&] () {
		char temp[1024];
		sprintf(temp,"runtime info: PC:%x expect:%x SP:%x daddr-data:%x daddr:%x\r\n",
				PC, expect, SP, pCVir8051->GetRamData(daddr), daddr);
		return "error :" + string(temp)+ pCVir8051->ToStr();
	};

	auto updatecpu = [&]()
	{
		pCVir8051->InitalReg();
		pCVir8051->SetRamDataAt(SP, spdata);
		pCVir8051->Sys.PC = PC;
		pCVir8051->Sys.sp = SP;
		pCVir8051->m_ExeFile[PC] = code;
		pCVir8051->m_ExeFile[PC + 1] = daddr;
	};

	for (PC = 0; PC <= CVir8051::MAX_ROM - 2; PC = PC + space) {
		if (PC % 1000 == 0) {
			LogPrint("code = %x PC = %d\n", code, PC);
		}
		for (SP = 8; SP <= 0xff; SP++) {
			for (daddr = 0; daddr <= 0xff; daddr++) {
				if (daddr == 0x81) {
					continue; //modify the sp
				}
				spdata = rand() % 256;
				expect = spdata;
				updatecpu();
				TestRun();
				if (!checkresult()) {
					return errstring();
				}
			}
		}
	}
	return "OK";
}
string CTestMcu::XCH_A_RnTest(int space) {
	byte code = 0, expectrr = 0, expectaa = 0;
	int PC = 0, rr = 0, aa = 0;

	auto checkresult = [&]()
	{
		byte ret = 0;
		switch (code) {
			case 0xc8:
			ret = pCVir8051->Rges.R0();
			break;
			case 0xc9:
			ret = pCVir8051->Rges.R1();
			break;
			case 0xca:
			ret = pCVir8051->Rges.R2();
			break;
			case 0xcb:
			ret = pCVir8051->Rges.R3();
			break;
			case 0xcc:
			ret = pCVir8051->Rges.R4();
			break;
			case 0xcd:
			ret = pCVir8051->Rges.R5();
			break;
			case 0xce:
			ret = pCVir8051->Rges.R6();
			break;
			case 0xcf:
			ret = pCVir8051->Rges.R7();
			break;
			default:
			break;
		}
		if(ret != expectrr)
		return false;
		if (pCVir8051->Sys.PC != (PC + 1))
		return false;
		if(pCVir8051->Sys.a() != expectaa)
		return false;
		if(IsOdd(pCVir8051->Sys.a()) != pCVir8051->Sys.psw().p)
		return false;
		return true;
	};

	auto errstring = [&] () {
		char temp[1024];
		sprintf(temp,"runtime info: PC:%x expectaa:%x expectrr:%x\r\n", PC, expectaa, expectrr);
		return "error :" + string(temp)+ pCVir8051->ToStr();
	};

	auto updatecpu = [&] () {
		pCVir8051->InitalReg();
		switch (code) {
			case 0xc8:
			pCVir8051->Rges.R0 = rr;
			break;
			case 0xc9:
			pCVir8051->Rges.R1 = rr;
			break;
			case 0xca:
			pCVir8051->Rges.R2 = rr;
			break;
			case 0xcb:
			pCVir8051->Rges.R3 = rr;
			break;
			case 0xcc:
			pCVir8051->Rges.R4 = rr;
			break;
			case 0xcd:
			pCVir8051->Rges.R5 = rr;
			break;
			case 0xce:
			pCVir8051->Rges.R6 = rr;
			break;
			case 0xcf:
			pCVir8051->Rges.R7 = rr;;
			break;
			default:
			break;
		}
		pCVir8051->Sys.a = aa;
		pCVir8051->Sys.PC = PC;
		pCVir8051->m_ExeFile[PC] = code;
	};

	for (code = 0xc8; code <= 0xcf; code++) {
		for (PC = 0; PC <= CVir8051::MAX_ROM - 1; PC = PC + space) {
			if (PC % 1000 == 0) {
				LogPrint("code = %x PC = %d\n", code, PC);
			}
			for (rr = 0; rr <= 0xff; rr++) {
				for (aa = 0; aa <= 0xff; aa++) {
					expectaa = rr;
					expectrr = aa;
					updatecpu();
					TestRun();
					if (!checkresult()) {
						return errstring();
					}
				}

			}
		}
	}
	return "OK";
}
string CTestMcu::XCH_A_DirectTest(int space) {
	int PC = 0, daddr = 0, aa = 0;
	byte code = 0xc5, expectaa = 0, expectd = 0, ddata = 0;

	auto checkresult = [&]()
	{
		if(pCVir8051->Sys.PC != (PC + 2))
		{
			LogPrint("1\n");
			return false;
		}
		if(pCVir8051->Sys.a() != expectaa)
		{
			LogPrint("2\n");
			return false;
		}
		if(IsOdd(pCVir8051->Sys.a()) != pCVir8051->Sys.psw().p)
		{
			LogPrint("3\n");
			return false;
		}
		if(pCVir8051->GetRamData((byte)daddr) != expectd)
		{
			LogPrint("4\n");
			return false;
		}
		return true;
	};

	auto errstring = [&] () {
		char temp[1024];
		sprintf(temp,"runtime info: PC:%x expectaa:%x expectd:%x daddr:%x\r\n", PC, expectaa, expectd, daddr);
		return "error :" + string(temp)+ pCVir8051->ToStr();
	};

	auto updatecpu = [&]()
	{
		pCVir8051->InitalReg();
		pCVir8051->SetRamData((byte)daddr, ddata);
		pCVir8051->Sys.PC = PC;
		pCVir8051->Sys.a = (byte)aa;
		pCVir8051->m_ExeFile[PC] = code;
		pCVir8051->m_ExeFile[PC + 1] = (byte)daddr;
	};

	for (PC = 0; PC <= CVir8051::MAX_ROM - 2; PC = PC + space) {
		if (PC % 1000 == 0) {
			LogPrint("code = %x PC = %d\n", code, PC);
		}
		for (aa = 0; aa <= 0xff; aa++) {
			for (daddr = 0; daddr <= 0xff; daddr++) {
				if (daddr == 0xd0 || daddr == 0xe0) {
					continue; //modify the psw and a
				}
				ddata = rand() % 256;
				expectaa = ddata;
				expectd = (byte) aa;
				updatecpu();
				TestRun();
				if (!checkresult()) {
					return errstring();
				}
			}
		}
	}
	return "OK";
}
string CTestMcu::SWAP_ATest(int space) {
	int PC = 0, aa = 0;
	byte code = 0xc4, expect = 0;

	auto checkresult = [&]()
	{
		if(pCVir8051->Sys.PC != (PC + 1))
		return false;
		if(pCVir8051->Sys.a() != expect)
		return false;
		return true;
	};

	auto errstring = [&] () {
		char temp[1024];
		sprintf(temp,"runtime info: PC:%x expect:%x aa:%x\r\n", PC, expect, aa);
		return "error :" + string(temp)+ pCVir8051->ToStr();
	};

	auto updatecpu = [&]()
	{
		pCVir8051->InitalReg();
		pCVir8051->Sys.PC = PC;
		pCVir8051->Sys.a = aa;
		pCVir8051->m_ExeFile[PC] = code;
	};

	for (PC = 0; PC <= CVir8051::MAX_ROM - 1; PC = PC + space) {
		if (PC % 1000 == 0) {
			LogPrint("code = %x PC = %d\n", code, PC);
		}
		for (aa = 0; aa <= 0xff; aa++) {
			expect = (byte) (((aa << 4) & 0xf0) | ((aa >> 4) & 0x0f));
			updatecpu();
			TestRun();
			if (!checkresult()) {
				return errstring();
			}
		}
	}
	return "OK";
}
string CTestMcu::CLR_CTest(int space) {
	int PC = 0;
	byte code = 0xc3, cc = 0, expect = 0;

	auto checkresult = [&]()
	{
		if(pCVir8051->Sys.PC != (PC + 1))
		return false;
		if(pCVir8051->Sys.psw().cy != expect)
		return false;
		return true;
	};

	auto errstring = [&] () {
		char temp[1024];
		sprintf(temp,"runtime info: PC:%x daddr:%x\r\n", PC, expect);
		return "error :" + string(temp)+ pCVir8051->ToStr();
	};

	auto updatecpu = [&]()
	{
		pCVir8051->InitalReg();
		pCVir8051->Sys.PC = PC;
		pCVir8051->Sys.psw().cy = cc;
		pCVir8051->m_ExeFile[PC] = code;
	};

	for (PC = 0; PC <= CVir8051::MAX_ROM - 1; PC = PC + space) {
		if (PC % 1000 == 0) {
			LogPrint("code = %x PC = %d\n", code, PC);
		}
		for (cc = 0; cc <= 0x01; cc++) {
			expect = 0;
			updatecpu();
			TestRun();
			if (!checkresult()) {
				return errstring();
			}
		}
	}
	return "OK";
}
string CTestMcu::CLR_BitTest(int space) {
	int PC = 0, baddr = 0;
	byte code = 0xc2, bb = 0, expect = 0;

	auto checkresult = [&]()
	{
		if(pCVir8051->Sys.PC != (PC + 2))
		return false;
		if(pCVir8051->GetBitFlag(baddr) != expect)
		return false;
		return true;
	};

	auto errstring = [&] () {
		char temp[1024];
		sprintf(temp,"runtime info: PC:%x daddr:%x\r\n", PC, expect);
		return "error :" + string(temp)+ pCVir8051->ToStr();
	};

	auto updatecpu = [&]()
	{
		pCVir8051->InitalReg();
		if(bb) {
			pCVir8051->SetBitFlag(baddr);
		}
		else {
			pCVir8051->ClrBitFlag(baddr);
		}
		pCVir8051->Sys.PC = PC;
		pCVir8051->m_ExeFile[PC] = code;
		pCVir8051->m_ExeFile[PC + 1] = baddr;
	};

	for (PC = 0; PC <= CVir8051::MAX_ROM - 1; PC = PC + space) {
		if (PC % 1000 == 0) {
			LogPrint("code = %x PC = %d\n", code, PC);
		}
		for (baddr = 0; baddr <= 0xff; baddr++) {
			bb = rand() % 2;
			expect = 0;
			updatecpu();
			TestRun();
			if (!checkresult()) {
				return errstring();
			}
		}
	}
	return "OK";
}
string CTestMcu::PUSH_DirectTest(int space) {
	int PC = 0, daddr = 0, SP = 0;
	byte code = 0xc0, expect = 0, ddata = 0;

	auto checkresult = [&]()
	{
		if(pCVir8051->Sys.PC != (PC + 2))
		{
			LogPrint("1\n");
			return false;
		}
		if(pCVir8051->Sys.sp() != (byte)(SP + 1))
		{
			LogPrint("2\n");
			return false;
		}
		if(pCVir8051->GetRamDataAt((byte)(SP + 1)) != expect)
		{
			LogPrint("3:%x %x\n", pCVir8051->GetRamDataAt((byte)(SP + 1)), pCVir8051->GetRamDataAt((byte)(SP)));
			return false;
		}
		return true;
	};

	auto errstring = [&] () {
		char temp[1024];
		sprintf(temp,"runtime info: PC:%x expect:%x\r\n", PC, expect);
		return "error :" + string(temp)+ pCVir8051->ToStr();
	};

	auto updatecpu = [&]()
	{
		pCVir8051->InitalReg();
		pCVir8051->SetRamData(daddr, ddata);
		pCVir8051->Sys.PC = PC;
		pCVir8051->Sys.sp = SP;
		pCVir8051->m_ExeFile[PC] = code;
		pCVir8051->m_ExeFile[PC + 1] = daddr;
	};

	for (PC = 0; PC <= CVir8051::MAX_ROM - 2; PC = PC + space) {
		if (PC % 1000 == 0) {
			LogPrint("code = %x PC = %d\n", code, PC);
		}
		for (SP = 0; SP <= 0xff; SP++) {
			for (daddr = 0; daddr <= 0xff; daddr++) {
				if (daddr == 0x81) {
					continue; //modify the sp
				}
				ddata = rand() % 256;
				expect = ddata;
				updatecpu();
				TestRun();
				if (!checkresult()) {
					return errstring();
				}
			}
		}
	}
	return "OK";
}

//spark
string CTestMcu::MOV_Direct_Rn_Test(int space) {
	byte code;
	int PC = 0, addr = 0, rr = 0;
	byte expect;

	auto checkresult = [&]()
	{
		if ((pCVir8051->GetRamData(addr) != expect) || (pCVir8051->Sys.PC != (PC + 2)))
		return false;
		return true;
	};

	auto errstring = [&] () {
		char temp[1024];
		sprintf(temp,"runtime info: PC:%x expect:%x addr:%x\r\n", PC, expect, addr);
		return "error :" + string(temp)+ pCVir8051->ToStr();
	};

	auto updatecpu = [&] () {
		pCVir8051->InitalReg();
		switch (code) {
			case 0x88:
			pCVir8051->Rges.R0 = rr;
			break;
			case 0x89:
			pCVir8051->Rges.R1 = rr;
			break;
			case 0x8a:
			pCVir8051->Rges.R2 = rr;
			break;
			case 0x8b:
			pCVir8051->Rges.R3 = rr;
			break;
			case 0x8c:
			pCVir8051->Rges.R4 = rr;
			break;
			case 0x8d:
			pCVir8051->Rges.R5 = rr;
			break;
			case 0x8e:
			pCVir8051->Rges.R6 = rr;
			break;
			case 0x8f:
			pCVir8051->Rges.R7 = rr;
			break;
			default:
			break;
		}
		pCVir8051->Sys.PC = PC;
		pCVir8051->m_ExeFile[PC] = code;
		pCVir8051->m_ExeFile[PC + 1] = addr;
	};

	for (code = 0x88; code <= 0x8f; code++) {
		for (PC = 0; PC <= CVir8051::MAX_ROM - 2; PC = PC + space) {
			if (PC % 1000 == 0) {
				LogPrint("code = %x PC = %d\n", code, PC);
			}
			for (rr = 0; rr <= 0xff; rr++) {
				for (addr = 8; addr <= CVir8051::MAX_IN_RAM; addr++) {
					expect = rr;
					updatecpu();
					TestRun();
					if (!checkresult()) {
						return errstring();
					}
				}
			}
		}
	}
	return "OK";
}
string CTestMcu::MOV_Direct_Ri_Test(int space) {
	byte code;
	int PC = 0, rr = 0, addr = 0;
	byte expect;
	byte data;

	auto checkresult = [&]()
	{
		if ((pCVir8051->GetRamData(addr) != expect) || (pCVir8051->Sys.PC != (PC + 2)))
		return false;
		return true;
	};

	auto errstring = [&] () {
		char temp[1024];
		sprintf(temp,"runtime info: PC:%x expect:%x rr:%x data:%x addr:%x\r\n", PC, expect, rr, data, addr);
		return "error :" + string(temp)+ pCVir8051->ToStr();
	};

	auto updatecpu = [&] () {
		pCVir8051->InitalReg();
		switch (code) {
			case 0x86:
			pCVir8051->Rges.R0 = rr;
			break;
			case 0x87:
			pCVir8051->Rges.R1 = rr;
			break;
			default:
			break;
		}
		pCVir8051->SetRamDataAt(rr, data);
		pCVir8051->Sys.PC = PC;
		pCVir8051->m_ExeFile[PC] = code;
		pCVir8051->m_ExeFile[PC + 1] = addr;
	};

	for (code = 0x86; code <= 0x87; code++) {
		for (PC = 0; PC <= CVir8051::MAX_ROM - 2; PC = PC + space) {
			if (PC % 1000 == 0) {
				LogPrint("NEW code = %x PC = %d\n", code, PC);
			}

			for (rr = 8; rr <= CVir8051::MAX_IN_RAM; rr++) {
				for (addr = 8; addr <= CVir8051::MAX_IN_RAM; addr++) {
					if (addr == rr)
						continue;
					data = rand() % 256;
					expect = data;
					updatecpu();
					TestRun();
					//check result
					if (!checkresult()) {
						return errstring();
					}
				}
			}
		}
	}
	return "OK";
}
string CTestMcu::MOV_DPTR_Data_Test(int space) {
	byte code = 0;
	int PC = 0, datah = 0, datal = 0;
	int expect;

	auto checkresult = [&]()
	{
		if ((pCVir8051->Sys.dptr() != (unsigned short)expect) || (pCVir8051->Sys.PC != (PC + 3)))
		return false;
		return true;
	};

	auto errstring = [&] () {
		char temp[1024];
		sprintf(temp,"runtime info: PC:%x expect:%x\r\n", PC, expect);
		return "error :" + string(temp)+ pCVir8051->ToStr();
	};

	auto updatecpu = [&] () {
		pCVir8051->InitalReg();
		pCVir8051->Sys.PC = PC;
		pCVir8051->m_ExeFile[PC] = code;
		pCVir8051->m_ExeFile[PC + 1] = datah;
		pCVir8051->m_ExeFile[PC + 2] = datal;
	};

	code = 0x90;
	for (PC = 0; PC <= CVir8051::MAX_ROM - 3; PC = PC + space) {
		if (PC % 1000 == 0) {
			LogPrint("code = %x PC = %d\n", code, PC);
		}
		for (datah = 0; datah <= 0xff; datah++) {
			for (datal = 0; datal <= 0xff; datal++) {
				expect = (datah << 8) | datal;
				updatecpu();
				TestRun();
				if (!checkresult()) {
					return errstring();
				}
			}
		}
	}
	return "OK";
}
string CTestMcu::MOV_Bit_C_Test(int space) {
	byte code;
	int addr = 0, PC = 0;
	byte expect;
	byte cc = 0;

	auto checkresult = [&]()
	{
		if ((pCVir8051->GetBitFlag(addr) != expect) || (pCVir8051->Sys.PC != (PC + 2)))
		return false;
		return true;
	};

	auto errstring = [&] () {
		char temp[1024];
		sprintf(temp,"runtime info: PC:%x expect:%x addr:%x\r\n", PC, expect, addr);
		return "error :" + string(temp)+ pCVir8051->ToStr();
	};

	auto updatecpu = [&] () {
		pCVir8051->InitalReg();
		pCVir8051->Sys.psw().cy = cc;
		pCVir8051->Sys.PC = PC;
		pCVir8051->m_ExeFile[PC] = code;
		pCVir8051->m_ExeFile[PC + 1] = addr;
	};

	code = 0x92;
	for (PC = 0; PC <= CVir8051::MAX_ROM - 2; PC = PC + space) {
		if (PC % 1000 == 0) {
			LogPrint("code = %x PC = %d\n", code, PC);
		}
		for (addr = 0; addr <= 0xff; addr++) {
			cc = rand() % 2;
			expect = cc;
			updatecpu();
			TestRun();
			if (!checkresult()) {
				return errstring();
			}
		}
	}
	return "OK";
}
string CTestMcu::MOV_C_Bit_Test(int space) {
	byte code;
	int addr = 0, PC = 0;
	byte expect;
	byte bb = 0;

	auto checkresult = [&]()
	{
		if ((pCVir8051->Sys.psw().cy != expect) || (pCVir8051->Sys.PC != (PC + 2)))
		return false;
		return true;
	};

	auto errstring = [&] () {
		char temp[1024];
		sprintf(temp,"runtime info: PC:%x expect:%x addr:%x\r\n", PC, expect, addr);
		return "error :" + string(temp)+ pCVir8051->ToStr();
	};

	auto updatecpu = [&] () {
		pCVir8051->InitalReg();
		if (bb) {
			pCVir8051->SetBitFlag(addr);
		} else {
			pCVir8051->ClrBitFlag(addr);
		}
		pCVir8051->Sys.PC = PC;
		pCVir8051->m_ExeFile[PC] = code;
		pCVir8051->m_ExeFile[PC + 1] = addr;
	};

	code = 0xa2;
	for (PC = 0; PC <= CVir8051::MAX_ROM - 2; PC = PC + space) {
		if (PC % 1000 == 0) {
			LogPrint("code = %x PC = %d\n", code, PC);
		}
		for (addr = 0; addr <= 0xff; addr++) {
			bb = rand() % 2;
			expect = bb;
			updatecpu();
			TestRun();
			if (!checkresult()) {
				return errstring();
			}
		}
	}
	return "OK";
}
string CTestMcu::MOVC_A_DPTR_Test(int space) {
	byte code;
	int PC;
	byte expect;
	byte data;
	byte aa;
	int dptr;

	auto checkresult = [&]()
	{
		if(IsOdd(pCVir8051->Sys.a()) != pCVir8051->Sys.psw().p)
		return false;
		if ((pCVir8051->Sys.a() != expect) || (pCVir8051->Sys.PC != (PC + 1)))
		return false;
		return true;
	};

	auto errstring = [&] () {
		char temp[1024];
		sprintf(temp,"runtime info: PC:%x expect:%x aa:%x dptr:%x mfile:%x\r\n",
				PC, expect, aa, dptr, pCVir8051->m_ExeFile[aa + dptr]);
		return "error :" + string(temp)+ pCVir8051->ToStr();
	};

	auto updatecpu = [&] () {
		pCVir8051->InitalReg();
		pCVir8051->m_ExeFile[aa + dptr] = data;
		pCVir8051->Sys.a = aa;
		pCVir8051->Sys.dptr = dptr;
		pCVir8051->Sys.PC = PC;
		pCVir8051->m_ExeFile[PC] = code;
	};

	code = 0x93;
	for (PC = 0; PC <= CVir8051::MAX_ROM - 1; PC = PC + space) {
		if (PC % 1000 == 0) {
			LogPrint("code = %x PC = %d\n", code, PC);
		}
		for (dptr = 0; dptr <= 0xffff; dptr++) {
			aa = rand() % 256;
			data = rand() % 256;
			if (aa + dptr > 0xffff || aa + dptr == PC)
				continue;
			expect = data;
			updatecpu();
			TestRun();
			if (!checkresult()) {
				return errstring();
			}
		}
	}

	return "OK";
}
string CTestMcu::SUBB_A_RnTest(int space) {
	byte code;
	int PC = 0, aa = 0, rr = 0;
	byte expect;
	byte cc;

	auto checkresult = [&]()
	{
		byte xx6 = (((aa & 0x7f) < ((rr & 0x7f) + cc))) ? (1) : (0);
		byte xx7 = (aa < (rr + cc))?(1):(0);
		if(IsOdd(pCVir8051->Sys.a()) != pCVir8051->Sys.psw().p)
		{
			LogPrint("1.....\n");
			return false;
		}
		if(pCVir8051->Sys.psw().cy != ((aa < (rr + cc))?(1):(0)))
		{
			LogPrint("2.....\n");
			return false;
		}
		if(pCVir8051->Sys.psw().ac != ((((aa&0x0f) < ((rr&0x0f) + cc)))?(1):(0)))
		{
			LogPrint("3.....\n");
			return false;
		}
		if(pCVir8051->Sys.psw().ov != ((xx7 ^ xx6)?(1):(0)))
		{
			LogPrint("4.....\n");
			return false;
		}
		if ((pCVir8051->Sys.a() != (byte)expect) || (pCVir8051->Sys.PC != (PC + 1)))
		{
			LogPrint("5.....\n");
			return false;
		}
		return true;
	};

	auto errstring = [&] () {
		char temp[1024];
		byte xx6 = (((aa & 0x7f) > (rr & 0x7f) + cc)) ? (1) : (0);
		byte xx7 = (expect > 0xff)?(1):(0);
		sprintf(temp,"runtime info: PC:%x expect:%x aa:%x rr:%x cc:%x xx6:%x xx7:%x\r\n", PC, expect, aa, rr, cc,
				xx6, xx7);
		return "error :" + string(temp)+ pCVir8051->ToStr();
	};

	auto updatecpu = [&] () {
		pCVir8051->InitalReg();
		switch (code) {
			case 0x98:
			pCVir8051->Rges.R0 = rr;
			break;
			case 0x99:
			pCVir8051->Rges.R1 = rr;
			break;
			case 0x9a:
			pCVir8051->Rges.R2 = rr;
			break;
			case 0x9b:
			pCVir8051->Rges.R3 = rr;
			break;
			case 0x9c:
			pCVir8051->Rges.R4 = rr;
			break;
			case 0x9d:
			pCVir8051->Rges.R5 = rr;
			break;
			case 0x9e:
			pCVir8051->Rges.R6 = rr;
			break;
			case 0x9f:
			pCVir8051->Rges.R7 = rr;
			break;
			default:
			break;
		}
		pCVir8051->Sys.PC = PC;
		pCVir8051->Sys.a = aa;
		pCVir8051->Sys.psw().cy = cc;
		memcpy(&pCVir8051->m_ExeFile[PC], &code, sizeof(code));
	};

	for (code = 0x98; code <= 0x9f; code++) {
		for (PC = 0; PC <= CVir8051::MAX_ROM - 1; PC = PC + space) {
			if (PC % 1000 == 0) {
				LogPrint("code = %x PC = %d\n", code, PC);
			}
			for (rr = 0; rr <= 0xff; rr++) {
				for (aa = 0; aa <= 0xff; aa++) {
					cc = rand() % 2;
					expect = aa - rr - cc;
					updatecpu();
					TestRun();
					if (!checkresult()) {
						return errstring();
					}
				}
			}
		}
	}
	return "OK";
}
string CTestMcu::SUBB_A_RiTest(int space) {
	byte code;
	int PC;
	int expect = 0, aa = 0, rr = 0;
	int cc;
	byte data;

	auto checkresult = [&]()
	{
		byte xx6 = (((aa & 0x7f) < (data & 0x7f) + cc)) ? (1) : (0);
		byte xx7 = (aa < (data + cc))?(1):(0);
		if(IsOdd(pCVir8051->Sys.a()) != pCVir8051->Sys.psw().p)
		{
			LogPrint("1\n");
			return false;
		}

		if(pCVir8051->Sys.psw().cy != ((aa < (data + cc))?(1):(0)))
		{
			LogPrint("2\n");
			return false;
		}
		if(pCVir8051->Sys.psw().ac != ((((aa&0x0f) < (data&0x0f) + cc))?(1):(0)))
		{
			LogPrint("3\n");
			return false;
		}
		if(pCVir8051->Sys.psw().ov != ((xx7 ^ xx6)?(1):(0)))
		{
			LogPrint("4\n");
			return false;
		}
		if ((pCVir8051->Sys.a() != (byte)expect) || (pCVir8051->Sys.PC != (PC + 1)))
		{
			LogPrint("5\n");
			return false;
		}
		return true;
	};

	auto errstring = [&] () {
		char temp[1024];
		byte xx6 = (((aa & 0x7f) > (data & 0x7f) + cc)) ? (1) : (0);
		byte xx7 = (aa < (data + cc))?(1):(0);
		sprintf(temp,"runtime info: PC:%x expect:%x aa:%x data:%x cc:%x xx6:%x xx7:%x\r\n", PC, expect, aa, data, cc,
				xx6, xx7);
		return "error :" + string(temp)+ pCVir8051->ToStr();
	};

	auto updatecpu = [&] () {
		pCVir8051->InitalReg();
		switch (code) {
			case 0x96:
			pCVir8051->Rges.R0 = rr;
			break;
			case 0x97:
			pCVir8051->Rges.R1 = rr;
			break;
			default:
			break;
		}
		pCVir8051->SetRamDataAt(rr, data);
		pCVir8051->Sys.PC = PC;
		pCVir8051->Sys.a = aa;
		pCVir8051->Sys.psw().cy = cc;
		pCVir8051->m_ExeFile[PC] = code;
	};

	for (code = 0x96; code <= 0x97; code++) {
		for (PC = 0; PC <= CVir8051::MAX_ROM - 1; PC = PC + space) {
			if (PC % 1000 == 0)
				LogPrint("code = %x PC = %d\n", code, PC);
			for (rr = 8; rr <= CVir8051::MAX_IN_RAM; rr++) {
				for (aa = 0; aa <= 0xff; aa++) {
					data = rand() % 256;
					cc = rand() % 2;
					expect = aa - data - cc;
					updatecpu();
					TestRun();
					if (!checkresult()) {
						return errstring();
					}
				}
			}
		}
	}
	return "OK";
}
string CTestMcu::SUBB_A_DirectTest(int space) {
	byte code;
	int PC;
	int expect = 0, aa = 0, addr = 0;
	byte data;
	int cc;

	auto checkresult = [&]()
	{
		byte xx6 = (((aa & 0x7f) < (data & 0x7f) + cc)) ? (1) : (0);
		byte xx7 = (aa < (data + cc))?(1):(0);
		if(IsOdd(pCVir8051->Sys.a()) != pCVir8051->Sys.psw().p)
		{
			LogPrint("1\n");
			return false;
		}
		if(pCVir8051->Sys.psw().cy != ((aa < (data + cc))?(1):(0)))
		{
			LogPrint("2\n");
			return false;
		}
		if(pCVir8051->Sys.psw().ac != ((((aa&0x0f) < (data&0x0f) + cc))?(1):(0)))
		{
			LogPrint("3\n");
			return false;
		}
		if(pCVir8051->Sys.psw().ov != ((xx7 ^ xx6)?(1):(0)))
		{
			LogPrint("4\n");
			return false;
		}
		if ((pCVir8051->Sys.a() != (byte)expect) || (pCVir8051->Sys.PC != (PC + 2)))
		{
			LogPrint("5\n");
			return false;
		}
		return true;
	};

	auto errstring = [&] () {
		char temp[1024];
		byte xx6 = (((aa & 0x7f) < (data & 0x7f) + cc)) ? (1) : (0);
		byte xx7 = (aa < (data + cc))?(1):(0);
		sprintf(temp,"runtime info: PC:%x expect:%x aa:%x data:%x cc:%x xx6:%x xx7:%x addr:%x\r\n",
				PC, expect, aa, data, cc, xx6, xx7, addr);
		return "error :" + string(temp)+ pCVir8051->ToStr();
	};

	auto updatecpu = [&] () {
		pCVir8051->InitalReg();
		pCVir8051->SetRamData(addr, data);
		pCVir8051->Sys.PC = PC;
		pCVir8051->Sys.a = aa;
		pCVir8051->Sys.psw().cy = cc;
		pCVir8051->m_ExeFile[PC] = code;
		pCVir8051->m_ExeFile[PC + 1] = addr;
	};

	code = 0x95;
	for (PC = 0; PC <= CVir8051::MAX_ROM - 2; PC = PC + space) {
		if (PC % 1000 == 0) {
			LogPrint("code = %x PC = %d\n", code, PC);
		}
		for (addr = 8; addr <= CVir8051::MAX_IN_RAM; addr++) {
			for (aa = 0; aa <= 0xff; aa++) {
				if (addr == 0xe0 || addr == 0xd0) {
					continue; //modify the psw and a
				}
				data = rand() % 256;
				cc = rand() % 2;
				expect = aa - data - cc;
				updatecpu();
				TestRun();
				if (!checkresult()) {
					return errstring();
				}
			}
		}
	}
	return "OK";
}
string CTestMcu::SUBB_A_Data_Test(int space) {
	byte code;
	int PC;
	int expect = 0, aa = 0, data = 0;
	int cc;

	auto checkresult = [&]()
	{
		byte xx6 = (((aa & 0x7f) < (data & 0x7f) + cc)) ? (1) : (0);
		byte xx7 = (aa < (data + cc))?(1):(0);
		if(IsOdd(pCVir8051->Sys.a()) != pCVir8051->Sys.psw().p)
		{
			LogPrint("1\n");
			return false;
		}
		if(pCVir8051->Sys.psw().cy != ((aa < (data + cc))?(1):(0)))
		{
			LogPrint("2\n");
			return false;
		}
		if(pCVir8051->Sys.psw().ac != ((((aa&0x0f) < (data&0x0f) + cc))?(1):(0)))
		{
			LogPrint("3\n");
			return false;
		}
		if(pCVir8051->Sys.psw().ov != ((xx7 ^ xx6)?(1):(0)))
		{
			LogPrint("4\n");
			return false;
		}
		if ((pCVir8051->Sys.a() != (byte)expect) || (pCVir8051->Sys.PC != (PC + 2)))
		{
			LogPrint("5\n");
			return false;
		}
		return true;
	};

	auto errstring = [&] () {
		char temp[1024];
		byte xx6 = (((aa & 0x7f) < (data & 0x7f) + cc)) ? (1) : (0);
		byte xx7 = (aa < (data + cc))?(1):(0);
		sprintf(temp,"runtime info: PC:%x expect:%x aa:%x data:%x cc:%x xx6:%x xx7:%x\r\n", PC, expect, aa, data, cc,
				xx6, xx7);
		return "error :" + string(temp)+ pCVir8051->ToStr();
	};

	auto updatecpu = [&] () {
		pCVir8051->InitalReg();
		pCVir8051->Sys.PC = PC;
		pCVir8051->Sys.a = aa;
		pCVir8051->Sys.psw().cy = cc;
		pCVir8051->m_ExeFile[PC] = code;
		pCVir8051->m_ExeFile[PC + 1] = data;
	};

	code = 0x94;
	for (PC = 0; PC <= CVir8051::MAX_ROM - 2; PC = PC + space) {
		if (PC % 1000 == 0) {
			LogPrint("code = %x PC = %d\n", code, PC);
		}
		for (data = 0; data <= 0xff; data++) {
			for (aa = 0; aa <= 0xff; aa++) {
				cc = rand() % 2;
				expect = aa - data - cc;
				updatecpu();
				TestRun();
				if (!checkresult()) {
					return errstring();
				}
			}
		}
	}
	return "OK";
}
string CTestMcu::MOV_Rn_Direct_Test(int space) {
	byte code;
	int PC;
	int rr;
	byte expect;
	byte data;
	int addr;

	auto run_code = [&] () {
		byte ret;
		switch (code) {
			case 0xa8:
			pCVir8051->Rges.R0 = rr;
			TestRun();
			ret = pCVir8051->Rges.R0();
			break;
			case 0xa9:
			pCVir8051->Rges.R1 = rr;
			TestRun();
			ret = pCVir8051->Rges.R1();
			break;
			case 0xaa:
			pCVir8051->Rges.R2 = rr;
			TestRun();
			ret = pCVir8051->Rges.R2();
			break;
			case 0xab:
			pCVir8051->Rges.R3 = rr;
			TestRun();
			ret = pCVir8051->Rges.R3();
			break;
			case 0xac:
			pCVir8051->Rges.R4 = rr;
			TestRun();
			ret = pCVir8051->Rges.R4();
			break;
			case 0xad:
			pCVir8051->Rges.R5 = rr;
			TestRun();
			ret = pCVir8051->Rges.R5();
			break;
			case 0xae:
			pCVir8051->Rges.R6 = rr;
			TestRun();
			ret = pCVir8051->Rges.R6();
			break;
			case 0xaf:
			pCVir8051->Rges.R7 = rr;
			TestRun();
			ret = pCVir8051->Rges.R7();
			break;
			default:
			ret = 0;
			break;
		}
		return ret;
	};

	auto errstring = [&] () {
		char temp[1024];
		sprintf(temp,"runtime info: PC:%x expect:%x\r\n", PC, expect);
		return "error :" + string(temp)+ pCVir8051->ToStr();
	};

	auto updatecpu = [&] () {
		pCVir8051->InitalReg();
		pCVir8051->SetRamData(addr, data);
		pCVir8051->Sys.PC = PC;
		pCVir8051->m_ExeFile[PC] = code;
		pCVir8051->m_ExeFile[PC + 1] = addr;
	};

	for (code = 0xa8; code <= 0xaf; code++) {
		for (PC = 0; PC <= CVir8051::MAX_ROM - 2; PC = PC + space) {
			if (PC % 1000 == 0) {
				LogPrint("code = %x PC = %d\n", code, PC);
			}
			for (rr = 0; rr <= 0xff; rr++) {
				for (addr = 8; addr <= CVir8051::MAX_IN_RAM; addr++) {
					data = rand() % 256;
					expect = data;
					updatecpu();
					if ((run_code() != expect) || (pCVir8051->Sys.PC != (PC + 2))) {
						return errstring();
					}
				}
			}
		}
	}
	return "OK";
}
string CTestMcu::MOV_Ri_Direct_Test(int space) {
	byte code;
	int PC = 0, addr = 0, rr = 0;
	byte expect;
	byte data;

	auto checkresult = [&]()
	{
		if ((pCVir8051->GetRamDataAt(rr) != expect) || (pCVir8051->Sys.PC != (PC + 2)))
		return false;
		return true;
	};

	auto errstring = [&] () {
		char temp[1024];
		sprintf(temp,"runtime info: PC:%x expect:%x addr:%x\r\n", PC, expect, addr);
		return "error :" + string(temp)+ pCVir8051->ToStr();
	};

	auto updatecpu = [&] () {
		pCVir8051->InitalReg();
		switch (code) {
			case 0xa6:
			pCVir8051->Rges.R0 = rr;
			break;
			case 0xa7:
			pCVir8051->Rges.R1 = rr;
			break;
			default:
			break;
		}
		pCVir8051->SetRamData(addr, data);
		pCVir8051->Sys.PC = PC;
		pCVir8051->m_ExeFile[PC] = code;
		pCVir8051->m_ExeFile[PC + 1] = addr;
	};

	for (code = 0xa6; code <= 0xa7; code++) {
		for (PC = 0; PC <= CVir8051::MAX_ROM - 2; PC = PC + space) {
			if (PC % 1000 == 0) {
				LogPrint("code = %x PC = %d\n", code, PC);
			}
			for (rr = 8; rr <= CVir8051::MAX_IN_RAM; rr++) {
				for (addr = 8; addr <= CVir8051::MAX_IN_RAM; addr++) {
					if (addr == rr)
						continue;
					data = rand() % 256;
					expect = data;
					updatecpu();
					TestRun();
					if (!checkresult()) {
						return errstring();
					}
				}
			}
		}
	}
	return "OK";
}
string CTestMcu::CJNE_Rn_Data_Rel_Test(int space) {
	byte code;
	int PC;
	int expect = 0, rr = 0, raddr = 0;
	byte addr;
	byte data;

	auto checkresult = [&]()
	{
		if (pCVir8051->Sys.PC != expect)
		return false;
		if(pCVir8051->Sys.psw().cy != ((rr < data)?(1):(0)))
		return false;
		return true;
	};

	auto errstring = [&] () {
		char temp[1024];
		sprintf(temp,"runtime info: PC:%x expect:%x addr:%x rr:%x data:%x\r\n", PC, expect, addr, rr, data);
		return "error :" + string(temp)+ pCVir8051->ToStr();
	};

	auto updatecpu = [&] () {
		pCVir8051->InitalReg();
		switch (code) {
			case 0xB8:
			pCVir8051->Rges.R0 = rr;
			break;
			case 0xB9:
			pCVir8051->Rges.R1 = rr;
			break;
			case 0xBa:
			pCVir8051->Rges.R2 = rr;
			break;
			case 0xbb:
			pCVir8051->Rges.R3 = rr;
			break;
			case 0xbc:
			pCVir8051->Rges.R4 = rr;
			break;
			case 0xbd:
			pCVir8051->Rges.R5 = rr;
			break;
			case 0xbe:
			pCVir8051->Rges.R6 = rr;
			break;
			case 0xbf:
			pCVir8051->Rges.R7 = rr;
			break;
			default:
			break;
		}
		pCVir8051->Sys.PC = PC;
		pCVir8051->m_ExeFile[PC] = code;
		pCVir8051->m_ExeFile[PC + 1] = data;
		pCVir8051->m_ExeFile[PC + 2] = addr;
	};

	for (code = 0xb8; code <= 0xbf; code++) {
		for (PC = 0; PC <= CVir8051::MAX_ROM - 3; PC = PC + space) {
			if (PC % 1000 == 0) {
				LogPrint("code = %x PC = %d\n", code, PC);
			}
			for (rr = 0; rr <= 0xff; rr++) {
				for (raddr = -128; raddr <= 127; raddr++) {
					data = rand() % 256;
					expect = (rr != data) ? (PC + 3 + addr) : (PC + 3);
					if (expect >= CVir8051::MAX_ROM || expect < 0) {
						continue;
					}
					updatecpu();
					TestRun();
					if (!checkresult()) {
						return errstring();
					}
				}
			}
		}
	}
	return "OK";
}
string CTestMcu::CJNE_Ri_Data_Rel_Test(int space) {
	byte code;
	int PC;
	int expect = 0, rr = 0, raddr = 0;
	byte addr;
	byte data;
	byte rdata;

	auto checkresult = [&]()
	{
		if (pCVir8051->Sys.PC != expect)
		return false;
		if(pCVir8051->Sys.psw().cy != ((rdata < data)?(1):(0)))
		return false;
		return true;
	};

	auto errstring = [&] () {
		char temp[1024];
		sprintf(temp,"runtime info: PC:%x expect:%x addr:%x rr:%x data:%x\r\n", PC, expect, addr, rr, data);
		return "error :" + string(temp)+ pCVir8051->ToStr();
	};

	auto updatecpu = [&] () {
		pCVir8051->InitalReg();
		switch (code) {
			case 0xB6:
			pCVir8051->Rges.R0 = rr;
			break;
			case 0xB7:
			pCVir8051->Rges.R1 = rr;
			break;
			default:
			break;
		}
		pCVir8051->SetRamDataAt(rr, rdata);
		pCVir8051->Sys.PC = PC;
		pCVir8051->m_ExeFile[PC] = code;
		pCVir8051->m_ExeFile[PC + 1] = data;
		pCVir8051->m_ExeFile[PC + 2] = addr;
	};

	for (code = 0xb6; code <= 0xb7; code++) {
		for (PC = 0; PC <= CVir8051::MAX_ROM - 3; PC = PC + space) {
			if (PC % 1000 == 0) {
				LogPrint("code = %x PC = %d\n", code, PC);
			}
			for (rr = 8; rr <= CVir8051::MAX_IN_RAM; rr++) {
				for (raddr = -128; raddr <= 127; raddr++) {
					data = rand() % 256;
					rdata = rand() % 256;
					expect = (rdata != data) ? (PC + 3 + addr) : (PC + 3);
					if (expect >= CVir8051::MAX_ROM || expect < 0) {
						continue;
					}
					updatecpu();
					TestRun();
					if (!checkresult()) {
						return errstring();
					}
				}
			}
		}
	}
	return "OK";
}
string CTestMcu::CJNE_A_Direct_Rel_Test(int space) {
	byte code;
	int PC;
	int expect = 0, addr = 0, raddr = 0;
	byte aa;
	byte data;

	auto checkresult = [&]()
	{
		if (pCVir8051->Sys.PC != expect)
		return false;
		if(pCVir8051->Sys.psw().cy != ((aa < pCVir8051->GetRamData(addr))?(1):(0)))
		return false;
		return true;
	};

	auto errstring = [&] () {
		char temp[1024];
		sprintf(temp,"runtime info: PC:%x expect:%x addr:%x\r\n", PC, expect, addr);
		return "error :" + string(temp)+ pCVir8051->ToStr();
	};

	auto updatecpu = [&] () {
		pCVir8051->InitalReg();
		pCVir8051->SetRamData(addr, data);
		pCVir8051->Sys.a = aa;
		pCVir8051->Sys.PC = PC;
		pCVir8051->m_ExeFile[PC] = code;
		pCVir8051->m_ExeFile[PC + 1] = addr;
		pCVir8051->m_ExeFile[PC + 2] = raddr;
	};

	code = 0xb5;
	for (PC = 0; PC <= CVir8051::MAX_ROM - 3; PC = PC + space) {
		if (PC % 1000 == 0) {
			LogPrint("code = %x PC = %d\n", code, PC);
		}
		for (addr = 8; addr <= CVir8051::MAX_IN_RAM; addr++) {
			for (raddr = -128; raddr <= 127; raddr++) {
				if (addr == 0xe0 || addr == 0xd0) {
					continue; //modify the cy
				}
				aa = rand() % 256;
				data = rand() % 256;
				expect = (aa != data) ? (PC + 3 + raddr) : (PC + 3);
				if (expect >= CVir8051::MAX_ROM || expect < 0) {
					continue;
				}
				updatecpu();
				TestRun();
				if (!checkresult()) {
					return errstring();
				}
			}
		}
	}
	return "OK";
}
string CTestMcu::CJNE_A_Data_Rel_Test(int space) {
	byte code;
	int PC;
	int expect = 0, raddr = 0;
	byte addr;
	byte aa;
	byte data;

	auto checkresult = [&]()
	{
		if (pCVir8051->Sys.PC != expect)
		return false;
		if(pCVir8051->Sys.psw().cy != ((aa < data)?(1):(0)))
		return false;
		return true;
	};

	auto errstring = [&] () {
		char temp[1024];
		sprintf(temp,"runtime info: PC:%x expect:%x addr:%x\r\n", PC, expect, addr);
		return "error :" + string(temp)+ pCVir8051->ToStr();
	};

	auto updatecpu = [&] () {
		pCVir8051->InitalReg();
		pCVir8051->Sys.a = aa;
		pCVir8051->Sys.PC = PC;
		pCVir8051->m_ExeFile[PC] = code;
		pCVir8051->m_ExeFile[PC + 1] = data;
		pCVir8051->m_ExeFile[PC + 2] = raddr;
	};

	code = 0xb4;
	for (PC = 0; PC <= CVir8051::MAX_ROM - 3; PC = PC + space) {
		if (PC % 1000 == 0) {
			LogPrint("code = %x PC = %d\n", code, PC);
		}
		for (raddr = -128; raddr <= 127; raddr++) {
			data = rand() % 0xff;
			aa = rand() % 0xff;
			expect = (aa != data) ? (PC + 3 + raddr) : (PC + 3);
			if (expect >= CVir8051::MAX_ROM || expect < 0) {
				continue;
			}
			updatecpu();
			TestRun();
			if (!checkresult()) {
				return errstring();
			}
		}
	}
	return "OK";
}
string CTestMcu::CPL_C_Test(int space) {
	byte code;
	int PC;
	byte expect;
	int cc = 0;

	auto checkresult = [&]()
	{
		if ((pCVir8051->Sys.psw().cy != expect) || (pCVir8051->Sys.PC != (PC + 1)))
		return false;
		return true;
	};

	auto errstring = [&] () {
		char temp[1024];
		sprintf(temp,"runtime info: PC:%x expect:%x\r\n", PC, expect);
		return "error :" + string(temp)+ pCVir8051->ToStr();
	};

	auto updatecpu = [&] () {
		pCVir8051->InitalReg();
		pCVir8051->Sys.psw().cy = cc;
		pCVir8051->Sys.PC = PC;
		pCVir8051->m_ExeFile[PC] = code;
	};

	code = 0xb3;
	for (PC = 0; PC <= CVir8051::MAX_ROM - 1; PC = PC + space) {
		if (PC % 1000 == 0) {
			LogPrint("code = %x PC = %d\n", code, PC);
		}
		cc = rand() % 2;
		expect = (cc) ? (0x00) : (0x01);
		updatecpu();
		TestRun();
		if (!checkresult()) {
			return errstring();
		}
	}
	return "OK";
}
string CTestMcu::CPL_Bit_Test(int space) {
	byte code;
	int PC;
	byte expect;
	int addr;
	int bb = 0;

	auto checkresult = [&]()
	{
		if ((pCVir8051->GetBitFlag(addr) != expect) || (pCVir8051->Sys.PC != (PC + 2)))
		return false;
		return true;
	};

	auto errstring = [&] () {
		char temp[1024];
		sprintf(temp,"runtime info: PC:%x expect:%x\r\n", PC, expect);
		return "error :" + string(temp)+ pCVir8051->ToStr();
	};

	auto updatecpu = [&] () {
		pCVir8051->InitalReg();
		if (bb) {
			pCVir8051->SetBitFlag(addr);
		} else {
			pCVir8051->ClrBitFlag(addr);
		}
		pCVir8051->Sys.PC = PC;
		pCVir8051->m_ExeFile[PC] = code;
		pCVir8051->m_ExeFile[PC + 1] = addr;
	};

	code = 0xb2;
	for (PC = 0; PC <= CVir8051::MAX_ROM - 2; PC = PC + space) {
		if (PC % 1000 == 0) {
			LogPrint("code = %x PC = %d\n", code, PC);
		}
		for (addr = 0; addr <= 0xff; addr++) {
			bb = rand() % 2;
			expect = (bb) ? (0x00) : (0x01);
			updatecpu();
			TestRun();
			if (!checkresult()) {
				return errstring();
			}
		}
	}
	return "OK";
}
string CTestMcu::ACALL_Addr11_Test(int space) {
//	pCVir8051->Opcode_B1_ACALL_Addr11();
	return "OK";
}
string CTestMcu::ANL_C_Bit_1_Test(int space) {
	byte code;
	int PC;
	bool expect;
	int addr;
	bool bb, cc;

	auto checkresult = [&]()
	{
		if ((pCVir8051->Sys.psw().cy != expect) || (pCVir8051->Sys.PC != (PC + 2)))
		return false;
		if(addr>= 0xe0 &&addr <= 0xe7)
		{
			if(IsOdd(pCVir8051->Sys.a()) != pCVir8051->Sys.psw().p)
			return false;
		}
		return true;
	};

	auto errstring = [&] () {
		char temp[1024];
		sprintf(temp,"runtime info: PC:%x expect:%x addr:%x\r\n", PC, expect, addr);
		return "error :" + string(temp)+ pCVir8051->ToStr();
	};

	auto updatecpu = [&] () {
		pCVir8051->InitalReg();
		pCVir8051->Sys.psw().cy = cc;
		if (bb) {
			pCVir8051->SetBitFlag(addr);
		} else {
			pCVir8051->ClrBitFlag(addr);
		}
		pCVir8051->Sys.PC = PC;
		pCVir8051->m_ExeFile[PC] = code;
		pCVir8051->m_ExeFile[PC + 1] = addr;
	};

	code = 0xb0;
	for (PC = 0; PC <= CVir8051::MAX_ROM - 2; PC = PC + space) {
		if (PC % 1000 == 0) {
			LogPrint("code = %x PC = %d\n", code, PC);
		}
		for (addr = 0; addr <= 0xff; addr++) {
			if (addr == CVir8051::psw_addr + 7) {
				continue; // because 0xd7 is the Sys.psw().cy falg;
			}
			bb = rand() % 2;
			cc = rand() % 2;
			expect = cc & (!bb);
			updatecpu();
			TestRun();
			if (!checkresult()) {
				return errstring();
			}
		}
	}
	return "OK";
}
string CTestMcu::MUL_AB_Test(int space) {
	byte code;
	int PC;
	int expect = 0, aa = 0, bb = 0;

	auto checkresult = [&]()
	{
		if (pCVir8051->Sys.a() != (byte)(expect&0xff))
		return false;
		if (pCVir8051->Sys.b() != (byte)(expect >> 8))
		return false;
		if(pCVir8051->Sys.psw().ov != ((expect > 255)?(1):(0)))
		return false;
		if(pCVir8051->Sys.psw().cy != 0)
		return false;
		if (pCVir8051->Sys.PC != (PC + 1))
		return false;
		return true;
	};

	auto errstring = [&] () {
		char temp[1024];
		sprintf(temp,"runtime info: PC:%x expect:%x\r\n", PC, expect);
		return "error :" + string(temp)+ pCVir8051->ToStr();
	};

	auto updatecpu = [&] () {
		pCVir8051->InitalReg();
		pCVir8051->Sys.psw = 0; //clear first
			pCVir8051->Sys.a = aa;
			pCVir8051->Sys.b = bb;
			pCVir8051->Sys.PC = PC;
			pCVir8051->m_ExeFile[PC] = code;
		};

	code = 0xa4;
	for (PC = 0; PC <= CVir8051::MAX_ROM - 1; PC = PC + space) {
		if (PC % 1000 == 0) {
			LogPrint("code = %x PC = %d\n", code, PC);
		}
		for (aa = 0; aa <= 0xff; aa++) {
			for (bb = 0; bb <= 0xff; bb++) {
				expect = bb * aa;
				updatecpu();
				TestRun();
				if (!checkresult()) {
					return errstring();
				}
			}
		}
	}
	return "OK";
}
string CTestMcu::INC_DPTR_Test(int space) {
	byte code;
	int PC;
	int expect;
	int dptr;

	auto checkresult = [&]()
	{
		if ((pCVir8051->Sys.dptr() != (unsigned short)expect) || (pCVir8051->Sys.PC != (PC + 1)))
		return false;
		return true;
	};

	auto errstring = [&] () {
		char temp[1024];
		sprintf(temp,"runtime info: PC:%x expect:%x\r\n", PC, expect);
		return "error :" + string(temp)+ pCVir8051->ToStr();
	};

	auto updatecpu = [&] () {
		pCVir8051->InitalReg();
		pCVir8051->Sys.PC = PC;
		pCVir8051->Sys.dptr = dptr;
		pCVir8051->m_ExeFile[PC] = code;
	};

	code = 0xa3;
	for (PC = 0; PC <= CVir8051::MAX_ROM - 1; PC = PC + space) {
		if (PC % 1000 == 0) {
			LogPrint("code = %x PC = %d\n", code, PC);
		}
		for (dptr = 0; dptr <= 0xffff; dptr++) {
			expect = dptr + 1;
			updatecpu();
			TestRun();
			if (!checkresult()) {
				return errstring();
			}
		}
	}
	return "OK";
}
string CTestMcu::AJMP_Addr11_Test(int space) {
	pCVir8051->Opcode_A1_AJMP_Addr11();
	return "OK";
}
string CTestMcu::ORL_C_Bit_Test(int space) {
	byte code;
	int PC;
	bool expect;
	int addr;
	bool bb, cc;

	auto checkresult = [&]()
	{
		if ((pCVir8051->Sys.psw().cy != expect) || (pCVir8051->Sys.PC != (PC + 2)))
		return false;
		if(addr>= 0xe0 &&addr <= 0xe7)
		{
			if(IsOdd(pCVir8051->Sys.a()) != pCVir8051->Sys.psw().p)
			return false;
		}
		return true;
	};

	auto errstring = [&] () {
		char temp[1024];
		sprintf(temp,"runtime info: PC:%x expect:%x addr:%x\r\n", PC, expect, addr);
		return "error :" + string(temp)+ pCVir8051->ToStr();
	};

	auto updatecpu = [&] () {
		pCVir8051->InitalReg();
		pCVir8051->Sys.psw().cy = cc;
		if (bb) {
			pCVir8051->SetBitFlag(addr);
		} else {
			pCVir8051->ClrBitFlag(addr);
		}
		pCVir8051->Sys.PC = PC;
		pCVir8051->m_ExeFile[PC] = code;
		pCVir8051->m_ExeFile[PC + 1] = addr;
	};

	code = 0xa0;
	for (PC = 0; PC <= CVir8051::MAX_ROM - 2; PC = PC + space) {
		if (PC % 1000 == 0) {
			LogPrint("code = %x PC = %d\n", code, PC);
		}
		for (addr = 0; addr <= 0xff; addr++) {
			if (addr == CVir8051::psw_addr + 7) {
				continue; // because 0xd7 is the Sys.psw().cy falg;
			}
			bb = rand() % 2;
			cc = rand() % 2;
			expect = cc | (!bb);
			updatecpu();
			TestRun();
			if (!checkresult()) {
				return errstring();
			}
		}
	}
	return "OK";
}

string CTestMcu::MOV_Ri_ATest(int space) {
	int PC = 0, aa = 0, ri = 0;
	byte code = 0xf2, expect = 0;

	auto checkresult = [&]()
	{
		if(pCVir8051->Sys.PC != (PC + 1))
		return false;
		if(pCVir8051->GetRamDataAt(ri) != expect)
		return false;
		return true;
	};

	auto errstring = [&] () {
		char temp[1024];
		sprintf(temp,"runtime info: PC:%x daddr:%x\r\n", PC, expect);
		return "error :" + string(temp)+ pCVir8051->ToStr();
	};

	auto updatecpu = [&]()
	{
		pCVir8051->InitalReg();
		if(code == 0xf6) {
			pCVir8051->Rges.R0 = ri;
		}
		else if(code == 0xf7) {
			pCVir8051->Rges.R1 = ri;
		}
		pCVir8051->Sys.PC = PC;
		pCVir8051->Sys.a = aa;
		pCVir8051->m_ExeFile[PC] = code;
	};

	for (code = 0xf6; code <= 0xf7; code++) {
		for (PC = 0; PC <= CVir8051::MAX_ROM - 1; PC = PC + space) {
			if (PC % 1000 == 0) {
				LogPrint("code = %x PC = %d\n", code, PC);
			}
			for (ri = 8; ri <= 0xff; ri++) {
				for (aa = 0; aa <= 0xff; aa++) {
					expect = aa;
					updatecpu();
					TestRun();
					if (!checkresult()) {
						return errstring();
					}
				}
			}
		}
	}
	return "OK";
}
string CTestMcu::XCH_A_RiTest(int space) {
	byte code = 0;
	int rr = 0, aa = 0, PC = 0;
	byte ridata = 0;
	byte expectaa = 0, expectri = 0;

	auto checkresult = [&]()
	{
		if(pCVir8051->GetRamDataAt(rr) != expectri)
		return false;
		if(IsOdd(pCVir8051->Sys.a()) != pCVir8051->Sys.psw().p)
		return false;
		if (pCVir8051->Sys.a() != expectaa)
		return false;
		if(pCVir8051->Sys.PC != (PC + 1))
		return false;
		return true;
	};

	auto errstring = [&] () {
		char temp[1024];
		sprintf(temp,"runtime info: PC:%x expectaa:%x aa:%x rr:%x expectri:%x ridata:%x\r\n",
				PC, expectaa, aa, rr, expectri, ridata);
		return "error :" + string(temp)+ pCVir8051->ToStr();
	};

	auto updatecpu = [&]() {
		pCVir8051->InitalReg();
		switch (code) {
			case 0xc6:
			pCVir8051->Rges.R0 = rr;
			break;
			case 0xc7:
			pCVir8051->Rges.R1 = rr;
			break;
			default:
			break;
		}
		pCVir8051->SetRamDataAt(rr, ridata);
		pCVir8051->Sys.PC = PC;
		pCVir8051->Sys.a = aa;
		pCVir8051->m_ExeFile[PC] = code;
	};

	for (code = 0xc6; code <= 0xc7; code++) {
		for (PC = 0; PC <= CVir8051::MAX_ROM - 1; PC = PC + space) {
			if (PC % 1000 == 0) {
				LogPrint("code = %x PC = %d\n", code, PC);
			}
			for (rr = 8; rr <= CVir8051::MAX_IN_RAM; rr++) {
				for (aa = 0; aa <= 0xff; aa++) {
					ridata = rand() % 256;
					expectaa = ridata;
					expectri = aa;
					updatecpu();
					TestRun();
					if (!checkresult()) {
						return errstring();
					}
				}
			}
		}
	}
	return "OK";
}
string CTestMcu::JNC_RelTest(int space) {
	byte code = 0x50, cc = 0;
	int raddr = 0;
	int PC = 0, expect = 0;

	auto checkresult = [&]()
	{
		if (pCVir8051->Sys.PC != expect)
		return false;
		return true;
	};

	auto errstring = [&] () {
		char temp[1024];
		sprintf(temp,"runtime info: PC:%x expect:%x\r\n", PC, expect);
		return "error :" + string(temp)+ pCVir8051->ToStr();
	};

	auto updatecpu = [&]() {
		pCVir8051->InitalReg();
		pCVir8051->Sys.PC = PC;
		pCVir8051->Sys.psw().cy = cc;
		pCVir8051->m_ExeFile[PC] = code;
		pCVir8051->m_ExeFile[PC + 1] = raddr;
	};

	for (PC = 0; PC <= CVir8051::MAX_ROM - 2; PC = PC + space) {
		if (PC % 1000 == 0) {
			LogPrint("code = %x PC = %d\n", code, PC);
		}
		for (raddr = -128; raddr <= 127; raddr++) {
			cc = rand() % 2;
			expect = (cc) ? (PC + 2) : (raddr + PC + 2);
			if (expect > CVir8051::MAX_ROM || expect < 0) {
				continue;
			}
			updatecpu();
			TestRun();
			if (!checkresult()) {
				return errstring();
			}
		}
	}
	return "OK";
}

bool CTestMcu::IsOdd(unsigned char un) {
	byte ii = 0;
	byte cnt = 0;
	for (ii = 0; ii < 8; ii++) {
		if ((un >> ii) & BIT0) {
			cnt++;
		}
	}
	if (cnt % 2) {
		return true;
	} else {
		return false;
	}
}

string CTestMcu::PrintCMDUntested(void) {
	int kk = 0;
	for (int ii = 0; ii <= 0xff; ii++) {
		if (pCVir8051->m_ChipRamoper[ii] == 0) {
			kk++;
			if (kk % 5 == 0) {
				LogPrint("\r\n");
			}
			LogPrint("%x ", ii);
		}
	}
	return "OK";
}
void CTestMcu::TestRun() {
	pCVir8051->StepRun(pCVir8051->GetOpcode());
}
CTestMcu::CTestMcu(CVir8051 *pCVir8051) :
		pCVir8051(pCVir8051) {
}

CTestMcu::~CTestMcu() {

}

