// ComDirver.cpp: implementation of the CComDirver class.
//
//////////////////////////////////////////////////////////////////////

#include "vmlua.h"
#include "lua/lua.hpp"

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

#if 0
typedef struct NumArray{
	int size;          // 数组大小
	double values[1];  //数组缓冲区
}NumArray;

/*
 * 获取userdatum*/
static NumArray *checkarray(lua_State *L){
	//检查在栈中指定位置的对象是否为带有给定名字的metatable的userdatum
    void *ud = luaL_checkudata(L,1,"LuaBook.array");
    luaL_argcheck(L,ud != NULL,1,"'array' expected");
    return (NumArray *)ud;
}
/*
 * 获取索引处的指针*/
static double *getelem(lua_State *L){
    NumArray *a = checkarray(L);
    int index = luaL_checkint(L,2);
    luaL_argcheck(L,1 <= index && index <= a->size,2,"index out of range");
    /*return element address*/
    return &a->values[index - 1];
}
/*
 * 创建新数组*/
int newarray(lua_State *L){
   int n = luaL_checkint(L,1);   //检查证实的luaL_checknumber的变体
   size_t nbytes = sizeof(NumArray) + (n -1) * sizeof(double);

	/*一个userdatum 提供一个在Lua中没有预定义操作的raw内存区域；
          按照指定的大小分配一块内存，将对应的userdatum放到栈内,并返回内存块的地址*/
   NumArray *a = (NumArray *)lua_newuserdata(L,nbytes);
   luaL_getmetatable(L,"LuaBook.array");   //获取registry中的tname对应的metatable
   lua_setmetatable(L,-2);   //将表出栈并将其设置为给定位置的对象的metatable  就是新的userdatum
   a->size = n;
   return 1; /*new userdatnum is already on the statck*/
}
/*
 * 存储元素,array.set(array,index,value)*/
int setarray(lua_State *L){
#if 0
	NumArray *a = (NumArray *)lua_touserdata(L,1);
	int index = luaL_checkint(L,2);
    double value = luaL_checknumber(L,3);

    luaL_argcheck(L,a != NULL,1,"'array' expected");
    luaL_argcheck(L,1 <= index && index <= a->size,2,"index out of range");
    a->values[index -1] = value;
#else
    double newvalue = luaL_checknumber(L,3);
    *getelem(L) = newvalue;
#endif
	return 0;
}
/*
 * 获取一个数组元素*/
int getarray(lua_State *L){
#if 0
	NumArray *a = (NumArray *)lua_touserdata(L,1);
    int index = luaL_checkint(L,2);

    luaL_argcheck(L,a != NULL,1,"'array' expected");
    luaL_argcheck(L,1 <= index && index <= a->size,2,"index out of range");
    lua_pushnumber(L,a->values[index - 1]);
#else
    lua_pushnumber(L,*getelem(L));
#endif
    return 1;
}
/*
 * 获取数组的大小*/
int getsize(lua_State *L){
#if 0
	NumArray *a = (NumArray *)lua_touserdata(L,1);
	luaL_argcheck(L,a != NULL,1,"'array' expected");
#else
	NumArray *a = checkarray(L);
#endif
	lua_pushnumber(L,a->size);
	return 1;
}
static const struct luaL_Reg arraylib[] = {
		{"new",newarray},
		{"set",setarray},
		{"get",getarray},
		{"size",getsize},
		{NULL,NULL}
};
static int luaopen_array(lua_State *L){
	/*创建数组userdata将要用到的metatable*/
	luaL_newmetatable(L,"LuaBook.array");
	luaL_openlib(L,"array",arraylib,0);

	/*now the statck has the metatable at index 1 and
	 * 'array' at index 2*/
    lua_pushstring(L,"__index");
    lua_pushstring(L,"get");
    lua_gettable(L,2); /*get array.get*/
    lua_settable(L,1); /*metatable.__index - array.get*/

    lua_pushstring(L,"__newindex");
    lua_pushstring(L,"set");
    lua_gettable(L,2); /*get array.get*/
    lua_settable(L,1); /*metatable.__newindex - array.get*/
	return 0;
}

#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CVmlua::CVmlua(const vector<unsigned char> & vuchRom, const vector<unsigned char> &vuchInputData) {
	unsigned long ulLen = 0;
	/*vRom 输入的是script,InputData 输入的是contract*/
	memset(m_arruchExRam, 0, sizeof(m_arruchExRam));
	memset(m_arruchExeFile, 0, sizeof(m_arruchExeFile));

	ulLen = vuchRom.size();
	if (ulLen >= sizeof(m_arruchExeFile)) {
		throw runtime_error("CVmlua::CVmlua() length of vRom exceptions");
	}
	memcpy(m_arruchExeFile, &vuchRom[0], ulLen);
	unsigned short usCount = vuchInputData.size();   //外面已限制小于4096字节
	if (usCount > sizeof(m_arruchExRam) - 2) {
		throw runtime_error("CVmlua::CVmlua() length of contract > 4094");
	}
	memcpy(m_arruchExRam, &usCount, 2);
	memcpy(&m_arruchExRam[2], &vuchInputData[0], usCount);
}

CVmlua::~CVmlua() {
}

#ifdef WIN_DLL
extern "C" __declspec(dllexport) int luaopen_mylib(lua_State *L);
#else
LUAMOD_API int luaopen_mylib(lua_State *L);
#endif

void vm_openlibs (lua_State *L) {
	static const luaL_Reg lualibs[] = {
			{ "base", luaopen_base },
			{ LUA_LOADLIBNAME, luaopen_package },
			{ LUA_TABLIBNAME, luaopen_table },
			{ LUA_MATHLIBNAME, luaopen_math },
			{ LUA_STRLIBNAME, luaopen_string},
			{ NULL, NULL }
	};

	const luaL_Reg *lib;

	for (lib = lualibs; lib->func; lib++) {
		luaL_requiref(L, lib->name, lib->func, 1);
		lua_pop(L, 1); /* remove lib */
	}
}

tuple<bool, string> CVmlua::syntaxcheck(bool bFile, const char* pszFilePathOrContent, int nLen) {
	//1.创建Lua运行环境
	lua_State *lua_state = luaL_newstate();
	if (NULL == lua_state) {
		LogPrint("vm", "luaL_newstate error\n");
		return std::make_tuple(false, string("luaL_newstate error\n"));
	}

	vm_openlibs(lua_state);
	//3.注册自定义模块
	luaL_requiref(lua_state, "mylib", luaopen_mylib, 1);

	int nRet = 0;
	if (bFile) {
		nRet = luaL_loadfile(lua_state, pszFilePathOrContent);
	} else {
		nRet = luaL_loadbuffer(lua_state, (char * )pszFilePathOrContent, nLen, "line");
	}
	if (nRet) {
		const char* errStr = lua_tostring(lua_state, -1);
		lua_close(lua_state);
		return std::make_tuple(false, string(errStr));
	}

	lua_close(lua_state);

	return std::make_tuple(true, string("OK"));
}

int64_t CVmlua::run(uint64_t ullMaxStep,CVmRunEvn *pcVmScriptRun) {
	long long llStep = 0;
	unsigned short usCount = 0;

	if ((ullMaxStep == 0) || (NULL == pcVmScriptRun)) {
		return -1;
	}

	//1.创建Lua运行环境
	lua_State *lua_state = luaL_newstate();
	if (NULL == lua_state) {
		LogPrint("vm", "luaL_newstate error\n");
		return -1;
	}

	vm_openlibs(lua_state);
	//3.注册自定义模块
	luaL_requiref(lua_state, "mylib", luaopen_mylib, 1);

	//4.往lua脚本传递合约内容
	lua_newtable(lua_state);    //新建一个表,压入栈顶
	lua_pushnumber(lua_state, -1);
	lua_rawseti(lua_state, -2, 0);
	memcpy(&usCount, m_arruchExRam, 2);    //外面已限制，合约内容小于4096字节
	for (unsigned short n = 0; n < usCount; n++) {
		lua_pushinteger(lua_state, m_arruchExRam[2 + n]);    // value值放入
		lua_rawseti(lua_state, -2, n + 1);  //set table at key 'n + 1'
	}
	lua_setglobal(lua_state, "contract");

	//传递pVmScriptRun指针，以便后面代码引用，去掉了使用全局变量保存该指针
	lua_pushlightuserdata(lua_state, pcVmScriptRun);
	lua_setglobal(lua_state, "VmScriptRun");

	LogPrint("vm", "pVmScriptRun=%p\n", pcVmScriptRun);

	//5.加载脚本
	llStep = ullMaxStep;

	if (luaL_loadbuffer(lua_state, (char * )m_arruchExeFile, strlen((char * )m_arruchExeFile), "line")
			|| lua_pcallk(lua_state, 0, 0, 0, 0, NULL, &llStep)) {
		LogPrint("vm", "luaL_loadbuffer fail:%s\n", lua_tostring(lua_state,-1));
		llStep = -1;
		lua_close(lua_state);
		LogPrint("vm", "run step=%ld\n", llStep);
		return llStep;
	}

	//6.平衡检查设置，默认关闭，如果脚本没设置该变量
	pcVmScriptRun->SetCheckAccount(false);
	int nRes = lua_getglobal(lua_state, "gCheckAccount");
	LogPrint("vm", "lua_getglobal:%d\n", nRes);

	if (LUA_TBOOLEAN == nRes) {
		if (lua_isboolean(lua_state, -1)) {
			bool bCheck = lua_toboolean(lua_state, -1);
			LogPrint("vm", "lua_toboolean:%d\n", bCheck);
			pcVmScriptRun->SetCheckAccount(bCheck);
		}
	}
	lua_pop(lua_state, 1);

	//7.关闭Lua虚拟机
	lua_close(lua_state);
	LogPrint("vm", "run step=%ld\n", llStep);
	return llStep;
}



