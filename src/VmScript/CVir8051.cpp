// ComDirver.cpp: implementation of the CComDirver class.
//
//////////////////////////////////////////////////////////////////////

#include "CVir8051.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "hash.h"
#include "key.h"
#include <openssl/des.h>
#include <vector>
//#include "Typedef.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

void CVir8051::InitalReg() {

	memset(m_ChipRam, 0, sizeof(m_ChipRam));
	memset(m_ChipSfr, 0, sizeof(m_ChipSfr));
	memset(m_ExRam, 0, sizeof(m_ExRam));
	memset(m_ChipRamoper, 0, sizeof(m_ChipRamoper));

	Sys.a.SetAddr(a_addr);
	Sys.b.SetAddr(b_addr);
	Sys.dptr.SetAddr(dptrl);
	Sys.psw.SetAddr(psw_addr);
	Sys.sp.SetAddr(sp_addr);

	Rges.R0.SetAddr(0x00);
	Rges.R1.SetAddr(0x01);
	Rges.R2.SetAddr(0x02);
	Rges.R3.SetAddr(0x03);
	Rges.R4.SetAddr(0x04);
	Rges.R5.SetAddr(0x05);
	Rges.R6.SetAddr(0x06);
	Rges.R7.SetAddr(0x07);

	SetRamData(0x80, 0xFF);
	SetRamData(0x90, 0xFF);
	SetRamData(0xa0, 0xFF);
	SetRamData(0xb0, 0xFF);
	Sys.sp = 0x07;

}

CVir8051::CVir8051(const vector<unsigned char> & vRom, const vector<unsigned char> &InputData) :
		Sys(this), Rges(this) {

	InitalReg();
	INT16U addr = 0xFC00;

	memcpy(m_ExeFile, &vRom[0], vRom.size());
	SetExRamData(addr, InputData);
}

CVir8051::~CVir8051() {

}

typedef bool (*pFun)(unsigned char *);

struct __MapExterFun {
	INT16U method;
	pFun fun;
};

static unsigned short GetParaLen(unsigned char * &pbuf) {
	unsigned char tem[5];
	unsigned short ret = 0;
	memcpy(tem, pbuf, 5);
	if (tem[0] < 0xFD) {
		ret = tem[0];
		pbuf += 1;
	} else if (tem[0] == 0xFD) {
		memcpy(&ret, &tem[1], 2);
		pbuf += 3;
	} else {
		/// never come here
	}
	return ret;
}

static void GetParaData(unsigned char * &pbuf, unsigned char * &pdata, unsigned short datalen) {
	pdata = pbuf;
	pbuf += datalen;
}

static bool ExInt64CompFunc(unsigned char *ipara) {
//	printf("the call func:%s\r\n", __FUNCTION__);
	unsigned char *pbuf = ipara;
	unsigned short len = GetParaLen(pbuf);
//	printf("the total len:%d\r\n", len);
	unsigned short data1len = GetParaLen(pbuf);
	unsigned char *pdata1 = NULL;
	GetParaData(pbuf, pdata1, data1len);
	unsigned short data2len = GetParaLen(pbuf);
	unsigned char *pdata2 = NULL;
	GetParaData(pbuf, pdata2, data2len);
//	printf("len:%d the data1:%s\r\n", data1len, HexStr(pdata1, pdata1 + data1len, true).c_str());
//	printf("len:%d the data2:%s\r\n", data2len, HexStr(pdata2, pdata2 + data2len, true).c_str());
	int64_t m1, m2;
	unsigned char rslt;
	memcpy(&m1, pdata1, sizeof(m1));
	memcpy(&m2, pdata2, sizeof(m2));
//	printf("m1:%I64d\r\n", m1);
//	printf("m2:%I64d\r\n", m2);
	if (m1 > m2) {
		rslt = 2;
	} else if (m1 == m2) {
		rslt = 0;
	} else {
		rslt = 1;
	}

//	printf("rslt:%d\r\n", rslt);
	memset(ipara, 0, 512);
	ipara[0] = sizeof(rslt);
	memcpy(&ipara[1], &rslt, sizeof(rslt));

	return true;
}

static bool ExInt64MullFunc(unsigned char *ipara) {
//	printf("the call func:%s\r\n", __FUNCTION__);

	unsigned char *pbuf = ipara;
	unsigned short len = GetParaLen(pbuf);
//	printf("the total len:%d\r\n", len);
	unsigned short data1len = GetParaLen(pbuf);
	unsigned char *pdata1 = NULL;
	GetParaData(pbuf, pdata1, data1len);
	unsigned short data2len = GetParaLen(pbuf);
	unsigned char *pdata2 = NULL;
	GetParaData(pbuf, pdata2, data2len);
//	printf("len:%d the data1:%s\r\n", data1len, HexStr(pdata1, pdata1 + data1len, true).c_str());
//	printf("len:%d the data2:%s\r\n", data2len, HexStr(pdata2, pdata2 + data2len, true).c_str());
	int64_t m1, m2, m3;
	memcpy(&m1, pdata1, sizeof(m1));
	memcpy(&m2, pdata2, sizeof(m2));
//	printf("m1:%I64d\r\n", m1);
//	printf("m2:%I64d\r\n", m2);
	m3 = m1 * m2;
//	printf("m3:%I64d\r\n", m3);
	memset(ipara, 0, 512);
	ipara[0] = sizeof(m3);
	memcpy(&ipara[1], &m3, sizeof(m3));
	return true;
}

static bool ExInt64AddFunc(unsigned char *ipara) {
//	printf("the call func:%s\r\n", __FUNCTION__);
	unsigned char *pbuf = ipara;
	unsigned short len = GetParaLen(pbuf);
//	printf("the total len:%d\r\n", len);
	unsigned short data1len = GetParaLen(pbuf);
	unsigned char *pdata1 = NULL;
	GetParaData(pbuf, pdata1, data1len);
	unsigned short data2len = GetParaLen(pbuf);
	unsigned char *pdata2 = NULL;
	GetParaData(pbuf, pdata2, data2len);
//	printf("len:%d the data1:%s\r\n", data1len, HexStr(pdata1, pdata1 + data1len, true).c_str());
//	printf("len:%d the data2:%s\r\n", data2len, HexStr(pdata2, pdata2 + data2len, true).c_str());
	int64_t m1, m2, m3;
	memcpy(&m1, pdata1, sizeof(m1));
	memcpy(&m2, pdata2, sizeof(m2));
//	printf("m1:%I64d\r\n", m1);
//	printf("m2:%I64d\r\n", m2);
	m3 = m1 + m2;
//	printf("m3:%I64d\r\n", m3);
	memset(ipara, 0, 512);
	ipara[0] = sizeof(m3);
	memcpy(&ipara[1], &m3, sizeof(m3));
	return true;
}

static bool ExInt64SubFunc(unsigned char *ipara) {
//	printf("the call func:%s\r\n", __FUNCTION__);

	unsigned char *pbuf = ipara;
	unsigned short len = GetParaLen(pbuf);
//	printf("the total len:%d\r\n", len);
	unsigned short data1len = GetParaLen(pbuf);
	unsigned char *pdata1 = NULL;
	GetParaData(pbuf, pdata1, data1len);
	unsigned short data2len = GetParaLen(pbuf);
	unsigned char *pdata2 = NULL;
	GetParaData(pbuf, pdata2, data2len);
//	printf("len:%d the data1:%s\r\n", data1len, HexStr(pdata1, pdata1 + data1len, true).c_str());
//	printf("len:%d the data2:%s\r\n", data2len, HexStr(pdata2, pdata2 + data2len, true).c_str());
	int64_t m1, m2, m3;
	memcpy(&m1, pdata1, sizeof(m1));
	memcpy(&m2, pdata2, sizeof(m2));
//	printf("m1:%I64d\r\n", m1);
//	printf("m2:%I64d\r\n", m2);
	m3 = m1 - m2;
//	printf("m3:%I64d\r\n", m3);
	memset(ipara, 0, 512);
	ipara[0] = sizeof(m3);
	memcpy(&ipara[1], &m3, sizeof(m3));
	return true;
}

static bool ExInt64DivFunc(unsigned char *ipara) {
//	printf("the call func:%s\r\n", __FUNCTION__);

	unsigned char *pbuf = ipara;
	unsigned short len = GetParaLen(pbuf);
//	printf("the total len:%d\r\n", len);
	unsigned short data1len = GetParaLen(pbuf);
	unsigned char *pdata1 = NULL;
	GetParaData(pbuf, pdata1, data1len);
	unsigned short data2len = GetParaLen(pbuf);
	unsigned char *pdata2 = NULL;
	GetParaData(pbuf, pdata2, data2len);
//	printf("len:%d the data1:%s\r\n", data1len, HexStr(pdata1, pdata1 + data1len, true).c_str());
//	printf("len:%d the data2:%s\r\n", data2len, HexStr(pdata2, pdata2 + data2len, true).c_str());
	int64_t m1, m2, m3;
	memcpy(&m1, pdata1, sizeof(m1));
	memcpy(&m2, pdata2, sizeof(m2));
//	printf("m1:%I64d\r\n", m1);
//	printf("m2:%I64d\r\n", m2);
	m3 = m1 / m2;
//	printf("m3:%I64d\r\n", m3);
	memset(ipara, 0, 512);
	ipara[0] = sizeof(m3);
	memcpy(&ipara[1], &m3, sizeof(m3));
	return true;
}

static bool ExSha256Func(unsigned char *ipara) {
//	printf("the call func:%s\r\n", __FUNCTION__);
	unsigned char *pbuf = (unsigned char *) ipara;
	unsigned short len = GetParaLen(pbuf);
	unsigned char *pdata = NULL;
	GetParaData(pbuf, pdata, len);
	uint256 rslt = Hash(pdata, pdata + len);
//	printf("the in para:%s\r\n", HexStr(pbuf + 1, pbuf + 1 + len, true).c_str());
//	printf("the rslt:%s\r\n", rslt.ToString().c_str());
	memset(ipara, 0, 512);
	ipara[0] = 32;
	memcpy(ipara + 1, rslt.begin(), 32);
	return true;
}

static bool ExDesFunc(unsigned char *ipara) {
//	printf("the call func:%s\r\n", __FUNCTION__);

	unsigned char *pbuf = ipara;
	unsigned short len = GetParaLen(pbuf);
//	printf("the total len:%d\r\n", len);
	unsigned short datalen = GetParaLen(pbuf);
	unsigned char *pdata = NULL;
	GetParaData(pbuf, pdata, datalen);
	unsigned short keylen = GetParaLen(pbuf);
	unsigned char *pkey = NULL;
	GetParaData(pbuf, pkey, keylen);
	unsigned short flaglen = GetParaLen(pbuf);
	unsigned char *pflag = NULL;
	GetParaData(pbuf, pflag, flaglen);

//	printf("len:%d the data:%s\r\n", datalen, HexStr(pdata, pdata + datalen, true).c_str());
//	printf("len:%d the key:%s\r\n", keylen, HexStr(pkey, pkey + keylen, true).c_str());
//	printf("len:%d the flag:%s\r\n", flaglen, HexStr(pflag, pflag + flaglen, true).c_str());
	DES_key_schedule deskey1, deskey2, deskey3;

	vector<unsigned char> desdata;
	vector<unsigned char> desout;
	unsigned char datalen_rest = datalen % 8;
	desdata.assign(pdata, pdata + datalen);
	if (datalen_rest) {
		desdata.insert(desdata.end(), 8 - datalen_rest, 0);
	}

//	printf("the rest len:%d the full data:%s\r\n", datalen_rest, HexStr(desdata.begin(), desdata.end(), true).c_str());
	const_DES_cblock in;
	DES_cblock out, key;

	desout.resize(desdata.size());

	if (*pflag == 1) {
		if (keylen == 8) {
//			printf("the des encrypt\r\n");
			memcpy(key, pkey, keylen);
			DES_set_key_unchecked(&key, &deskey1);
			for (int ii = 0; ii < desdata.size() / 8; ii++) {
				memcpy(&in, &desdata[ii * 8], sizeof(in));
//				printf("in :%s\r\n", HexStr(in, in + 8, true).c_str());
				DES_ecb_encrypt(&in, &out, &deskey1, DES_ENCRYPT);
//				printf("out :%s\r\n", HexStr(out, out + 8, true).c_str());
				memcpy(&desout[ii * 8], &out, sizeof(out));
			}
		} else {
//			printf("the 3 des encrypt\r\n");
			memcpy(key, pkey, keylen);
			DES_set_key_unchecked(&key, &deskey1);
			DES_set_key_unchecked(&key, &deskey3);
			memcpy(key, pkey + 8, keylen);
			DES_set_key_unchecked(&key, &deskey2);
			for (int ii = 0; ii < desdata.size() / 8; ii++) {
				memcpy(&in, &desdata[ii * 8], sizeof(in));
				DES_ecb3_encrypt(&in, &out, &deskey1, &deskey2, &deskey3, DES_ENCRYPT);
				memcpy(&desout[ii * 8], &out, sizeof(out));
			}

		}
	} else {
		if (keylen == 8) {
//			printf("the des decrypt\r\n");
			memcpy(key, pkey, keylen);
			DES_set_key_unchecked(&key, &deskey1);
			for (int ii = 0; ii < desdata.size() / 8; ii++) {
				memcpy(&in, &desdata[ii * 8], sizeof(in));
//				printf("in :%s\r\n", HexStr(in, in + 8, true).c_str());
				DES_ecb_encrypt(&in, &out, &deskey1, DES_DECRYPT);
//				printf("out :%s\r\n", HexStr(out, out + 8, true).c_str());
				memcpy(&desout[ii * 8], &out, sizeof(out));
			}
		} else {
//			printf("the 3 des decrypt\r\n");
			memcpy(key, pkey, keylen);
			DES_set_key_unchecked(&key, &deskey1);
			DES_set_key_unchecked(&key, &deskey3);
			memcpy(key, pkey + 8, keylen);
			DES_set_key_unchecked(&key, &deskey2);
			for (int ii = 0; ii < desdata.size() / 8; ii++) {
				memcpy(&in, &desdata[ii * 8], sizeof(in));
				DES_ecb3_encrypt(&in, &out, &deskey1, &deskey2, &deskey3, DES_DECRYPT);
				memcpy(&desout[ii * 8], &out, sizeof(out));
			}
		}
	}

//	printf("***rslt len:%d the rslt:%s\r\n", desout.size(), HexStr(desout.begin(), desout.end(), true).c_str());
	memset(ipara, 0, 512);
	ipara[0] = desout.size();
	memcpy(&ipara[1], &desout[0], desout.size());
	return true;
}

static bool ExVerifySignatureFunc(unsigned char *ipara) {
//	printf("the call func:%s\r\n", __FUNCTION__);
	unsigned char *pbuf = ipara;
	unsigned short len = GetParaLen(pbuf);
//	printf("the total len:%d\r\n", len);
	unsigned short datalen = GetParaLen(pbuf);
	unsigned char *pdata = NULL;
	GetParaData(pbuf, pdata, datalen);
	unsigned short keylen = GetParaLen(pbuf);
	unsigned char *pkey = NULL;
	GetParaData(pbuf, pkey, keylen);
	unsigned short hashlen = GetParaLen(pbuf);
	unsigned char *phash = NULL;
	GetParaData(pbuf, phash, hashlen);
//	printf("len:%d the data:%s\r\n", datalen, HexStr(pdata, pdata + datalen, true).c_str());
//	printf("len:%d the key:%s\r\n", keylen, HexStr(pkey, pkey + keylen, true).c_str());
//	printf("len:%d the hash:%s\r\n", hashlen, HexStr(phash, phash + hashlen, true).c_str());

	CPubKey pk(pkey, pkey + keylen);
	vector<unsigned char> sig(pdata, pdata + datalen);
	uint256 hash(vector<unsigned char>(phash, phash + hashlen));

//	printf("pk:%s\r\n", HexStr(pk.begin(), pk.end(), true).c_str());
//	printf("sig:%s\r\n", HexStr(sig.begin(), sig.end(), true).c_str());
//	printf("hash:%s\r\n", HexStr(hash.begin(), hash.end(), true).c_str());
	bool rlt;
//	if (pk.Verify(hash, sig)) {
//		memset(ipara, 0, 512);
//		ipara[0] = 32;
//		memcpy(ipara, hash.begin(), 32);
//		return true;
//	} else {
//		memset(ipara, 0, 512);
//	}
	if (pk.Verify(hash, sig)) {
//		printf("verify ok!\r\n");
		rlt = true;
	} else {
//		printf("verify Error!\r\n");
		rlt = false;
	}

	memset(ipara, 0, 512);
	ipara[0] = sizeof(rlt);
	memcpy(&ipara[1], &rlt, sizeof(rlt));
	return true;
}

static bool ExSignatureFunc(unsigned char *ipara) {
//	printf("the call func:%s\r\n", __FUNCTION__);
	return true;
}

static bool ExLogPrintFunc(unsigned char *ipara) {
	unsigned char *pbuf = ipara;
	unsigned short len = GetParaLen(pbuf);
	unsigned short infolen = GetParaLen(pbuf);
	unsigned char *pinfo = NULL;
	GetParaData(pbuf, pinfo, infolen);
	if (infolen > 1) {
		pinfo[infolen - 1] = '\0';
	}
	unsigned short datalen = GetParaLen(pbuf);
	unsigned char *pdata = NULL;
	GetParaData(pbuf, pdata, datalen);

	printf("%s%s\r\n", pinfo, HexStr(pdata, pdata + datalen, true).c_str());

	return true;
}

const static struct __MapExterFun FunMap[] = { { 0, ExInt64CompFunc },			//
		{ 1, ExInt64MullFunc },			//
		{ 2, ExInt64AddFunc },			//
		{ 3, ExInt64SubFunc },			//
		{ 4, ExInt64DivFunc },			//
		{ 5, ExSha256Func },			//
		{ 6, ExDesFunc },			    //
		{ 7, ExVerifySignatureFunc },   //
		{ 8, ExSignatureFunc },			//
		{ 9, ExLogPrintFunc },			//
		};

bool CallExternalFunc(INT16U method, unsigned char *ipara) {
	return FunMap[method].fun(ipara);
}

int CVir8051::run(int maxstep) {
	INT8U code = 0;
	int step = 0;

	while (1) {
		code = GetOpcode();
		StepRun(code);
		step++;
		//call func out of 8051
		if (Sys.PC == 0x0012) {
			//get what func will be called
			INT16U method = ((INT16U) GetExRam(0xFBFE) | ((INT16U) GetExRam(0xFBFF) << 8));
			unsigned char *ipara = (unsigned char *) GetExRamAddr(0xF7FE);		//input para
			CallExternalFunc(method, ipara);
		} else if (Sys.PC == 0x0008) {
			return step;		//return total step
		}
		if (maxstep != 0 && step > maxstep) {
			return 0;		//force return
		}
	}

	return 1;
}

bool CVir8051::run() {
	INT8U code = 0;
	INT16U flag;
	while (1) {
		code = GetOpcode();
		StepRun(code);
		UpDataDebugInfo();

		//call func out of 8051
		if (Sys.PC == 0x0012) {
			//get what func will be called
			INT16U method = ((INT16U) GetExRam(0xFBFE) | ((INT16U) GetExRam(0xFBFF) << 8));
			flag = method;
			unsigned char *ipara = (unsigned char *) GetExRamAddr(0xF7FE);		//input para
			CallExternalFunc(method, ipara);
		}
		if (Sys.PC == 0x0007) {

		}
		if (Sys.PC == 0x0008) {
			{
				INT8U result = GetExRam(0xFBFF);
				if (result == 0x00) {
					return 0;
				} else if (result == 0x08) {
					unsigned char *pcheck = (unsigned char *) GetExRamAddr(0xF000);		//check data
					INT8U len = pcheck[0];
					switch (flag) {
					case 0: {
						if (pcheck[1] == 2) {
							return true;
						} else {
							return false;
						}
					}
						break;
					case 1: {
						int64_t rslt;
						memcpy(&rslt, &pcheck[1], len);
						if (rslt == 285916242777615) {
							return true;
						} else {
							return false;
						}
					}
						break;
					case 2: {
						int64_t rslt;
						memcpy(&rslt, &pcheck[1], len);
						if (rslt == 4328785416) {
							return true;
						} else {
							return false;
						}
					}

						break;
					case 3: {
						int64_t rslt;
						memcpy(&rslt, &pcheck[1], len);
						if (rslt == 4328653314) {
							return true;
						} else {
							return false;
						}
					}

						break;
					case 4: {
						int64_t rslt;
						memcpy(&rslt, &pcheck[1], len);
						if (rslt == 65536) {
							return true;
						} else {
							return false;
						}
					}

						break;
					case 5: {
						char xx[] = { 0, 1, 2, 3, 4, 5, 6, 7 };
						uint256 expect = Hash(xx, xx + sizeof(xx));
						uint256 rslt;
						memcpy(rslt.begin(), &pcheck[1], len);
						if (expect == rslt) {
							return true;
						} else {
							return false;
						}
					}
						break;
					case 6: {
						char xx[] = { 0x17, 0x26, 0xc7, 0x5f, 0x28, 0x16, 0x71, 0x5f, 0xde, 0x89, 0x62, 0x08, 0x43,
								0x34, 0x39, 0xa7 };
						if (!memcmp(xx, &pcheck[1], len)) {
							return true;
						} else {
							return false;
						}
					}
						break;
					case 7: {
						if (pcheck[1] == true) {
							return true;
						} else {
							return false;
						}
					}
						break;
					case 8: {
						assert(0);
					}
						break;
					default: {
						assert(0);
					}
						break;
					}

				} else {
					return 1;
				}
			}
		}
	}
	return 1;
}
void CVir8051::SetExRamData(INT16U addr, const vector<unsigned char> data) {
	memcpy(&m_ExRam[addr], &data[0], data.size());
}
void CVir8051::StepRun(INT8U code) {
//		INT8U tempa = Sys.a();
	m_ChipRamoper[code] = 1;
//	char temp[1024];
//	sprintf(temp, "pc--->%x,code:%x \r\n", Sys.PC, code);
//	cout << "before" << temp << endl;
	switch (code) {
	case 0x00: {
		Opcode_00_NOP();
		break;
	}
	case 0x01: {
		Opcode_01_AJMP_Addr11();
		break;
	}
	case 0x02: {
		Opcode_02_LJMP_Addr16();
		break;
	}
	case 0x03: {
		Opcode_03_RR_A();
		break;
	}
	case 0x04: {
		Opcode_04_INC_A();
		break;
	}
	case 0x05: {
		Opcode_05_INC_Direct();
		break;
	}
	case 0x06: {
		Opcode_06_INC_R0_1();
		break;
	}
	case 0x07: {
		Opcode_07_INC_R1_1();
		break;
	}
	case 0x08: {
		Opcode_08_INC_R0();
		break;
	}
	case 0x09: {
		Opcode_09_INC_R1();
		break;
	}
	case 0x0A: {
		Opcode_0A_INC_R2();
		break;
	}
	case 0x0B: {
		Opcode_0B_INC_R3();
		break;
	}
	case 0x0C: {
		Opcode_0C_INC_R4();
		break;
	}
	case 0x0D: {
		Opcode_0D_INC_R5();
		break;
	}
	case 0x0E: {
		Opcode_0E_INC_R6();
		break;
	}
	case 0x0F: {
		Opcode_0F_INC_R7();
		break;
	}
	case 0x10: {
		Opcode_10_JBC_Bit_Rel();
		break;
	}
	case 0x11: {
		Opcode_11_ACALL_Addr11();
		break;
	}
	case 0x12: {
		Opcode_12_LCALL_Addr16();
		break;
	}
	case 0x13: {
		Opcode_13_RRC_A();
		break;
	}
	case 0x14: {
		Opcode_14_DEC_A();
		break;
	}
	case 0x15: {
		Opcode_15_DEC_Direct();
		break;
	}
	case 0x16: {
		Opcode_16_DEC_R0_1();
		break;
	}
	case 0x17: {
		Opcode_17_DEC_R1_1();
		break;
	}
	case 0x18: {
		Opcode_18_DEC_R0();
		break;
	}
	case 0x19: {
		Opcode_19_DEC_R1();
		break;
	}
	case 0x1A: {
		Opcode_1A_DEC_R2();
		break;
	}
	case 0x1B: {
		Opcode_1B_DEC_R3();
		break;
	}
	case 0x1C: {
		Opcode_1C_DEC_R4();
		break;
	}
	case 0x1D: {
		Opcode_1D_DEC_R5();
		break;
	}
	case 0x1E: {
		Opcode_1E_DEC_R6();
		break;
	}
	case 0x1F: {
		Opcode_1F_DEC_R7();
		break;
	}
	case 0x20: {
		Opcode_20_JB_Bit_Rel();
		break;
	}
	case 0x21: {
		Opcode_21_AJMP_Addr11();
		break;
	}
	case 0x22: {
		Opcode_22_RET();
		break;
	}
	case 0x23: {
		Opcode_23_RL_A();
		break;
	}
	case 0x24: {
		Opcode_24_ADD_A_Data();
		break;
	}
	case 0x25: {
		Opcode_25_ADD_A_Direct();
		break;
	}
	case 0x26: {
		Opcode_26_ADD_A_R0_1();
		break;
	}
	case 0x27: {
		Opcode_27_ADD_A_R1_1();
		break;
	}
	case 0x28: {
		Opcode_28_ADD_A_R0();
		break;
	}
	case 0x29: {
		Opcode_29_ADD_A_R1();
		break;
	}
	case 0x2A: {
		Opcode_2A_ADD_A_R2();
		break;
	}
	case 0x2B: {
		Opcode_2B_ADD_A_R3();
		break;
	}
	case 0x2C: {
		Opcode_2C_ADD_A_R4();
		break;
	}
	case 0x2D: {
		Opcode_2D_ADD_A_R5();
		break;
	}
	case 0x2E: {
		Opcode_2E_ADD_A_R6();
		break;
	}
	case 0x2F: {
		Opcode_2F_ADD_A_R7();
		break;
	}
	case 0x30: {
		Opcode_30_JNB_Bit_Rel();
		break;
	}
	case 0x31: {
		Opcode_31_ACALL_Addr11();
		break;
	}
	case 0x32: {
		Opcode_32_RETI();
		break;
	}
		/*下面已经测试PASS*/
	case 0x33: {
		Opcode_33_RLC_A();
		break;
	}
	case 0x34: {
		Opcode_34_ADDC_A_Data();
		break;
	}
	case 0x35: {
		Opcode_35_ADDC_A_Direct();
		break;
	}
	case 0x36: {
		Opcode_36_ADDC_A_R0_1();
		break;
	}
	case 0x37: {
		Opcode_37_ADDC_A_R1_1();
		break;
	}
	case 0x38: {
		Opcode_38_ADDC_A_R0();
		break;
	}
	case 0x39: {
		Opcode_39_ADDC_A_R1();
		break;
	}
	case 0x3A: {
		Opcode_3A_ADDC_A_R2();
		break;
	}
	case 0x3B: {
		Opcode_3B_ADDC_A_R3();
		break;
	}
	case 0x3C: {
		Opcode_3C_ADDC_A_R4();
		break;
	}
	case 0x3D: {
		Opcode_3D_ADDC_A_R5();
		break;
	}
	case 0x3E: {
		Opcode_3E_ADDC_A_R6();
		break;
	}
	case 0x3F: {
		Opcode_3F_ADDC_A_R7();
		break;
	}
		/*上面已经测试PASS*/
	case 0x40: {
		Opcode_40_JC_Rel();
		break;
	}
	case 0x41: {
		Opcode_41_AJMP_Addr11();
		break;
	}
	case 0x42: {
		Opcode_42_ORL_Direct_A();
		break;
	}
	case 0x43: {
		Opcode_43_ORL_Direct_Data();
		break;
	}
	case 0x44: {
		Opcode_44_ORL_A_Data();
		break;
	}
	case 0x45: {
		Opcode_45_ORL_A_Direct();
		break;
	}
	case 0x46: {
		Opcode_46_ORL_A_R0_1();
		break;
	}
	case 0x47: {
		Opcode_47_ORL_A_R1_1();
		break;
	}
	case 0x48: {
		Opcode_48_ORL_A_R0();
		break;
	}
	case 0x49: {
		Opcode_49_ORL_A_R1();
		break;
	}
	case 0x4A: {
		Opcode_4A_ORL_A_R2();
		break;
	}
	case 0x4B: {
		Opcode_4B_ORL_A_R3();
		break;
	}
	case 0x4C: {
		Opcode_4C_ORL_A_R4();
		break;
	}
	case 0x4D: {
		Opcode_4D_ORL_A_R5();
		break;
	}
	case 0x4E: {
		Opcode_4E_ORL_A_R6();
		break;
	}
	case 0x4F: {
		Opcode_4F_ORL_A_R7();
		break;
	}
	case 0x50: {
		Opcode_50_JNC_Rel();
		break;
	}
	case 0x51: {
		Opcode_51_ACALL_Addr11();
		break;
	}
	case 0x52: {
		Opcode_52_ANL_Direct_A();
		break;
	}
	case 0x53: {
		Opcode_53_ANL_Direct_Data();
		break;
	}
	case 0x54: {
		Opcode_54_ANL_A_Data();
		break;
	}
	case 0x55: {
		Opcode_55_ANL_A_Direct();
		break;
	}
	case 0x56: {
		Opcode_56_ANL_A_R0_1();
		break;
	}
	case 0x57: {
		Opcode_57_ANL_A_R1_1();
		break;
	}
	case 0x58: {
		Opcode_58_ANL_A_R0();
		break;
	}
	case 0x59: {
		Opcode_59_ANL_A_R1();
		break;
	}
	case 0x5A: {
		Opcode_5A_ANL_A_R2();
		break;
	}
	case 0x5B: {
		Opcode_5B_ANL_A_R3();
		break;
	}
	case 0x5C: {
		Opcode_5C_ANL_A_R4();
		break;
	}
	case 0x5D: {
		Opcode_5D_ANL_A_R5();
		break;
	}
	case 0x5E: {
		Opcode_5E_ANL_A_R6();
		break;
	}
	case 0x5F: {
		Opcode_5F_ANL_A_R7();
		break;
	}
	case 0x60: {
		Opcode_60_JZ_Rel();
		break;
	}
	case 0x61: {
		Opcode_61_AJMP_Addr11();
		break;
	}
	case 0x62: {
		Opcode_62_XRL_Direct_A();
		break;
	}
	case 0x63: {
		Opcode_63_XRL_Direct_Data();
		break;
	}
	case 0x64: {
		Opcode_64_XRL_A_Data();
		break;
	}
	case 0x65: {
		Opcode_65_XRL_A_Direct();
		break;
	}
	case 0x66: {
		Opcode_66_XRL_A_R0_1();
		break;
	}
	case 0x67: {
		Opcode_67_XRL_A_R1_1();
		break;
	}
	case 0x68: {
		Opcode_68_XRL_A_R0();
		break;
	}
	case 0x69: {
		Opcode_69_XRL_A_R1();
		break;
	}
	case 0x6A: {
		Opcode_6A_XRL_A_R2();
		break;
	}
	case 0x6B: {
		Opcode_6B_XRL_A_R3();
		break;
	}
	case 0x6C: {
		Opcode_6C_XRL_A_R4();
		break;
	}
	case 0x6D: {
		Opcode_6D_XRL_A_R5();
		break;
	}
	case 0x6E: {
		Opcode_6E_XRL_A_R6();
		break;
	}
	case 0x6F: {
		Opcode_6F_XRL_A_R7();
		break;
	}
	case 0x70: {
		Opcode_70_JNZ_Rel();
		break;
	}
	case 0x71: {
		Opcode_71_ACALL_Addr11();
		break;
	}
	case 0x72: {
		Opcode_72_ORL_C_Direct();
		break;
	}
	case 0x73: {
		Opcode_73_JMP_A_DPTR();
		break;
	}
	case 0x74: {
		Opcode_74_MOV_A_Data();
		break;
	}
	case 0x75: {
		Opcode_75_MOV_Direct_Data();
		break;
	}
	case 0x76: {
		Opcode_76_MOV_R0_1_Data();
		break;
	}
	case 0x77: {
		Opcode_77_MOV_R1_1_Data();
		break;
	}
	case 0x78: {
		Opcode_78_MOV_R0_Data();
		break;
	}
	case 0x79: {
		Opcode_79_MOV_R1_Data();
		break;
	}
	case 0x7A: {
		Opcode_7A_MOV_R2_Data();
		break;
	}
	case 0x7B: {
		Opcode_7B_MOV_R3_Data();
		break;
	}
	case 0x7C: {
		Opcode_7C_MOV_R4_Data();
		break;
	}
	case 0x7D: {
		Opcode_7D_MOV_R5_Data();
		break;
	}
	case 0x7E: {
		Opcode_7E_MOV_R6_Data();
		break;
	}
	case 0x7F: {
		Opcode_7F_MOV_R7_Data();
		break;
	}
	case 0x80: {
		Opcode_80_SJMP_Rel();
		break;
	}
	case 0x81: {
		Opcode_81_AJMP_Addr11();
		break;
	}
	case 0x82: {
		Opcode_82_ANL_C_Bit();
		break;
	}
	case 0x83: {
		Opcode_83_MOVC_A_PC();
		break;
	}
	case 0x84: {
		Opcode_84_DIV_AB();
		break;
	}
	case 0x85: {
		Opcode_85_MOV_Direct_Direct();
		break;
	}
	case 0x86: {
		Opcode_86_MOV_Direct_R0_1();
		break;
	}
	case 0x87: {
		Opcode_87_MOV_Direct_R1_1();
		break;
	}
	case 0x88: {
		Opcode_88_MOV_Direct_R0();
		break;
	}
	case 0x89: {
		Opcode_89_MOV_Direct_R1();
		break;
	}
	case 0x8A: {
		Opcode_8A_MOV_Direct_R2();
		break;
	}
	case 0x8B: {
		Opcode_8B_MOV_Direct_R3();
		break;
	}
	case 0x8C: {
		Opcode_8C_MOV_Direct_R4();
		break;
	}
	case 0x8D: {
		Opcode_8D_MOV_Direct_R5();
		break;
	}
	case 0x8E: {
		Opcode_8E_MOV_Direct_R6();
		break;
	}
	case 0x8F: {
		Opcode_8F_MOV_Direct_R7();
		break;
	}
	case 0x90: {
		Opcode_90_MOV_DPTR_Data();
		break;
	}
	case 0x91: {
		Opcode_91_ACALL_Addr11();
		break;
	}
	case 0x92: {
		Opcode_92_MOV_Bit_C();
		break;
	}
	case 0x93: {
		Opcode_93_MOVC_A_DPTR();
		break;
	}
		/*下面已经测试PASS*/
	case 0x94: {
		Opcode_94_SUBB_A_Data();
		break;
	}
	case 0x95: {
		Opcode_95_SUBB_A_Direct();
		break;
	}
	case 0x96: {
		Opcode_96_SUBB_A_R0_1();
		break;
	}
	case 0x97: {
		Opcode_97_SUBB_A_R1_1();
		break;
	}
	case 0x98: {
		Opcode_98_SUBB_A_R0();
		break;
	}
	case 0x99: {
		Opcode_99_SUBB_A_R1();
		break;
	}
	case 0x9A: {
		Opcode_9A_SUBB_A_R2();
		break;
	}
	case 0x9B: {
		Opcode_9B_SUBB_A_R3();
		break;
	}
	case 0x9C: {
		Opcode_9C_SUBB_A_R4();
		break;
	}
	case 0x9D: {
		Opcode_9D_SUBB_A_R5();
		break;
	}
	case 0x9E: {
		Opcode_9E_SUBB_A_R6();
		break;
	}
	case 0x9F: {
		Opcode_9F_SUBB_A_R7();
		break;
	}
	case 0xA0: {
		Opcode_A0_ORL_C_Bit();
		break;
	}
	case 0xA1: {
		Opcode_A1_AJMP_Addr11();
		break;
	}
	case 0xA2: {
		Opcode_A2_MOV_C_Bit();
		break;
	}
	case 0xA3: {
		Opcode_A3_INC_DPTR();
		break;
	}
	case 0xA4: {
		Opcode_A4_MUL_AB();
		break;
	}
	case 0xA5: {
		Opcode_A5();
		break;
	}
	case 0xA6: {
		Opcode_A6_MOV_R0_1_Direct();
		break;
	}
	case 0xA7: {
		Opcode_A7_MOV_R1_1_Direct();
		break;
	}
	case 0xA8: {
		Opcode_A8_MOV_R0_Direct();
		break;
	}
	case 0xA9: {
		Opcode_A9_MOV_R1_Direct();
		break;
	}
	case 0xAA: {
		Opcode_AA_MOV_R2_Direct();
		break;
	}
	case 0xAB: {
		Opcode_AB_MOV_R3_Direct();
		break;
	}
	case 0xAC: {
		Opcode_AC_MOV_R4_Direct();
		break;
	}
	case 0xAD: {
		Opcode_AD_MOV_R5_Direct();
		break;
	}
	case 0xAE: {
		Opcode_AE_MOV_R6_Direct();
		break;
	}
	case 0xAF: {
		Opcode_AF_MOV_R7_Direct();
		break;
	}
	case 0xB0: {
		Opcode_B0_ANL_C_Bit_1();
		break;
	}
	case 0xB1: {
		Opcode_B1_ACALL_Addr11();
		break;
	}
	case 0xB2: {
		Opcode_B2_CPL_Bit();
		break;
	}
	case 0xB3: {
		Opcode_B3_CPL_C();
		break;
	}
	case 0xB4: {
		Opcode_B4_CJNE_A_Data_Rel();
		break;
	}
	case 0xB5: {
		Opcode_B5_CJNE_A_Direct_Rel();
		break;
	}
	case 0xB6: {
		Opcode_B6_CJNE_R0_1_Data_Rel();
		break;
	}
	case 0xB7: {
		Opcode_B7_CJNE_R1_1_Data_Rel();
		break;
	}
	case 0xB8: {
		Opcode_B8_CJNE_R0_Data_Rel();
		break;
	}
	case 0xB9: {
		Opcode_B9_CJNE_R1_Data_Rel();
		break;
	}
	case 0xBA: {
		Opcode_BA_CJNE_R2_Data_Rel();
		break;
	}
	case 0xBB: {
		Opcode_BB_CJNE_R3_Data_Rel();
		break;
	}
	case 0xBC: {
		Opcode_BC_CJNE_R4_Data_Rel();
		break;
	}
	case 0xBD: {
		Opcode_BD_CJNE_R5_Data_Rel();
		break;
	}
	case 0xBE: {
		Opcode_BE_CJNE_R6_Data_Rel();
		break;
	}
	case 0xBF: {
		Opcode_BF_CJNE_R7_Data_Rel();
		break;
	}
	case 0xC0: {
		Opcode_C0_PUSH_Direct();
		break;
	}
	case 0xC1: {
		Opcode_C1_AJMP_Addr11();
		break;
	}
	case 0xC2: {
		Opcode_C2_CLR_Bit();
		break;
	}
	case 0xC3: {
		Opcode_C3_CLR_C();
		break;
	}
	case 0xC4: {
		Opcode_C4_SWAP_A();
		break;
	}
	case 0xC5: {
		Opcode_C5_XCH_A_Direct();
		break;
	}
	case 0xC6: {
		Opcode_C6_XCH_A_R0_1();
		break;
	}
	case 0xC7: {
		Opcode_C7_XCH_A_R1_1();
		break;
	}
	case 0xC8: {
		Opcode_C8_XCH_A_R0();
		break;
	}
	case 0xC9: {
		Opcode_C9_XCH_A_R1();
		break;
	}
	case 0xCA: {
		Opcode_CA_XCH_A_R2();
		break;
	}
	case 0xCB: {
		Opcode_CB_XCH_A_R3();
		break;
	}
	case 0xCC: {
		Opcode_CC_XCH_A_R4();
		break;
	}
	case 0xCD: {
		Opcode_CD_XCH_A_R5();
		break;
	}
	case 0xCE: {
		Opcode_CE_XCH_A_R6();
		break;
	}
	case 0xCF: {
		Opcode_CF_XCH_A_R7();
		break;
	}
	case 0xD0: {
		Opcode_D0_POP_Direct();
		break;
	}
	case 0xD1: {
		Opcode_D1_ACALL_Addr11();
		break;
	}
	case 0xD2: {
		Opcode_D2_SETB_Bit();
		break;
	}
	case 0xD3: {
		Opcode_D3_SETB_C();
		break;
	}
	case 0xD4: {
		Opcode_D4_DA_A();
		break;
	}
	case 0xD5: {
		Opcode_D5_DJNZ_Direct_Rel();
		break;
	}
	case 0xD6: {
		Opcode_D6_XCHD_A_R0_1();
		break;
	}
	case 0xD7: {
		Opcode_D7_XCHD_A_R1_1();
		break;
	}
	case 0xD8: {
		Opcode_D8_DJNZ_R0_Rel();
		break;
	}
	case 0xD9: {
		Opcode_D9_DJNZ_R1_Rel();
		break;
	}
	case 0xDA: {
		Opcode_DA_DJNZ_R2_Rel();
		break;
	}
	case 0xDB: {
		Opcode_DB_DJNZ_R3_Rel();
		break;
	}
	case 0xDC: {
		Opcode_DC_DJNZ_R4_Rel();
		break;
	}
	case 0xDD: {
		Opcode_DD_DJNZ_R5_Rel();
		break;
	}
	case 0xDE: {
		Opcode_DE_DJNZ_R6_Rel();
		break;
	}
	case 0xDF: {
		Opcode_DF_DJNZ_R7_Rel();
		break;
	}
	case 0xE0: {
		Opcode_E0_MOVX_A_DPTR();
		break;
	}
	case 0xE1: {
		Opcode_E1_AJMP_Addr11();
		break;
	}
	case 0xE2: {
		Opcode_E2_MOVX_A_R0_1();
		break;
	}
	case 0xE3: {
		Opcode_E3_MOVX_A_R1_1();
		break;
	}
	case 0xE4: {
		Opcode_E4_CLR_A();
		break;
	}
	case 0xE5: {
		Opcode_E5_MOV_A_Direct();
		break;
	}
	case 0xE6: {
		Opcode_E6_MOV_A_R0_1();
		break;
	}
	case 0xE7: {
		Opcode_E7_MOV_A_R1_1();
		break;
	}
	case 0xE8: {
		Opcode_E8_MOV_A_R0();
		break;
	}
	case 0xE9: {
		Opcode_E9_MOV_A_R1();
		break;
	}
	case 0xEA: {
		Opcode_EA_MOV_A_R2();
		break;
	}
	case 0xEB: {
		Opcode_EB_MOV_A_R3();
		break;
	}
	case 0xEC: {
		Opcode_EC_MOV_A_R4();
		break;
	}
	case 0xED: {
		Opcode_ED_MOV_A_R5();
		break;
	}
	case 0xEE: {
		Opcode_EE_MOV_A_R6();
		break;
	}
	case 0xEF: {
		Opcode_EF_MOV_A_R7();
		break;
	}
	case 0xF0: {
		Opcode_F0_MOVX_DPTR_A();
		break;
	}
	case 0xF1: {
		Opcode_F1_ACALL_Addr11();
		break;
	}
	case 0xF2: {
		Opcode_F2_MOVX_R0_1_A();
		break;
	}
	case 0xF3: {
		Opcode_F3_MOVX_R1_1_A();
		break;
	}
	case 0xF4: {
		Opcode_F4_CPL_A();
		break;
	}
	case 0xF5: {
		Opcode_F5_MOV_Direct_A();
		break;
	}
	case 0xF6: {
		Opcode_F6_MOV_R0_1_A();
		break;
	}
	case 0xF7: {
		Opcode_F7_MOV_R1_1_A();
		break;
	}
	case 0xF8: {
		Opcode_F8_MOV_R0_A();
		break;
	}
	case 0xF9: {
		Opcode_F9_MOV_R1_A();
		break;
	}
	case 0xFA: {
		Opcode_FA_MOV_R2_A();
		break;
	}
	case 0xFB: {
		Opcode_FB_MOV_R3_A();
		break;
	}
	case 0xFC: {
		Opcode_FC_MOV_R4_A();
		break;
	}
	case 0xFD: {
		Opcode_FD_MOV_R5_A();
		break;
	}
	case 0xFE: {
		Opcode_FE_MOV_R6_A();
		break;
	}
	case 0xFF: {
		Opcode_FF_MOV_R7_A();
		break;
	}
	default:
		assert(0);
		break;
	}
//		if (tempa != Sys.a())
//				{
//			UpDataPFlag();
//		}
//	UpDataDebugInfo();
}
//static INT8U getAddr(INT8U addr) {
//	if (addr <= 0x7F) {
//		return (addr / 8) + 0x20;
//	} else {
//		return addr - ((addr - 0x80) % 8);
//	}
//	assert(0);
//	return addr;
//}

bool CVir8051::GetBitFlag(INT8U addr) {
	return (GetBitRamRef(addr) & (BIT0 << (addr % 8))) != 0;
}

//bool CVir8051::GetBitFlag(INT8U addr, INT8U n) const { // addr单元地址，n是对应的第几位
//	INT8U temp;
//	Assert(addr <= 0xF0);
//	// memcpy(&temp,GetPointRamAddr((addr/8)+0x20),sizeof(temp));
//	// return (((BIT0<<(addr%8))& temp) != 0);
//	memcpy(&temp, GetPointRamAddr(addr), sizeof(temp));
//	return (((BIT0 << n) & temp) != 0);
//}
void CVir8051::SetBitFlag(INT8U addr) {

//	INT8U tempaddr = getAddr(addr);
	GetBitRamRef(addr) |= (BIT0 << (addr % 8));
	if (addr >= 0xe0 && addr <= 0xe7) { //this opcode A
		Updata_A_P_Flag();
	}
}

void CVir8051::ClrBitFlag(INT8U addr) {
//	Assert(addr <= 0xF0);
//	INT8U temp;
//	//void  *p = GetPointRamAddr((addr/8)+0x20);
//	void *p = GetPointRamAddr(addr);
//	memcpy(&temp, p, sizeof(temp));
//	//temp &=(~(BIT0<<(addr%8)));
//	temp &= (~(BIT0 << n));
//	memcpy(p, &temp, sizeof(temp));
//	INT8U tempaddr = getAddr(addr);

	GetBitRamRef(addr) &= (~(BIT0 << (addr % 8)));
	if (addr >= 0xe0 && addr <= 0xe7) //this opcode A
			{
		Updata_A_P_Flag();
	}
//
//
//INT8U tempaddr = 0;
//if (addr <= 0x7F) {
//	tempaddr = (addr / 8) + 0x20;
//	m_ChipRam[tempaddr] &= (~(BIT0 << (addr % 8)));
//} else {
//	addr -= 0x80;
//	tempaddr = (addr / 8);
//	m_ChipSfr[tempaddr] &= (~(BIT0 << (addr % 8)));
//}
}

INT8U CVir8051::GetOpcode(void) const {
	return m_ExeFile[Sys.PC];
}

INT8U& CVir8051::GetRamRef(INT8U addr) {

	static const INT8U sfrarry[] = { a_addr, b_addr, dptrl, dptrh, psw_addr, sp_addr };
//	if (find(sfrarry, sfrarry + sizeof(sfrarry), addr) != sfrarry + sizeof(sfrarry)) {
	if (addr > 0x7F) {
		return m_ChipSfr[addr];
	} else {
		return m_ChipRam[addr];
	}
}

INT16U CVir8051::GetDebugOpcode(void) const {
	INT16U temp = 0;
	memcpy(&temp, &m_ExeFile[Sys.PC], 2);
	return temp;
}

bool CVir8051::GetDebugPC(INT16U pc) const {
	return (pc == Sys.PC);
}

void CVir8051::GetOpcodeData(void * const p, INT8U len) const {  //先取高位字节
	memcpy(p, &m_ExeFile[Sys.PC + 1], len);
}
void CVir8051::AJMP(INT8U opCode, INT8U data) {
	INT16U tmppc = Sys.PC + 2;
// tmppc  |= ((((((INT16U)opCode)<<5)&(0x7)))|((INT16U)data));
	tmppc &= 0xF800;
	tmppc |= ((((((INT16U) opCode) >> 5) & 0x7) << 8) | ((INT16U) data));

	Sys.PC = tmppc;
}
void CVir8051::ACALL(INT8U opCode) {
	INT8U data = 0;
	GetOpcodeData(&data, sizeof(data));

	Sys.PC += 2;
	Sys.sp = Sys.sp() + 1;
//	p = GetPointRamAddr(Sys.sp());
//	temp = (INT8U) (Sys.PC & 0x00ff);
//	memcpy(p, &temp, sizeof(temp));
//	m_ChipRam[Sys.sp()]=(INT8U) (Sys.PC & 0x00ff);
	SetRamDataAt(Sys.sp(), (INT8U) (Sys.PC & 0x00ff));

	Sys.sp = Sys.sp() + 1;
//	p = GetPointRamAddr(Sys.sp());
//	temp = (INT8U) (Sys.PC >> 8);
//	memcpy(p, &temp, sizeof(temp));
//	SetRamData(Sys.sp(), (INT8U) (INT8U) (Sys.PC >> 8));
	SetRamDataAt(Sys.sp(), (INT8U) (Sys.PC >> 8));
//	temp = (INT8U)(data>>8);
//	data = (data<<8)|temp;
	Sys.PC = ((Sys.PC & 0xF800) | (((((INT16U) opCode) >> 5) << 8) | (INT16U) data));
}

void CVir8051::UpDataDebugInfo(void) {
	d_Rges.R0 = Rges.R0();
	d_Rges.R1 = Rges.R1();
	d_Rges.R2 = Rges.R2();
	d_Rges.R3 = Rges.R3();
	d_Rges.R4 = Rges.R4();
	d_Rges.R5 = Rges.R5();
	d_Rges.R6 = Rges.R6();
	d_Rges.R7 = Rges.R7();

	d_Sys.a = Sys.a();
	d_Sys.b = Sys.b();
	d_Sys.sp = Sys.sp();
	d_Sys.dptr = Sys.dptr();
	d_Sys.psw = Sys.psw();
	d_Sys.PC = Sys.PC;
}

void * CVir8051::GetExRamAddr(INT16U addr) const {
	Assert(addr < sizeof(m_ExRam));
	return (void *) &m_ExRam[addr];
}
INT8U CVir8051::GetExRam(INT16U addr) const {
	Assert(addr < sizeof(m_ExRam));
	return m_ExRam[addr];
}
INT8U CVir8051::SetExRam(INT16U addr, INT8U data) {
	Assert(addr < sizeof(m_ExRam));
	return m_ExRam[addr] = data;
}
void * CVir8051::GetPointRamAddr(INT16U addr) const {
	Assert(addr < sizeof(m_ChipRam));
	return (void *) &m_ChipRam[addr];
}

INT8U CVir8051::GetRamDataAt(INT8U addr) {
	return m_ChipRam[addr];
}
INT8U CVir8051::SetRamDataAt(INT8U addr, INT8U data) {
	return m_ChipRam[addr] = data;
}
INT8U CVir8051::GetRamData(INT8U addr) {
	return GetRamRef(addr);
}
INT8U CVir8051::SetRamData(INT8U addr, INT8U data) {
	GetRamRef(addr) = data;
	return data;
}

void* CVir8051::GetPointFileAddr(INT16U addr) const {
	Assert(addr < sizeof(m_ExeFile));
	return (void*) &m_ExeFile[addr];
}

void CVir8051::Opcode_00_NOP(void) {
	Sys.PC++;
}

void CVir8051::Opcode_01_AJMP_Addr11(void) {
	INT8U temp = 0;
	GetOpcodeData(&temp, 1);
	AJMP(0x01, temp);
}

void CVir8051::Opcode_21_AJMP_Addr11(void) {
	INT8U temp = 0;
	GetOpcodeData(&temp, 1);
	AJMP(0x21, temp);
}
void CVir8051::Opcode_41_AJMP_Addr11(void) {
	INT8U temp = 0;
	GetOpcodeData(&temp, 1);
	AJMP(0x41, temp);
}
void CVir8051::Opcode_61_AJMP_Addr11(void) {
	INT8U temp = 0;
	GetOpcodeData(&temp, 1);
	AJMP(0x61, temp);
}
void CVir8051::Opcode_81_AJMP_Addr11(void) {
	INT8U temp = 0;
	GetOpcodeData(&temp, 1);
	AJMP(0x81, temp);
}
void CVir8051::Opcode_A1_AJMP_Addr11(void) {
	INT8U temp = 0;
	GetOpcodeData(&temp, 1);
	AJMP(0xA1, temp);
}
void CVir8051::Opcode_C1_AJMP_Addr11(void) {
	INT8U temp = 0;
	GetOpcodeData(&temp, 1);
	AJMP(0xC1, temp);
}
void CVir8051::Opcode_E1_AJMP_Addr11(void) {
	INT8U temp = 0;
	GetOpcodeData(&temp, 1);
	AJMP(0xE1, temp);
}
void CVir8051::Opcode_02_LJMP_Addr16(void) {
	INT8U temp[2];
	INT16U data;
	GetOpcodeData(temp, 2);
	data = (((INT16U) (temp[0])) << 8);
	Sys.PC = data | temp[1];
}

void CVir8051::Opcode_03_RR_A(void) {
	INT8U temp = (Sys.a() & 0x1);
	Sys.a = Sys.a() >> 1;
	Sys.a = Sys.a() | (temp << 7);
//	Sys.a =Sys.a() | (temp << 7);

	Sys.PC++;
}
void CVir8051::Opcode_04_INC_A(void) {
	Sys.a = Sys.a() + 1;
	Sys.PC++;
}
void CVir8051::Opcode_05_INC_Direct(void) {
	INT8U temp = 0;
	INT8U addr = 0;
//	void *p = NULL;
	GetOpcodeData(&addr, 1);
//	p = GetPointRamAddr(addr);
//	memcpy(&temp, p, sizeof(temp));
	temp = GetRamData(addr);
	++temp;
//	memcpy(p, &temp, sizeof(temp));
	SetRamData(addr, temp);
	Sys.PC = Sys.PC + 2;
}

void CVir8051::Opcode_06_INC_R0_1(void) {
	INT8U data = 0;
	INT8U addr = Rges.R0();
	data = GetRamDataAt(addr);
	++data;
	SetRamDataAt(addr, data);
//	++m_ChipRam[Rges.R0()];
//	void *p = GetPointRamAddr(Rges.R0());
//	memcpy(&temp, p, sizeof(temp));
//	temp++;
//	memcpy(p, &temp, sizeof(temp));
	Sys.PC++;
}
void CVir8051::Opcode_07_INC_R1_1(void) {
//	INT8U temp = 0;
//	void *p = GetPointRamAddr(Rges.R1());
//	memcpy(&temp, p, sizeof(temp));
//	temp++;
//	memcpy(p, &temp, sizeof(temp));

	INT8U data = 0;
	INT8U addr = Rges.R1();
	data = GetRamDataAt(addr);
	++data;
	SetRamDataAt(addr, data);
//	++m_ChipRam[Rges.R1()];
	Sys.PC++;
}

void CVir8051::Opcode_08_INC_R0(void) {
	Rges.R0 = Rges.R0() + 1;
	Sys.PC++;
}
void CVir8051::Opcode_09_INC_R1(void) {
	Rges.R1 = Rges.R1() + 1;
	Sys.PC++;
}
void CVir8051::Opcode_0A_INC_R2(void) {
	Rges.R2 = Rges.R2() + 1;
	Sys.PC++;
}
void CVir8051::Opcode_0B_INC_R3(void) {
	Rges.R3 = Rges.R3() + 1;
	Sys.PC++;
}
void CVir8051::Opcode_0C_INC_R4(void) {
	Rges.R4 = Rges.R4() + 1;
	Sys.PC++;
}
void CVir8051::Opcode_0D_INC_R5(void) {
	Rges.R5 = Rges.R5() + 1;
	Sys.PC++;
}
void CVir8051::Opcode_0E_INC_R6(void) {
	Rges.R6 = Rges.R6() + 1;
	Sys.PC++;
}
void CVir8051::Opcode_0F_INC_R7(void) {
	Rges.R7 = Rges.R7() + 1;
	Sys.PC++;
}
void CVir8051::Opcode_10_JBC_Bit_Rel(void) {
	INT8U temp[2];
	char tem2;

	GetOpcodeData(&temp[0], sizeof(temp));
	Sys.PC += 3;
//	Assert(temp[0]<=0xF7);

//	if (temp[0] <= 0x7F) {
	if (GetBitFlag((temp[0]))) {
		ClrBitFlag(temp[0]);
		memcpy(&tem2, &temp[1], sizeof(temp[1]));
		Sys.PC += tem2;
	}
//	} else {
//		if (GetBitFlag(ChangeBitAddr(temp[0]), temp[0] % 8)) {
//			ClrBitFlag(ChangeBitAddr(temp[0]), temp[0] % 8);
//			Sys.PC = GetTargPC(temp[1]);
//			;
//		}
//	}
}

void CVir8051::Opcode_11_ACALL_Addr11(void) {
	ACALL(0x11);
}
void CVir8051::Opcode_31_ACALL_Addr11(void) {
	ACALL(0x31);
}
void CVir8051::Opcode_51_ACALL_Addr11(void) {
	ACALL(0x51);
}
void CVir8051::Opcode_71_ACALL_Addr11(void) {
	ACALL(0x71);
}
void CVir8051::Opcode_91_ACALL_Addr11(void) {
	ACALL(0x91);
}
void CVir8051::Opcode_B1_ACALL_Addr11(void) {
	ACALL(0xB1);
}
void CVir8051::Opcode_D1_ACALL_Addr11(void) {
	ACALL(0xD1);
}
void CVir8051::Opcode_F1_ACALL_Addr11(void) {
	ACALL(0xF1);
}

INT16U CVir8051::GetLcallAddr(void) {
	INT16U addr = 0;
	GetOpcodeData((INT8U*) &addr, 2);
	Assert(GetOpcode() == 0x12);
	return (addr >> 8) | (addr << 8);

}
//
//bool CVir8051::SubstitutionLcall(char *RetData,enum FUN_TYPE type,INT16U Retlen)
//{
//    static const INT16U map[][2] =
//    {
//        {RET_VOID__VOID,0},
//        {RET_VOID__PARA_CHAR,0},
//        {RET_VOID__PARA_CHAR_CHAR,0},
//        {RET_VOID__PARA_CHAR_CHAR_P,0},
//
//        {RET_CHAR__VOID,1},
//        {RET_CHAR__VOID,1},
//        {RET_VOID__PARA_CHAR_CHAR,1},
//        {RET_VOID__PARA_CHAR_CHAR_P,1},
//    };
//    INT16U i = sizeof(map)/(sizeof(INT16U)*2);
//    INT16U temp =0xFFFF;
//
//    while(i--)
//    {
//        if(map[i][0] == type)
//            {
//               temp = map[i][1];
//               break;
//            }
//    }
//
//
//    Sys.PC += 3;
//
//    if(temp ==0xFFFF)
//    {
//        Assert(0);
//    }
//      switch (temp)
//        {
//
//            case 0:
//            {
//                 return true;
//            }
//           case 1:
//            {
//               Rges.R7 = RetData[0];//放入返回数据
//              return true;
//            }
//
//        }
//    return false;
//}

//
//bool CVir8051::GetParameter(char *rxbuffer,enum FUN_TYPE type,INT16U rxlen)
//    {
//    INT32U Addr;
//    void *p_tem;
//    switch (type)
//        {
//        case RET_VOID__VOID:
//                {
//                     return true;
//                 }
//          case RET_CHAR__PARA_CHAR_CHAR:
//          case RET_VOID__PARA_CHAR_CHAR:
//            {
//                Assert(rxlen >=2);
//                rxbuffer[0]= Rges.R7();
//                rxbuffer[1]= Rges.R5();
//                 return true;
//             }
//
//           case RET_CHAR__VOID:
//           case RET_VOID__PARA_CHAR:
//            {
//                Assert(rxlen >= 1);
//                 rxbuffer[0] = Rges.R7();
//                 return true;
//            }
//
//           case  RET_CHAR__PARA_CHAR_CHAR_P:
//           case RET_VOID__PARA_CHAR_CHAR_P:
//            {
//                rxbuffer[0]= Rges.R7();
//                rxbuffer[1]= Rges.R5();
//                rxbuffer[2]= Rges.R1();
//                rxbuffer[3]= Rges.R2();
//                rxbuffer[4]= Rges.R3();
//                Addr = 0 ;
//                memcpy(&Addr,&rxbuffer[2],3);
//                p_tem = Chang8051pToSelfp(Addr);
//                memcpy(&rxbuffer[2],&p_tem,sizeof(p_tem));
//                return true;
//            }
//        }
//    return false;
//    }

//void CVir8051::SubstitutionLcall(INT16U ret)
//    {
//       Sys.PC+=3;
//       Rges.R6 =  ((INT8U)(ret>>8));
//       Rges.R7 = ((INT8U)(ret));
//    }

void CVir8051::Opcode_12_LCALL_Addr16(void) {
	INT8U temp = 0;
	INT16U addr = 0;
	void *p = NULL;

	GetOpcodeData((INT8U*) &addr, 2);
	Sys.PC += 3;
	Sys.sp = Sys.sp() + 1;
//	p = GetPointRamAddr(Sys.sp());
//	temp = (INT8U) (Sys.PC & 0x00ff);
//	memcpy(p, &temp, sizeof(temp));
	SetRamDataAt(Sys.sp(), (INT8U) (Sys.PC & 0x00ff));
	Sys.sp = Sys.sp() + 1;
//	p = GetPointRamAddr(Sys.sp());
//	temp = (INT8U) (Sys.PC >> 8);
//	memcpy(p, &temp, sizeof(temp));
	SetRamDataAt(Sys.sp(), (INT8U) (Sys.PC >> 8));
// GetOpcodeData(&Sys.PC,2);
	Sys.PC = (addr >> 8) | (addr << 8);
}
void CVir8051::Opcode_13_RRC_A(void) {
	INT8U temp = Sys.a() & BIT0;

	Sys.a = ((Sys.a() >> 1) | (Sys.psw().cy << 7));

	Sys.psw().cy = (temp == 0) ? 0 : 1;
	Sys.PC++;
}
void CVir8051::Opcode_14_DEC_A(void) {
	Sys.a = Sys.a() - 1;
	Sys.PC++;
}
void CVir8051::Opcode_15_DEC_Direct(void) {
	INT8U temp = 0;
	INT8U addr = 0;
	void *p = NULL;
	GetOpcodeData(&addr, 1);
//	p = GetPointRamAddr(addr);

//	memcpy(&temp, p, sizeof(temp));
	temp = GetRamData(addr);
	temp--;
//	memcpy(p, &temp, sizeof(temp));
	SetRamData(addr, temp);
	Sys.PC = Sys.PC + 2;
}
void CVir8051::Opcode_16_DEC_R0_1(void) {
	INT8U temp = 0;
	INT8U addr = Rges.R0();
//	void *p = GetPointRamAddr(Rges.R0());
//	memcpy(&temp, p, sizeof(temp));
	temp = GetRamDataAt(addr);
	temp--;
	SetRamDataAt(addr, temp);
	Sys.PC++;
}
void CVir8051::Opcode_17_DEC_R1_1(void) {
//	INT8U temp = 0;
//	void *p = GetPointRamAddr(Rges.R1());
//	memcpy(&temp, p, sizeof(temp));
//	temp--;
//	memcpy(p, &temp, sizeof(temp));
//	Sys.PC++;

	INT8U temp = 0;
	INT8U addr = Rges.R1();
//	void *p = GetPointRamAddr(Rges.R0());
//	memcpy(&temp, p, sizeof(temp));
	temp = GetRamDataAt(addr);
	temp--;
	SetRamDataAt(addr, temp);
	Sys.PC++;

}
void CVir8051::Opcode_18_DEC_R0(void) {
//	Rges.R0--;
	Rges.R0 = Rges.R0() - 1;

	Sys.PC++;
}
void CVir8051::Opcode_19_DEC_R1(void) {
	Rges.R1 = Rges.R1() - 1;

	Sys.PC++;
}
void CVir8051::Opcode_1A_DEC_R2(void) {
	Rges.R2 = Rges.R2() - 1;

	Sys.PC++;
}
void CVir8051::Opcode_1B_DEC_R3(void) {
	Rges.R3 = Rges.R3() - 1;

	Sys.PC++;
}
void CVir8051::Opcode_1C_DEC_R4(void) {
	Rges.R4 = Rges.R4() - 1;

	Sys.PC++;
}
void CVir8051::Opcode_1D_DEC_R5(void) {
	Rges.R5 = Rges.R5() - 1;

	Sys.PC++;
}
void CVir8051::Opcode_1E_DEC_R6(void) {
	Rges.R6 = Rges.R6() - 1;
	;
	Sys.PC++;
}
void CVir8051::Opcode_1F_DEC_R7(void) {
	Rges.R7 = Rges.R7() - 1;
	;
	Sys.PC++;
}
void CVir8051::Opcode_20_JB_Bit_Rel(void) {
	INT8U temp[2];

	GetOpcodeData(&temp[0], sizeof(temp));
	Sys.PC += 3;

	if (GetBitFlag((temp[0]))) {
		char tem2;
		memcpy(&tem2, &temp[1], sizeof(temp[1]));
		Sys.PC += tem2;
//		Sys.PC = GetTargPC(temp[1]);
	}

//	Assert(temp[0]<0xF7);
//	if (temp[0] <= 0x7F) {
//		if (GetBitFlag((temp[0] / 8) + 0x20, temp[0] % 8)) {
//			Sys.PC = GetTargPC(temp[1]);
//		}
//	} else {
//		if (GetBitFlag(ChangeBitAddr(temp[0]), temp[0] % 8)) {
//			Sys.PC = GetTargPC(temp[1]);
//		}
//	}
}

void CVir8051::Opcode_22_RET(void) {
	INT8U temp = 0;
//	p = GetPointRamAddr(Sys.sp());
//	memcpy(&temp, p, sizeof(temp));
	temp = GetRamDataAt(Sys.sp());
	Sys.PC = (Sys.PC & 0x00FF) | (((INT16U) temp) << 8);
// memcpy((INT8U*)(&Sys.PC)+1,&temp,1);
	Sys.sp = Sys.sp() - 1;
//	p = GetPointRamAddr(Sys.sp());
//	memcpy(&temp, p, sizeof(temp));
	temp = GetRamDataAt(Sys.sp());
	Sys.PC = (Sys.PC & 0xFF00) | temp;
//memcpy((INT8U*)(&Sys.PC),&temp,1);
	Sys.sp = Sys.sp() - 1;
}

void CVir8051::Opcode_23_RL_A(void) {
	INT8U temp = Sys.a() & 0x80;
	INT8U tem2 = Sys.a();
	Sys.a = (INT8U) ((tem2 << 1) | (temp >> 7));
	Sys.PC++;
}

void CVir8051::MD_ADDC(INT8U data) {
	INT8U flagAC = Sys.psw().cy;

	if (flagAC > 1) {
		assert(0);
		flagAC = 1;
	}

	INT16U tep = (INT16U) Sys.a() + (INT16U) data + flagAC;

	INT16U tep2 = ((INT16U) (Sys.a() & 0x7F) + (INT16U) (data & 0x7F)) + flagAC;

	if ((Sys.a() & 0x0f) + (data & 0x0f) + flagAC > 0x0F) {
		Sys.psw().ac = 1;
	} else {
		Sys.psw().ac = 0;

	}
	if (tep > 0xFF) {
		Sys.psw().cy = 1;
	} else {
		Sys.psw().cy = 0;

	}

	flagAC = 0;

	if (tep > 0xFF) {
		flagAC++;
	}

	if (tep2 > 0x7F) {
		flagAC++;
	}

	Sys.psw().ov = flagAC == 1 ? 1 : 0;

	Sys.a = (INT8U) tep;

}
void CVir8051::MD_SUBB(INT8U data) {
	INT16U tepa = Sys.a();

	INT8U bcy = Sys.psw().cy;
//	data += Sys.psw().cy;

	if ((tepa & 0x0f) < (data & 0x0f) + Sys.psw().cy) {
		Sys.psw().ac = 1;
	} else {

		Sys.psw().ac = 0;

	}

	if (tepa < data + bcy) {
		Sys.psw().cy = 1;
	} else {
		Sys.psw().cy = 0;
	}

//下面查溢出位
	INT8U flagac = 0;
	if (tepa < data + bcy) {
		flagac++;
	}

	if ((tepa & 0x7F) < (data & 0x7F) + bcy) {

		flagac++;

	}

	Sys.psw().ov = flagac == 1 ? 1 : 0;
	Sys.a = (INT8U) (tepa - data - bcy);

}

void CVir8051::MD_ADD(INT8U data) {
	INT16U tep = (INT16U) Sys.a() + (INT16U) data;

	INT16U tep2 = ((INT16U) (Sys.a() & 0x7F) + (INT16U) (data & 0x7F));

	if ((Sys.a() & 0x0f) + (data & 0x0f) > 0x0F) {
		Sys.psw().ac = 1;
	} else {
		Sys.psw().ac = 0;
	}

	if (tep > 0xFF) {
		Sys.psw().cy = 1;
	} else {
		Sys.psw().cy = 0;
	}

	INT8U flagAC = 0;

	if (tep > 0xFF) {
		flagAC++;
	}

	if (tep2 > 0x7F) {
		flagAC++;
	}

	Sys.psw().ov = flagAC == 1 ? 1 : 0;

	Sys.a = (INT8U) tep;

}

void CVir8051::Opcode_24_ADD_A_Data(void) {
	INT8U temp;
	GetOpcodeData(&temp, 1);
	MD_ADD(temp);
	Sys.PC += 2;
}
void CVir8051::Opcode_25_ADD_A_Direct(void) {
	INT8U temp;
	INT8U addr;
//	void *p = NULL;
	GetOpcodeData(&addr, 1);
//	p = GetPointRamAddr(addr);
//	memcpy(&temp, p, sizeof(temp));

	temp = GetRamData(addr);
	MD_ADD(temp);
	Sys.PC += 2;
}
void CVir8051::Opcode_26_ADD_A_R0_1(void) {
	INT8U temp = 0;
//	void *p = GetPointRamAddr(Rges.R0());
	temp = GetRamDataAt(Rges.R0());
//	memcpy(&temp, p, sizeof(temp));
	MD_ADD(temp);
//Sys.a +=temp;
	Sys.PC++;
}
void CVir8051::Opcode_27_ADD_A_R1_1(void) {
	INT8U temp = 0;
//	void *p = GetPointRamAddr(Rges.R1());
//	memcpy(&temp, p, sizeof(temp));
	temp = GetRamDataAt(Rges.R1());
	MD_ADD(temp);
//Sys.a +=temp;
	Sys.PC++;
}
void CVir8051::Opcode_28_ADD_A_R0(void) {
	MD_ADD(Rges.R0());
	Sys.PC++;
}
void CVir8051::Opcode_29_ADD_A_R1(void) {
	MD_ADD(Rges.R1());
	Sys.PC++;
}
void CVir8051::Opcode_2A_ADD_A_R2(void) {
	MD_ADD(Rges.R2());
	Sys.PC++;
}
void CVir8051::Opcode_2B_ADD_A_R3(void) {
	MD_ADD(Rges.R3());
	Sys.PC++;
}
void CVir8051::Opcode_2C_ADD_A_R4(void) {
	MD_ADD(Rges.R4());
	Sys.PC++;
}
void CVir8051::Opcode_2D_ADD_A_R5(void) {
	MD_ADD(Rges.R5());
	Sys.PC++;
}
void CVir8051::Opcode_2E_ADD_A_R6(void) {
	MD_ADD(Rges.R6());
	Sys.PC++;
}
void CVir8051::Opcode_2F_ADD_A_R7(void) {
	MD_ADD(Rges.R7());
	Sys.PC++;
}
void CVir8051::Opcode_30_JNB_Bit_Rel(void) {
	INT8U temp[2];

	GetOpcodeData(&temp[0], sizeof(temp));
//	Assert(temp[0]<0xF7);

	Sys.PC += 3;
	if (!GetBitFlag((temp[0]))) {
		char tem2;
		memcpy(&tem2, &temp[1], sizeof(temp[1]));
		Sys.PC += tem2;
//		Sys.PC = GetTargPC(temp[1]);
	}

//	if (temp[0] <= 0x7F) {
//		if (!GetBitFlag((temp[0] / 8) + 0x20, temp[0] % 8)) {
//			Sys.PC = GetTargPC(temp[1]);
//		}
//	} else {
//		if (!GetBitFlag(ChangeBitAddr(temp[0]), temp[0] % 8)) {
//			Sys.PC = GetTargPC(temp[1]);
//		}
//	}
}

void CVir8051::Opcode_32_RETI(void) {
	INT8U temp = 0;
	void *p = NULL;
//	p = GetPointRamAddr(Sys.sp());
//	memcpy(&temp, p, sizeof(temp));
	temp = GetRamData(Sys.sp());
	Sys.PC = (Sys.PC & 0x00FF) | (((INT16U) temp) << 8);
// memcpy((INT8U*)(&Sys.PC)+1,&temp,1);
	Sys.sp = Sys.sp() - 1;
//	p = GetPointRamAddr(Sys.sp());
//	memcpy(&temp, p, sizeof(temp));
	temp = GetRamData(Sys.sp());
	Sys.PC = (Sys.PC & 0xFF00) | temp;
//memcpy((INT8U*)(&Sys.PC),&temp,1);
	Sys.sp = Sys.sp() - 1;
	Sys.PC++;
}
void CVir8051::Opcode_33_RLC_A(void) {
	INT8U temp = Sys.a() & 0x80;
	Sys.a = ((Sys.a() << 1) | Sys.psw().cy);
	Sys.psw().cy = temp == 0 ? 0 : 1;

	Sys.PC++;
}
void CVir8051::Opcode_34_ADDC_A_Data(void) {
	INT8U data;
	GetOpcodeData(&data, 1);
	MD_ADDC(data);   //加上

	Sys.PC += 2;
}
void CVir8051::Opcode_35_ADDC_A_Direct(void) {
	INT8U data;
	INT8U addr;
//	void *p = NULL;
	GetOpcodeData(&addr, 1);
//	p = GetPointRamAddr(addr);
//	memcpy(&temp, p, sizeof(temp));
	data = GetRamData(addr);
	MD_ADDC(data);

	Sys.PC += 2;
}
void CVir8051::Opcode_36_ADDC_A_R0_1(void) {
	INT8U temp = 0;
//	void *p = GetPointRamAddr(Rges.R0());
//	memcpy(&temp, p, sizeof(temp));
	temp = GetRamDataAt(Rges.R0());
	MD_ADDC(temp);

	Sys.PC++;
}
void CVir8051::Opcode_37_ADDC_A_R1_1(void) {
	INT8U temp = 0;
//	void *p = GetPointRamAddr(Rges.R1());
//	memcpy(&temp, p, sizeof(temp));
	temp = GetRamDataAt(Rges.R1());

// GetOpcodeData(&temp,1);
	MD_ADDC(temp);

	Sys.PC++;
}
void CVir8051::Opcode_38_ADDC_A_R0(void) {

//GetOpcodeData(&temp,1);
	MD_ADDC(Rges.R0());

	Sys.PC++;
}
void CVir8051::Opcode_39_ADDC_A_R1(void) {
	MD_ADDC(Rges.R1());

	Sys.PC++;
}
void CVir8051::Opcode_3A_ADDC_A_R2(void) {
	MD_ADDC(Rges.R2());

	Sys.PC++;
}
void CVir8051::Opcode_3B_ADDC_A_R3(void) {
	MD_ADDC(Rges.R3());

	Sys.PC++;
}
void CVir8051::Opcode_3C_ADDC_A_R4(void) {
	MD_ADDC(Rges.R4());

	Sys.PC++;
}
void CVir8051::Opcode_3D_ADDC_A_R5(void) {
	MD_ADDC(Rges.R5());

	Sys.PC++;
}
void CVir8051::Opcode_3E_ADDC_A_R6(void) {
	MD_ADDC(Rges.R6());

	Sys.PC++;
}
void CVir8051::Opcode_3F_ADDC_A_R7(void) {
	MD_ADDC(Rges.R7());

	Sys.PC++;
}
void CVir8051::Opcode_40_JC_Rel(void) {
	char tem2;
	GetOpcodeData(&tem2, 1);
	Sys.PC += 2;
	if (Sys.psw().cy) {
		memcpy(&tem2, &tem2, sizeof(tem2));
		Sys.PC += tem2;
//		Sys.PC = GetTargPC(temp);
	}
}

void CVir8051::Opcode_42_ORL_Direct_A(void) {
	INT8U temp;
	INT8U addr;
//	void *p = NULL;
	GetOpcodeData(&addr, 1);
//	p = GetPointRamAddr(addr);
//	memcpy(&temp, p, sizeof(temp));
	temp = GetRamData(addr);
	temp |= Sys.a();
//	memcpy(p, &temp, sizeof(temp));
	SetRamData(addr, temp);
	Sys.PC += 2;
}
void CVir8051::Opcode_43_ORL_Direct_Data(void) {
	INT8U temp[2] = { 0 };
	INT8U data;
//	void *p = NULL;
	GetOpcodeData(&temp[0], 2); //direct ~{JG~}1~{8vWV=Z~}
//	p = GetPointRamAddr(temp[0]);
//	memcpy(&data, p, 1);
	data = GetRamData(temp[0]);
	data |= temp[1];
//	memcpy(p, &data, 1);
	SetRamData(temp[0], data);
	Sys.PC += 3;
}
void CVir8051::Updata_A_P_Flag(void) {
	INT8U a = Sys.a();
	a ^= a >> 4;
	a ^= a >> 2;
	a ^= a >> 1;
	Sys.psw().p = (a & 1);

}
void CVir8051::Opcode_44_ORL_A_Data(void) {
	INT8U temp;
	GetOpcodeData(&temp, 1);
	Sys.a = Sys.a() | temp;
	Sys.PC += 2;
}
void CVir8051::Opcode_45_ORL_A_Direct(void) {
	INT8U data;
	INT8U addr;
	GetOpcodeData(&addr, 1);
	data = GetRamData(addr);
	Sys.a = Sys.a() | data;
	Sys.PC += 2;
}
void CVir8051::Opcode_46_ORL_A_R0_1(void) {
//	INT8U temp = 0;
//	void *p = GetPointRamAddr(Rges.R0());
//	memcpy(&temp, p, sizeof(temp));
	Sys.a = Sys.a() | GetRamDataAt(Rges.R0());
	Sys.PC++;
}
void CVir8051::Opcode_47_ORL_A_R1_1(void) {
	INT8U temp = 0;
//	void *p = GetPointRamAddr(Rges.R1());
//	memcpy(&temp, p, sizeof(temp));
	temp = GetRamDataAt(Rges.R1());
	Sys.a = Sys.a() | temp;
	Sys.PC++;
}
void CVir8051::Opcode_48_ORL_A_R0(void) {
	Sys.a = Sys.a() | Rges.R0();
	Sys.PC++;
}
void CVir8051::Opcode_49_ORL_A_R1(void) {
	Sys.a = Sys.a() | Rges.R1();
	Sys.PC++;
}
void CVir8051::Opcode_4A_ORL_A_R2(void) {
	Sys.a = Sys.a() | Rges.R2();
	Sys.PC++;
}
void CVir8051::Opcode_4B_ORL_A_R3(void) {
	Sys.a = Sys.a() | Rges.R3();
	Sys.PC++;
}
void CVir8051::Opcode_4C_ORL_A_R4(void) {
	Sys.a = Sys.a() | Rges.R4();
	Sys.PC++;
}
void CVir8051::Opcode_4D_ORL_A_R5(void) {
	Sys.a = Sys.a() | Rges.R5();
	Sys.PC++;
}
void CVir8051::Opcode_4E_ORL_A_R6(void) {
	Sys.a = Sys.a() | Rges.R6();
	Sys.PC++;
}
void CVir8051::Opcode_4F_ORL_A_R7(void) {
	Sys.a = Sys.a() | Rges.R7();
	Sys.PC++;
}
void CVir8051::Opcode_50_JNC_Rel(void) {
	char tem2;
	GetOpcodeData(&tem2, 1);
	Sys.PC += 2;
	if (Sys.psw().cy == 0) {
		Sys.PC += tem2;
	}
}
void CVir8051::Opcode_52_ANL_Direct_A(void) {
	INT8U temp;
	INT8U addr;
//	void *p = NULL;
	GetOpcodeData(&addr, 1);
//	p = GetPointRamAddr(addr);
	temp = GetRamData(addr);
//	memcpy(&temp, p, sizeof(temp));
	temp &= Sys.a();
//	memcpy(p, &temp, sizeof(temp));
	SetRamData(addr, temp);

	Sys.PC += 2;
}
void CVir8051::Opcode_53_ANL_Direct_Data(void) {
	INT8U temp[2] = { 0 };
	INT8U data;
//	void *p = NULL;
	GetOpcodeData(&temp[0], 2);
//	p = GetPointRamAddr(temp[0]);
//	memcpy(&data, p, 1);
	data = GetRamData(temp[0]);
	data &= temp[1];
//	memcpy(p, &data, 1);
	SetRamData(temp[0], data);
	Sys.PC += 3;
}
void CVir8051::Opcode_54_ANL_A_Data(void) {
	INT8U temp;
	GetOpcodeData(&temp, 1);
	Sys.a = Sys.a() & temp;
	Sys.PC += 2;
}

void CVir8051::Opcode_55_ANL_A_Direct(void) {
	INT8U temp;
	INT8U addr;
//	void *p = NULL;
	GetOpcodeData(&addr, 1);
//	p = GetPointRamAddr(addr);
//	memcpy(&temp, p, 1);
	temp = GetRamData(addr);
	Sys.a = Sys.a() & temp;
	Sys.PC += 2;
}
void CVir8051::Opcode_56_ANL_A_R0_1(void) {
//	INT8U temp = 0;
//	void *p = GetPointRamAddr(Rges.R0());
//	memcpy(&temp, p, sizeof(temp));
	Sys.a = Sys.a() & GetRamDataAt(Rges.R0());
	Sys.PC++;
}
void CVir8051::Opcode_57_ANL_A_R1_1(void) {
	INT8U temp = 0;
//	void *p = GetPointRamAddr(Rges.R1());
//	memcpy(&temp, p, sizeof(temp));
	temp = GetRamDataAt(Rges.R1());
	Sys.a = Sys.a() & temp;
	Sys.PC++;
}
void CVir8051::Opcode_58_ANL_A_R0(void) {
	Sys.a = Sys.a() & Rges.R0();
	Sys.PC++;
}
void CVir8051::Opcode_59_ANL_A_R1(void) {
	Sys.a = Sys.a() & Rges.R1();
	Sys.PC++;
}
void CVir8051::Opcode_5A_ANL_A_R2(void) {
	Sys.a = Sys.a() & Rges.R2();
	Sys.PC++;
}
void CVir8051::Opcode_5B_ANL_A_R3(void) {
	Sys.a = Sys.a() & Rges.R3();
	Sys.PC++;
}
void CVir8051::Opcode_5C_ANL_A_R4(void) {
	Sys.a = Sys.a() & Rges.R4();
	Sys.PC++;
}
void CVir8051::Opcode_5D_ANL_A_R5(void) {
	Sys.a = Sys.a() & Rges.R5();
	Sys.PC++;
}
void CVir8051::Opcode_5E_ANL_A_R6(void) {
	Sys.a = Sys.a() & Rges.R6();
	Sys.PC++;
}
void CVir8051::Opcode_5F_ANL_A_R7(void) {
	Sys.a = Sys.a() & Rges.R7();
	Sys.PC++;
}
void CVir8051::Opcode_60_JZ_Rel(void) {
	char temp;
	GetOpcodeData(&temp, 1);
	Sys.PC += 2;
	if (Sys.a() == 0) {
//		Sys.PC = GetTargPC(temp);
		Sys.PC += temp;
	}
}
void CVir8051::Opcode_62_XRL_Direct_A(void) {
	INT8U temp;
	INT8U addr;
	void *p = NULL;
	GetOpcodeData(&addr, 1);
//	p = GetPointRamAddr(addr);
//	memcpy(&temp, p, sizeof(temp));
	temp = GetRamData(addr);
	temp ^= Sys.a();
//	memcpy(p, &temp, sizeof(temp));
	SetRamData(addr, temp);
	Sys.PC += 2;
}

void CVir8051::Opcode_63_XRL_Direct_Data(void) {
	INT8U temp[2] = { 0 };
	INT8U data;
	void *p = NULL;
	GetOpcodeData(&temp[0], 2);
//	p = GetPointRamAddr(temp[0]);
//	memcpy(&data, p, 1);
	data = GetRamData(temp[0]);
	data ^= temp[1];
//	memcpy(p, &data, 1);
	SetRamData(temp[0], data);
	Sys.PC += 3;
}
void CVir8051::Opcode_64_XRL_A_Data(void) {
	INT8U temp;
	GetOpcodeData(&temp, 1);
	Sys.a = Sys.a() ^ temp;
	Sys.PC += 2;
}

void CVir8051::Opcode_65_XRL_A_Direct(void) {
	INT8U temp;
	INT8U addr;
	void *p = NULL;
	GetOpcodeData(&addr, 1);
//	p = GetPointRamAddr(addr);
//	memcpy(&temp, p, 1);
	temp = GetRamData(addr);
	Sys.a = Sys.a() ^ temp;
	Sys.PC += 2;
}
void CVir8051::Opcode_66_XRL_A_R0_1(void) {
	INT8U temp = 0;
//	void *p = GetPointRamAddr(Rges.R0());
//	memcpy(&temp, p, sizeof(temp));
	temp = GetRamDataAt(Rges.R0());
	Sys.a = Sys.a() ^ temp;
	Sys.PC++;
}
void CVir8051::Opcode_67_XRL_A_R1_1(void) {
	INT8U temp = 0;
//	void *p = GetPointRamAddr(Rges.R1());
//	memcpy(&temp, p, sizeof(temp));
	temp = GetRamDataAt(Rges.R1());
	Sys.a = Sys.a() ^ temp;
	Sys.PC++;
}
void CVir8051::Opcode_68_XRL_A_R0(void) {
	Sys.a = Sys.a() ^ Rges.R0();
	Sys.PC++;
}
void CVir8051::Opcode_69_XRL_A_R1(void) {
	Sys.a = Sys.a() ^ Rges.R1();
	Sys.PC++;
}
void CVir8051::Opcode_6A_XRL_A_R2(void) {
	Sys.a = Sys.a() ^ Rges.R2();
	Sys.PC++;
}
void CVir8051::Opcode_6B_XRL_A_R3(void) {
	Sys.a = Sys.a() ^ Rges.R3();
	Sys.PC++;
}
void CVir8051::Opcode_6C_XRL_A_R4(void) {
	Sys.a = Sys.a() ^ Rges.R4();
	Sys.PC++;
}
void CVir8051::Opcode_6D_XRL_A_R5(void) {
	Sys.a = Sys.a() ^ Rges.R5();
	Sys.PC++;
}
void CVir8051::Opcode_6E_XRL_A_R6(void) {
	Sys.a = Sys.a() ^ Rges.R6();
	Sys.PC++;
}
void CVir8051::Opcode_6F_XRL_A_R7(void) {
	Sys.a = Sys.a() ^ Rges.R7();
	Sys.PC++;
}
void CVir8051::Opcode_70_JNZ_Rel(void) {
	char temp;
	GetOpcodeData(&temp, 1);
	Sys.PC += 2;
	if (Sys.a() != 0) {
//		Sys.PC = GetTargPC(temp);
		Sys.PC += temp;
	}
}
void CVir8051::Opcode_72_ORL_C_Direct(void) {
	INT8U addr, data;
	GetOpcodeData(&addr, 1);

	data = GetBitFlag(addr);
//	Assert(temp<0xF7);
//	if (temp <= 0x7F) {
//		data = (GetBitFlag((temp / 8) + 0x20, temp % 8)) ? 1 : 0;
//	} else {
//		data = (GetBitFlag(ChangeBitAddr(temp), temp % 8)) ? 1 : 0;
//	}
//	temp = 0;
//	if (Sys.psw().cy) {
//		temp++;
//	}
//	if (data) {
//		temp++;
//	}
	Sys.psw().cy = Sys.psw().cy | data;

	Sys.PC += 2;
}
void CVir8051::Opcode_73_JMP_A_DPTR(void) {
	Sys.PC++;
	Sys.PC = (INT16U) Sys.a() + Sys.dptr();
}
void CVir8051::Opcode_74_MOV_A_Data(void) {
	INT8U temp;
	GetOpcodeData(&temp, 1);
	Sys.a = temp;

	Sys.PC += 2;
}
void CVir8051::Opcode_75_MOV_Direct_Data(void) {
	INT8U temp[2] = { 0 };
	INT8U data;
	void *p = NULL;
	GetOpcodeData(&temp[0], 2);
//	p = GetPointRamAddr(temp[0]);

//	data = temp[1];
	SetRamData(temp[0], temp[1]);
//	memcpy(p, &data, 1);
	Sys.PC += 3;
}
void CVir8051::Opcode_76_MOV_R0_1_Data(void) {
//	INT8U temp;
//	INT8U data;
//	void *p = GetPointRamAddr(Rges.R0());
	GetOpcodeData(&m_ChipRam[Rges.R0()], 1);
//	data = temp;
//	memcpy(p, &data, sizeof(data));
	Sys.PC += 2;
}
void CVir8051::Opcode_77_MOV_R1_1_Data(void) {
//	INT8U temp;
//	INT8U data;
//	void *p = GetPointRamAddr(Rges.R1());
//	GetOpcodeData(&temp, 1);
//	data = temp;
//	memcpy(p, &data, sizeof(data));
	GetOpcodeData(&m_ChipRam[Rges.R1()], 1);
	Sys.PC += 2;
}
void CVir8051::Opcode_78_MOV_R0_Data(void) {
	INT8U temp;
	GetOpcodeData(&temp, 1);
	Rges.R0 = temp;
	Sys.PC += 2;
}
void CVir8051::Opcode_79_MOV_R1_Data(void) {
	INT8U temp;
	GetOpcodeData(&temp, 1);
	Rges.R1 = temp;
	Sys.PC += 2;
}
void CVir8051::Opcode_7A_MOV_R2_Data(void) {
	INT8U temp;
	GetOpcodeData(&temp, 1);
	Rges.R2 = temp;
	Sys.PC += 2;
}
void CVir8051::Opcode_7B_MOV_R3_Data(void) {
	INT8U temp;
	GetOpcodeData(&temp, 1);
	Rges.R3 = temp;
	Sys.PC += 2;
}
void CVir8051::Opcode_7C_MOV_R4_Data(void) {
	INT8U temp;
	GetOpcodeData(&temp, 1);
	Rges.R4 = temp;
	Sys.PC += 2;
}
void CVir8051::Opcode_7D_MOV_R5_Data(void) {
	INT8U temp;
	GetOpcodeData(&temp, 1);
	Rges.R5 = temp;
	Sys.PC += 2;
}
void CVir8051::Opcode_7E_MOV_R6_Data(void) {
	INT8U temp;
	GetOpcodeData(&temp, 1);
	Rges.R6 = temp;
	Sys.PC += 2;
}
void CVir8051::Opcode_7F_MOV_R7_Data(void) {
	INT8U temp;
	GetOpcodeData(&temp, 1);
	Rges.R7 = temp;
	Sys.PC += 2;
}
void CVir8051::Opcode_80_SJMP_Rel(void) {
	char temp;
	GetOpcodeData(&temp, 1);
//	printf("temp hex %d \r\n",temp);
	Sys.PC += 2;
//	Sys.PC = GetTargPC(temp);
	Sys.PC += temp;
}
void CVir8051::Opcode_82_ANL_C_Bit(void) {
//	INT8U temp;
//	GetOpcodeData(&temp, 1);
	Sys.psw().cy = Sys.psw().cy & GetBitFlag(m_ExeFile[Sys.PC + 1]);
	Sys.PC += 2;
}
void CVir8051::Opcode_83_MOVC_A_PC(void) {
//	INT8U temp;
	INT16U addr;
//	void*p = NULL;
	++Sys.PC;
	addr = (INT16U) (Sys.PC + Sys.a());
//	p = GetPointFileAddr(addr);
//	memcpy(&temp, p, sizeof(temp));
	Sys.a = (Sys.PC > CVir8051::MAX_ROM - Sys.a()) ? (0x00) : (m_ExeFile[addr]);

}
void CVir8051::Opcode_84_DIV_AB(void) {
	INT8U data1, data2;
	data1 = Sys.a();
	data2 = Sys.b();
	if (data2 == 0) {
		Sys.psw().ov = 1;
		goto Ret;
	}
	Sys.a = data1 / data2;
	Sys.b = data1 % data2;
	Sys.psw().ov = 0;
	Ret: Sys.psw().cy = 0;
	Sys.PC++;
}
void CVir8051::Opcode_85_MOV_Direct_Direct(void) {
	INT8U temp[2];
	INT8U data[2];
//	void *p0 = NULL;
//	void *p1 = NULL;
	INT8U tem;
	GetOpcodeData(&temp[0], 2);
//	p0 = GetPointRamAddr(temp[0]);
	tem = GetRamData(temp[0]);
//	memcpy(&data[0], p0, 1);
//	p1 = GetPointRamAddr(temp[1]);
//	memcpy(p1, &data[0], 1);
	SetRamData(temp[1], tem);
	Sys.PC += 3;
}
void CVir8051::Opcode_86_MOV_Direct_R0_1(void) {
	INT8U addr;
	INT8U data;
//	void*p0 = NULL;
//	void *p1 = NULL;
	GetOpcodeData(&addr, 1);
//	p0 = GetPointRamAddr(temp);
//	p1 = GetPointRamAddr(Rges.R0());
	data = GetRamDataAt(Rges.R0());
//	memcpy(&data, p1, 1);
//	memcpy(p0, &data, 1);
	SetRamData(addr, data);
	Sys.PC += 2;
}
void CVir8051::Opcode_87_MOV_Direct_R1_1(void) {
	INT8U addr;
	INT8U data;
//	void*p0 = NULL;
//	void *p1 = NULL;
	GetOpcodeData(&addr, 1);
//	p0 = GetPointRamAddr(temp);
//	p1 = GetPointRamAddr(Rges.R1());
	data = GetRamDataAt(Rges.R1());
//	memcpy(&data, p1, 1);
//	memcpy(p0, &data, 1);
	SetRamData(addr, data);
	Sys.PC += 2;
}
void CVir8051::Opcode_88_MOV_Direct_R0(void) {
	INT8U addr;
//	void*p = NULL;
	GetOpcodeData(&addr, 1);
	SetRamData(addr, Rges.R0());

//	temp = Rges.R0();
//	memcpy(p, &temp, 1);
	Sys.PC += 2;
}
void CVir8051::Opcode_89_MOV_Direct_R1(void) {
	INT8U addr;
//	void*p = NULL;
	GetOpcodeData(&addr, 1);
	SetRamData(addr, Rges.R1());
//	p = GetPointRamAddr(temp);
//	temp = Rges.R1();
//	memcpy(p, &temp, 1);
	Sys.PC += 2;
}
void CVir8051::Opcode_8A_MOV_Direct_R2(void) {
	INT8U temp;
//	void *p = NULL;
	GetOpcodeData(&temp, 1);
//temp = Rges.R2();
//	p = GetPointRamAddr(temp);
//	temp = Rges.R2();
//	memcpy(p, &temp, 1);
	SetRamData(temp, Rges.R2());
	Sys.PC += 2;
}
void CVir8051::Opcode_8B_MOV_Direct_R3(void) {
	INT8U temp;
//	void *p = NULL;
	GetOpcodeData(&temp, 1);
//temp = Rges.R2();
//	p = GetPointRamAddr(temp);
//	temp = Rges.R2();
//	memcpy(p, &temp, 1);
	SetRamData(temp, Rges.R3());
	Sys.PC += 2;
}
void CVir8051::Opcode_8C_MOV_Direct_R4(void) {
	INT8U temp;
//	void *p = NULL;
	GetOpcodeData(&temp, 1);
//temp = Rges.R2();
//	p = GetPointRamAddr(temp);
//	temp = Rges.R2();
//	memcpy(p, &temp, 1);
	SetRamData(temp, Rges.R4());
	Sys.PC += 2;
}
void CVir8051::Opcode_8D_MOV_Direct_R5(void) {
	INT8U temp;
//	void *p = NULL;
	GetOpcodeData(&temp, 1);
//temp = Rges.R2();
//	p = GetPointRamAddr(temp);
//	temp = Rges.R2();
//	memcpy(p, &temp, 1);
	SetRamData(temp, Rges.R5());
	Sys.PC += 2;
}
void CVir8051::Opcode_8E_MOV_Direct_R6(void) {
	INT8U temp;
//	void *p = NULL;
	GetOpcodeData(&temp, 1);
//temp = Rges.R2();
//	p = GetPointRamAddr(temp);
//	temp = Rges.R2();
//	memcpy(p, &temp, 1);
	SetRamData(temp, Rges.R6());
	Sys.PC += 2;
}
void CVir8051::Opcode_8F_MOV_Direct_R7(void) {
	INT8U temp;
//	void *p = NULL;
	GetOpcodeData(&temp, 1);
//temp = Rges.R2();
//	p = GetPointRamAddr(temp);
//	temp = Rges.R2();
//	memcpy(p, &temp, 1);
	SetRamData(temp, Rges.R7());
	Sys.PC += 2;
}
void CVir8051::Opcode_90_MOV_DPTR_Data(void) {
	INT16U temp;
	GetOpcodeData(&temp, 2);
	temp = (temp >> 8) | (temp << 8);
	Sys.dptr = temp;
	Sys.PC += 3;
}
void CVir8051::Opcode_92_MOV_Bit_C(void) {
	INT8U temp;
	GetOpcodeData(&temp, 1);
//	Assert(temp<=0xF7);
	if (Sys.psw().cy) {
//		if (temp <= 0x7F) {
//			SetBitFlag((temp / 8) + 0x20, temp % 8);
//		} else {
//			SetBitFlag(ChangeBitAddr(temp), temp % 8);
//		}
		SetBitFlag(temp);
	} else {
//		if (temp <= 0x7F) {
//			ClrBitFlag((temp / 8) + 0x20, temp % 8);
//		} else {
//			ClrBitFlag(ChangeBitAddr(temp), temp % 8);
//		}
		ClrBitFlag(temp);
	}
	Sys.PC += 2;
}
void CVir8051::Opcode_93_MOVC_A_DPTR(void) {
	INT8U temp;
	void *p = NULL;
	p = GetPointFileAddr((INT16U) (Sys.a() + Sys.dptr()));
	memcpy(&temp, p, sizeof(temp));
	Sys.a = (Sys.dptr() > CVir8051::MAX_ROM - Sys.a()) ? (0x00) : (temp);
	Sys.PC++;
}

void CVir8051::Opcode_94_SUBB_A_Data(void) {
	INT8U temp;
	GetOpcodeData(&temp, 1);
	MD_SUBB(temp);

	Sys.PC += 2;
}
void CVir8051::Opcode_95_SUBB_A_Direct(void) {
	INT8U addr;
	INT8U data;
//	void *p = NULL;

	GetOpcodeData(&addr, 1);
//	p = GetPointRamAddr(addr);
//	memcpy(&data, p, 1);
	data = GetRamData(addr);
	MD_SUBB(data);
	Sys.PC += 2;
}
void CVir8051::Opcode_96_SUBB_A_R0_1(void) {
//	INT8U temp;
//	void *p = NULL;
//	p = GetPointRamAddr(Rges.R0());
//	memcpy(&temp, p, sizeof(temp));
	MD_SUBB(GetRamDataAt(Rges.R0()));

	Sys.PC++;
}
void CVir8051::Opcode_97_SUBB_A_R1_1(void) {
//	INT8U temp;
//	void *p = NULL;
//	p = GetPointRamAddr(Rges.R1());
//	memcpy(&temp, p, sizeof(temp));
	MD_SUBB(GetRamDataAt(Rges.R1()));

	Sys.PC++;
}
void CVir8051::Opcode_98_SUBB_A_R0(void) {
	MD_SUBB(Rges.R0());
	Sys.PC++;
}
void CVir8051::Opcode_99_SUBB_A_R1(void) {
	MD_SUBB(Rges.R1());
	Sys.PC++;
}
void CVir8051::Opcode_9A_SUBB_A_R2(void) {
	MD_SUBB(Rges.R2());
	Sys.PC++;
}
void CVir8051::Opcode_9B_SUBB_A_R3(void) {
	MD_SUBB(Rges.R3());
	Sys.PC++;
}
void CVir8051::Opcode_9C_SUBB_A_R4(void) {
	MD_SUBB(Rges.R4());
	Sys.PC++;
}
void CVir8051::Opcode_9D_SUBB_A_R5(void) {
	MD_SUBB(Rges.R5());
	Sys.PC++;
}
void CVir8051::Opcode_9E_SUBB_A_R6(void) {
	MD_SUBB(Rges.R6());
	Sys.PC++;
}
void CVir8051::Opcode_9F_SUBB_A_R7(void) {
	MD_SUBB(Rges.R7());
	Sys.PC++;
}
void CVir8051::Opcode_A0_ORL_C_Bit(void) {
	INT8U temp;
	GetOpcodeData(&temp, 1);
//	;
//
//	Assert(temp<0xF7);
//	if (temp <= 0x7F) {
//		data = (GetBitFlag((temp / 8) + 0x20, temp % 8)) ? 1 : 0;
//	} else {
//		data = (GetBitFlag(ChangeBitAddr(temp), temp % 8)) ? 1 : 0;
//	}
//	temp = 0;
//	if (Sys.psw().cy) {
//		temp++;
//	}
//	if (!data) //把反
//	{
//		temp++;
//	}
	Sys.psw().cy = Sys.psw().cy | (!GetBitFlag(temp));

	Sys.PC += 2;
}
void CVir8051::Opcode_A2_MOV_C_Bit(void) {
	INT8U temp;
	GetOpcodeData(&temp, sizeof(temp));
//	Assert(temp<=0xF7);
//	if (temp <= 0x7F) {
//		Sys.psw().cy = (GetBitFlag((temp / 8) + 0x20, temp % 8)) ? 1 : 0;
//	} else {
//		Sys.psw().cy = (GetBitFlag(ChangeBitAddr(temp), temp % 8)) ? 1 : 0;
//	}
	Sys.psw().cy = GetBitFlag(temp);
	Sys.PC += 2;
}
void CVir8051::Opcode_A3_INC_DPTR(void) {
	Sys.dptr = Sys.dptr() + 1;
	;

	Sys.PC++;
}
void CVir8051::Opcode_A4_MUL_AB(void) {
	INT16U temp;
	temp = (INT16U) Sys.a() * (INT16U) Sys.b();
	Sys.psw().ov = (temp > 255) ? 1 : 0;
	Sys.a = (INT8U) temp;
	Sys.b = (INT8U) (temp >> 8);

	Sys.PC++;
}

void CVir8051::Opcode_A5(void) {
}

void CVir8051::Opcode_A6_MOV_R0_1_Direct(void) {
	INT8U addr;
	INT8U data;
//	void*p0 = NULL;
//	void *p1 = NULL;

//	p0 = GetPointRamAddr(Rges.R0());
	GetOpcodeData(&addr, 1);
	data = GetRamData(addr);
//	p1 = GetPointRamAddr(addr);
//	memcpy(&data, p1, 1);
//	memcpy(p0, &data, 1);
	SetRamDataAt(Rges.R0(), data);
	Sys.PC += 2;
}
void CVir8051::Opcode_A7_MOV_R1_1_Direct(void) {
	INT8U addr;
	INT8U data;
	GetOpcodeData(&addr, 1);
	data = GetRamData(addr);
	SetRamDataAt(Rges.R1(), data);
	Sys.PC += 2;
}
void CVir8051::Opcode_A8_MOV_R0_Direct(void) {
	INT8U Addr;

	GetOpcodeData(&Addr, 1);
	Rges.R0 = GetRamData(Addr);
	Sys.PC += 2;
}
void CVir8051::Opcode_A9_MOV_R1_Direct(void) {
	INT8U Addr;

	GetOpcodeData(&Addr, 1);
	Rges.R1 = GetRamData(Addr);
	Sys.PC += 2;
}
void CVir8051::Opcode_AA_MOV_R2_Direct(void) {

	INT8U Addr;
	GetOpcodeData(&Addr, 1);
	Rges.R2 = GetRamData(Addr);
	Sys.PC += 2;
}
void CVir8051::Opcode_AB_MOV_R3_Direct(void) {
	INT8U Addr;

	GetOpcodeData(&Addr, 1);
	Rges.R3 = GetRamData(Addr);
	Sys.PC += 2;
}
void CVir8051::Opcode_AC_MOV_R4_Direct(void) {
	INT8U Addr;

	GetOpcodeData(&Addr, 1);
	Rges.R4 = GetRamData(Addr);
	Sys.PC += 2;
}
void CVir8051::Opcode_AD_MOV_R5_Direct(void) {
	INT8U Addr;
	GetOpcodeData(&Addr, 1);
	Rges.R5 = GetRamData(Addr);
	Sys.PC += 2;
}
void CVir8051::Opcode_AE_MOV_R6_Direct(void) {
	INT8U Addr;
	GetOpcodeData(&Addr, 1);
	Rges.R6 = GetRamData(Addr);
	Sys.PC += 2;
}
void CVir8051::Opcode_AF_MOV_R7_Direct(void) {
	INT8U Addr;

	GetOpcodeData(&Addr, 1);
	Rges.R7 = GetRamData(Addr);
	Sys.PC += 2;
}
void CVir8051::Opcode_B0_ANL_C_Bit_1(void) {  // ANL C ~Bit
	INT8U temp;
//	INT8U data;
	GetOpcodeData(&temp, 1);
//	Assert(temp<=0xF7);
//	if (temp <= 0x7F) {
//		data = (GetBitFlag((temp / 8) + 0x20, temp % 8)) ? 1 : 0;
//	} else {
//		data = (GetBitFlag(ChangeBitAddr(temp), temp % 8)) ? 1 : 0;
//	}
//	if (Sys.psw().cy) {
//		temp++;
//	}
//
//	if (!data)  //取反
//	{
//		temp++;
//	}
	Sys.psw().cy = Sys.psw().cy & (!GetBitFlag(temp));
	Sys.PC += 2;
}
void CVir8051::Opcode_B2_CPL_Bit(void) {
	INT8U temp;
	GetOpcodeData(&temp, sizeof(temp));
//	Assert(temp<=0xF7);
	if (GetBitFlag(temp)) {
		ClrBitFlag(temp);
	} else {
		SetBitFlag(temp);
	}
//	if (temp <= 0x7F) {
//		if (GetBitFlag((temp / 8) + 0x20, temp % 8)) {
//			ClrBitFlag((temp / 8) + 0x20, temp % 8);
//		} else {
//			SetBitFlag((temp / 8) + 0x20, temp % 8);
//		}
//	} else {
//		if (GetBitFlag(ChangeBitAddr(temp), temp % 8)) {
//			ClrBitFlag(ChangeBitAddr(temp), temp % 8);
//		} else {
//			SetBitFlag(ChangeBitAddr(temp), temp % 8);
//		}
//	}
	Sys.PC += 2;
}
void CVir8051::Opcode_B3_CPL_C(void) {
	Sys.psw().cy = ~Sys.psw().cy;
	Sys.PC++;
}

//INT16U CVir8051::GetTargPC(INT8U rel) {
//
//	if (rel < 0x80) {
//		return Sys.PC + rel;
//	} else {
//		return Sys.PC +(char)rel;
//	}
//}
void CVir8051::Opcode_B4_CJNE_A_Data_Rel(void) {
	INT8U temp[2];

	GetOpcodeData(&temp[0], sizeof(temp));
	Sys.PC += 3;

	if (Sys.a() != temp[0]) {
		char tem;
		memcpy(&tem, &temp[1], 1);
		Sys.PC += tem;
	}
	if ((Sys.a() < temp[0])) {
		Sys.psw().cy = 1;
	} else {
		Sys.psw().cy = 0;
	}
}

void CVir8051::Opcode_B5_CJNE_A_Direct_Rel(void) {
	INT8U temp[2];
	INT8U data;

	GetOpcodeData(&temp[0], sizeof(temp));
	Sys.PC += 3;
	data = GetRamData(temp[0]);
	if (Sys.a() != data) {
		char tem;
		memcpy(&tem, &temp[1], 1);
		Sys.PC += tem;
	}
	Sys.psw().cy = (Sys.a() < data) ? 1 : 0;
}
void CVir8051::Opcode_B6_CJNE_R0_1_Data_Rel(void) {
	INT8U temp[2];
	INT8U data;
	GetOpcodeData(&temp[0], sizeof(temp));
	data = GetRamDataAt(Rges.R0());
	Sys.PC += 3;

	if (data != temp[0]) {
		char tem;
		memcpy(&tem, &temp[1], 1);
		Sys.PC += tem;
	}
	Sys.psw().cy = (data < temp[0]) ? 1 : 0;
}
void CVir8051::Opcode_B7_CJNE_R1_1_Data_Rel(void) {
	INT8U temp[2];
	INT8U data;
	GetOpcodeData(&temp[0], sizeof(temp));
	data = GetRamDataAt(Rges.R1());
	Sys.PC += 3;

	if (data != temp[0]) {
		char tem;
		memcpy(&tem, &temp[1], 1);
		Sys.PC += tem;
	}
	Sys.psw().cy = (data < temp[0]) ? 1 : 0;
}
void CVir8051::Opcode_B8_CJNE_R0_Data_Rel(void) {
	INT8U temp[2];

	GetOpcodeData(&temp[0], sizeof(temp));
	Sys.PC += 3;
	if (Rges.R0() != temp[0]) {
		char tem;
		memcpy(&tem, &temp[1], 1);
		Sys.PC += tem;
	}
	Sys.psw().cy = (Rges.R0() < temp[0]) ? 1 : 0;
}
void CVir8051::Opcode_B9_CJNE_R1_Data_Rel(void) {
	INT8U temp[2];
	GetOpcodeData(&temp[0], sizeof(temp));
	Sys.PC += 3;

	if (Rges.R1() != temp[0]) {
//		Sys.PC = GetTargPC(temp[1]);
		char tem;
		memcpy(&tem, &temp[1], 1);
		Sys.PC += tem;
	}
	Sys.psw().cy = (Rges.R1() < temp[0]) ? 1 : 0;
}
void CVir8051::Opcode_BA_CJNE_R2_Data_Rel(void) {
	INT8U temp[2];
	GetOpcodeData(&temp[0], sizeof(temp));
	Sys.PC += 3;
	if (Rges.R2() != temp[0]) {
//		Sys.PC = GetTargPC(temp[1]);
		char tem;
		memcpy(&tem, &temp[1], 1);
		Sys.PC += tem;
	}
	Sys.psw().cy = (Rges.R2() < temp[0]) ? 1 : 0;
}
void CVir8051::Opcode_BB_CJNE_R3_Data_Rel(void) {
	INT8U temp[2];
	GetOpcodeData(&temp[0], sizeof(temp));
	Sys.PC += 3;
	if (Rges.R3() != temp[0]) {
//		Sys.PC = GetTargPC(temp[1]);
		char tem;
		memcpy(&tem, &temp[1], 1);
		Sys.PC += tem;
	}
	Sys.psw().cy = (Rges.R3() < temp[0]) ? 1 : 0;
}
void CVir8051::Opcode_BC_CJNE_R4_Data_Rel(void) {
	INT8U temp[2];
	GetOpcodeData(&temp[0], sizeof(temp));
	Sys.PC += 3;

	if (Rges.R4() != temp[0]) {
//		Sys.PC = GetTargPC(temp[1]);
		char tem;
		memcpy(&tem, &temp[1], 1);
		Sys.PC += tem;
	}
	Sys.psw().cy = (Rges.R4() < temp[0]) ? 1 : 0;
}
void CVir8051::Opcode_BD_CJNE_R5_Data_Rel(void) {
	INT8U temp[2];
	GetOpcodeData(&temp[0], sizeof(temp));
	Sys.PC += 3;
	if (Rges.R5() != temp[0]) {
//		Sys.PC = GetTargPC(temp[1]);
		char tem;
		memcpy(&tem, &temp[1], 1);
		Sys.PC += tem;
	}
	Sys.psw().cy = (Rges.R5() < temp[0]) ? 1 : 0;
}
void CVir8051::Opcode_BE_CJNE_R6_Data_Rel(void) {
	INT8U temp[2];
	GetOpcodeData(&temp[0], sizeof(temp));
	Sys.PC += 3;
	if (Rges.R6() != temp[0]) {
//		Sys.PC = GetTargPC(temp[1]);
		char tem;
		memcpy(&tem, &temp[1], 1);
		Sys.PC += tem;
	}
	Sys.psw().cy = (Rges.R6() < temp[0]) ? 1 : 0;
}
void CVir8051::Opcode_BF_CJNE_R7_Data_Rel(void) {
	INT8U temp[2];
	GetOpcodeData(&temp[0], sizeof(temp));
	Sys.PC += 3;
	if (Rges.R7() != temp[0]) {
//		Sys.PC = GetTargPC(temp[1]);
		char tem;
		memcpy(&tem, &temp[1], 1);
		Sys.PC += tem;
	}
	Sys.psw().cy = (Rges.R7() < temp[0]) ? 1 : 0;
}
void CVir8051::Opcode_C0_PUSH_Direct(void) {
	INT8U addr;
	INT8U data = 0;
	GetOpcodeData(&addr, sizeof(addr));
	data = GetRamData(addr);
	Sys.sp = Sys.sp() + 1;
	SetRamDataAt(Sys.sp(), data);
	Sys.PC += 2;
}

void CVir8051::Opcode_C2_CLR_Bit(void) { // 位寻址
	INT8U temp;
	GetOpcodeData(&temp, sizeof(temp));
//	Assert(temp<=0xF7);
//	if (temp <= 0x7F) {
//		ClrBitFlag((temp / 8) + 0x20, temp % 8);
//	} else {
//		ClrBitFlag(ChangeBitAddr(temp), temp % 8);
//	}
	ClrBitFlag(temp);
	Sys.PC += 2;
}
void CVir8051::Opcode_C3_CLR_C(void) {
	Sys.psw().cy = 0;
	Sys.PC++;
}
void CVir8051::Opcode_C4_SWAP_A(void) {
	INT8U temp = Sys.a() & 0x0F;
	Sys.a = (Sys.a() >> 4) | (temp << 4);
	Sys.PC++;
}
void CVir8051::Opcode_C5_XCH_A_Direct(void) {
	INT8U addr;
	INT8U data;
	INT8U TT;
	GetOpcodeData(&addr, sizeof(addr));
	data = GetRamData(addr);
	TT = Sys.a();
	Sys.a = data;
	SetRamData(addr, TT);
	Sys.PC += 2;
}
void CVir8051::Opcode_C6_XCH_A_R0_1(void) {
	INT8U data;
	INT8U TT;
	data = GetRamDataAt(Rges.R0());
	TT = Sys.a();
	Sys.a = data;
	SetRamDataAt(Rges.R0(), TT);
	Sys.PC++;
}
void CVir8051::Opcode_C7_XCH_A_R1_1(void) {
	INT8U data;
	INT8U TT;
	data = GetRamDataAt(Rges.R1());
	TT = Sys.a();
	Sys.a = data;
	SetRamDataAt(Rges.R1(), TT);
	Sys.PC++;
}
void CVir8051::Opcode_C8_XCH_A_R0(void) {
	INT8U temp;
	temp = Sys.a();
	Sys.a = Rges.R0();
	Rges.R0 = temp;
	Sys.PC++;
}
void CVir8051::Opcode_C9_XCH_A_R1(void) {
	INT8U temp;
	temp = Sys.a();
	Sys.a = Rges.R1();
	Rges.R1 = temp;
	Sys.PC++;
}
void CVir8051::Opcode_CA_XCH_A_R2(void) {
	INT8U temp;
	temp = Sys.a();
	Sys.a = Rges.R2();
	Rges.R2 = temp;
	Sys.PC++;
}
void CVir8051::Opcode_CB_XCH_A_R3(void) {
	INT8U temp;
	temp = Sys.a();
	Sys.a = Rges.R3();
	Rges.R3 = temp;
	Sys.PC++;
}
void CVir8051::Opcode_CC_XCH_A_R4(void) {
	INT8U temp;
	temp = Sys.a();
	Sys.a = Rges.R4();
	Rges.R4 = temp;
	Sys.PC++;
}
void CVir8051::Opcode_CD_XCH_A_R5(void) {
	INT8U temp;
	temp = Sys.a();
	Sys.a = Rges.R5();
	Rges.R5 = temp;
	Sys.PC++;
}
void CVir8051::Opcode_CE_XCH_A_R6(void) {
	INT8U temp;
	temp = Sys.a();
	Sys.a = Rges.R6();
	Rges.R6 = temp;
	Sys.PC++;
}
void CVir8051::Opcode_CF_XCH_A_R7(void) {
	INT8U temp;
	temp = Sys.a();
	Sys.a = Rges.R7();
	Rges.R7 = temp;
	Sys.PC++;
}
void CVir8051::Opcode_D0_POP_Direct(void) {
	INT8U addr;
	INT8U data;
	GetOpcodeData(&addr, sizeof(addr));
	data = GetRamDataAt(Sys.sp());
	SetRamData(addr, data);
	Sys.sp = Sys.sp() - 1;
	Sys.PC += 2;
}

void CVir8051::Opcode_D2_SETB_Bit(void) {
	INT8U temp;
	GetOpcodeData(&temp, sizeof(temp));  //bit
	SetBitFlag(temp);
//	Assert(temp<=0xF7);
//	if (temp <= 0x7F) {
//		SetBitFlag((temp / 8) + 0x20, temp % 8);
//	} else {
//		SetBitFlag(ChangeBitAddr(temp), temp % 8);
//	}
	Sys.PC += 2;
}
void CVir8051::Opcode_D3_SETB_C(void) {
	Sys.psw().cy = 1;
	Sys.PC++;
}
void CVir8051::Opcode_D4_DA_A(void) {
	INT8U temp;
	if (((Sys.a() & 0x0f) > 9) || (Sys.psw().ac == 1)) {
		temp = ((Sys.a() & 0x0f) + 6) % 16;
		Sys.a = Sys.a() & 0xF0;
		Sys.a = Sys.a() | temp;
	}
	temp = ((Sys.a() & 0xF0) >> 4);
	if ((temp > 9) || (Sys.psw().cy == 1)) {
		temp = (temp + 6) % 16;
		Sys.a = Sys.a() & 0x0F;
		Sys.a = Sys.a() | (temp << 4);
	}
	Sys.PC++;
}
void CVir8051::Opcode_D5_DJNZ_Direct_Rel(void) {
	INT8U temp[2];
	INT8U data;

	GetOpcodeData(&temp[0], sizeof(temp));

	data = GetRamData(temp[0]);

	data--;

	SetRamData(temp[0], data);

	Sys.PC += 3;
	if (data != 0) {
		char tem;
		memcpy(&tem, &temp[1], 1);
		Sys.PC += tem;
	}
}
void CVir8051::Opcode_D6_XCHD_A_R0_1(void) {
	INT8U temp = 0;
	INT8U data = GetRamDataAt(Rges.R0());
	temp = Sys.a() & 0x0F;
	Sys.a = Sys.a() & 0xF0;
	Sys.a = Sys.a() | (data & 0x0F);
	data &= 0xF0;
	data |= temp;
	SetRamDataAt(Rges.R0(), data);
	Sys.PC++;
}
void CVir8051::Opcode_D7_XCHD_A_R1_1(void) {
	INT8U temp = 0;
	INT8U data;
	data = GetRamDataAt(Rges.R1());
	temp = Sys.a() & 0x0F;
	Sys.a = Sys.a() & 0xF0;
	Sys.a = Sys.a() | (data & 0x0F);
	data &= 0xF0;
	data |= temp;
	SetRamDataAt(Rges.R1(), data);
	Sys.PC++;
}
void CVir8051::Opcode_D8_DJNZ_R0_Rel(void) {
	char temp;
	GetOpcodeData(&temp, sizeof(temp));
	Sys.PC += 2;
	Rges.R0 = Rges.R0() - 1;
	if (Rges.R0() != 0) {
		// pc(0440+2)+temp(FD)=043f
		//Sys.PC+=temp;
//		Sys.PC = GetTargPC(temp);

		Sys.PC += temp;
	}
}
void CVir8051::Opcode_D9_DJNZ_R1_Rel(void) {
	char temp;
	GetOpcodeData(&temp, sizeof(temp));
	Sys.PC += 2;
	Rges.R1 = Rges.R1() - 1;
	if (Rges.R1() != 0) {
//		Sys.PC = GetTargPC(temp);
		Sys.PC += temp;
	}
}
void CVir8051::Opcode_DA_DJNZ_R2_Rel(void) {
	char temp;
	GetOpcodeData(&temp, sizeof(temp));
	Sys.PC += 2;
	Rges.R2 = Rges.R2() - 1;
	if (Rges.R2() != 0) {
//		Sys.PC = GetTargPC(temp);
		Sys.PC += temp;
	}
}
void CVir8051::Opcode_DB_DJNZ_R3_Rel(void) {
	char temp;
	GetOpcodeData(&temp, sizeof(temp));
	Sys.PC += 2;
	Rges.R3 = Rges.R3() - 1;
	if (Rges.R3() != 0) {
//		Sys.PC = GetTargPC(temp);
		Sys.PC += temp;
	}
}
void CVir8051::Opcode_DC_DJNZ_R4_Rel(void) {
	char temp;
	GetOpcodeData(&temp, sizeof(temp));
	Sys.PC += 2;
	Rges.R4 = Rges.R4() - 1;
	if (Rges.R4() != 0) {
//		Sys.PC = GetTargPC(temp);
		Sys.PC += temp;
	}
}
void CVir8051::Opcode_DD_DJNZ_R5_Rel(void) {
	char temp;
	GetOpcodeData(&temp, sizeof(temp));
	Sys.PC += 2;
	Rges.R5 = Rges.R5() - 1;
	if (Rges.R5() != 0) {
//		Sys.PC = GetTargPC(temp);
		Sys.PC += temp;
	}
}
void CVir8051::Opcode_DE_DJNZ_R6_Rel(void) {
	char temp;
	GetOpcodeData(&temp, sizeof(temp));
	Sys.PC += 2;
	Rges.R6 = Rges.R6() - 1;
	if (Rges.R6() != 0) {
//		Sys.PC = GetTargPC(temp);
		Sys.PC += temp;
	}
}
void CVir8051::Opcode_DF_DJNZ_R7_Rel(void) {
	char temp;
	GetOpcodeData(&temp, sizeof(temp));
	Sys.PC += 2;
	Rges.R7 = Rges.R7() - 1;
	if (Rges.R7() != 0) {
//		Sys.PC = GetTargPC(temp);
		Sys.PC += temp;
	}
}
void CVir8051::Opcode_E0_MOVX_A_DPTR(void) {
	INT8U temp = m_ExRam[Sys.dptr()];
//	void*p = GetExRamAddr(Sys.dptr());
//	memcpy(&temp, p, sizeof(temp));
	Sys.a = temp;
	Sys.PC++;
}
void CVir8051::Opcode_E2_MOVX_A_R0_1(void) {
	INT8U temp = m_ExRam[Rges.R0()];
	Sys.a = temp;
	Sys.PC++;
}
void CVir8051::Opcode_E3_MOVX_A_R1_1(void) {
	INT8U temp = m_ExRam[Rges.R1()];
	Sys.a = temp;
	Sys.PC++;
}
void CVir8051::Opcode_E4_CLR_A(void) {
	Sys.a = 0;
	Sys.PC++;
}
void CVir8051::Opcode_E5_MOV_A_Direct(void) {
	INT8U addr;
	GetOpcodeData(&addr, sizeof(addr));
	Sys.a = GetRamData(addr);
	Sys.PC += 2;
}
void CVir8051::Opcode_E6_MOV_A_R0_1(void) {
	Sys.a = GetRamDataAt(Rges.R0());
	Sys.PC++;
}
void CVir8051::Opcode_E7_MOV_A_R1_1(void) {
	Sys.a = GetRamDataAt(Rges.R1());
	Sys.PC++;
}
void CVir8051::Opcode_E8_MOV_A_R0(void) {
	Sys.a = Rges.R0();

	Sys.PC++;
}
void CVir8051::Opcode_E9_MOV_A_R1(void) {
	Sys.a = Rges.R1();
	Sys.PC++;
}
void CVir8051::Opcode_EA_MOV_A_R2(void) {
	Sys.a = Rges.R2();
	Sys.PC++;
}
void CVir8051::Opcode_EB_MOV_A_R3(void) {
	Sys.a = Rges.R3();
	Sys.PC++;
}
void CVir8051::Opcode_EC_MOV_A_R4(void) {
	Sys.a = Rges.R4();
	Sys.PC++;
}
void CVir8051::Opcode_ED_MOV_A_R5(void) {
	Sys.a = Rges.R5();
	Sys.PC++;
}
void CVir8051::Opcode_EE_MOV_A_R6(void) {
	Sys.a = Rges.R6();
	Sys.PC++;
}
void CVir8051::Opcode_EF_MOV_A_R7(void) {
	Sys.a = Rges.R7();
	Sys.PC++;
}
void CVir8051::Opcode_F0_MOVX_DPTR_A(void) {
//	INT8U data;
//	void*p = GetExRamAddr(Sys.dptr());
//	data = Sys.a();
//	memcpy(p, &data, sizeof(data));
	SetExRam(Sys.dptr(), Sys.a());
	Sys.PC++;
}
void CVir8051::Opcode_F2_MOVX_R0_1_A(void) {
//	INT8U data;
//	void*p = GetExRamAddr(Rges.R0());
//	data = Sys.a();
//	memcpy(p, &data, sizeof(data));
	SetExRam(Rges.R0(), Sys.a());
	Sys.PC++;
}
void CVir8051::Opcode_F3_MOVX_R1_1_A(void) {
//	INT8U data;
//	void*p = GetExRamAddr(Rges.R1());
//	data = Sys.a();
//	memcpy(p, &data, sizeof(data));
	SetExRam(Rges.R1(), Sys.a());
	Sys.PC++;
}
void CVir8051::Opcode_F4_CPL_A(void) {
	Sys.a = ~(Sys.a());
	Sys.PC++;
}
void CVir8051::Opcode_F5_MOV_Direct_A(void) {
	INT8U addr;
//	INT8U data;
//	void * p = NULL;

	GetOpcodeData(&addr, sizeof(addr));
//	p = GetPointRamAddr(addr);
//	data = Sys.a();
//	memcpy(p, &data, sizeof(data));
	SetRamData(addr, Sys.a());
	Sys.PC += 2;
}
void CVir8051::Opcode_F6_MOV_R0_1_A(void) {
//	INT8U addr;
//	void *p = NULL;

//	p = GetPointRamAddr(Rges.R0());
//	addr = Sys.a();
//	memcpy(p, &addr, sizeof(addr));
	SetRamDataAt(Rges.R0(), Sys.a());
	Sys.PC++;
}
void CVir8051::Opcode_F7_MOV_R1_1_A(void) {
//	INT8U data;
//	void *p = NULL;
//
//	p = GetPointRamAddr(Rges.R1());
//	data = Sys.a();
//	memcpy(p, &data, sizeof(data));
	SetRamDataAt(Rges.R1(), Sys.a());
	Sys.PC++;
}
void CVir8051::Opcode_F8_MOV_R0_A(void) {
	Rges.R0 = Sys.a();
	Sys.PC++;
}
void CVir8051::Opcode_F9_MOV_R1_A(void) {
	Rges.R1 = Sys.a();
	Sys.PC++;
}
void CVir8051::Opcode_FA_MOV_R2_A(void) {
	Rges.R2 = Sys.a();
	Sys.PC++;
}
void CVir8051::Opcode_FB_MOV_R3_A(void) {
	Rges.R3 = Sys.a();
	Sys.PC++;
}
void CVir8051::Opcode_FC_MOV_R4_A(void) {
	Rges.R4 = Sys.a();
	Sys.PC++;
}
void CVir8051::Opcode_FD_MOV_R5_A(void) {
	Rges.R5 = Sys.a();
	Sys.PC++;
}
void CVir8051::Opcode_FE_MOV_R6_A(void) {
	Rges.R6 = Sys.a();
	Sys.PC++;
}
void CVir8051::Opcode_FF_MOV_R7_A(void) {
	Rges.R7 = Sys.a();
	Sys.PC++;
}
shared_ptr<vector<unsigned char>> CVir8051::GetRetData(void) const {
	auto tem = make_shared<vector<unsigned char>>();

	char buffer[1024] = { 0 };
	memcpy(buffer, &m_ExRam[0xFC00], 1023);
	tem.get()->assign(buffer, buffer + 1023);
	return tem;
}

//
//INT8U& CUPReg_a::GetRegRe(void) {
//	Assert(m_Addr != 255);
//	return pmcu->m_ChipRam[m_Addr];
//}
template<class T2>
T2& CUPReg<T2>::GetRegRe(void) {
	assert(m_Addr != 255);
	return *((T2*) (&pmcu->m_ChipRam[m_Addr]));
}
INT8U& CUPReg_a::GetRegRe(void) {
	assert(m_Addr != 256);
	return pmcu->m_ChipSfr[m_Addr];
}

INT8U& CUPSfr::GetRegRe(void) {
	assert(m_Addr != 255);
	return pmcu->m_ChipSfr[m_Addr];
}
INT16U& CUPSfr16::GetRegRe(void) {
	assert(m_Addr != 255);
	return *((INT16U*) &pmcu->m_ChipSfr[m_Addr]);
}

template<class T2>
T2 CUPReg<T2>::getValue() {
	return GetRegRe();
}

void CUPReg_a::Updataflag() {
	pmcu->Updata_A_P_Flag();
}

PSW& CUPPSW_8::operator()(void) {
	return *((PSW*) &(GetRegRe()));
}

