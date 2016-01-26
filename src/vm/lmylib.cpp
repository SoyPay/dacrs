/*
** $Id: ldblib.c,v 1.149 2015/02/19 17:06:21 roberto Exp $
** Interface from Lua to its debug API
** See Copyright Notice in lua.h
*/

//#define ldblib_c
//#define LUA_LIB

//#include "lprefix.h"
//
//
//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>
//
//#include "lua.h"
//
//#include "lauxlib.h"
//#include "lualib.h"

#include "lua/lua.hpp"
#include "hash.h"
#include "key.h"
#include "main.h"
#include <openssl/des.h>
#include <vector>
#include "vmrunevn.h"



/*
 *  //3.往函数私有栈里存运算后的结果*/
static inline int RetRstToLua(lua_State *L,const vector<unsigned char> &ResultData )
{
	int len = ResultData.size();
    if(len > 0)
    {
		LogPrint("vm", "value:%s\n",HexStr(ResultData));
		for(int i = len -1;i >= 0;i--){
			lua_pushinteger(L,(lua_Integer)ResultData[i]);
		}
    }
	return len ;
}
static inline int RetFalse(const string reason)
{
	 LogPrint("vm", reason.c_str());
	 LogPrint("vm", "\n");
	 return 0;
}

static bool GetKeyId(const CAccountViewCache &view, vector<unsigned char> &ret,
		CKeyID &KeyId) {
	if (ret.size() == 6) {
		CRegID reg(ret);
		KeyId = reg.getKeyID(view);
	} else if (ret.size() == 34) {
		string addr(ret.begin(), ret.end());
		KeyId = CKeyID(addr);
	}else{
		return false;
	}
	if (KeyId.IsEmpty())
		return false;

	return true;
}
static bool GetData(lua_State *L, vector<std::shared_ptr < std::vector<unsigned char> > > &ret) {
	int totallen = lua_gettop(L);
	if(totallen <= 0)
	{
		return false;
	}
	LogPrint("vm","totallen =%d\n",totallen);
//	if(totallen>= CVm8051::MAX_SHARE_RAM)
//	{
//		LogPrint("vm","%s\r\n","data over flaw");
//		return false;
//	}
    int j = 0;
    unsigned char buff[1024] = {0};
	while (j < totallen) {
		int length = lua_tonumber(L,-1 - j);  //取长度
		j++;
		LogPrint("vm","length =%d\n",length);
		// 待确定L 到底占几个字节
		if ((length <= 0) || (length + 1 > totallen)) {
           LogPrint("vm","%s\r\n","data over flaw");
			return false;
		}
		memset(buff,0,sizeof(buff));
        for(int i = 0;i < length;i++)  //取value
        {
        	if(!lua_isnumber(L,-1 - j))
        	{
				LogPrint("vm","%s\r\n","data isnot number");
				return false;
        	}
        	buff[i] = lua_tonumber(L,-1 - j);
        	j++;
        	LogPrint("vm","value = %d\n",buff[i]);
        }
        reverse(buff,buff + length);
		ret.insert(ret.end(),std::make_shared<vector<unsigned char>>(buff, buff + length));
	}
	return true;
}

static double getfield(lua_State *L,char * key){
	// 从栈里取一个值
    double res = 0;
    //默认栈顶是table，将key入栈
    lua_pushstring(L,key);
    lua_gettable(L,-2);  //查找键值为key的元素，置于栈顶
    if(!lua_isnumber(L,-1))
    {
    	LogPrint("vm","num get error! %s\n",lua_tostring(L,-1));
    }else{
    	res = lua_tonumber(L,-1);
    }
    lua_pop(L,1); //删掉产生的查找结果
    return res;
}
#if 0
static void setfield(lua_State *L,char * key,double value){
	 //默认栈顶是table
	lua_pushstring(L,key);
	lua_pushnumber(L,value);
	lua_settable(L,-3);	//将这一对键值设成元素
}
static void stackDump(lua_State *L){
	int i;
	int top = lua_gettop(L);
//	int top = 20;//debug
	for(i = 0;i < top;i++){
		int t = lua_type(L,-1 - i);
		switch(t){
		case LUA_TSTRING:
			LogPrint("vm","str =%s\n",lua_tostring(L,-1 - i));
			break;
		case LUA_TBOOLEAN:
			LogPrint("vm","boolean =%d\n",lua_toboolean(L,-1 - i));
			break;
		case LUA_TNUMBER:
			LogPrint("vm","number =%d\n",lua_tonumber(L,-1 - i));
			break;
		default:
			LogPrint("vm","default =%s\n",lua_typename(L,-1 - i));
			break;
		}
	   LogPrint("vm"," ");
	}
	LogPrint("vm","\n");
}
#endif

static bool getfieldTable(lua_State *L,char * pKey,unsigned short usLen,vector<unsigned char> &vOut){
	//从栈里取 指定长度的table
	if((L == NULL) || (pKey == NULL) || (usLen < 1)){
		LogPrint("vm","getfieldTable para error\n");
		return false;
    }
	//默认栈顶是table，将key入栈
    lua_pushstring(L,pKey);
    lua_gettable(L,1);
    if(!lua_istable(L,-1))
    {
    	LogPrint("vm","getfieldTable is not table\n");
    	return false;
    }

    unsigned char value = 0;
    vOut.clear();
	for (int i = 0; i < usLen; ++i)
	{
		lua_pushnumber(L, i+1);
		lua_gettable(L, -2);
		if(!lua_isnumber(L,-1))
		{
			LogPrint("vm","getfieldTable is not number\n");
			return false;
		}
		value = 0;
		value = lua_tonumber(L, -1);
		vOut.insert(vOut.begin(),value);  //最前面插入
		lua_pop(L, 1);
	}
    lua_pop(L,1); //删掉产生的查找结果
    return true;

}

static bool GetDataTableMoney(lua_State *L, vector<std::shared_ptr < std::vector<unsigned char> > > &ret) {
	// 取 int64_t ,int64_t
	int totallen = lua_gettop(L);
	if(totallen <= 0)
	{
		return false;
	}
//	LogPrint("vm","totallen =%d\n",totallen);
    if(!lua_istable(L,-1))
    {
    	LogPrint("vm","is not table\n");
    	return false;
    }

    vector<unsigned char> vBuf ;
    if(!getfieldTable(L,(char *)"money1",sizeof(int64_t),vBuf))
    {
    	return false;
    }else{
    	ret.insert(ret.end(),std::make_shared<vector<unsigned char>>(vBuf.begin(), vBuf.end()));
    	LogPrint("vm", "money1:%s, len:%d\n", HexStr(vBuf).c_str(), vBuf.size());
    }
    if(!getfieldTable(L,(char *)"money2",sizeof(int64_t),vBuf))
    {
    	return false;
    }else{
    	ret.insert(ret.end(),std::make_shared<vector<unsigned char>>(vBuf.begin(), vBuf.end()));
    	LogPrint("vm", "money2:%s, len:%d\n", HexStr(vBuf).c_str(), vBuf.size());
    }
	return true;
}

/**
 *COMP_RET Int64Compare(const Int64* const pM1, const Int64* const pM2)
 * 这个函数式从中间层传了两个参数过来:
 * 1.第一个是int64_t类型的数据
 * 2.第一个是int64_t类型的数据
 */
static int ExInt64CompFunc(lua_State *L) {
	vector<std::shared_ptr < vector<unsigned char> > > retdata;
	 if(!GetDataTableMoney(L,retdata) || retdata.size() != 2||
	    	retdata.at(0).get()->size() != sizeof(int64_t)||retdata.at(1).get()->size() != sizeof(int64_t))
	    {
		 return RetFalse("ExInt64CompFunc para err1");
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

    CDataStream tep(SER_DISK, CLIENT_VERSION);
    tep << rslt;
    vector<unsigned char> tep1(tep.begin(),tep.end());
    return  RetRstToLua(L,tep1);
}

/**
 *bool Int64Mul(const Int64* const pM1, const Int64* const pM2, Int64* const pOutM)
 * 这个函数式从中间层传了两个参数过来:
 * 1.第一个是int64_t类型的数据
 * 2.第一个是int64_t类型的数据
 */
static int ExInt64MullFunc(lua_State *L) {
	vector<std::shared_ptr < vector<unsigned char> > > retdata;

	 if(!GetDataTableMoney(L,retdata) ||retdata.size() != 2||
	        	retdata.at(0).get()->size() != sizeof(int64_t)||retdata.at(1).get()->size() != sizeof(int64_t))
	    {
    	return RetFalse("ExInt64MullFunc para err1");
    }

	int64_t m1, m2, m3;
	memcpy(&m1, &retdata.at(0).get()->at(0), sizeof(m1));
	memcpy(&m2, &retdata.at(1).get()->at(0), sizeof(m2));
	m3 = m1 * m2;

    CDataStream tep(SER_DISK, CLIENT_VERSION);
    tep << m3;
    vector<unsigned char> tep1(tep.begin(),tep.end());
    return  RetRstToLua(L,tep1);
}
/**
 *bool Int64Add(const Int64* const pM1, const Int64* const pM2, Int64* const pOutM)
 * 这个函数式从中间层传了两个参数过来:
 * 1.第一个是int64_t类型的数据
 * 2.第一个是int64_t类型的数据
 */
static int ExInt64AddFunc(lua_State *L) {
	vector<std::shared_ptr < vector<unsigned char> > > retdata;

    if(!GetDataTableMoney(L,retdata) ||retdata.size() != 2||
        retdata.at(0).get()->size() != sizeof(int64_t)||retdata.at(1).get()->size() != sizeof(int64_t))
    {
    	return RetFalse("ExInt64AddFunc para err1");
    }

	int64_t m1, m2, m3;
	memcpy(&m1,  &retdata.at(0).get()->at(0), sizeof(m1));
	memcpy(&m2,  &retdata.at(1).get()->at(0), sizeof(m2));
	m3 = m1 + m2;

    CDataStream tep(SER_DISK, CLIENT_VERSION);
    tep << m3;
    vector<unsigned char> tep1(tep.begin(),tep.end());
    return  RetRstToLua(L,tep1);
}
/**
 *bool Int64Sub(const Int64* const pM1, const Int64* const pM2, Int64* const pOutM)
 * 这个函数式从中间层传了两个参数过来:
 * 1.第一个是int64_t类型的数据
 * 2.第一个是int64_t类型的数据
 */
static int ExInt64SubFunc(lua_State *L) {
	vector<std::shared_ptr < vector<unsigned char> > > retdata;

    if(!GetDataTableMoney(L,retdata) ||retdata.size() != 2||
            retdata.at(0).get()->size() != sizeof(int64_t)||retdata.at(1).get()->size() != sizeof(int64_t))
    {
    	return RetFalse("ExInt64SubFunc para err1");
    }
	int64_t m1, m2, m3;
	memcpy(&m1, &retdata.at(0).get()->at(0), sizeof(m1));
	memcpy(&m2, &retdata.at(1).get()->at(0), sizeof(m2));
	m3 = m1 - m2;

    CDataStream tep(SER_DISK, CLIENT_VERSION);
    tep << m3;
    vector<unsigned char> tep1(tep.begin(),tep.end());
    return  RetRstToLua(L,tep1);
}
/**
 *bool Int64Div(const Int64* const pM1, const Int64* const pM2, Int64* const pOutM)
 * 这个函数式从中间层传了两个参数过来:
 * 1.第一个是int64_t类型的数据
 * 2.第一个是int64_t类型的数据
 */
static int ExInt64DivFunc(lua_State *L) {
	vector<std::shared_ptr < vector<unsigned char> > > retdata;
    if(!GetDataTableMoney(L,retdata) ||retdata.size() != 2||
            retdata.at(0).get()->size() != sizeof(int64_t)||retdata.at(1).get()->size() != sizeof(int64_t))
    {
    	return RetFalse("ExInt64DivFunc para err1");
    }
	int64_t m1, m2, m3;
	memcpy(&m1, &retdata.at(0).get()->at(0), sizeof(m1));
	memcpy(&m2, &retdata.at(1).get()->at(0), sizeof(m2));

	if( m2 == 0)
	{
		return 0;
	}
	m3 = m1 / m2;
    CDataStream tep(SER_DISK, CLIENT_VERSION);
    tep << m3;
    vector<unsigned char> tep1(tep.begin(),tep.end());
    return  RetRstToLua(L,tep1);
}

#if 0
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

/**
 *void LogPrint(const void *pdata, const unsigned short datalen,PRINT_FORMAT flag )
 * 这个函数式从中间层传了两个个参数过来:
 * 1.第一个是打印数据的表示符号，true是一十六进制打印,否则以字符串的格式打印
 * 2.第二个是打印的字符串
 */
static int ExLogPrintFunc(lua_State *L) {
	vector<std::shared_ptr < vector<unsigned char> > > retdata;
//    if(!GetData(ipara,retdata) ||retdata.size() != 2)
//    {
//    	return RetFalse(string(__FUNCTION__)+"para  err !");
//    }
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

#endif
/**
 *unsigned short GetAccounts(const unsigned char *txhash,void* const paccoutn,unsigned short maxlen)
 * 这个函数式从中间层传了一个参数过来:
 * 1.第一个是 hash
 */
static int ExGetTxAccountsFunc(lua_State *L) {
	vector<std::shared_ptr<vector<unsigned char> > > retdata;
    if(!GetData(L,retdata) ||retdata.size() != 1|| retdata.at(0).get()->size() != 32)
    {
    	return RetFalse("ExGetTxAccountsFunc para err1");
    }

	CDataStream tep1(*retdata.at(0), SER_DISK, CLIENT_VERSION);
	uint256 hash1;
	tep1 >>hash1;
//	LogPrint("vm","ExGetTxAccountsFunc:%s",hash1.GetHex().c_str());
	std::shared_ptr<CBaseTransaction> pBaseTx;

	auto tem = make_shared<std::vector<vector<unsigned char> > >();
    int len = 0;
	if (GetTransaction(pBaseTx, hash1, *pVmRunEvn->GetScriptDB(), false)) {
		CTransaction *tx = static_cast<CTransaction*>(pBaseTx.get());
		vector<unsigned char> item = boost::get<CRegID>(tx->srcRegId).GetVec6();
		len = RetRstToLua(L,item);
	}
	return len;
}

/**
 *unsigned short GetAccountPublickey(const void* const accounid,void * const pubkey,const unsigned short maxlength)
 * 这个函数式从中间层传了一个参数过来:
 * 1.第一个是 账户id,六个字节
 */
static int ExGetAccountPublickeyFunc(lua_State *L) {
	vector<std::shared_ptr<vector<unsigned char> > > retdata;
    if(!GetData(L,retdata) ||retdata.size() != 1
    	|| !(retdata.at(0).get()->size() == 6 || retdata.at(0).get()->size() == 34))
    {
    	return RetFalse("ExGetAccountPublickeyFunc para err1");
    }

	 CKeyID addrKeyId;
	 if (!GetKeyId(*(pVmRunEvn->GetCatchView()),*retdata.at(0).get(), addrKeyId)) {
	    	return RetFalse("ExGetAccountPublickeyFunc para err2");
	 }

	CUserID userid(addrKeyId);
	CAccount aAccount;
	if (!pVmRunEvn->GetCatchView()->GetAccount(userid, aAccount)) {
		return RetFalse("ExGetAccountPublickeyFunc para err3");
	}

    CDataStream tep(SER_DISK, CLIENT_VERSION);
    vector<char> te;
    tep << aAccount.PublicKey;
    assert(aAccount.PublicKey.IsFullyValid());
    tep >>te;
    vector<unsigned char> tep1(te.begin(),te.end());
    int len =  RetRstToLua(L,tep1);

	return len;
}


/**
 *bool QueryAccountBalance(const unsigned char* const account,Int64* const pBalance)
 * 这个函数式从中间层传了一个参数过来:
 * 1.第一个是 账户id,六个字节
 */
static int ExQueryAccountBalanceFunc(lua_State *L) {
	vector<std::shared_ptr < vector<unsigned char> > > retdata;
    if(!GetData(L,retdata) ||retdata.size() != 1
    	|| !(retdata.at(0).get()->size() == 6 || retdata.at(0).get()->size() == 34))
    {
    	return RetFalse("ExQueryAccountBalanceFunc para err1");
    }

	 CKeyID addrKeyId;
	 if (!GetKeyId(*(pVmRunEvn->GetCatchView()),*retdata.at(0).get(), addrKeyId)) {
	    	return RetFalse("ExQueryAccountBalanceFunc para err2");
	 }

	 CUserID userid(addrKeyId);
	 CAccount aAccount;
	 int len = 0;
	if (!pVmRunEvn->GetCatchView()->GetAccount(userid, aAccount)) {
		len = 0;
	}
	else
	{
		uint64_t nbalance = aAccount.GetRawBalance();
		CDataStream tep(SER_DISK, CLIENT_VERSION);
		tep << nbalance;
		vector<unsigned char> TMP(tep.begin(),tep.end());
		len = RetRstToLua(L,TMP);
	}
	return len;
}

/**
 *unsigned long GetTxConFirmHeight(const void * const txhash)
 * 这个函数式从中间层传了一个参数过来:
 * 1.第一个入参: hash,32个字节
 */
static int ExGetTxConFirmHeightFunc(lua_State *L) {
	vector<std::shared_ptr < vector<unsigned char> > > retdata;

    if(!GetData(L,retdata) ||retdata.size() != 1|| retdata.at(0).get()->size() != 32)
    {
    	return RetFalse("ExGetTxConFirmHeightFunc para err1");
    }
	uint256 hash1(*retdata.at(0));
//	LogPrint("vm","ExGetTxContractsFunc1:%s",hash1.GetHex().c_str());

	int nHeight = GetTxComfirmHigh(hash1, *pVmRunEvn->GetScriptDB());
	if(-1 == nHeight)
	{
		return RetFalse("ExGetTxConFirmHeightFunc para err2");
	}

   CDataStream tep(SER_DISK, CLIENT_VERSION);
	tep << nHeight;
	vector<unsigned char> TMP(tep.begin(),tep.end());
	return RetRstToLua(L,TMP);
}

/**
 *bool GetBlockHash(const unsigned long height,void * const pblochHash)
 * 这个函数式从中间层传了一个参数过来:
 * 1.第一个是 int类型的参数
 */
static int ExGetBlockHashFunc(lua_State *L) {
	vector<std::shared_ptr < vector<unsigned char> > > retdata;
//	CVmRunEvn *pVmScript = (CVmRunEvn *)pVmScriptRun;
    if(!GetData(L,retdata) ||retdata.size() != 1 || retdata.at(0).get()->size() != sizeof(int))
    {
    	return RetFalse("ExGetBlockHashFunc para err1");
    }
	int height = 0;
	memcpy(&height, &retdata.at(0).get()->at(0), sizeof(int));
	if (height <= 0 || height >= pVmRunEvn->GetComfirHeight()) //当前block 是不可以获取hash的
	{
		return RetFalse("ExGetBlockHashFunc para err2");
	}

	if(chainActive.Height() < height){	         //获取比当前高度高的数据是不可以的
		return RetFalse("ExGetBlockHashFunc para err3");
	}
	CBlockIndex *pindex = chainActive[height];
	uint256 blockHash = pindex->GetBlockHash();

//	LogPrint("vm","ExGetBlockHashFunc:%s",HexStr(blockHash).c_str());
    CDataStream tep(SER_DISK, CLIENT_VERSION);
    tep << blockHash;
    vector<unsigned char> TMP(tep.begin(),tep.end());
    return RetRstToLua(L,TMP);
}

static int ExGetCurRunEnvHeightFunc(lua_State *L) {
//	CVmRunEvn *pVmRunEvn = (CVmRunEvn *)pVmEvn;
	int height = pVmRunEvn->GetComfirHeight();

    CDataStream tep(SER_DISK, CLIENT_VERSION);
    tep << height;
    vector<unsigned char> tep1(tep.begin(),tep.end());
    return RetRstToLua(L,tep1);
}

/**
 *bool WriteDataDB(const void* const key,const unsigned char keylen,const void * const value,const unsigned short valuelen,const unsigned long time)
 * 这个函数式从中间层传了三个个参数过来:
 * 1.第一个是 key值
 * 2.第二个是value值
 */
static int ExWriteDataDBFunc(lua_State *L) {
	vector<std::shared_ptr < vector<unsigned char> > > retdata;

    if(!GetData(L,retdata) ||retdata.size() != 2)
    {
    	return RetFalse("ExWriteDataDBFunc key err1");
    }

	const CRegID scriptid = pVmRunEvn->GetScriptRegID();
	bool flag = true;
	CScriptDBViewCache* scriptDB = pVmRunEvn->GetScriptDB();

	CScriptDBOperLog operlog;
//	int64_t step = (*retdata.at(1)).size() -1;
	if (!scriptDB->SetScriptData(scriptid, *retdata.at(0), *retdata.at(1),operlog)) {
		flag = false;
	} else {
		shared_ptr<vector<CScriptDBOperLog> > m_dblog = pVmRunEvn->GetDbLog();
		(*m_dblog.get()).push_back(operlog);
	}
    CDataStream tep(SER_DISK, CLIENT_VERSION);
    tep << flag;
    vector<unsigned char> tep1(tep.begin(),tep.end());
	return RetRstToLua(L,tep1);
}

/**
 *bool DeleteDataDB(const void* const key,const unsigned char keylen)
 * 这个函数式从中间层传了一个参数过来:
 * 1.第一个是 key值
 */
static int ExDeleteDataDBFunc(lua_State *L) {
	vector<std::shared_ptr < vector<unsigned char> > > retdata;

	if(!GetData(L,retdata) ||retdata.size() != 1)
    {
    	LogPrint("vm", "GetData return error!\n");
    	return RetFalse(string(__FUNCTION__)+"para  err !");
    }
	CRegID scriptid = pVmRunEvn->GetScriptRegID();

	CScriptDBViewCache* scriptDB = pVmRunEvn->GetScriptDB();

	bool flag = true;
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
    CDataStream tep(SER_DISK, CLIENT_VERSION);
    tep << flag;
    vector<unsigned char> tep1(tep.begin(),tep.end());
    return RetRstToLua(L,tep1);
}

/**
 *unsigned short ReadDataValueDB(const void* const key,const unsigned char keylen, void* const value,unsigned short const maxbuffer)
 * 这个函数式从中间层传了一个参数过来:
 * 1.第一个是 key值
 */
static int ExReadDataValueDBFunc(lua_State *L) {
	vector<std::shared_ptr < vector<unsigned char> > > retdata;

    if(!GetData(L,retdata) ||retdata.size() != 1)
    {
    	return RetFalse("ExReadDataValueDBFunc key err1");
    }

	CRegID scriptid = pVmRunEvn->GetScriptRegID();

	vector_unsigned_char vValue;
	CScriptDBViewCache* scriptDB = pVmRunEvn->GetScriptDB();
	int len = 0;
	if(!scriptDB->GetScriptData(pVmRunEvn->GetComfirHeight(),scriptid, *retdata.at(0), vValue))
	{
		len = 0;
	}
	else
	{
		len = RetRstToLua(L,vValue);
	}
	return len;
}


static int ExGetDBSizeFunc(lua_State *L) {
//	CVmRunEvn *pVmRunEvn = (CVmRunEvn *)pVmEvn;
	CRegID scriptid = pVmRunEvn->GetScriptRegID();
	int count = 0;
	CScriptDBViewCache* scriptDB = pVmRunEvn->GetScriptDB();
	if(!scriptDB->GetScriptDataCount(scriptid,count))
	{
		return RetFalse("ExGetDBSizeFunc can't use");
	}
	else
	{
		CDataStream tep(SER_DISK, CLIENT_VERSION);
		tep << count;
		vector<unsigned char> tep1(tep.begin(),tep.end());
        return RetRstToLua(L,tep1);
	}
}

/**
 *bool GetDBValue(const unsigned long index,void* const key,unsigned char * const keylen,unsigned short maxkeylen,void* const value,unsigned short* const maxbuffer, unsigned long* const ptime)
 * 当传的第一个参数index == 0，则传了一个参数过来
 * 1.第一个是 index值
 * 当传的第一个参数index == 1，则传了两个个参数过来
 * 1.第一个是 index值
 * 2.第二是key值
 */
static int ExGetDBValueFunc(lua_State *L) {

	if (SysCfg().GetArg("-isdbtraversal", 0) == 0) {
    	return RetFalse("ExGetDBValueFunc can't use");
	}

	vector<std::shared_ptr < vector<unsigned char> > > retdata;
    if(!GetData(L,retdata) ||(retdata.size() != 2 && retdata.size() != 1))
    {
    	return RetFalse("ExGetDBValueFunc index err1");
    }
	int index = 0;
	bool flag = true;
	memcpy(&index,&retdata.at(0).get()->at(0),sizeof(int));
	if(!(index == 0 ||(index == 1 && retdata.size() == 2)))
	{
	    return RetFalse("ExGetDBValueFunc para err2");
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
    int len = 0;
	if(flag){
	    len = RetRstToLua(L,vScriptKey) + RetRstToLua(L,vValue);
	}
	return len;
}

static int ExGetCurTxHash(lua_State *L) {
	uint256 hash = pVmRunEvn->GetCurTxHash();
    CDataStream tep(SER_DISK, CLIENT_VERSION);
    tep << hash;
    vector<unsigned char> tep1(tep.begin(),tep.end());
    return RetRstToLua(L,tep1);
}

/**
 *bool ModifyDataDBVavle(const void* const key,const unsigned char keylen, const void* const pvalue,const unsigned short valuelen)
 * 中间层传了两个参数
 * 1.第一个是 key
 * 2.第二个是 value
 */
static int ExModifyDataDBValueFunc(lua_State *L)
{
	vector<std::shared_ptr < vector<unsigned char> > > retdata;
	if(!GetData(L,retdata) ||retdata.size() != 2 )
    {
    	return RetFalse("ExModifyDataDBValueFunc key err1");
    }

	CRegID scriptid = pVmRunEvn->GetScriptRegID();
	bool flag = false;
	CScriptDBViewCache* scriptDB = pVmRunEvn->GetScriptDB();

//	int64_t step = 0;
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

//	step =(((int64_t)(*retdata.at(1)).size())- (int64_t)(vTemp.size()) -1);
    CDataStream tep(SER_DISK, CLIENT_VERSION);
    tep << flag;
    vector<unsigned char> TMP(tep.begin(),tep.end());
    return RetRstToLua(L,TMP);
}


static bool GetDataTableWriteOutput(lua_State *L, vector<std::shared_ptr < std::vector<unsigned char> > > &ret) {
	int totallen = lua_gettop(L);
	unsigned short len = 0;
	if(totallen <= 0)
	{
		LogPrint("vm", "GetDataTableWriteOutput totallen error\n");
		return false;
	}

//	LogPrint("vm","totallen =%d\n",totallen);
    if(!lua_istable(L,-1))
    {
    	LogPrint("vm","GetDataTableWriteOutput is not table\n");
    	return false;
    }
    vector<unsigned char> vBuf ;
    CVmOperate temp;
	temp.nacctype = getfield(L,(char *)"addrType");
	if(temp.nacctype == 1)
	{
       len = 6;
	}else if(temp.nacctype == 2){
       len = 34;
	}else{
		LogPrint("vm", "error nacctype:%d\n", temp.nacctype);
		return false;
	}
	memset(temp.accountid,0,sizeof(temp.accountid));
    if(!getfieldTable(L,(char *)"accountidTbl",len,vBuf))
    {
    	LogPrint("vm","moneyTbl not table\n");
    	return false;
    }else{
       memcpy(&temp.accountid,&vBuf[0],len);
       LogPrint("vm", "accountid:%s\n", HexStr(vBuf).c_str());
    }
    temp.opeatortype = getfield(L,(char *)"opeatortype");
    LogPrint("vm", "opeatortype:%d\n", temp.opeatortype);
    temp.outheight = getfield(L,(char *)"outheight");
    LogPrint("vm", "outheight:%d\n", temp.outheight);
	memset(temp.money,0,sizeof(temp.money));
	if(!getfieldTable(L,(char *)"moneyTbl",sizeof(temp.money),vBuf))
	{
		LogPrint("vm","moneyTbl not table\n");
		return false;
	}else{
		memcpy(temp.money,&vBuf[0],sizeof(temp.money));
		LogPrint("vm", "money:%s\n", HexStr(vBuf).c_str());
	}

    CDataStream tep(SER_DISK, CLIENT_VERSION);
    tep << temp;
    vector<unsigned char> tep1(tep.begin(),tep.end());
	ret.insert(ret.end(),std::make_shared<vector<unsigned char>>(tep1.begin(), tep1.end()));
	return true;
}
/**
 *bool WriteOutput( const VM_OPERATE* data, const unsigned short conter)
 * 中间层传了一个参数 ,写 CVmOperate操作结果
 * 1.第一个是输出指令
 */
static int ExWriteOutputFunc(lua_State *L)
{
	vector<std::shared_ptr < vector<unsigned char> > > retdata;

    if(!GetDataTableWriteOutput(L,retdata) ||retdata.size() != 1 )
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
//	  assert(0);
	 return RetFalse("para err");
	}
	CDataStream ss(*retdata.at(0),SER_DISK, CLIENT_VERSION);

	while(count--)
	{
		ss >> temp;
      source.push_back(temp);
	}
	auto tem =  make_shared<std::vector< vector<unsigned char> > >();
	if(!pVmRunEvn->InsertOutputData(source)) {
		 return RetFalse("InsertOutput err");
	}

	/*
	* 每个函数里的Lua栈是私有的,当把返回值压入Lua栈以后，该栈会自动被情况*/
	return 0;
}




/**
 *bool GetScriptData(const void* const scriptID,void* const pkey,short len,void* const pvalve,short maxlen)
 * 中间层传了两个个参数
 * 1.脚本的id号
 * 2.数据库的key值
 */
static int ExGetScriptDataFunc(lua_State *L)
{
	vector<std::shared_ptr < vector<unsigned char> > > retdata;

    if(!GetData(L,retdata) ||retdata.size() != 2 || retdata.at(0).get()->size() != 6)
    {
    	return RetFalse("ExModifyDataDBValueFunc tep1 err1");
    }

	vector_unsigned_char vValue;
//	CVmRunEvn *pVmRunEvn = (CVmRunEvn *)pVmEvn;
	CScriptDBViewCache* scriptDB = pVmRunEvn->GetScriptDB();
	CRegID scriptid(*retdata.at(0));
    int len = 0;
	if(!scriptDB->GetScriptData(pVmRunEvn->GetComfirHeight(), scriptid, *retdata.at(1), vValue))
	{
		len = 0;
	}
	else
	{
	   //3.往函数私有栈里存运算后的结果
		len = RetRstToLua(L,vValue);
	}
   /*
	* 每个函数里的Lua栈是私有的,当把返回值压入Lua栈以后，该栈会自动被情况*/
	return len;
}



/**
 * 取目的账户ID
 * @param ipara
 * @param pVmEvn
 * @return
 */
static int ExGetScriptIDFunc(lua_State *L)
{
   //1.从lua取参数
   //2.调用C++库函数 执行运算
	vector_unsigned_char scriptid = pVmRunEvn->GetScriptRegID().GetVec6();
   //3.往函数私有栈里存运算后的结果
	int len = RetRstToLua(L,scriptid);
   /*
	* 每个函数里的Lua栈是私有的,当把返回值压入Lua栈以后，该栈会自动被情况*/
	return len; //number of results 告诉Lua返回了几个返回值
}
static int ExGetCurTxAccountFunc(lua_State *L)
{
   //1.从lua取参数
   //2.调用C++库函数 执行运算
	vector_unsigned_char vUserId =pVmRunEvn->GetTxAccount().GetVec6();

   //3.往函数私有栈里存运算后的结果
	int len = RetRstToLua(L,vUserId);
   /*
    * 每个函数里的Lua栈是私有的,当把返回值压入Lua栈以后，该栈会自动被情况*/
	return len; //number of results 告诉Lua返回了几个返回值
}

#if 0
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

#endif

static int GetCurTxPayAmountFunc(lua_State *L){

	uint64_t lvalue =pVmRunEvn->GetValue();

    CDataStream tep(SER_DISK, CLIENT_VERSION);
    tep << lvalue;
    vector<unsigned char> tep1(tep.begin(),tep.end());
    int len = RetRstToLua(L,tep1);
    /*
    * 每个函数里的Lua栈是私有的,当把返回值压入Lua栈以后，该栈会自动被情况*/
   	return len; //number of results 告诉Lua返回了几个返回值
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

static int GetUserAppAccValue(lua_State *L){

	vector<std::shared_ptr < vector<unsigned char> > > retdata;

	int totallen = lua_gettop(L);
	if(totallen <= 0)
	{
		LogPrint("vm", "GetDataTableOutAppOperate totallen error\n");
		return 0;
	}

//	LogPrint("vm","totallen =%d\n",totallen);
    if(!lua_istable(L,-1))
    {
    	LogPrint("vm","is not table\n");
    	return 0;
    }

    vector<unsigned char> vBuf ;
    S_APP_ID accid;
    memset(&accid,0,sizeof(accid));
    accid.idlen = getfield(L,(char *)"idlen");
    if(accid.idlen < 1){
    	return 0;
    }
    if(!getfieldTable(L,(char *)"idValueTbl",accid.idlen,vBuf))
    {
    	LogPrint("vm","idValueTbl not table\n");
    	return false;
    }else{
       memcpy(&accid.ID[0],&vBuf[0],accid.idlen);
       LogPrint("vm", "ID:%s\n", HexStr(vBuf).c_str());
    }

//    if(!GetData(L,retdata) ||retdata.size() != 1
//    	|| retdata.at(0).get()->size() != sizeof(S_APP_ID))
//    {
//    	return RetFalse("GetUserAppAccValue para err0");
//    }
//    S_APP_ID accid;
//    memcpy(&accid, &retdata.at(0).get()->at(0), sizeof(S_APP_ID));

   	shared_ptr<CAppUserAccout> sptrAcc;
   	uint64_t valueData = 0 ;
   	int len = 0;
   	if(pVmRunEvn->GetAppUserAccout(accid.GetIdV(),sptrAcc))
	{
   		valueData = sptrAcc->getllValues();

		CDataStream tep(SER_DISK, CLIENT_VERSION);
		tep << valueData;
		vector<unsigned char> TMP(tep.begin(),tep.end());
		len = RetRstToLua(L,TMP);
	}
    return len;
}


static int GetUserAppAccFoudWithTag(lua_State *L){
	vector<std::shared_ptr < vector<unsigned char> > > retdata;
	unsigned int Size(0);
	CAppFundOperate temp;
	Size = ::GetSerializeSize(temp, SER_NETWORK, PROTOCOL_VERSION);

    if(!GetData(L,retdata) ||retdata.size() != 1
    	|| retdata.at(0).get()->size() !=Size)
    {
			 return RetFalse("GetUserAppAccFoudWithTag para err0");
    }

    CDataStream ss(*retdata.at(0),SER_DISK, CLIENT_VERSION);
    CAppFundOperate userfund;
    ss>>userfund;

   	shared_ptr<CAppUserAccout> sptrAcc;
    CAppCFund fund;
	int len = 0;
	if(pVmRunEvn->GetAppUserAccout(userfund.GetAppUserV(),sptrAcc))
	{
		if(!sptrAcc->GetAppCFund(fund,userfund.GetFundTagV(),userfund.outheight))	{
			return RetFalse("GetUserAppAccFoudWithTag para err1");
		}
		CDataStream tep(SER_DISK, CLIENT_VERSION);
		tep << fund.getvalue() ;
		vector<unsigned char> TMP(tep.begin(),tep.end());
		len = RetRstToLua(L,TMP);
	}
    return len;
}
static bool GetDataTableOutAppOperate(lua_State *L, vector<std::shared_ptr < std::vector<unsigned char> > > &ret) {
	int totallen = lua_gettop(L);
	if(totallen <= 0)
	{
		LogPrint("vm", "GetDataTableOutAppOperate totallen error\n");
		return false;
	}

//	LogPrint("vm","totallen =%d\n",totallen);
    if(!lua_istable(L,-1))
    {
    	LogPrint("vm","is not table\n");
    	return false;
    }
    vector<unsigned char> vBuf ;
	CAppFundOperate temp;
	temp.opeatortype = getfield(L,(char *)"opeatortype");
	temp.outheight = getfield(L,(char *)"outheight");
	LogPrint("vm", "opeatortype:%d\n", temp.opeatortype);
	LogPrint("vm", "outheight:%d\n", temp.outheight);
    if(!getfieldTable(L,(char *)"moneyTbl",sizeof(temp.mMoney),vBuf))
    {
    	LogPrint("vm","moneyTbl not table\n");
    	return false;
    }else{
       memcpy(&temp.mMoney,&vBuf[0],sizeof(temp.mMoney));
       LogPrint("vm", "moneyTbl:%s\n", HexStr(vBuf).c_str());
    }
    temp.appuserIDlen = getfield(L,(char *)"useridlen");
    LogPrint("vm", "appuserIDlen:%d\n", temp.appuserIDlen);
	memset(temp.vAppuser,0,sizeof(temp.vAppuser));
	if(!getfieldTable(L,(char *)"useridTbl",temp.appuserIDlen,vBuf))
	{
		LogPrint("vm","useridTbl not table\n");
		return false;
	}else{
		memcpy(temp.vAppuser,&vBuf[0],temp.appuserIDlen);
		LogPrint("vm", "vAppuser:%s\n", HexStr(vBuf).c_str());
	}
    temp.FundTaglen = getfield(L,(char *)"FundTaglen");
    LogPrint("vm", "FundTaglen:%d\n", temp.FundTaglen);
	memset(temp.vFundTag,0,sizeof(temp.vFundTag));
    if(temp.FundTaglen > 0)
    {
		if(!getfieldTable(L,(char *)"FundTagTbl",temp.FundTaglen,vBuf))
		{
			LogPrint("vm","FundTagTbl not table\n");
			return false;
		}else{
			memcpy(temp.vFundTag,&vBuf[0],temp.FundTaglen);
			LogPrint("vm", "vFundTag:%s\n", HexStr(vBuf).c_str());
		}
    }
    CDataStream tep(SER_DISK, CLIENT_VERSION);
    tep << temp;
    vector<unsigned char> tep1(tep.begin(),tep.end());
	ret.insert(ret.end(),std::make_shared<vector<unsigned char>>(tep1.begin(), tep1.end()));
	return true;
}
/**
 *   写 应用操作输出到 pVmRunEvn->MapAppOperate[0]
 * @param ipara
 * @param pVmEvn
 * @return
 */
static int ExWriteOutAppOperateFunc(lua_State *L)
{
	vector<std::shared_ptr < vector<unsigned char> > > retdata;

	CAppFundOperate temp;
	unsigned int Size = ::GetSerializeSize(temp, SER_NETWORK, PROTOCOL_VERSION);

	if(!GetDataTableOutAppOperate(L,retdata) ||retdata.size() != 1 || (retdata.at(0).get()->size()%Size) != 0 )
    {
  		 return RetFalse("ExWriteOutAppOperateFunc para err1");
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

	return 0;
}
static int ExGetBase58AddrFunc(lua_State *L){
	vector<std::shared_ptr < vector<unsigned char> > > retdata;
//	int totallen = lua_gettop(L);
//	if(totallen <= 0)
//	{
//		return false;
//	}
//	LogPrint("vm","totallen =%d\n",totallen);
//	if(!lua_istable(L,-1))
//	{
//		LogPrint("vm","is not table\n");
//		return false;
//	}
	vector<unsigned char> vBuf ;
	if(!getfieldTable(L,(char *)"accountTbl",6,vBuf))
	{
		return 0;
	}else{
		retdata.insert(retdata.end(),std::make_shared<vector<unsigned char>>(vBuf.begin(), vBuf.end()));
		LogPrint("vm", "accountTbl:%s, len:%d\n", HexStr(vBuf).c_str(), vBuf.size());
	}
    if(retdata.size() != 1 || retdata.at(0).get()->size() != 6) //!GetData(L,retdata) ||
    {
    	return RetFalse("ExGetBase58AddrFunc para err0");
    }

	 CKeyID addrKeyId;
	 if (!GetKeyId(*pVmRunEvn->GetCatchView(),*retdata.at(0).get(), addrKeyId)) {
		    return RetFalse("ExGetBase58AddrFunc para err1");
	 }
	 string dacrsaddr = addrKeyId.ToAddress();
	 int len = dacrsaddr.length();
	 lua_pushlstring(L,dacrsaddr.c_str(),len);
	 return len;
}


static const luaL_Reg mylib[] = { //
		{ "Int64Compare", ExInt64CompFunc },			//
		{ "Int64Mull", ExInt64MullFunc },			//
		{ "Int64Add", ExInt64AddFunc },			//
		{ "Int64Sub", ExInt64SubFunc },			//
		{ "Int64Div", ExInt64DivFunc },			//
//		{ SHA256_FUNC, ExSha256Func },			//
//		{ DES_FUNC, ExDesFunc },			    //
//		{ "VerifySignature", ExVerifySignatureFunc },   //
//		{ SIGNATURE_FUNC, ExSignatureFunc },			//
//		{ "LogPrint", ExLogPrintFunc },         //
//		{GETTX_CONTRACT_FUNC,ExGetTxContractsFunc},            //
		{"GetTxAccounts",ExGetTxAccountsFunc},
		{"GetAccountPublickey",ExGetAccountPublickeyFunc},
		{"QueryAccountBalance",ExQueryAccountBalanceFunc},
		{"GetTxConFirmHeight",ExGetTxConFirmHeightFunc},
		{"GetBlockHash",ExGetBlockHashFunc},


		{"GetCurRunEnvHeight",ExGetCurRunEnvHeightFunc},
		{"WriteDataDB",ExWriteDataDBFunc},
		{"DeleteDataDB",ExDeleteDataDBFunc},
		{"ReadDataValueDB",ExReadDataValueDBFunc},
		{"GetDBSize",ExGetDBSizeFunc},
		{"GetDBValue",ExGetDBValueFunc},
		{"GetCurTxHash",ExGetCurTxHash},
		{"ModifyDataDBValue",ExModifyDataDBValueFunc},
		{"WriteOutput",ExWriteOutputFunc},
		{"GetScriptData",ExGetScriptDataFunc},
		{"GetScriptID",ExGetScriptIDFunc},
		{"GetCurTxAccount",ExGetCurTxAccountFunc	  },
//		{GETCURTXCONTACT_FUNC,ExGetCurTxContactFunc		},
//		{GETCURDECOMPRESSCONTACR_FUNC,ExCurDeCompressContactFunc },
//		{GETDECOMPRESSCONTACR_FUNC,ExDeCompressContactFunc  	},
		{"GetCurTxPayAmount",GetCurTxPayAmountFunc},

		{"GetUserAppAccValue",GetUserAppAccValue},
		{"GetUserAppAccFoudWithTag",GetUserAppAccFoudWithTag},
		{"WriteOutAppOperate",ExWriteOutAppOperateFunc},

		{"GetBase58Addr",ExGetBase58AddrFunc},
		{NULL,NULL}

		};

/*
 * 注册一个新Lua模块*/
LUAMOD_API int luaopen_mylib(lua_State *L){
	luaL_newlib(L,mylib);//生成一个table,把mylibs所有函数填充进去
	return 1;
}

