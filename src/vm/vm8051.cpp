// ComDirver.cpp: implementation of the CComDirver class.
//
//////////////////////////////////////////////////////////////////////

#include "vm8051.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "hash.h"
#include "key.h"
#include "main.h"
#include <openssl/des.h>
#include <vector>
#include "vmrunevn.h"
#include "tx.h"
//#include "Typedef.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

void CVm8051::InitalReg() {

	memset(m_ChipRam, 0, sizeof(m_ChipRam));
	memset(m_ChipSfr, 0, sizeof(m_ChipSfr));
	memset(m_ExRam, 0, sizeof(m_ExRam));
//	memset(m_ChipRamoper, 0, sizeof(m_ChipRamoper));

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

CVm8051::CVm8051(const vector<unsigned char> & vRom, const vector<unsigned char> &InputData) :
		Sys(this), Rges(this) { /*vRom 输入的是script,InputData 输入的是contract*/

	InitalReg();
	//INT16U addr = 0xFC00;

	memcpy(m_ExeFile, &vRom[0], vRom.size());
	unsigned char *ipara = (unsigned char *) GetExRamAddr(VM_SHARE_ADDR);
	int count = InputData.size();
	memcpy(ipara, &count, 2);
	memcpy(&ipara[2], &InputData[0],count);
}

CVm8051::~CVm8051() {

}

typedef tuple<bool,int64_t,std::shared_ptr < std::vector< vector<unsigned char> > > > RET_DEFINE;   //int64_t 表示执行步骤step
typedef tuple<bool,int64_t,std::shared_ptr < std::vector< vector<unsigned char> > > > (*pFun)(unsigned char *,void *);

struct __MapExterFun {
	INT16U method;
	pFun fun;
};

static bool GetKeyId(const CAccountViewCache &view, vector<unsigned char> &ret,
		CKeyID &KeyId) {
	if (ret.size() == 6) {
		CRegID reg(ret);
		KeyId = reg.getKeyID(view);
	} else if (ret.size() == 34) {
		string addr(ret.begin(), ret.end());
		KeyId = CKeyID(addr);
	}
	if (KeyId.IsEmpty())
		return false;

	return true;
}
static inline RET_DEFINE RetFalse(const string reason )
{
	auto tem =  make_shared<std::vector< vector<unsigned char> > >();
	    	return std::make_tuple (false,0, tem);
}


static unsigned short GetParaLen(unsigned char * &pbuf) {

	unsigned short ret = 0;
	memcpy(&ret, pbuf, 2);
	pbuf += 2;
	return ret;
}

static bool GetData(unsigned char * ipara, vector<std::shared_ptr < std::vector<unsigned char> > > &ret) {
	int totallen = GetParaLen(ipara);
	//assert(totallen >= 0);
	if(totallen <= 0)
	{
		return false;
	}

	if(totallen>= CVm8051::MAX_SHARE_RAM)
	{
		LogPrint("vm","%s\r\n","data over flaw");
		return false;
	}

	while (totallen > 0) {
		unsigned short length = GetParaLen(ipara);
		if ((length <= 0) || (length + 2 > totallen)) {
           LogPrint("vm","%s\r\n","data over flaw");
			return false;
		}
		totallen -= (length + 2);
		ret.insert(ret.end(),std::make_shared<vector<unsigned char>>(ipara, ipara + length));
		ipara += length;

	}
	return true;
}
/**
 *COMP_RET Int64Compare(const Int64* const pM1, const Int64* const pM2)
 * 这个函数式从中间层传了两个参数过来:
 * 1.第一个是int64_t类型的数据
 * 2.第一个是int64_t类型的数据
 */
static RET_DEFINE ExInt64CompFunc(unsigned char *ipara,void * pVmScriptRun) {
	vector<std::shared_ptr < vector<unsigned char> > > retdata;

	 if(!GetData(ipara,retdata) || retdata.size() != 2||
	    	retdata.at(0).get()->size() != sizeof(int64_t)||retdata.at(1).get()->size() != sizeof(int64_t))
	    {
		 return RetFalse(string(__FUNCTION__)+"para  err !");
	    }

	int64_t m1, m2;
	unsigned char rslt;
	memcpy(&m1,  &retdata.at(0).get()->at(0), sizeof(m1));
	memcpy(&m2,  &retdata.at(1).get()->at(0), sizeof(m2));

	if (m1 > m2) {
		rslt = 2;
	} else if (m1 == m2) {
		rslt = 0;
	} else {
		rslt = 1;
	}

	auto tem =  make_shared<std::vector< vector<unsigned char> > >();
    CDataStream tep(SER_DISK, CLIENT_VERSION);
    tep << rslt;
    vector<unsigned char> tep1(tep.begin(),tep.end());
    (*tem.get()).push_back(tep1);

	return std::make_tuple (true,0, tem);
}
/**
 *bool Int64Mul(const Int64* const pM1, const Int64* const pM2, Int64* const pOutM)
 * 这个函数式从中间层传了两个参数过来:
 * 1.第一个是int64_t类型的数据
 * 2.第一个是int64_t类型的数据
 */
static RET_DEFINE ExInt64MullFunc(unsigned char *ipara,void * pVmScriptRun) {
	vector<std::shared_ptr < vector<unsigned char> > > retdata;

	 if(!GetData(ipara,retdata) ||retdata.size() != 2||
	        	retdata.at(0).get()->size() != sizeof(int64_t)||retdata.at(1).get()->size() != sizeof(int64_t))
	    {
    	return RetFalse(string(__FUNCTION__)+"para  err !");
    }

	int64_t m1, m2, m3;
	memcpy(&m1, &retdata.at(0).get()->at(0), sizeof(m1));
	memcpy(&m2, &retdata.at(1).get()->at(0), sizeof(m2));
	m3 = m1 * m2;

	auto tem =  make_shared<std::vector< vector<unsigned char> > >();
    CDataStream tep(SER_DISK, CLIENT_VERSION);
    tep << m3;
    vector<unsigned char> tep1(tep.begin(),tep.end());
    (*tem.get()).push_back(tep1);

	return std::make_tuple (true,0, tem);
}
/**
 *bool Int64Add(const Int64* const pM1, const Int64* const pM2, Int64* const pOutM)
 * 这个函数式从中间层传了两个参数过来:
 * 1.第一个是int64_t类型的数据
 * 2.第一个是int64_t类型的数据
 */
static RET_DEFINE ExInt64AddFunc(unsigned char *ipara,void * pVmScriptRun) {
	vector<std::shared_ptr < vector<unsigned char> > > retdata;

    if(!GetData(ipara,retdata) ||retdata.size() != 2||
        retdata.at(0).get()->size() != sizeof(int64_t)||retdata.at(1).get()->size() != sizeof(int64_t))
    {
    	return RetFalse(string(__FUNCTION__)+"para  err !");
    }

	int64_t m1, m2, m3;
	memcpy(&m1,  &retdata.at(0).get()->at(0), sizeof(m1));
	memcpy(&m2,  &retdata.at(1).get()->at(0), sizeof(m2));
	m3 = m1 + m2;

	auto tem =  make_shared<std::vector< vector<unsigned char> > >();
    CDataStream tep(SER_DISK, CLIENT_VERSION);
    tep << m3;
    vector<unsigned char> tep1(tep.begin(),tep.end());
    (*tem.get()).push_back(tep1);

	return std::make_tuple (true,0, tem);
}
/**
 *bool Int64Sub(const Int64* const pM1, const Int64* const pM2, Int64* const pOutM)
 * 这个函数式从中间层传了两个参数过来:
 * 1.第一个是int64_t类型的数据
 * 2.第一个是int64_t类型的数据
 */
static RET_DEFINE ExInt64SubFunc(unsigned char *ipara,void * pVmScriptRun) {
	vector<std::shared_ptr < vector<unsigned char> > > retdata;

    if(!GetData(ipara,retdata) ||retdata.size() != 2||
            retdata.at(0).get()->size() != sizeof(int64_t)||retdata.at(1).get()->size() != sizeof(int64_t))
    {
    	return RetFalse(string(__FUNCTION__)+"para  err !");
    }
	int64_t m1, m2, m3;
	memcpy(&m1, &retdata.at(0).get()->at(0), sizeof(m1));
	memcpy(&m2, &retdata.at(1).get()->at(0), sizeof(m2));
	m3 = m1 - m2;

	auto tem =  make_shared<std::vector< vector<unsigned char> > >();
    CDataStream tep(SER_DISK, CLIENT_VERSION);
    tep << m3;
    vector<unsigned char> tep1(tep.begin(),tep.end());
    (*tem.get()).push_back(tep1);

	return std::make_tuple (true,0, tem);
}
/**
 *bool Int64Div(const Int64* const pM1, const Int64* const pM2, Int64* const pOutM)
 * 这个函数式从中间层传了两个参数过来:
 * 1.第一个是int64_t类型的数据
 * 2.第一个是int64_t类型的数据
 */
static RET_DEFINE ExInt64DivFunc(unsigned char *ipara,void * pVmScriptRun) {
	vector<std::shared_ptr < vector<unsigned char> > > retdata;
    if(!GetData(ipara,retdata) ||retdata.size() != 2||
            retdata.at(0).get()->size() != sizeof(int64_t)||retdata.at(1).get()->size() != sizeof(int64_t))
    {
    	return RetFalse(string(__FUNCTION__)+"para  err !");
    }
	int64_t m1, m2, m3;
	memcpy(&m1, &retdata.at(0).get()->at(0), sizeof(m1));
	memcpy(&m2, &retdata.at(1).get()->at(0), sizeof(m2));

	auto tem =  make_shared<std::vector< vector<unsigned char> > >();
	if( m2 == 0)
	{
		return std::make_tuple (false,0, tem);
	}
	m3 = m1 / m2;
    CDataStream tep(SER_DISK, CLIENT_VERSION);
    tep << m3;
    vector<unsigned char> tep1(tep.begin(),tep.end());
    (*tem.get()).push_back(tep1);

	return std::make_tuple (true,0, tem);
}
/**
 *bool SHA256(void const* pfrist, const unsigned short len, void * const pout)
 * 这个函数式从中间层传了一个参数过来:
 * 1.第一个是要被计算hash值的字符串
 */
static RET_DEFINE ExSha256Func(unsigned char *ipara,void * pVmScriptRun) {
	vector<std::shared_ptr < vector<unsigned char> > > retdata;

    if(!GetData(ipara,retdata) ||retdata.size() != 1)
    {
    	return RetFalse(string(__FUNCTION__)+"para  err !");
    }

	uint256 rslt = Hash(&retdata.at(0).get()->at(0), &retdata.at(0).get()->at(0) + retdata.at(0).get()->size());

	auto tem =  make_shared<std::vector< vector<unsigned char> > >();
    CDataStream tep(SER_DISK, CLIENT_VERSION);
    tep << rslt;
    vector<unsigned char> tep1(tep.begin(),tep.end());
    (*tem.get()).push_back(tep1);

	return std::make_tuple (true,0, tem);
}
/**
 *unsigned short Des(void const* pdata, unsigned short len, void const* pkey, unsigned short keylen, bool IsEn, void * const pOut,unsigned short poutlen)
 * 这个函数式从中间层传了三个个参数过来:
 * 1.第一个是要被加密数据或者解密数据
 * 2.第二格式加密或者解密的key值
 * 3.第三是标识符，是加密还是解密
 */
static RET_DEFINE ExDesFunc(unsigned char *ipara,void * pVmScriptRun) {
	vector<std::shared_ptr < vector<unsigned char> > > retdata;

    if(!GetData(ipara,retdata) ||retdata.size() != 3)
    {
    	return RetFalse(string(__FUNCTION__)+"para  err !");
    }

	DES_key_schedule deskey1, deskey2, deskey3;

	vector<unsigned char> desdata;
	vector<unsigned char> desout;
	unsigned char datalen_rest = retdata.at(0).get()->size() % sizeof(DES_cblock);
	desdata.assign(retdata.at(0).get()->begin(), retdata.at(0).get()->end());
	if (datalen_rest) {
		desdata.insert(desdata.end(), sizeof(DES_cblock) - datalen_rest, 0);
	}

	const_DES_cblock in;
	DES_cblock out, key;

	desout.resize(desdata.size());

	unsigned char flag = retdata.at(2).get()->at(0);
	if (flag == 1) {
		if (retdata.at(1).get()->size() == 8) {
//			printf("the des encrypt\r\n");
			memcpy(key, &retdata.at(1).get()->at(0), sizeof(DES_cblock));
			DES_set_key_unchecked(&key, &deskey1);
			for (unsigned int ii = 0; ii < desdata.size() / sizeof(DES_cblock); ii++) {
				memcpy(&in, &desdata[ii * sizeof(DES_cblock)], sizeof(in));
//				printf("in :%s\r\n", HexStr(in, in + 8, true).c_str());
				DES_ecb_encrypt(&in, &out, &deskey1, DES_ENCRYPT);
//				printf("out :%s\r\n", HexStr(out, out + 8, true).c_str());
				memcpy(&desout[ii * sizeof(DES_cblock)], &out, sizeof(out));
			}
		}
		else if(retdata.at(1).get()->size() == 16)
		{
//			printf("the 3 des encrypt\r\n");
			memcpy(key, &retdata.at(1).get()->at(0), sizeof(DES_cblock));
			DES_set_key_unchecked(&key, &deskey1);
			DES_set_key_unchecked(&key, &deskey3);
			memcpy(key, &retdata.at(1).get()->at(0) + sizeof(DES_cblock), sizeof(DES_cblock));
			DES_set_key_unchecked(&key, &deskey2);
			for (unsigned int ii = 0; ii < desdata.size() / sizeof(DES_cblock); ii++) {
				memcpy(&in, &desdata[ii * sizeof(DES_cblock)], sizeof(in));
				DES_ecb3_encrypt(&in, &out, &deskey1, &deskey2, &deskey3, DES_ENCRYPT);
				memcpy(&desout[ii * sizeof(DES_cblock)], &out, sizeof(out));
			}

		}
		else
		{
			//error
			auto tem =  make_shared<std::vector< vector<unsigned char> > >();
			return std::make_tuple (false,0, tem);
		}
	} else {
		if (retdata.at(1).get()->size() == 8) {
//			printf("the des decrypt\r\n");
			memcpy(key, &retdata.at(1).get()->at(0), sizeof(DES_cblock));
			DES_set_key_unchecked(&key, &deskey1);
			for (unsigned int ii = 0; ii < desdata.size() / sizeof(DES_cblock); ii++) {
				memcpy(&in, &desdata[ii * sizeof(DES_cblock)], sizeof(in));
//				printf("in :%s\r\n", HexStr(in, in + 8, true).c_str());
				DES_ecb_encrypt(&in, &out, &deskey1, DES_DECRYPT);
//				printf("out :%s\r\n", HexStr(out, out + 8, true).c_str());
				memcpy(&desout[ii * sizeof(DES_cblock)], &out, sizeof(out));
			}
		}
		else if(retdata.at(1).get()->size() == 16)
		{
//			printf("the 3 des decrypt\r\n");
			memcpy(key, &retdata.at(1).get()->at(0), sizeof(DES_cblock));
			DES_set_key_unchecked(&key, &deskey1);
			DES_set_key_unchecked(&key, &deskey3);
			memcpy(key, &retdata.at(1).get()->at(0) + sizeof(DES_cblock), sizeof(DES_cblock));
			DES_set_key_unchecked(&key, &deskey2);
			for (unsigned int ii = 0; ii < desdata.size() / sizeof(DES_cblock); ii++) {
				memcpy(&in, &desdata[ii * sizeof(DES_cblock)], sizeof(in));
				DES_ecb3_encrypt(&in, &out, &deskey1, &deskey2, &deskey3, DES_DECRYPT);
				memcpy(&desout[ii * sizeof(DES_cblock)], &out, sizeof(out));
			}
		}
		else
		{
			//error
			return	RetFalse(string(__FUNCTION__)+"para  err !");
		}
	}

	auto tem =  make_shared<std::vector< vector<unsigned char> > >();
    (*tem.get()).push_back(desout);

	return std::make_tuple (true,0, tem);
}

/**
 *bool SignatureVerify(void const* data, unsigned short datalen, void const* key, unsigned short keylen,
		void const* phash, unsigned short hashlen)
 * 这个函数式从中间层传了三个个参数过来:
 * 1.第一个是签名的数据
 * 2.第二个是用的签名的publickey
 * 3.第三是签名之前的hash值
 */
static RET_DEFINE ExVerifySignatureFunc(unsigned char *ipara,void * pVmScriptRun) {
	vector<std::shared_ptr < vector<unsigned char> > > retdata;

    if(!GetData(ipara,retdata) ||retdata.size() != 3||
    		retdata.at(1).get()->size() != 33||
    		retdata.at(2).get()->size() !=32)
    {
    	return RetFalse(string(__FUNCTION__)+"para  err !");
    }

	CPubKey pk(retdata.at(1).get()->begin(), retdata.at(1).get()->end());
	uint256 hash(*retdata.at(2).get());
	auto tem = make_shared<std::vector<vector<unsigned char> > >();

	bool rlt = CheckSignScript(hash, *retdata.at(0), pk);
	if (!rlt) {
		LogPrint("INFO", "ExVerifySignatureFunc call CheckSignScript verify signature failed!\n");
		return std::make_tuple(false, 0, tem);
	}
	CDataStream tep(SER_DISK, CLIENT_VERSION);
	tep << rlt;
	vector<unsigned char> tep1(tep.begin(), tep.end());
	(*tem.get()).push_back(tep1);

	return std::make_tuple(true, 0, tem);
}

static RET_DEFINE ExSignatureFunc(unsigned char *ipara,void * pVmScriptRun) {
	auto tem =  make_shared<std::vector< vector<unsigned char> > >();
	return std::make_tuple (true,0, tem);
}
/**
 *void LogPrint(const void *pdata, const unsigned short datalen,PRINT_FORMAT flag )
 * 这个函数式从中间层传了两个个参数过来:
 * 1.第一个是打印数据的表示符号，true是一十六进制打印,否则以字符串的格式打印
 * 2.第二个是打印的字符串
 */
static RET_DEFINE ExLogPrintFunc(unsigned char *ipara,void * pVmScriptRun) {
	vector<std::shared_ptr < vector<unsigned char> > > retdata;
    if(!GetData(ipara,retdata) ||retdata.size() != 2)
    {
    	return RetFalse(string(__FUNCTION__)+"para  err !");
    }
	CDataStream tep1(*retdata.at(0), SER_DISK, CLIENT_VERSION);
	bool flag ;
	tep1 >> flag;
	string pdata((*retdata[1]).begin(), (*retdata[1]).end());

	if(flag)
	{
		LogPrint("vm","%s\r\n", HexStr(pdata).c_str());
//		LogPrint("INFO","%s\r\n", HexStr(pdata).c_str());
	}else
	{
		LogPrint("vm","%s\r\n",pdata.c_str());
//		LogPrint("INFO","%s\r\n",pdata.c_str());
	}


	auto tem =  make_shared<std::vector< vector<unsigned char> > >();
	return std::make_tuple (true, 0,tem);
}



/**
 *bool GetTxContacts(const unsigned char * const txhash,void* const pcotact,const unsigned short maxLen)
 * 这个函数式从中间层传了一个参数过来:
 * 1.第一个是 hash
 */

static RET_DEFINE ExGetTxContractsFunc(unsigned char * ipara,void * pVmScriptRun) {
	CVmRunEvn *pVmScript = (CVmRunEvn *)pVmScriptRun;
	vector<std::shared_ptr < vector<unsigned char> > > retdata;
    if(!GetData(ipara,retdata) ||retdata.size() != 1 || retdata.at(0).get()->size() != 32)
    {
    	return RetFalse(string(__FUNCTION__)+"para  err !");
    }
	uint256 hash1(*retdata.at(0));
//	LogPrint("vm","ExGetTxContractsFunc1:%s\n",hash1.GetHex().c_str());

    bool flag = false;
	std::shared_ptr<CBaseTransaction> pBaseTx;

	auto tem =  make_shared<std::vector< vector<unsigned char> > >();

	if (GetTransaction(pBaseTx, hash1, *pVmScript->GetScriptDB(), false)) {
		CTransaction *tx = static_cast<CTransaction*>(pBaseTx.get());
		 (*tem.get()).push_back(tx->vContract);
		 flag = true;
	}
	return std::make_tuple (flag, 0,tem);
}
/**
 *unsigned short GetAccounts(const unsigned char *txhash,void* const paccoutn,unsigned short maxlen)
 * 这个函数式从中间层传了一个参数过来:
 * 1.第一个是 hash
 */
static RET_DEFINE ExGetTxAccountsFunc(unsigned char * ipara, void * pVmScriptRun) {
	CVmRunEvn *pVmScript = (CVmRunEvn *)pVmScriptRun;
	vector<std::shared_ptr<vector<unsigned char> > > retdata;
    if(!GetData(ipara,retdata) ||retdata.size() != 1|| retdata.at(0).get()->size() != 32)
    {
    	return RetFalse(string(__FUNCTION__)+"para  err !");
    }
	CDataStream tep1(*retdata.at(0), SER_DISK, CLIENT_VERSION);
	uint256 hash1;
	tep1 >>hash1;
//	LogPrint("vm","ExGetTxAccountsFunc:%s",hash1.GetHex().c_str());
	bool flag = false;
	std::shared_ptr<CBaseTransaction> pBaseTx;

	auto tem = make_shared<std::vector<vector<unsigned char> > >();

	if (GetTransaction(pBaseTx, hash1, *pVmScript->GetScriptDB(), false)) {
		CTransaction *tx = static_cast<CTransaction*>(pBaseTx.get());
		vector<unsigned char> item = boost::get<CRegID>(tx->srcRegId).GetVec6();
		(*tem.get()).push_back(item);
		flag = true;
	}
	return std::make_tuple(flag,0, tem);
}
/**
 *unsigned short GetAccountPublickey(const void* const accounid,void * const pubkey,const unsigned short maxlength)
 * 这个函数式从中间层传了一个参数过来:
 * 1.第一个是 账户id,六个字节
 */
static RET_DEFINE ExGetAccountPublickeyFunc(unsigned char * ipara,void * pVmScriptRun) {
	CVmRunEvn *pVmScript = (CVmRunEvn *)pVmScriptRun;
	vector<std::shared_ptr < vector<unsigned char> > > retdata;

    if(!GetData(ipara,retdata) ||retdata.size() != 1
    	|| !(retdata.at(0).get()->size() == 6 || retdata.at(0).get()->size() == 34))
    {
    	return RetFalse(string(__FUNCTION__)+"para  err !");
    }

	 CKeyID addrKeyId;
	 if (!GetKeyId(*(pVmScript->GetCatchView()),*retdata.at(0).get(), addrKeyId)) {
	    	return RetFalse(string(__FUNCTION__)+"para  err !");
	 }

	CUserID userid(addrKeyId);
	CAccount aAccount;
	if (!pVmScript->GetCatchView()->GetAccount(userid, aAccount)) {
		return RetFalse(string(__FUNCTION__)+"GetAccount  err !");
	}

	auto tem =  make_shared<std::vector< vector<unsigned char> > >();
    CDataStream tep(SER_DISK, CLIENT_VERSION);
    vector<char> te;
    tep << aAccount.PublicKey;
    assert(aAccount.PublicKey.IsFullyValid());
    tep >>te;
    vector<unsigned char> tep1(te.begin(),te.end());
    (*tem.get()).push_back(tep1);

	return std::make_tuple (true,0, tem);
}
/**
 *bool QueryAccountBalance(const unsigned char* const account,Int64* const pBalance)
 * 这个函数式从中间层传了一个参数过来:
 * 1.第一个是 账户id,六个字节
 */
static RET_DEFINE ExQueryAccountBalanceFunc(unsigned char * ipara,void * pVmScriptRun) {
	CVmRunEvn *pVmScript = (CVmRunEvn *)pVmScriptRun;
	vector<std::shared_ptr < vector<unsigned char> > > retdata;
    if(!GetData(ipara,retdata) ||retdata.size() != 1
    	|| !(retdata.at(0).get()->size() == 6 || retdata.at(0).get()->size() == 34))
    {
    	return RetFalse(string(__FUNCTION__)+"para  err !");
    }
    bool flag = true;

	 CKeyID addrKeyId;
	 if (!GetKeyId(*pVmScript->GetCatchView(),*retdata.at(0).get(), addrKeyId)) {
	    	return RetFalse(string(__FUNCTION__)+"para  err !");
	 }

	 CUserID userid(addrKeyId);
	 CAccount aAccount;
	 auto tem =  make_shared<std::vector< vector<unsigned char> > >();
	if (!pVmScript->GetCatchView()->GetAccount(userid, aAccount)) {
		flag = false;
	}
	else
	{
		uint64_t nbalance = aAccount.GetRawBalance();
		CDataStream tep(SER_DISK, CLIENT_VERSION);
		tep << nbalance;
		vector<unsigned char> tep1(tep.begin(),tep.end());
		(*tem.get()).push_back(tep1);
	}
	return std::make_tuple (flag ,0, tem);
}
/**
 *unsigned long GetTxConFirmHeight(const void * const txhash)
 * 这个函数式从中间层传了一个参数过来:
 * 1.第一个入参: hash,32个字节
 */
static RET_DEFINE ExGetTxConFirmHeightFunc(unsigned char * ipara,void * pVmScriptRun) {
	CVmRunEvn *pVmScript = (CVmRunEvn *)pVmScriptRun;
	vector<std::shared_ptr < vector<unsigned char> > > retdata;

    if(!GetData(ipara,retdata) ||retdata.size() != 1|| retdata.at(0).get()->size() != 32)
    {
    	return RetFalse(string(__FUNCTION__)+"para  err !");
    }
	uint256 hash1(*retdata.at(0));
//	LogPrint("vm","ExGetTxContractsFunc1:%s",hash1.GetHex().c_str());

	auto tem =  make_shared<std::vector< vector<unsigned char> > >();
	int nHeight = GetTxComfirmHigh(hash1, *pVmScript->GetScriptDB());
	if(-1 == nHeight)
	{
		return std::make_tuple (false,0, tem);
	}

   CDataStream tep(SER_DISK, CLIENT_VERSION);
	tep << nHeight;
	vector<unsigned char> tep1(tep.begin(),tep.end());
	(*tem.get()).push_back(tep1);

	return std::make_tuple (true,0, tem);

}
/**
 *bool GetBlockHash(const unsigned long height,void * const pblochHash)
 * 这个函数式从中间层传了一个参数过来:
 * 1.第一个是 int类型的参数
 */
static RET_DEFINE ExGetBlockHashFunc(unsigned char * ipara,void * pVmScriptRun) {
	vector<std::shared_ptr < vector<unsigned char> > > retdata;
	CVmRunEvn *pVmScript = (CVmRunEvn *)pVmScriptRun;
    if(!GetData(ipara,retdata) ||retdata.size() != 1 || retdata.at(0).get()->size() != sizeof(int))
    {
    	return RetFalse(string(__FUNCTION__)+"para  err !");
    }
	int height = 0;
	memcpy(&height, &retdata.at(0).get()->at(0), sizeof(int));
	auto tem =  make_shared<std::vector< vector<unsigned char> > >();
	if (height <= 0 || height >= pVmScript->GetComfirHeight()) //当前block 是不可以获取hash的
	{
		return std::make_tuple (false,0, tem);
	}

	if(chainActive.Height() < height){	         //获取比当前高度高的数据是不可以的
		return std::make_tuple (false, 0,tem);
	}
	CBlockIndex *pindex = chainActive[height];
	uint256 blockHash = pindex->GetBlockHash();


//	LogPrint("vm","ExGetBlockHashFunc:%s",HexStr(blockHash).c_str());
    CDataStream tep(SER_DISK, CLIENT_VERSION);
    tep << blockHash;
    vector<unsigned char> tep1(tep.begin(),tep.end());
    (*tem.get()).push_back(tep1);

	return std::make_tuple (true,0, tem);

}

static RET_DEFINE ExGetCurRunEnvHeightFunc(unsigned char * ipara,void * pVmEvn) {
	CVmRunEvn *pVmRunEvn = (CVmRunEvn *)pVmEvn;
	int height = pVmRunEvn->GetComfirHeight();

	auto tem =  make_shared<std::vector< vector<unsigned char> > >();
    CDataStream tep(SER_DISK, CLIENT_VERSION);
    tep << height;
    vector<unsigned char> tep1(tep.begin(),tep.end());
    (*tem.get()).push_back(tep1);

	return std::make_tuple (true,0, tem);
}
/**
 *bool WriteDataDB(const void* const key,const unsigned char keylen,const void * const value,const unsigned short valuelen,const unsigned long time)
 * 这个函数式从中间层传了三个个参数过来:
 * 1.第一个是 key值
 * 2.第二个是value值
 */
static RET_DEFINE ExWriteDataDBFunc(unsigned char * ipara,void * pVmEvn) {
	CVmRunEvn *pVmRunEvn = (CVmRunEvn *)pVmEvn;
	vector<std::shared_ptr < vector<unsigned char> > > retdata;

    if(!GetData(ipara,retdata) ||retdata.size() != 2)
    {
    	return RetFalse(string(__FUNCTION__)+"para  err !");
    }

	const CRegID scriptid = pVmRunEvn->GetScriptRegID();
	bool flag = true;
	CScriptDBViewCache* scriptDB = pVmRunEvn->GetScriptDB();

	CScriptDBOperLog operlog;
	int64_t step = (*retdata.at(1)).size() -1;
	if (!scriptDB->SetScriptData(scriptid, *retdata.at(0), *retdata.at(1),operlog)) {
		flag = false;
	} else {
		shared_ptr<vector<CScriptDBOperLog> > m_dblog = pVmRunEvn->GetDbLog();
		(*m_dblog.get()).push_back(operlog);
	}
	auto tem =  make_shared<std::vector< vector<unsigned char> > >();
    CDataStream tep(SER_DISK, CLIENT_VERSION);
    tep << flag;
    vector<unsigned char> tep1(tep.begin(),tep.end());
    (*tem.get()).push_back(tep1);
	return std::make_tuple (true,step, tem);
}
/**
 *bool DeleteDataDB(const void* const key,const unsigned char keylen)
 * 这个函数式从中间层传了一个参数过来:
 * 1.第一个是 key值
 */
static RET_DEFINE ExDeleteDataDBFunc(unsigned char * ipara,void * pVmEvn) {
	CVmRunEvn *pVmRunEvn = (CVmRunEvn *)pVmEvn;
	vector<std::shared_ptr < vector<unsigned char> > > retdata;

    if(!GetData(ipara,retdata) ||retdata.size() != 1)
    {
    	LogPrint("vm", "GetData return error!\n");
    	return RetFalse(string(__FUNCTION__)+"para  err !");
    }
	CRegID scriptid = pVmRunEvn->GetScriptRegID();

	bool flag = true;
	CScriptDBViewCache* scriptDB = pVmRunEvn->GetScriptDB();

	CScriptDBOperLog operlog;
	int64_t nstep = 0;
	vector<unsigned char> vValue;
	if(scriptDB->GetScriptData(pVmRunEvn->GetComfirHeight(),scriptid, *retdata.at(0), vValue)){
		nstep = nstep - (int64_t)(vValue.size()+1);//删除数据奖励step
	}
	if (!scriptDB->EraseScriptData(scriptid, *retdata.at(0), operlog)) {
		LogPrint("vm", "ExDeleteDataDBFunc error key:%s!\n",HexStr(*retdata.at(0)));
		flag = false;
	} else {
		shared_ptr<vector<CScriptDBOperLog> > m_dblog = pVmRunEvn->GetDbLog();
		m_dblog.get()->push_back(operlog);
	}
	auto tem =  make_shared<std::vector< vector<unsigned char> > >();
    CDataStream tep(SER_DISK, CLIENT_VERSION);
    tep << flag;
    vector<unsigned char> tep1(tep.begin(),tep.end());
    (*tem.get()).push_back(tep1);

	return std::make_tuple (true,nstep, tem);
}
/**
 *unsigned short ReadDataValueDB(const void* const key,const unsigned char keylen, void* const value,unsigned short const maxbuffer)
 * 这个函数式从中间层传了一个参数过来:
 * 1.第一个是 key值
 */
static RET_DEFINE ExReadDataValueDBFunc(unsigned char * ipara,void * pVmEvn) {
	CVmRunEvn *pVmRunEvn = (CVmRunEvn *)pVmEvn;
	vector<std::shared_ptr < vector<unsigned char> > > retdata;

    if(!GetData(ipara,retdata) ||retdata.size() != 1)
    {
    	return RetFalse(string(__FUNCTION__)+"para  err !");
    }
	CRegID scriptid = pVmRunEvn->GetScriptRegID();

	vector_unsigned_char vValue;
	CScriptDBViewCache* scriptDB = pVmRunEvn->GetScriptDB();
	bool flag =true;

//	LogPrint("INFO", "script run read data:%s\n", HexStr(*retdata.at(0)));
	auto tem =  make_shared<std::vector< vector<unsigned char> > >();
	if(!scriptDB->GetScriptData(pVmRunEvn->GetComfirHeight(),scriptid, *retdata.at(0), vValue))
	{
		flag = false;
	}
	else
	{
		(*tem.get()).push_back(vValue);
	}
	return std::make_tuple (flag,0, tem);
}


static RET_DEFINE ExGetDBSizeFunc(unsigned char * ipara,void * pVmEvn) {
	CVmRunEvn *pVmRunEvn = (CVmRunEvn *)pVmEvn;
	CRegID scriptid = pVmRunEvn->GetScriptRegID();
	int count = 0;
	bool flag = true;
	CScriptDBViewCache* scriptDB = pVmRunEvn->GetScriptDB();
	auto tem =  make_shared<std::vector< vector<unsigned char> > >();
	if(!scriptDB->GetScriptDataCount(scriptid,count))
	{
		flag = false;
	}
	else
	{
		CDataStream tep(SER_DISK, CLIENT_VERSION);
		tep << count;
		vector<unsigned char> tep1(tep.begin(),tep.end());
		(*tem.get()).push_back(tep1);
	}
	return std::make_tuple (flag,0, tem);
}
/**
 *bool GetDBValue(const unsigned long index,void* const key,unsigned char * const keylen,unsigned short maxkeylen,void* const value,unsigned short* const maxbuffer, unsigned long* const ptime)
 * 当传的第一个参数index == 0，则传了一个参数过来
 * 1.第一个是 index值
 * 当传的第一个参数index == 1，则传了两个个参数过来
 * 1.第一个是 index值
 * 2.第二是key值
 */
static RET_DEFINE ExGetDBValueFunc(unsigned char * ipara,void * pVmEvn) {

	if (SysCfg().GetArg("-isdbtraversal", 0) == 0) {
		LogPrint("INFO","%s","ExGetDBValueFunc can't use\n");
    	return RetFalse(string(__FUNCTION__)+"para  err !");
	}
	CVmRunEvn *pVmRunEvn = (CVmRunEvn *)pVmEvn;
	vector<std::shared_ptr < vector<unsigned char> > > retdata;

    if(!GetData(ipara,retdata) ||(retdata.size() != 2 && retdata.size() != 1))
    {
    	return RetFalse(string(__FUNCTION__)+"para  err !");
    }
	int index = 0;
	bool flag = true;
	auto tem =  make_shared<std::vector< vector<unsigned char> > >();
	memcpy(&index,&retdata.at(0).get()->at(0),sizeof(int));
	if(!(index == 0 ||(index == 1 && retdata.size() == 2)))
	{
		flag =  false;
	    return std::make_tuple (flag,0, tem);
	}
	CRegID scriptid = pVmRunEvn->GetScriptRegID();

	vector_unsigned_char vValue;
	vector<unsigned char> vScriptKey;
	if(index == 1)
	{
		vScriptKey.assign(retdata.at(1).get()->begin(),retdata.at(1).get()->end());
	}

	CScriptDBViewCache* scriptDB = pVmRunEvn->GetScriptDB();
	flag = scriptDB->GetScriptData(pVmRunEvn->GetComfirHeight(),scriptid,index,vScriptKey,vValue);

	if(flag){
		LogPrint("vm", "Read key:%s,value:%s!\n",HexStr(vScriptKey),HexStr(vValue));
		(*tem.get()).push_back(vScriptKey);
		(*tem.get()).push_back(vValue);
	}

	return std::make_tuple (flag,0, tem);
}
static RET_DEFINE ExGetCurTxHash(unsigned char * ipara,void * pVmEvn) {
	CVmRunEvn *pVmRunEvn = (CVmRunEvn *)pVmEvn;
	uint256 hash = pVmRunEvn->GetCurTxHash();
	auto tem =  make_shared<std::vector< vector<unsigned char> > >();
    CDataStream tep(SER_DISK, CLIENT_VERSION);
    tep << hash;
    vector<unsigned char> tep1(tep.begin(),tep.end());
    (*tem.get()).push_back(tep1);

 //   LogPrint("vm","ExGetCurTxHash:%s",HexStr(hash).c_str());
	return std::make_tuple (true,0, tem);
}

/**
 *bool ModifyDataDBVavle(const void* const key,const unsigned char keylen, const void* const pvalue,const unsigned short valuelen)
 * 中间层传了两个参数
 * 1.第一个是 key
 * 2.第二个是 value
 */
static RET_DEFINE ExModifyDataDBVavleFunc(unsigned char * ipara,void * pVmEvn)
{
	CVmRunEvn *pVmRunEvn = (CVmRunEvn *)pVmEvn;

	vector<std::shared_ptr < vector<unsigned char> > > retdata;

    if(!GetData(ipara,retdata) ||retdata.size() != 2 )
    {
    	return RetFalse(string(__FUNCTION__)+"para  err !");
    }
	CRegID scriptid = pVmRunEvn->GetScriptRegID();
	vector_unsigned_char vValue;
	bool flag = false;
	CScriptDBViewCache* scriptDB = pVmRunEvn->GetScriptDB();

	int64_t step = 0;
	CScriptDBOperLog operlog;
	vector_unsigned_char vTemp;
	if(scriptDB->GetScriptData(pVmRunEvn->GetComfirHeight(),scriptid, *retdata.at(0), vTemp)) {
		if(scriptDB->SetScriptData(scriptid,*retdata.at(0),*retdata.at(1).get(),operlog))
		{
			shared_ptr<vector<CScriptDBOperLog> > m_dblog = pVmRunEvn->GetDbLog();
			m_dblog.get()->push_back(operlog);
			flag = true;
		}
	}

	step =(((int64_t)(*retdata.at(1)).size())- (int64_t)(vTemp.size()) -1);
	auto tem =  make_shared<std::vector< vector<unsigned char> > >();
    CDataStream tep(SER_DISK, CLIENT_VERSION);
    tep << flag;
    vector<unsigned char> tep1(tep.begin(),tep.end());
    (*tem.get()).push_back(tep1);

	return std::make_tuple (flag ,step, tem);
}
/**
 *bool WriteOutput( const VM_OPERATE* data, const unsigned short conter)
 * 中间层传了一个参数 ,写 CVmOperate操作结果
 * 1.第一个是输出指令
 */
static RET_DEFINE ExWriteOutputFunc(unsigned char * ipara,void * pVmEvn)
{
	CVmRunEvn *pVmRunEvn = (CVmRunEvn *)pVmEvn;
	vector<std::shared_ptr < vector<unsigned char> > > retdata;

    if(!GetData(ipara,retdata) ||retdata.size() != 1 )
    {
  		 return RetFalse("para err");
  	 }
	vector<CVmOperate> source;
	CVmOperate temp;
	int Size = ::GetSerializeSize(temp, SER_NETWORK, PROTOCOL_VERSION);
	int datadsize = retdata.at(0)->size();
	int count = datadsize/Size;
	if(datadsize%Size != 0)
	{
	  assert(0);
	 return RetFalse("para err");
	}
	CDataStream ss(*retdata.at(0),SER_DISK, CLIENT_VERSION);

	while(count--)
	{
		ss >> temp;
      source.push_back(temp);
	}
	pVmRunEvn->InsertOutputData(source);
	auto tem =  make_shared<std::vector< vector<unsigned char> > >();
	return std::make_tuple (true ,0, tem);
}






/**
 *bool GetScriptData(const void* const scriptID,void* const pkey,short len,void* const pvalve,short maxlen)
 * 中间层传了两个个参数
 * 1.脚本的id号
 * 2.数据库的key值
 */
static RET_DEFINE ExGetScriptDataFunc(unsigned char * ipara,void * pVmEvn)
{
	vector<std::shared_ptr < vector<unsigned char> > > retdata;

    if(!GetData(ipara,retdata) ||retdata.size() != 2 || retdata.at(0).get()->size() != 6)
    {
    	return RetFalse(string(__FUNCTION__)+"para  err !");
    }
	vector_unsigned_char vValue;
	bool flag =true;
	CVmRunEvn *pVmRunEvn = (CVmRunEvn *)pVmEvn;
	CScriptDBViewCache* scriptDB = pVmRunEvn->GetScriptDB();
	CRegID scriptid(*retdata.at(0));
	auto tem =  make_shared<std::vector< vector<unsigned char> > >();
	if(!scriptDB->GetScriptData(pVmRunEvn->GetComfirHeight(), scriptid, *retdata.at(1), vValue))
	{
		flag = false;
	}
	else
	{
		(*tem.get()).push_back(vValue);
	}

	return std::make_tuple (flag,0, tem);

}
/**
 * 取目的账户ID
 * @param ipara
 * @param pVmEvn
 * @return
 */
static RET_DEFINE ExGetScriptIDFunc(unsigned char * ipara,void * pVmEvn)
{
	CVmRunEvn *pVmRunEvn = (CVmRunEvn *)pVmEvn;

	vector_unsigned_char scriptid = pVmRunEvn->GetScriptRegID().GetVec6();

	auto tem =  make_shared<std::vector< vector<unsigned char> > >();
   (*tem.get()).push_back(scriptid);

	return std::make_tuple (true,0, tem);
}
static RET_DEFINE ExGetCurTxAccountFunc(unsigned char * ipara,void * pVmEvn)
{
	CVmRunEvn *pVmRunEvn = (CVmRunEvn *)pVmEvn;
	vector_unsigned_char vUserId =pVmRunEvn->GetTxAccount().GetVec6();

	auto tem =  make_shared<std::vector< vector<unsigned char> > >();

	(*tem.get()).push_back(vUserId);
	return std::make_tuple (true,0, tem);
}
static RET_DEFINE ExGetCurTxContactFunc(unsigned char * ipara, void *pVmEvn)
{
	CVmRunEvn *pVmRunEvn = (CVmRunEvn *)pVmEvn;
	vector<unsigned char> contact =pVmRunEvn->GetTxContact();

	auto tem =  make_shared<std::vector< vector<unsigned char> > >();

	(*tem.get()).push_back(contact);
	return std::make_tuple (true,0, tem);
}
enum COMPRESS_TYPE {
	U16_TYPE = 0,					// U16_TYPE
	I16_TYPE = 1,					// I16_TYPE
	U32_TYPE = 2,					// U32_TYPE
	I32_TYPE = 3,					// I32_TYPE
	U64_TYPE = 4,					// U64_TYPE
	I64_TYPE = 5,					// I64_TYPE
	NO_TYPE = 6,                   // NO_TYPE +n (tip char)
};
static bool Decompress(vector<unsigned char>& format,vector<unsigned char> &contact,std::vector<unsigned char> &ret){

	try {
		CDataStream ds(contact,SER_DISK, CLIENT_VERSION);
		CDataStream retdata(SER_DISK, CLIENT_VERSION);
		for (auto item = format.begin(); item != format.end();item++) {
			switch(*item) {
				case U16_TYPE: {
					unsigned short i = 0;
					ds >> VARINT(i);
					retdata<<i;
					break;
				}
				case I16_TYPE:
				{
					short i = 0;
					ds >> VARINT(i);
					retdata<<i;
					break;
				}
				case U32_TYPE:
				{
					short i = 0;
					ds >> VARINT(i);
					retdata<<i;
					break;
				}
				case I32_TYPE:
				{
					unsigned int i = 0;
					ds >> VARINT(i);
					retdata<<i;
					break;
				}
				case U64_TYPE:
				{
					uint64_t i = 0;
					ds >> VARINT(i);
					retdata<<i;
					break;
				}
				case I64_TYPE:
				{
					int64_t i = 0;
					ds >> VARINT(i);
					retdata<<i;
					break;
				}
				case NO_TYPE:
				{
					unsigned char temp = 0;
					item++;
					int te = *item;
					while (te--) {
						ds >> VARINT(temp);
						retdata<<temp;
					}
					break;
				}
				default:
				{
				   return ERRORMSG("%s\r\n",__FUNCTION__);
				}
			}
		}
		ret.insert(ret.begin(),retdata.begin(),retdata.end());
	} catch (...) {
		LogPrint("vm","seseril err in funciton:%s",__FUNCTION__);
		return false;
	}
	return true;

}

static RET_DEFINE ExCurDeCompressContactFunc(unsigned char *ipara,void *pVmEvn){

	vector<std::shared_ptr < vector<unsigned char> > > retdata;

    if(!GetData(ipara,retdata) ||retdata.size() != 1 )
    {
    	return RetFalse(string(__FUNCTION__)+"para  err !");
    }
	vector<unsigned char> contact =((CVmRunEvn *)pVmEvn)->GetTxContact();

	std::vector<unsigned char> outContact;
	 if(!Decompress(*retdata.at(0),contact,outContact))
	 {
		return RetFalse(string(__FUNCTION__)+"para  err !");
	 }

	auto tem =  make_shared<std::vector< vector<unsigned char> > >();
	(*tem.get()).push_back(outContact);
	return std::make_tuple (true,0, tem);

}
static RET_DEFINE ExDeCompressContactFunc(unsigned char *ipara,void *pVmEvn){
	CVmRunEvn *pVmScript = (CVmRunEvn *)pVmEvn;
	vector<std::shared_ptr < vector<unsigned char> > > retdata;
    if(!GetData(ipara,retdata) ||retdata.size() != 2 || retdata.at(1).get()->size() != 32)
    {
    	return RetFalse(string(__FUNCTION__)+"para  err !");
    }
	uint256 hash1(*retdata.at(1));
//	LogPrint("vm","ExGetTxContractsFunc1:%s\n",hash1.GetHex().c_str());
    bool flag = false;
	std::shared_ptr<CBaseTransaction> pBaseTx;
	auto tem =  make_shared<std::vector< vector<unsigned char> > >();
	if (GetTransaction(pBaseTx, hash1, *pVmScript->GetScriptDB(), false)) {
		CTransaction *tx = static_cast<CTransaction*>(pBaseTx.get());
		 std::vector<unsigned char> outContact;
		if (!Decompress(*retdata.at(0), tx->vContract, outContact)) {
			return RetFalse(string(__FUNCTION__) + "para  err !");
		}
		 (*tem.get()).push_back(outContact);
		 flag = true;
	}

	return std::make_tuple (flag,0, tem);
}

static RET_DEFINE GetCurTxPayAmountFunc(unsigned char *ipara,void *pVmEvn){
	CVmRunEvn *pVmRunEvn = (CVmRunEvn *)pVmEvn;
	uint64_t lvalue =pVmRunEvn->GetValue();

	auto tem =  make_shared<std::vector< vector<unsigned char> > >();
    CDataStream tep(SER_DISK, CLIENT_VERSION);

    tep << lvalue;
    vector<unsigned char> tep1(tep.begin(),tep.end());
	(*tem.get()).push_back(tep1);
	return std::make_tuple (true,0, tem);
}
struct S_APP_ID
{
	unsigned char idlen;                    //!the len of the tag
	unsigned char ID[CAppCFund::MAX_TAG_SIZE];     //! the ID for the

	const vector<unsigned char> GetIdV() const {
		assert(sizeof(ID) >= idlen);
		vector<unsigned char> Id(&ID[0], &ID[idlen]);
		return (Id);
	}
}__attribute((aligned (1)));

static RET_DEFINE GetUserAppAccValue(unsigned char * ipara,void * pVmScript){
	CVmRunEvn *pVmScriptRun = (CVmRunEvn *)pVmScript;

	vector<std::shared_ptr < vector<unsigned char> > > retdata;

    if(!GetData(ipara,retdata) ||retdata.size() != 1
    	|| retdata.at(0).get()->size() != sizeof(S_APP_ID))
    {
    	return RetFalse(string(__FUNCTION__)+"para  err !");
    }

    S_APP_ID accid;
    memcpy(&accid, &retdata.at(0).get()->at(0), sizeof(S_APP_ID));

    bool flag = false;
   	shared_ptr<CAppUserAccout> sptrAcc;
   	uint64_t value = 0 ;
   	auto tem =  make_shared<std::vector< vector<unsigned char> > >();
   	if(pVmScriptRun->GetAppUserAccout(accid.GetIdV(),sptrAcc))
	{
		value = sptrAcc->getllValues();
//	 	cout<<"read:"<<endl;
//	 	cout<<sptrAcc->toString()<<endl;
		CDataStream tep(SER_DISK, CLIENT_VERSION);
		tep << value;
		vector<unsigned char> tep1(tep.begin(),tep.end());
		(*tem.get()).push_back(tep1);
		flag = true;
	}
	return std::make_tuple (flag,0, tem);
}


static RET_DEFINE GetUserAppAccFoudWithTag(unsigned char * ipara,void * pVmScript){
	CVmRunEvn *pVmScriptRun = (CVmRunEvn *)pVmScript;

	vector<std::shared_ptr < vector<unsigned char> > > retdata;
	unsigned int Size(0);
	CAppFundOperate temp;
	Size = ::GetSerializeSize(temp, SER_NETWORK, PROTOCOL_VERSION);

    if(!GetData(ipara,retdata) ||retdata.size() != 1
    	|| retdata.at(0).get()->size() !=Size)
    {
			return RetFalse(string(__FUNCTION__)+"para err !");
    }

    CDataStream ss(*retdata.at(0),SER_DISK, CLIENT_VERSION);
    CAppFundOperate userfund;
    ss>>userfund;

   	shared_ptr<CAppUserAccout> sptrAcc;
    bool flag = false;
    auto tem =  make_shared<std::vector< vector<unsigned char> > >();
    CAppCFund fund;
	if(pVmScriptRun->GetAppUserAccout(userfund.GetAppUserV(),sptrAcc))
	{
		if(!sptrAcc->GetAppCFund(fund,userfund.GetFundTagV(),userfund.outheight))	{
			return RetFalse(string(__FUNCTION__)+"tag err !");
		}
		CDataStream tep(SER_DISK, CLIENT_VERSION);
		tep << fund.getvalue() ;
		vector<unsigned char> tep1(tep.begin(),tep.end());
		(*tem.get()).push_back(tep1);
		flag = true;
	}
	return std::make_tuple (flag,0, tem);
}
/**
 *   写 应用操作输出到 pVmRunEvn->MapAppOperate[0]
 * @param ipara
 * @param pVmEvn
 * @return
 */
static RET_DEFINE ExWriteOutAppOperateFunc(unsigned char * ipara,void * pVmEvn)
{
	CVmRunEvn *pVmRunEvn = (CVmRunEvn *)pVmEvn;
	vector<std::shared_ptr < vector<unsigned char> > > retdata;

	CAppFundOperate temp;
	unsigned int Size = ::GetSerializeSize(temp, SER_NETWORK, PROTOCOL_VERSION);

    if(!GetData(ipara,retdata) ||retdata.size() != 1 || (retdata.at(0).get()->size()%Size) != 0 )
    {
  		 return RetFalse("para err");
  	 }

	int count = retdata.at(0).get()->size()/Size;
	CDataStream ss(*retdata.at(0),SER_DISK, CLIENT_VERSION);

	int64_t step =-1;
	while(count--)
	{
		ss >> temp;
		if(pVmRunEvn->GetComfirHeight() > nFreezeBlackAcctHeight && temp.mMoney < 0) //不能小于0,防止 上层传错金额小于20150904
		{
			return RetFalse("para err");
		}
		pVmRunEvn->InsertOutAPPOperte(temp.GetAppUserV(),temp);
		step +=Size;
	}

	auto tem =  make_shared<std::vector< vector<unsigned char> > >();
	return std::make_tuple (true ,step, tem);
}
static RET_DEFINE ExGetBase58AddrFunc(unsigned char * ipara,void * pVmEvn){
	CVmRunEvn *pVmRunEvn = (CVmRunEvn *)pVmEvn;
	vector<std::shared_ptr < vector<unsigned char> > > retdata;

    if(!GetData(ipara,retdata) ||retdata.size() != 1 || retdata.at(0).get()->size() != 6)
    {
    	return RetFalse(string(__FUNCTION__)+"para  err !");
    }

	 CKeyID addrKeyId;
	 if (!GetKeyId(*pVmRunEvn->GetCatchView(),*retdata.at(0).get(), addrKeyId)) {
	    	return RetFalse(string(__FUNCTION__)+"para  err !");
	 }
	 string dacrsaddr = addrKeyId.ToAddress();

	 auto tem =  make_shared<std::vector< vector<unsigned char> > >();
	vector<unsigned char> vTemp;
	vTemp.assign(dacrsaddr.c_str(),dacrsaddr.c_str()+dacrsaddr.length());
	(*tem.get()).push_back(vTemp);
	return std::make_tuple (true,0, tem);
}

enum CALL_API_FUN {
	COMP_FUNC = 0,            //!< COMP_FUNC
	MULL_MONEY,              //!< MULL_MONEY
	ADD_MONEY,               //!< ADD_MONEY
	SUB_MONEY,                //!< SUB_MONEY
	DIV_MONEY,                //!< DIV_MONEY
	SHA256_FUNC,              //!< SHA256_FUNC
	DES_FUNC,                 //!< DES_FUNC
	VERFIY_SIGNATURE_FUNC,    //!< VERFIY_SIGNATURE_FUNC
	SIGNATURE_FUNC,           //!< SIGNATURE_FUNC
	PRINT_FUNC,               //!< PRINT_FUNC
	GETTX_CONTRACT_FUNC,      //!< GETTX_CONTRACT_FUNC
	GETTX_ACCOUNT_FUNC,       //!< GETTX_ACCOUNT_FUNC
	GETACCPUB_FUNC,           //!< GETACCPUB_FUNC
	QUEYACCBALANCE_FUNC,      //!< QUEYACCBALANCE_FUNC
	GETTXCONFIRH_FUNC,        //!< GETTXCONFIRH_FUNC
	GETBLOCKHASH_FUNC,        //!< GETBLOCKHASH_FUNC

	//// tx api
	GETCTXCONFIRMH_FUNC,        //!< GETCTXCONFIRMH_FUNC
	WRITEDB_FUNC,       //!< WRITEDB_FUNC
	DELETEDB_FUNC,      //!< DELETEDB_FUNC
	READDB_FUNC,        //!< READDB_FUNC
	GETDBSIZE_FUNC,     //!< GETDBSIZE_FUNC
	GETDBVALUE_FUNC,    //!< GETDBVALUE_FUNC
	GetCURTXHASH_FUNC,  //!< GetCURTXHASH_FUNC
	MODIFYDBVALUE_FUNC,  //!< MODIFYDBVALUE_FUNC
	WRITEOUTPUT_FUNC,     //!<WRITEOUTPUT_FUNC
	GETSCRIPTDATA_FUNC,		  //!<GETSCRIPTDATA_FUNC
	GETSCRIPTID_FUNC,		  //!<GETSCRIPTID_FUNC
	GETCURTXACCOUNT_FUNC,		  //!<GETCURTXACCOUNT_FUNC
	GETCURTXCONTACT_FUNC,		 //!<GETCURTXCONTACT_FUNC
	GETCURDECOMPRESSCONTACR_FUNC,   //!<GETCURDECOMPRESSCONTACR_FUNC
	GETDECOMPRESSCONTACR_FUNC,   	//!<GETDECOMPRESSCONTACR_FUNC
	GETCURPAYMONEY_FUN,             //!<GETCURPAYMONEY_FUN
	GET_APP_USER_ACC_VALUE_FUN,             //!<GET_APP_USER_ACC_FUN
	GET_APP_USER_ACC_FUND_WITH_TAG_FUN,             //!<GET_APP_USER_ACC_FUND_WITH_TAG_FUN
	GET_WIRITE_OUT_APP_OPERATE_FUN,             //!<GET_WIRITE_OUT_APP_OPERATE_FUN
	GET_DACRS_ADDRESS_FUN,
};

const static struct __MapExterFun FunMap[] = { //
		{ COMP_FUNC, ExInt64CompFunc },			//
		{ MULL_MONEY, ExInt64MullFunc },			//
		{ ADD_MONEY, ExInt64AddFunc },			//
		{ SUB_MONEY, ExInt64SubFunc },			//
		{ DIV_MONEY, ExInt64DivFunc },			//
		{ SHA256_FUNC, ExSha256Func },			//
		{ DES_FUNC, ExDesFunc },			    //
		{ VERFIY_SIGNATURE_FUNC, ExVerifySignatureFunc },   //
		{ SIGNATURE_FUNC, ExSignatureFunc },			//
		{ PRINT_FUNC, ExLogPrintFunc },         //
		{GETTX_CONTRACT_FUNC,ExGetTxContractsFunc},            //
		{GETTX_ACCOUNT_FUNC,ExGetTxAccountsFunc},
		{GETACCPUB_FUNC,ExGetAccountPublickeyFunc},
		{QUEYACCBALANCE_FUNC,ExQueryAccountBalanceFunc},
		{GETTXCONFIRH_FUNC,ExGetTxConFirmHeightFunc},
		{GETBLOCKHASH_FUNC,ExGetBlockHashFunc},


		{GETCTXCONFIRMH_FUNC,ExGetCurRunEnvHeightFunc},
		{WRITEDB_FUNC,ExWriteDataDBFunc},
		{DELETEDB_FUNC,ExDeleteDataDBFunc},
		{READDB_FUNC,ExReadDataValueDBFunc},
		{GETDBSIZE_FUNC,ExGetDBSizeFunc},
		{GETDBVALUE_FUNC,ExGetDBValueFunc},
		{GetCURTXHASH_FUNC,ExGetCurTxHash},
		{MODIFYDBVALUE_FUNC,ExModifyDataDBVavleFunc},
		{WRITEOUTPUT_FUNC,ExWriteOutputFunc},
		{GETSCRIPTDATA_FUNC,ExGetScriptDataFunc},
		{GETSCRIPTID_FUNC,ExGetScriptIDFunc},
		{GETCURTXACCOUNT_FUNC,ExGetCurTxAccountFunc	  },
		{GETCURTXCONTACT_FUNC,ExGetCurTxContactFunc		},
		{GETCURDECOMPRESSCONTACR_FUNC,ExCurDeCompressContactFunc },
		{GETDECOMPRESSCONTACR_FUNC,ExDeCompressContactFunc  	},
		{GETCURPAYMONEY_FUN,GetCurTxPayAmountFunc},

		{GET_APP_USER_ACC_VALUE_FUN,GetUserAppAccValue},
		{GET_APP_USER_ACC_FUND_WITH_TAG_FUN,GetUserAppAccFoudWithTag},
		{GET_WIRITE_OUT_APP_OPERATE_FUN,ExWriteOutAppOperateFunc},
		{GET_DACRS_ADDRESS_FUN,ExGetBase58AddrFunc},

		};

RET_DEFINE CallExternalFunc(INT16U method, unsigned char *ipara,CVmRunEvn *pVmEvn) {
	return FunMap[method].fun(ipara, pVmEvn);

}

int64_t CVm8051::run(uint64_t maxstep, CVmRunEvn *pVmEvn) {
	INT8U code = 0;
	int64_t step = 0;  //uint64_t

	if(maxstep == 0){
		return -1;
	}

	while (1) {
		code = GetOpcode();
		StepRun(code);
		step++;
		//call func out of 8051
		if (Sys.PC == 0x0012) {
			//get what func will be called
			INT16U methodID = ((INT16U) GetExRam(VM_FUN_CALL_ADDR) | ((INT16U) GetExRam(VM_FUN_CALL_ADDR+1) << 8));
			unsigned char *ipara = (unsigned char *) GetExRamAddr(VM_SHARE_ADDR);		//input para
			RET_DEFINE retdata = CallExternalFunc(methodID, ipara, pVmEvn);
			memset(ipara, 0, MAX_SHARE_RAM);
			step += std::get<1>(retdata) - 1;
			if (std::get<0>(retdata)) {
				auto tem = std::get<2>(retdata);
				int pos = 0;
				int totalsize = 0;
				for (auto& it : *tem.get()) {
					totalsize += it.size() + 2;
				}
				if (totalsize + 2 < MAX_SHARE_RAM) { //if data not over
					for (auto& it : *tem.get()) {
						int size = it.size();

						memcpy(&ipara[pos], &size, 2);
						memcpy(&ipara[pos + 2], &it.at(0), size);
						pos += size + 2;
					}
				}
			}
		} else if (Sys.PC == 0x0008) {   //要求退出
			INT8U result = GetExRam(0xEFFD);
			if (result == 0x01) {
				return step;
			}
			return 0;
		}
		if (step >= (int64_t)MAX_BLOCK_RUN_STEP || step >= (int64_t)maxstep){
			LogPrint("CONTRACT_TX", "failed step:%ld\n", step);
			return -1;		//force return
		}
	}

	return 1;
}

bool CVm8051::run() {

	INT8U code = 0;
//	INT16U flag;
	while (1) {
		code = GetOpcode();
		StepRun(code);
//		UpDataDebugInfo();

		//call func out of 8051
		if (Sys.PC == 0x0012) {
			//get what func will be called
			INT16U method = ((INT16U) GetExRam(VM_FUN_CALL_ADDR) | ((INT16U) GetExRam(VM_FUN_CALL_ADDR+1) << 8));
//			flag = method;
			unsigned char *ipara = (unsigned char *) GetExRamAddr(VM_SHARE_ADDR);		//input para
			CVmRunEvn *pVmScript = NULL;
			RET_DEFINE retdata = CallExternalFunc(method, ipara, pVmScript);
			//memset(ipara, 0, MAX_SHARE_RAM);
			memset(ipara, 0, 8);
			if (std::get<0>(retdata)) {
				auto tem = std::get<2>(retdata);
				int pos = 0;
				int totalsize = 0;
				for (auto& it : *tem.get()) {
					totalsize += it.size() + 2;
				}
				if (totalsize + 2 < MAX_SHARE_RAM) {
					for (auto& it : *tem.get()) {
						int size = it.size();
						memcpy(&ipara[pos], &size, 2);
						memcpy(&ipara[pos + 2], &it.at(0), size);
						pos += size + 2;
					}
				}

			}
		}else if (Sys.PC == 0x0008) {
			INT8U result=GetExRam(0xEFFD);
			if(result == 0x01)
			{
				return 1;
			}
			return 0;
		}
	}

	return 1;
}
void CVm8051::SetExRamData(INT16U addr, const vector<unsigned char> data) {
	memcpy(&m_ExRam[addr], &data[0], data.size());
}
void CVm8051::StepRun(INT8U code) {
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
}

bool CVm8051::GetBitFlag(INT8U addr) {
	return (GetBitRamRef(addr) & (BIT0 << (addr % 8))) != 0;
}

void CVm8051::SetBitFlag(INT8U addr) {

	GetBitRamRef(addr) |= (BIT0 << (addr % 8));
	if (addr >= 0xe0 && addr <= 0xe7) { // 操作的是 累加器A
		Updata_A_P_Flag();
	}
}

void CVm8051::ClrBitFlag(INT8U addr) {

	GetBitRamRef(addr) &= (~(BIT0 << (addr % 8)));
	if (addr >= 0xe0 && addr <= 0xe7) // 操作的是 累加器A
			{
		Updata_A_P_Flag();
	}
}

INT8U CVm8051::GetOpcode(void) const {
	return m_ExeFile[Sys.PC];
}

INT8U& CVm8051::GetRamRef(INT8U addr) {

	if (addr > 0x7F) {
		return m_ChipSfr[addr];
	} else {
		return m_ChipRam[addr];
	}
}

INT16U CVm8051::GetDebugOpcode(void) const {
	INT16U temp = 0;
	memcpy(&temp, &m_ExeFile[Sys.PC], 2);
	return temp;
}

bool CVm8051::GetDebugPC(INT16U pc) const {
	return (pc == Sys.PC);
}

void CVm8051::GetOpcodeData(void * const p, INT8U len) const {  //先取高位字节
	memcpy(p, &m_ExeFile[Sys.PC + 1], len);
}
void CVm8051::AJMP(INT8U opCode, INT8U data) {
	INT16U tmppc = Sys.PC + 2;
	tmppc &= 0xF800;
	tmppc |= ((((((INT16U) opCode) >> 5) & 0x7) << 8) | ((INT16U) data));

	Sys.PC = tmppc;
}
void CVm8051::ACALL(INT8U opCode) {
	INT8U data = Get1Opcode();
	Sys.PC += 2;
	Sys.sp = Sys.sp() + 1;
	SetRamDataAt(Sys.sp(), (INT8U) (Sys.PC & 0x00ff));

	Sys.sp = Sys.sp() + 1;
	SetRamDataAt(Sys.sp(), (INT8U) (Sys.PC >> 8));
	Sys.PC = ((Sys.PC & 0xF800) | (((((INT16U) opCode) >> 5) << 8) | (INT16U) data));
}

void CVm8051::UpDataDebugInfo(void) {
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

void * CVm8051::GetExRamAddr(INT16U addr) const {
//	Assert(addr < sizeof(m_ExRam));
	return (void *) &m_ExRam[addr];
}
INT8U CVm8051::GetExRam(INT16U addr) const {
//	Assert(addr < sizeof(m_ExRam));
	return m_ExRam[addr];
}
INT8U CVm8051::SetExRam(INT16U addr, INT8U data) {
//	Assert(addr < sizeof(m_ExRam));
	return m_ExRam[addr] = data;
}
void * CVm8051::GetPointRamAddr(INT16U addr) const {
//	Assert(addr < sizeof(m_ChipRam));
	return (void *) &m_ChipRam[addr];
}

INT8U CVm8051::GetRamDataAt(INT8U addr) {
	return m_ChipRam[addr];
}
INT8U CVm8051::SetRamDataAt(INT8U addr, INT8U data) {
	return m_ChipRam[addr] = data;
}
INT8U CVm8051::GetRamData(INT8U addr) {
	return GetRamRef(addr);
}
INT8U CVm8051::SetRamData(INT8U addr, INT8U data) {
	GetRamRef(addr) = data;
	return data;
}

void* CVm8051::GetPointFileAddr(INT16U addr) const {
	Assert(addr < sizeof(m_ExeFile));
	return (void*) &m_ExeFile[addr];
}

void CVm8051::Opcode_02_LJMP_Addr16(void) {
	INT8U temp[2];
	INT16U data;
	GetOpcodeData(temp, 2);
	data = (((INT16U) (temp[0])) << 8);
	Sys.PC = data | temp[1];
}

void CVm8051::Opcode_03_RR_A(void) {
	INT8U temp = (Sys.a() & 0x1);
	Sys.a = Sys.a() >> 1;
	Sys.a = Sys.a() | (temp << 7);

	++Sys.PC;
}

void CVm8051::Opcode_05_INC_Direct(void) {
	INT8U temp = 0;
	INT8U addr = Get1Opcode();
	temp = GetRamData(addr);
	++temp;
	SetRamData(addr, temp);
	Sys.PC = Sys.PC + 2;
}

void CVm8051::Opcode_06_INC_R0_1(void) {
	INT8U data = 0;
	INT8U addr = Rges.R0();
	data = GetRamDataAt(addr);
	++data;
	SetRamDataAt(addr, data);
	++Sys.PC;
}
void CVm8051::Opcode_07_INC_R1_1(void) {

	INT8U data = 0;
	INT8U addr = Rges.R1();
	data = GetRamDataAt(addr);
	++data;
	SetRamDataAt(addr, data);
	++Sys.PC;
}

void CVm8051::Opcode_10_JBC_Bit_Rel(void) {
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
}



INT16U CVm8051::GetLcallAddr(void) {
	INT16U addr = 0;
	GetOpcodeData((INT8U*) &addr, 2);
	Assert(GetOpcode() == 0x12);
	return (addr >> 8) | (addr << 8);

}

void CVm8051::Opcode_12_LCALL_Addr16(void) {
//	INT8U temp = 0;
	INT16U addr = 0;
//	void *p = NULL;

	GetOpcodeData((INT8U*) &addr, 2);
	Sys.PC += 3;
	Sys.sp = Sys.sp() + 1;
	SetRamDataAt(Sys.sp(), (INT8U) (Sys.PC & 0x00ff));
	Sys.sp = Sys.sp() + 1;
	SetRamDataAt(Sys.sp(), (INT8U) (Sys.PC >> 8));
// GetOpcodeData(&Sys.PC,2);
	Sys.PC = (addr >> 8) | (addr << 8);
}
void CVm8051::Opcode_13_RRC_A(void) {
	INT8U temp = Sys.a() & BIT0;

	Sys.a = ((Sys.a() >> 1) | (Sys.psw().cy << 7));

	Sys.psw().cy = (temp == 0) ? 0 : 1;
	++Sys.PC;
}
void CVm8051::Opcode_14_DEC_A(void) {
	Sys.a = Sys.a() - 1;
	++Sys.PC;
}
void CVm8051::Opcode_15_DEC_Direct(void) {
	INT8U temp = 0;
	INT8U addr = Get1Opcode();

	temp = GetRamData(addr);
	--temp;
	SetRamData(addr, temp);
	Sys.PC = Sys.PC + 2;
}
void CVm8051::Opcode_16_DEC_R0_1(void) {
	INT8U temp = 0;
	INT8U addr = Rges.R0();
	temp = GetRamDataAt(addr);
	--temp;
	SetRamDataAt(addr, temp);
	++Sys.PC;
}
void CVm8051::Opcode_17_DEC_R1_1(void) {

	INT8U temp = 0;
	INT8U addr = Rges.R1();

	temp = GetRamDataAt(addr);
	temp--;
	SetRamDataAt(addr, temp);
	++Sys.PC;

}

void CVm8051::Opcode_20_JB_Bit_Rel(void) {
	INT8U temp[2];

	GetOpcodeData(&temp[0], sizeof(temp));
	Sys.PC += 3;

	if (GetBitFlag((temp[0]))) {
		char tem2;
		memcpy(&tem2, &temp[1], sizeof(temp[1]));
		Sys.PC += tem2;
//		Sys.PC = GetTargPC(temp[1]);
	}

}

void CVm8051::Opcode_22_RET(void) {
	INT8U temp = 0;

	temp = GetRamDataAt(Sys.sp());
	Sys.PC = (Sys.PC & 0x00FF) | (((INT16U) temp) << 8);
	Sys.sp = Sys.sp() - 1;
	temp = GetRamDataAt(Sys.sp());
	Sys.PC = (Sys.PC & 0xFF00) | temp;
	Sys.sp = Sys.sp() - 1;
}

void CVm8051::Opcode_23_RL_A(void) {
	INT8U temp = Sys.a() & 0x80;
	INT8U tem2 = Sys.a();
	Sys.a = (INT8U) ((tem2 << 1) | (temp >> 7));
	++Sys.PC;
}

void CVm8051::MD_ADDC(INT8U data) {
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
void CVm8051::MD_SUBB(INT8U data) {
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

void CVm8051::MD_ADD(INT8U data) {
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


void CVm8051::Opcode_30_JNB_Bit_Rel(void) {
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


}

void CVm8051::Opcode_32_RETI(void) {
	INT8U temp = 0;
	temp = GetRamData(Sys.sp());
	Sys.PC = (Sys.PC & 0x00FF) | (((INT16U) temp) << 8);
	Sys.sp = Sys.sp() - 1;

	temp = GetRamData(Sys.sp());
	Sys.PC = (Sys.PC & 0xFF00) | temp;

	Sys.sp = Sys.sp() - 1;
	++Sys.PC;
}
void CVm8051::Opcode_33_RLC_A(void) {
	INT8U temp = Sys.a() & 0x80;
	Sys.a = ((Sys.a() << 1) | Sys.psw().cy);
	Sys.psw().cy = temp == 0 ? 0 : 1;

	++Sys.PC;
}

void CVm8051::Opcode_40_JC_Rel(void) {
	char tem2= Get1Opcode();
//	GetOpcodeData(&tem2, 1);
	Sys.PC += 2;
	if (Sys.psw().cy) {
		memcpy(&tem2, &tem2, sizeof(tem2));
		Sys.PC += tem2;
//		Sys.PC = GetTargPC(temp);
	}
}

void CVm8051::Opcode_42_ORL_Direct_A(void) {
	INT8U temp;
	INT8U addr = Get1Opcode();
	temp = GetRamData(addr);
	temp |= Sys.a();

	SetRamData(addr, temp);
	Sys.PC += 2;
}
void CVm8051::Opcode_43_ORL_Direct_Data(void) {
	INT8U temp[2] = { 0 };
	INT8U data;
//	void *p = NULL;
	GetOpcodeData(&temp[0], 2); //direct ~{JG~}1~{8vWV=Z~}

	data = GetRamData(temp[0]);
	data |= temp[1];
//	memcpy(p, &data, 1);
	SetRamData(temp[0], data);
	Sys.PC += 3;
}
void CVm8051::Updata_A_P_Flag(void) {
	INT8U a = Sys.a();
	a ^= a >> 4;
	a ^= a >> 2;
	a ^= a >> 1;
	Sys.psw().p = (a & 1);

}
void CVm8051::Opcode_44_ORL_A_Data(void) {
	INT8U temp = Get1Opcode();
	Sys.a = Sys.a() | temp;
	Sys.PC += 2;
}
void CVm8051::Opcode_45_ORL_A_Direct(void) {
	INT8U data;
	INT8U addr = Get1Opcode();
//	GetOpcodeData(&addr, 1);
	data = GetRamData(addr);
	Sys.a = Sys.a() | data;
	Sys.PC += 2;
}

void CVm8051::Opcode_50_JNC_Rel(void) {
	char tem2 = Get1Opcode();
//	GetOpcodeData(&tem2, 1);
	Sys.PC += 2;
	if (Sys.psw().cy == 0) {
		Sys.PC += tem2;
	}
}
void CVm8051::Opcode_52_ANL_Direct_A(void) {
	INT8U temp;
	INT8U addr =Get1Opcode();
//	void *p = NULL;
	GetOpcodeData(&addr, 1);
	temp = GetRamData(addr);
	temp &= Sys.a();
	SetRamData(addr, temp);

	Sys.PC += 2;
}
void CVm8051::Opcode_53_ANL_Direct_Data(void) {
	INT8U temp[2] = { 0 };
	INT8U data;
	GetOpcodeData(&temp[0], 2);
	data = GetRamData(temp[0]);
	data &= temp[1];
	SetRamData(temp[0], data);
	Sys.PC += 3;
}

void CVm8051::Opcode_60_JZ_Rel(void) {
	char temp= Get1Opcode();
//	GetOpcodeData(&temp, 1);
	Sys.PC += 2;
	if (Sys.a() == 0) {
//		Sys.PC = GetTargPC(temp);
		Sys.PC += temp;
	}
}
void CVm8051::Opcode_62_XRL_Direct_A(void) {
	INT8U temp;
	INT8U addr = Get1Opcode();

	temp = GetRamData(addr);
	temp ^= Sys.a();
	SetRamData(addr, temp);
	Sys.PC += 2;
}

void CVm8051::Opcode_63_XRL_Direct_Data(void) {
	INT8U temp[2] = { 0 };
	INT8U data;

	GetOpcodeData(&temp[0], 2);
	data = GetRamData(temp[0]);
	data ^= temp[1];

	SetRamData(temp[0], data);
	Sys.PC += 3;
}

void CVm8051::Opcode_70_JNZ_Rel(void) {
	char temp = Get1Opcode();

	Sys.PC += 2;
	if (Sys.a() != 0) {
		Sys.PC += temp;
	}
}

void CVm8051::Opcode_75_MOV_Direct_Data(void) {
	INT8U temp[2] = { 0 };
	GetOpcodeData(&temp[0], 2);

	SetRamData(temp[0], temp[1]);
	Sys.PC += 3;
}

void CVm8051::Opcode_80_SJMP_Rel(void) {
	char temp=  Get1Opcode();

	Sys.PC += 2;
	Sys.PC += temp;
}

void CVm8051::Opcode_83_MOVC_A_PC(void) {
//	INT8U temp;
	INT16U addr;
//	void*p = NULL;
	++Sys.PC;
	addr = (INT16U) (Sys.PC + Sys.a());
	Sys.a = (Sys.PC > CVm8051::MAX_ROM - Sys.a()) ? (0x00) : (m_ExeFile[addr]);

}
void CVm8051::Opcode_84_DIV_AB(void) {
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
	++Sys.PC;
}
void CVm8051::Opcode_85_MOV_Direct_Direct(void) {
	INT8U temp[2];

	INT8U tem;
	GetOpcodeData(&temp[0], 2);

	tem = GetRamData(temp[0]);
	SetRamData(temp[1], tem);
	Sys.PC += 3;
}

void CVm8051::Opcode_90_MOV_DPTR_Data(void) {
	INT16U temp;
	GetOpcodeData(&temp, 2);
	temp = (temp >> 8) | (temp << 8);
	Sys.dptr = temp;
	Sys.PC += 3;
}
void CVm8051::Opcode_92_MOV_Bit_C(void) {
	INT8U temp = Get1Opcode();

	if (Sys.psw().cy) {
		SetBitFlag(temp);
	} else {
		ClrBitFlag(temp);
	}
	Sys.PC += 2;
}
void CVm8051::Opcode_93_MOVC_A_DPTR(void) {
	INT8U temp;
	void *p = NULL;
	p = GetPointFileAddr((INT16U) (Sys.a() + Sys.dptr()));
	memcpy(&temp, p, sizeof(temp));
	Sys.a = (Sys.dptr() > CVm8051::MAX_ROM - Sys.a()) ? (0x00) : (temp);
	++Sys.PC;
}

void CVm8051::Opcode_A4_MUL_AB(void) {
	INT16U temp;
	temp = (INT16U) Sys.a() * (INT16U) Sys.b();
	Sys.psw().ov = (temp > 255) ? 1 : 0;
	Sys.a = (INT8U) temp;
	Sys.b = (INT8U) (temp >> 8);
	++Sys.PC;
}

void CVm8051::Opcode_B2_CPL_Bit(void) {
	INT8U temp=Get1Opcode();
	if (GetBitFlag(temp)) {
		ClrBitFlag(temp);
	} else {
		SetBitFlag(temp);
	}
	Sys.PC += 2;
}

void CVm8051::Opcode_B4_CJNE_A_Data_Rel(void) {
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

void CVm8051::Opcode_B5_CJNE_A_Direct_Rel(void) {
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
void CVm8051::Opcode_B6_CJNE_R0_1_Data_Rel(void) {
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
void CVm8051::Opcode_B7_CJNE_R1_1_Data_Rel(void) {
	INT8U temp[2];
	INT8U data;
	GetOpcodeData(&temp[0], sizeof(temp));
	data = GetRamDataAt(Rges.R1());
	Sys.PC += 3;

	if (data != temp[0]) {
		Sys.PC += (char)temp[1];
	}
	Sys.psw().cy = (data < temp[0]) ? 1 : 0;
}
void CVm8051::Opcode_B8_CJNE_R0_Data_Rel(void) {
	INT8U temp[2];

	GetOpcodeData(&temp[0], sizeof(temp));
	Sys.PC += 3;
	if (Rges.R0() != temp[0]) {
		Sys.PC += (char)temp[1];
	}
	Sys.psw().cy = (Rges.R0() < temp[0]) ? 1 : 0;
}
void CVm8051::Opcode_B9_CJNE_R1_Data_Rel(void) {
	INT8U temp[2];
	GetOpcodeData(&temp[0], sizeof(temp));
	Sys.PC += 3;

	if (Rges.R1() != temp[0]) {
		Sys.PC += (char)temp[1];
	}
	Sys.psw().cy = (Rges.R1() < temp[0]) ? 1 : 0;
}
void CVm8051::Opcode_BA_CJNE_R2_Data_Rel(void) {
	INT8U temp[2];
	GetOpcodeData(&temp[0], sizeof(temp));
	Sys.PC += 3;
	if (Rges.R2() != temp[0]) {
		Sys.PC += (char)temp[1];
	}
	Sys.psw().cy = (Rges.R2() < temp[0]) ? 1 : 0;
}
void CVm8051::Opcode_BB_CJNE_R3_Data_Rel(void) {
	INT8U temp[2];
	GetOpcodeData(&temp[0], sizeof(temp));
	Sys.PC += 3;
	if (Rges.R3() != temp[0]) {
		Sys.PC += (char)temp[1];
	}
	Sys.psw().cy = (Rges.R3() < temp[0]) ? 1 : 0;
}
void CVm8051::Opcode_BC_CJNE_R4_Data_Rel(void) {
	INT8U temp[2];
	GetOpcodeData(&temp[0], sizeof(temp));
	Sys.PC += 3;

	if (Rges.R4() != temp[0]) {
		Sys.PC += (char)temp[1];
	}
	Sys.psw().cy = (Rges.R4() < temp[0]) ? 1 : 0;
}
void CVm8051::Opcode_BD_CJNE_R5_Data_Rel(void) {
	INT8U temp[2];
	GetOpcodeData(&temp[0], sizeof(temp));
	Sys.PC += 3;
	if (Rges.R5() != temp[0]) {
		Sys.PC += (char)temp[1];
	}
	Sys.psw().cy = (Rges.R5() < temp[0]) ? 1 : 0;
}
void CVm8051::Opcode_BE_CJNE_R6_Data_Rel(void) {
	INT8U temp[2];
	GetOpcodeData(&temp[0], sizeof(temp));
	Sys.PC += 3;
	if (Rges.R6() != temp[0]) {
		Sys.PC += (char)temp[1];
	}
	Sys.psw().cy = (Rges.R6() < temp[0]) ? 1 : 0;
}
void CVm8051::Opcode_BF_CJNE_R7_Data_Rel(void) {
	INT8U temp[2];
	GetOpcodeData(&temp[0], sizeof(temp));
	Sys.PC += 3;
	if (Rges.R7() != temp[0]) {
		Sys.PC += (char)temp[1];
	}
	Sys.psw().cy = (Rges.R7() < temp[0]) ? 1 : 0;
}

void CVm8051::Opcode_C5_XCH_A_Direct(void) {
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
void CVm8051::Opcode_C6_XCH_A_R0_1(void) {
	INT8U TT;
	TT = Sys.a();
	Sys.a = GetRamDataAt(Rges.R0());
	SetRamDataAt(Rges.R0(), TT);
	++Sys.PC;
}
void CVm8051::Opcode_C7_XCH_A_R1_1(void) {
	INT8U TT;
	TT = Sys.a();
	Sys.a = GetRamDataAt(Rges.R1());
	SetRamDataAt(Rges.R1(), TT);
	++Sys.PC;
}
void CVm8051::Opcode_C8_XCH_A_R0(void) {
	INT8U temp;
	temp = Sys.a();
	Sys.a = Rges.R0();
	Rges.R0 = temp;
	++Sys.PC;
}
void CVm8051::Opcode_C9_XCH_A_R1(void) {
	INT8U temp;
	temp = Sys.a();
	Sys.a = Rges.R1();
	Rges.R1 = temp;
	++Sys.PC;
}
void CVm8051::Opcode_CA_XCH_A_R2(void) {
	INT8U temp;
	temp = Sys.a();
	Sys.a = Rges.R2();
	Rges.R2 = temp;
	++Sys.PC;
}
void CVm8051::Opcode_CB_XCH_A_R3(void) {
	INT8U temp;
	temp = Sys.a();
	Sys.a = Rges.R3();
	Rges.R3 = temp;
	++Sys.PC;
}
void CVm8051::Opcode_CC_XCH_A_R4(void) {
	INT8U temp;
	temp = Sys.a();
	Sys.a = Rges.R4();
	Rges.R4 = temp;
	++Sys.PC;
}
void CVm8051::Opcode_CD_XCH_A_R5(void) {
	INT8U temp;
	temp = Sys.a();
	Sys.a = Rges.R5();
	Rges.R5 = temp;
	++Sys.PC;
}
void CVm8051::Opcode_CE_XCH_A_R6(void) {
	INT8U temp;
	temp = Sys.a();
	Sys.a = Rges.R6();
	Rges.R6 = temp;
	++Sys.PC;
}
void CVm8051::Opcode_CF_XCH_A_R7(void) {
	INT8U temp;
	temp = Sys.a();
	Sys.a = Rges.R7();
	Rges.R7 = temp;
	++Sys.PC;
}
void CVm8051::Opcode_D0_POP_Direct(void) {
	SetRamData(Get1Opcode(), GetRamDataAt(Sys.sp()));
	Sys.sp = Sys.sp() - 1;
	Sys.PC += 2;
}

void CVm8051::Opcode_D4_DA_A(void) {
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
	++Sys.PC;
}
void CVm8051::Opcode_D5_DJNZ_Direct_Rel(void) {
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
void CVm8051::Opcode_D6_XCHD_A_R0_1(void) {
	INT8U temp = 0;
	INT8U data = GetRamDataAt(Rges.R0());
	temp = Sys.a() & 0x0F;
	Sys.a = Sys.a() & 0xF0;
	Sys.a = Sys.a() | (data & 0x0F);
	data &= 0xF0;
	data |= temp;
	SetRamDataAt(Rges.R0(), data);
	++Sys.PC;
}
void CVm8051::Opcode_D7_XCHD_A_R1_1(void) {
	INT8U temp = 0;
	INT8U data;
	data = GetRamDataAt(Rges.R1());
	temp = Sys.a() & 0x0F;
	Sys.a = Sys.a() & 0xF0;
	Sys.a = Sys.a() | (data & 0x0F);
	data &= 0xF0;
	data |= temp;
	SetRamDataAt(Rges.R1(), data);
	++Sys.PC;
}
void CVm8051::Opcode_D8_DJNZ_R0_Rel(void) {
	char temp = Get1Opcode();
	Sys.PC += 2;
	Rges.R0 = Rges.R0() - 1;
	if (Rges.R0() != 0) {
		Sys.PC += temp;
	}
}
void CVm8051::Opcode_D9_DJNZ_R1_Rel(void) {
	char temp = Get1Opcode();
	Sys.PC += 2;
	Rges.R1 = Rges.R1() - 1;
	if (Rges.R1() != 0) {
		Sys.PC += temp;
	}
}
void CVm8051::Opcode_DA_DJNZ_R2_Rel(void) {
	char temp = Get1Opcode();
	Sys.PC += 2;
	Rges.R2 = Rges.R2() - 1;
	if (Rges.R2() != 0) {
		Sys.PC += temp;
	}
}
void CVm8051::Opcode_DB_DJNZ_R3_Rel(void) {
	char temp = Get1Opcode();
	Sys.PC += 2;
	Rges.R3 = Rges.R3() - 1;
	if (Rges.R3() != 0) {
		Sys.PC += temp;
	}
}
void CVm8051::Opcode_DC_DJNZ_R4_Rel(void) {
	char temp = Get1Opcode();
	Sys.PC += 2;
	Rges.R4 = Rges.R4() - 1;
	if (Rges.R4() != 0) {
		Sys.PC += temp;
	}
}
void CVm8051::Opcode_DD_DJNZ_R5_Rel(void) {
	char temp = Get1Opcode();
	Sys.PC += 2;
	Rges.R5 = Rges.R5() - 1;
	if (Rges.R5() != 0) {
		Sys.PC += temp;
	}
}
void CVm8051::Opcode_DE_DJNZ_R6_Rel(void) {
	char temp = Get1Opcode();
	Sys.PC += 2;
	Rges.R6 = Rges.R6() - 1;
	if (Rges.R6() != 0) {
		Sys.PC += temp;
	}
}
void CVm8051::Opcode_DF_DJNZ_R7_Rel(void) {
	char temp = Get1Opcode();
	Sys.PC += 2;
	Rges.R7 = Rges.R7() - 1;
	if (Rges.R7() != 0) {
		Sys.PC += temp;
	}
}
void CVm8051::Opcode_E0_MOVX_A_DPTR(void) {
	Sys.a = m_ExRam[Sys.dptr()];
	++Sys.PC;
}

shared_ptr<vector<unsigned char>> CVm8051::GetRetData(void) const {
	auto tem = make_shared<vector<unsigned char>>();

	char buffer[1024] = { 0 };
	memcpy(buffer, &m_ExRam[0xFC00], 1023);
	tem.get()->assign(buffer, buffer + 1023);
	return tem;
}

template<class T2>
T2& CUPReg<T2>::GetRegRe(void) {
	assert(m_Addr != 255);
	return *((T2*) (&pmcu->m_ChipRam[m_Addr]));
}
INT8U& CUPReg_a::GetRegRe(void) {
	assert(m_Addr != 255);
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

