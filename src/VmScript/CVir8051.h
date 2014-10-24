// ComDirver.h: interface for the CComDirver class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CVir8051COMDIRVER_H__97382C29_90C6_4E40_9581_6474D5D71406__INCLUDED_)
#define AFX_CVir8051COMDIRVER_H__97382C29_90C6_4E40_9581_6474D5D71406__INCLUDED_
typedef unsigned char INT8U;
typedef unsigned short INT16U;


#define BIT0              (0x01ul<<0)   //(INT8U)(0x01)#define BIT1              (0x01ul<<1)  //(INT8U)(0x02)#define BIT2              (0x01ul<<2)#define BIT3              (0x01ul<<3)#define BIT4              (0x01ul<<4)#define BIT5              (0x01ul<<5)
#define BIT6              (0x01ul<<6)
#define BIT7              (0x01ul<<7)
#define BIT8              (0x01ul<<8)
#define BIT9              (0x01ul<<9)
#define BITA              (0x01ul<<10)
#define BITB              (0x01ul<<11)
#define BITC              (0x01ul<<12)
#define BITD              (0x01ul<<13)
#define BITE              (0x01ul<<14)
#define BITF              (0x01ul<<15)
#define BIT16            (0x01ul<<16)
#define BIT17            (0x01ul<<17)
#define BIT18            (0x01ul<<18)
#define BIT19            (0x01ul<<19)
#define BIT20            (0x01ul<<20)
#define BIT21            (0x01ul<<21)
#define BIT22            (0x01ul<<22)
#define BIT23            (0x01ul<<23)
#define BIT24           (0x01ul<<24)
#define BIT25           (0x01ul<<25)
#define BIT26           (0x01ul<<26)
#define BIT27           (0x01ul<<27)
#define BIT28           (0x01ul<<28)
#define BIT29           (0x01ul<<29)
#define BIT30           (0x01ul<<30)
#define BIT31           (0x01ul<<31)
#define Assert(flag)

#include <vector>
#include <string>
#include <memory>
using namespace std;
class CVir8051;
class CUPSfr;
typedef struct tagpsw PSW;
typedef vector<unsigned char> vector_unsigned_char;
template<class T2>
class CUPReg {

protected:

	CVir8051 *pmcu;
public:
	INT8U m_Addr;
	CUPReg(CVir8051 *mcu) {
		m_Addr = 255;
		pmcu = mcu;
	}
	virtual ~CUPReg() {
	}
	;
	virtual T2& GetRegRe(void);
	void SetAddr(INT8U Addr) {
		m_Addr = Addr;
	}
	T2 getValue();

	T2 operator()(void) {
		return GetRegRe();
	}
	template<class T>
	T2 operator=(T data) {
		return GetRegRe() = data;
	}

};
typedef CUPReg<INT8U> CUPReg_8;
typedef CUPReg<INT16U> CUPReg_16;

class CUPSfr: public CUPReg<INT8U> {
public:
	CUPSfr(CVir8051 *mcu) :
			CUPReg<INT8U>(mcu) {
	}
	virtual ~CUPSfr() {
	}
	;

	virtual INT8U& GetRegRe();

	template<class T>
	INT8U operator=(T data) {
		INT8U tep = (GetRegRe() = data);
		return tep;
	}
};
class CUPReg_a: public CUPReg<INT8U> {
public:
	CUPReg_a(CVir8051 *mcu) :
			CUPReg<INT8U>(mcu) {
	}
	virtual ~CUPReg_a() {
	}
	;
	void Updataflag();
	virtual INT8U& GetRegRe();

	template<class T>
	INT8U operator=(T data) {
		INT8U tep = (GetRegRe() = data);
		Updataflag();
		return tep;
	}
};

class CUPSfr16: public CUPReg_16 {
public:
	CUPSfr16(CVir8051 *mcu) :
			CUPReg_16(mcu) {
	}
	template<class T>
	INT16U operator=(T data) {
		return (GetRegRe() = data);
	}
	virtual INT16U& GetRegRe();
};

typedef struct tagpsw {
	INT8U p :1;
	INT8U null :1;
	INT8U ov :1;
	INT8U rs0 :1;
	INT8U rs1 :1;
	INT8U f0 :1;
	INT8U ac :1;
	INT8U cy :1;
} PSW;

class CUPPSW_8: public CUPSfr {
public:

	INT8U getValue() {
		return GetRegRe();
	}
	CUPPSW_8(CVir8051 *mcu) :
			CUPSfr(mcu) {
	}
	PSW& operator()(void); // {return *((PSW*)&(CUPReg_8::GetRegRe()))};
};

class CRges {
public:
	CRges(CVir8051 *mcu) :
			R0(mcu), R1(mcu), R2(mcu), R3(mcu), R4(mcu), R5(mcu), R6(mcu), R7(mcu) {
	}

	CUPReg_8 R0;
	CUPReg_8 R1;
	CUPReg_8 R2;
	CUPReg_8 R3;
	CUPReg_8 R4;
	CUPReg_8 R5;
	CUPReg_8 R6;
	CUPReg_8 R7;

	string ToStr() {
		char temp[1024];
		sprintf(temp, "Rges-> R0:%0x R1:%0x R2:%0x R3:%0x R4:%0x R5:%0x R6:%0x R7:%0x \r\n", R0(), R1(), R2(), R3(),
				R4(), R5(), R6(), R7());
		return string(temp);
	}
};

class CSys {
public:
	CSys(CVir8051 *mcu) :
			a(mcu), b(mcu), sp(mcu), psw(mcu), dptr(mcu), PC(0) {
	}
	CUPReg_a a;
	CUPSfr b;
	CUPSfr sp;
	CUPPSW_8 psw;
	CUPSfr16 dptr;
	INT16U PC;
	string ToStr() {
		char temp[1024];
		sprintf(temp, "Sys -> a:%0x b:%0x sp:%0x psw:%0x dptr:%X PC:%x \r\n", a(), b(), sp(), psw.getValue(), dptr(),
				PC);
		return string(temp);
	}
};

struct DebugRges {
	INT8U R0;
	INT8U R1;
	INT8U R2;
	INT8U R3;
	INT8U R4;
	INT8U R5;
	INT8U R6;
	INT8U R7;

};
struct DebugSys {
	INT8U a;
	INT8U b;
	INT8U sp;
	PSW psw;
	INT16U dptr;
	INT16U PC;

};
class CVir8051 {

	static const int a_addr = 0xE0;
	static const int b_addr = 0xF0;
	static const int sp_addr = 0x81;
	static const int dptrl = 0x82;
	static const int dptrh = 0x83;
	static const int psw_addr = 0xD0;

	static const int MAX_ROM = 0xFFFF;
	static const int MAX_EX_RAM = 0xFFFF;
	static const int MAX_IN_RAM = 0xff;


public:
	void InitalReg();
	CVir8051(const vector<unsigned char> & vRom,const vector<unsigned char> &InputData);
	shared_ptr<vector<unsigned char>> GetRetData(void) const;
	void SetExRamData(INT16U addr, const vector<unsigned char> data);
	INT8U SetRamDataAt(INT8U addr,INT8U data);
	INT8U GetRamDataAt(INT8U addr);
	INT8U GetRamData(INT8U addr);
	INT8U SetRamData(INT8U addr, INT8U data);
	~CVir8051();
	bool run();
	int run(int maxstep);
	void StepRun(INT8U code);

	INT8U GetOpcode(void) const;
	INT8U& GetRamRef(INT8U addr);
	INT8U& GetBitRamRef(INT8U addr) {
		if (addr <= 0x7F) {
			return m_ChipRam[(addr / 8) + 0x20];
		} else {
			return m_ChipSfr[addr - ((addr - 0x80) % 8)];
		}

	}
	INT16U GetLcallAddr(void);
	void SubstitutionLcall(INT16U);

	INT16U GetDebugOpcode(void) const;
	bool GetDebugPC(INT16U pc) const;
	string ToStr() {
		string tem = "CVir8051:";
		tem += Sys.ToStr() + Rges.ToStr();
		return tem;
	}
private:
	CSys Sys;
	CRges Rges;

	DebugRges d_Rges;
	DebugSys d_Sys;
	INT8U m_ChipRam[256];
	INT8U m_ChipSfr[256];
	INT8U m_ExRam[65536];
	INT8U m_ExeFile[65536];
	INT8U m_ChipRamoper[256];

	template<class T> friend class CUPReg;
	friend class CUPReg_a;

	friend class CUPSfr;
	friend class CUPSfr16;
	friend class CUPPSW_8;
	friend class CTestMcu;

	void UpDataDebugInfo(void);

	void * GetExRamAddr(INT16U addr) const;
	INT8U GetExRam(INT16U addr) const;
	INT8U SetExRam(INT16U addr,INT8U data)  ;
	void * GetPointRamAddr(INT16U addr) const;

	void * GetPointFileAddr(INT16U addr) const;

//	INT16U inline GetTargPC(INT8U rel);

	bool GetBitFlag(INT8U addr);
	void SetBitFlag(INT8U addr);
	void ClrBitFlag(INT8U addr);
	void GetOpcodeData(void * const p, INT8U len) const;
	void AJMP(INT8U opCode, INT8U data);
	void ACALL(INT8U opCode);

	void MD_ADD(INT8U data);
	void MD_ADDC(INT8U data);
	void MD_SUBB(INT8U data);

	void Updata_A_P_Flag(void);
	void Opcode_00_NOP(void);
	void Opcode_01_AJMP_Addr11(void);
	void Opcode_02_LJMP_Addr16(void);
	void Opcode_03_RR_A(void);
	void Opcode_04_INC_A(void);
	void Opcode_05_INC_Direct(void);
	void Opcode_06_INC_R0_1(void);
	void Opcode_07_INC_R1_1(void);
	void Opcode_08_INC_R0(void);
	void Opcode_09_INC_R1(void);
	void Opcode_0A_INC_R2(void);
	void Opcode_0B_INC_R3(void);
	void Opcode_0C_INC_R4(void);
	void Opcode_0D_INC_R5(void);
	void Opcode_0E_INC_R6(void);
	void Opcode_0F_INC_R7(void);

	void Opcode_10_JBC_Bit_Rel(void);
	void Opcode_11_ACALL_Addr11(void);
	void Opcode_12_LCALL_Addr16(void);
	void Opcode_13_RRC_A(void);
	void Opcode_14_DEC_A(void);
	void Opcode_15_DEC_Direct(void);
	void Opcode_16_DEC_R0_1(void);
	void Opcode_17_DEC_R1_1(void);
	void Opcode_18_DEC_R0(void);
	void Opcode_19_DEC_R1(void);
	void Opcode_1A_DEC_R2(void);
	void Opcode_1B_DEC_R3(void);
	void Opcode_1C_DEC_R4(void);
	void Opcode_1D_DEC_R5(void);
	void Opcode_1E_DEC_R6(void);
	void Opcode_1F_DEC_R7(void);

	void Opcode_20_JB_Bit_Rel(void);
	void Opcode_21_AJMP_Addr11(void);
	void Opcode_22_RET(void);
	void Opcode_23_RL_A(void);
	void Opcode_24_ADD_A_Data(void);
	void Opcode_25_ADD_A_Direct(void);
	void Opcode_26_ADD_A_R0_1(void);
	void Opcode_27_ADD_A_R1_1(void);
	void Opcode_28_ADD_A_R0(void);
	void Opcode_29_ADD_A_R1(void);
	void Opcode_2A_ADD_A_R2(void);
	void Opcode_2B_ADD_A_R3(void);
	void Opcode_2C_ADD_A_R4(void);
	void Opcode_2D_ADD_A_R5(void);
	void Opcode_2E_ADD_A_R6(void);
	void Opcode_2F_ADD_A_R7(void);

	void Opcode_30_JNB_Bit_Rel(void);
	void Opcode_31_ACALL_Addr11(void);
	void Opcode_32_RETI(void);
	void Opcode_33_RLC_A(void);
	void Opcode_34_ADDC_A_Data(void);
	void Opcode_35_ADDC_A_Direct(void);
	void Opcode_36_ADDC_A_R0_1(void);
	void Opcode_37_ADDC_A_R1_1(void);
	void Opcode_38_ADDC_A_R0(void);
	void Opcode_39_ADDC_A_R1(void);
	void Opcode_3A_ADDC_A_R2(void);
	void Opcode_3B_ADDC_A_R3(void);
	void Opcode_3C_ADDC_A_R4(void);
	void Opcode_3D_ADDC_A_R5(void);
	void Opcode_3E_ADDC_A_R6(void);
	void Opcode_3F_ADDC_A_R7(void);

	void Opcode_40_JC_Rel(void);
	void Opcode_41_AJMP_Addr11(void);
	void Opcode_42_ORL_Direct_A(void);
	void Opcode_43_ORL_Direct_Data(void);
	void Opcode_44_ORL_A_Data(void);
	void Opcode_45_ORL_A_Direct(void);
	void Opcode_46_ORL_A_R0_1(void);
	void Opcode_47_ORL_A_R1_1(void);
	void Opcode_48_ORL_A_R0(void);
	void Opcode_49_ORL_A_R1(void);
	void Opcode_4A_ORL_A_R2(void);
	void Opcode_4B_ORL_A_R3(void);
	void Opcode_4C_ORL_A_R4(void);
	void Opcode_4D_ORL_A_R5(void);
	void Opcode_4E_ORL_A_R6(void);
	void Opcode_4F_ORL_A_R7(void);

	void Opcode_50_JNC_Rel(void);
	void Opcode_51_ACALL_Addr11(void);
	void Opcode_52_ANL_Direct_A(void);
	void Opcode_53_ANL_Direct_Data(void);
	void Opcode_54_ANL_A_Data(void);
	void Opcode_55_ANL_A_Direct(void);
	void Opcode_56_ANL_A_R0_1(void);
	void Opcode_57_ANL_A_R1_1(void);
	void Opcode_58_ANL_A_R0(void);
	void Opcode_59_ANL_A_R1(void);
	void Opcode_5A_ANL_A_R2(void);
	void Opcode_5B_ANL_A_R3(void);
	void Opcode_5C_ANL_A_R4(void);
	void Opcode_5D_ANL_A_R5(void);
	void Opcode_5E_ANL_A_R6(void);
	void Opcode_5F_ANL_A_R7(void);

	void Opcode_60_JZ_Rel(void);
	void Opcode_61_AJMP_Addr11(void);
	void Opcode_62_XRL_Direct_A(void);
	void Opcode_63_XRL_Direct_Data(void);
	void Opcode_64_XRL_A_Data(void);
	void Opcode_65_XRL_A_Direct(void);
	void Opcode_66_XRL_A_R0_1(void);
	void Opcode_67_XRL_A_R1_1(void);
	void Opcode_68_XRL_A_R0(void);
	void Opcode_69_XRL_A_R1(void);
	void Opcode_6A_XRL_A_R2(void);
	void Opcode_6B_XRL_A_R3(void);
	void Opcode_6C_XRL_A_R4(void);
	void Opcode_6D_XRL_A_R5(void);
	void Opcode_6E_XRL_A_R6(void);
	void Opcode_6F_XRL_A_R7(void);

	void Opcode_70_JNZ_Rel(void);
	void Opcode_71_ACALL_Addr11(void);
	void Opcode_72_ORL_C_Direct(void);
	void Opcode_73_JMP_A_DPTR(void);
	void Opcode_74_MOV_A_Data(void);
	void Opcode_75_MOV_Direct_Data(void);
	void Opcode_76_MOV_R0_1_Data(void);
	void Opcode_77_MOV_R1_1_Data(void);
	void Opcode_78_MOV_R0_Data(void);
	void Opcode_79_MOV_R1_Data(void);
	void Opcode_7A_MOV_R2_Data(void);
	void Opcode_7B_MOV_R3_Data(void);
	void Opcode_7C_MOV_R4_Data(void);
	void Opcode_7D_MOV_R5_Data(void);
	void Opcode_7E_MOV_R6_Data(void);
	void Opcode_7F_MOV_R7_Data(void);

	void Opcode_80_SJMP_Rel(void);
	void Opcode_81_AJMP_Addr11(void);
	void Opcode_82_ANL_C_Bit(void);
	void Opcode_83_MOVC_A_PC(void);
	void Opcode_84_DIV_AB(void);
	void Opcode_85_MOV_Direct_Direct(void);
	void Opcode_86_MOV_Direct_R0_1(void);
	void Opcode_87_MOV_Direct_R1_1(void);
	void Opcode_88_MOV_Direct_R0(void);
	void Opcode_89_MOV_Direct_R1(void);
	void Opcode_8A_MOV_Direct_R2(void);
	void Opcode_8B_MOV_Direct_R3(void);
	void Opcode_8C_MOV_Direct_R4(void);
	void Opcode_8D_MOV_Direct_R5(void);
	void Opcode_8E_MOV_Direct_R6(void);
	void Opcode_8F_MOV_Direct_R7(void);

	void Opcode_90_MOV_DPTR_Data(void);
	void Opcode_91_ACALL_Addr11(void);
	void Opcode_92_MOV_Bit_C(void);
	void Opcode_93_MOVC_A_DPTR(void);
	void Opcode_94_SUBB_A_Data(void);
	void Opcode_95_SUBB_A_Direct(void);
	void Opcode_96_SUBB_A_R0_1(void);
	void Opcode_97_SUBB_A_R1_1(void);
	void Opcode_98_SUBB_A_R0(void);
	void Opcode_99_SUBB_A_R1(void);
	void Opcode_9A_SUBB_A_R2(void);
	void Opcode_9B_SUBB_A_R3(void);
	void Opcode_9C_SUBB_A_R4(void);
	void Opcode_9D_SUBB_A_R5(void);
	void Opcode_9E_SUBB_A_R6(void);
	void Opcode_9F_SUBB_A_R7(void);

	void Opcode_A0_ORL_C_Bit(void);
	void Opcode_A1_AJMP_Addr11(void);
	void Opcode_A2_MOV_C_Bit(void);
	void Opcode_A3_INC_DPTR(void);
	void Opcode_A4_MUL_AB(void);
	void Opcode_A5(void);
	void Opcode_A6_MOV_R0_1_Direct(void);
	void Opcode_A7_MOV_R1_1_Direct(void);
	void Opcode_A8_MOV_R0_Direct(void);
	void Opcode_A9_MOV_R1_Direct(void);
	void Opcode_AA_MOV_R2_Direct(void);
	void Opcode_AB_MOV_R3_Direct(void);
	void Opcode_AC_MOV_R4_Direct(void);
	void Opcode_AD_MOV_R5_Direct(void);
	void Opcode_AE_MOV_R6_Direct(void);
	void Opcode_AF_MOV_R7_Direct(void);

	void Opcode_B0_ANL_C_Bit_1(void);
	void Opcode_B1_ACALL_Addr11(void);
	void Opcode_B2_CPL_Bit(void);
	void Opcode_B3_CPL_C(void);
	void Opcode_B4_CJNE_A_Data_Rel(void);
	void Opcode_B5_CJNE_A_Direct_Rel(void);
	void Opcode_B6_CJNE_R0_1_Data_Rel(void);
	void Opcode_B7_CJNE_R1_1_Data_Rel(void);
	void Opcode_B8_CJNE_R0_Data_Rel(void);
	void Opcode_B9_CJNE_R1_Data_Rel(void);
	void Opcode_BA_CJNE_R2_Data_Rel(void);
	void Opcode_BB_CJNE_R3_Data_Rel(void);
	void Opcode_BC_CJNE_R4_Data_Rel(void);
	void Opcode_BD_CJNE_R5_Data_Rel(void);
	void Opcode_BE_CJNE_R6_Data_Rel(void);
	void Opcode_BF_CJNE_R7_Data_Rel(void);

	void Opcode_C0_PUSH_Direct(void);
	void Opcode_C1_AJMP_Addr11(void);
	void Opcode_C2_CLR_Bit(void);
	void Opcode_C3_CLR_C(void);
	void Opcode_C4_SWAP_A(void);
	void Opcode_C5_XCH_A_Direct(void);
	void Opcode_C6_XCH_A_R0_1(void);
	void Opcode_C7_XCH_A_R1_1(void);
	void Opcode_C8_XCH_A_R0(void);
	void Opcode_C9_XCH_A_R1(void);
	void Opcode_CA_XCH_A_R2(void);
	void Opcode_CB_XCH_A_R3(void);
	void Opcode_CC_XCH_A_R4(void);
	void Opcode_CD_XCH_A_R5(void);
	void Opcode_CE_XCH_A_R6(void);
	void Opcode_CF_XCH_A_R7(void);

	void Opcode_D0_POP_Direct(void);
	void Opcode_D1_ACALL_Addr11(void);
	void Opcode_D2_SETB_Bit(void);
	void Opcode_D3_SETB_C(void);
	void Opcode_D4_DA_A(void);
	void Opcode_D5_DJNZ_Direct_Rel(void);
	void Opcode_D6_XCHD_A_R0_1(void);
	void Opcode_D7_XCHD_A_R1_1(void);
	void Opcode_D8_DJNZ_R0_Rel(void);
	void Opcode_D9_DJNZ_R1_Rel(void);
	void Opcode_DA_DJNZ_R2_Rel(void);
	void Opcode_DB_DJNZ_R3_Rel(void);
	void Opcode_DC_DJNZ_R4_Rel(void);
	void Opcode_DD_DJNZ_R5_Rel(void);
	void Opcode_DE_DJNZ_R6_Rel(void);
	void Opcode_DF_DJNZ_R7_Rel(void);

	void Opcode_E0_MOVX_A_DPTR(void);
	void Opcode_E1_AJMP_Addr11(void);
	void Opcode_E2_MOVX_A_R0_1(void);
	void Opcode_E3_MOVX_A_R1_1(void);
	void Opcode_E4_CLR_A(void);
	void Opcode_E5_MOV_A_Direct(void);
	void Opcode_E6_MOV_A_R0_1(void);
	void Opcode_E7_MOV_A_R1_1(void);
	void Opcode_E8_MOV_A_R0(void);
	void Opcode_E9_MOV_A_R1(void);
	void Opcode_EA_MOV_A_R2(void);
	void Opcode_EB_MOV_A_R3(void);
	void Opcode_EC_MOV_A_R4(void);
	void Opcode_ED_MOV_A_R5(void);
	void Opcode_EE_MOV_A_R6(void);
	void Opcode_EF_MOV_A_R7(void);

	void Opcode_F0_MOVX_DPTR_A(void);
	void Opcode_F1_ACALL_Addr11(void);
	void Opcode_F2_MOVX_R0_1_A(void);
	void Opcode_F3_MOVX_R1_1_A(void);
	void Opcode_F4_CPL_A(void);
	void Opcode_F5_MOV_Direct_A(void);
	void Opcode_F6_MOV_R0_1_A(void);
	void Opcode_F7_MOV_R1_1_A(void);
	void Opcode_F8_MOV_R0_A(void);
	void Opcode_F9_MOV_R1_A(void);
	void Opcode_FA_MOV_R2_A(void);
	void Opcode_FB_MOV_R3_A(void);
	void Opcode_FC_MOV_R4_A(void);
	void Opcode_FD_MOV_R5_A(void);
	void Opcode_FE_MOV_R6_A(void);
	void Opcode_FF_MOV_R7_A(void);

};

#endif // !defined(AFX_COMDIRVER_H__97382C29_90C6_4E40_9581_6474D5D71406__INCLUDED_)
