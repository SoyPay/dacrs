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
CVmlua::CVmlua(const vector<unsigned char> & vRom, const vector<unsigned char> &InputData){
	unsigned long len = 0;
	/*vRom 输入的是script,InputData 输入的是contract*/
    memset(m_ExRam,0,sizeof(m_ExRam));
    memset(m_ExeFile,0,sizeof(m_ExeFile));

    len = vRom.size();
    if(len >= sizeof(m_ExeFile)){
    	len = sizeof(m_ExeFile) -1;
    }
	memcpy(m_ExeFile, &vRom[0], len);
	int count = InputData.size();
	memcpy(m_ExRam, &count, 2);
	memcpy(&m_ExRam[2], &InputData[0],count);

}

CVmlua::~CVmlua() {

}
LUAMOD_API int luaopen_mylib(lua_State *L);
int64_t CVmlua::run(uint64_t maxstep,CVmRunEvn *pVmScriptRun) {
	 long long step = 0;
	 int16_t count = 0;

	if((maxstep == 0) || (NULL == pVmScriptRun)){
		return -1;
	}

	//1.创建Lua运行环境
   lua_State *lua_state = luaL_newstate();
   if(NULL == lua_state){
	   LogPrint("vm", "luaL_newstate error\n");
	   return -1;
   }
#if 1
   //2.设置待注册的Lua标准库
   static const luaL_Reg lualibs[] =
   {
	   {"base",luaopen_base},
	   {LUA_LOADLIBNAME, luaopen_package},
//	   {LUA_COLIBNAME, luaopen_coroutine},
	   {LUA_TABLIBNAME, luaopen_table},
	   {LUA_IOLIBNAME, luaopen_io},
//	   {LUA_OSLIBNAME, luaopen_os},
	   {LUA_STRLIBNAME, luaopen_string},
	   {LUA_MATHLIBNAME, luaopen_math},
//	   {LUA_UTF8LIBNAME, luaopen_utf8},
	   {LUA_DBLIBNAME, luaopen_debug},
	   {NULL,NULL}
   };
   //3.注册Lua标准库并清空栈
   const luaL_Reg *lib = lualibs;
   for(;lib->func != NULL;lib++)
   {
	   lib->func(lua_state);
	   lua_settop(lua_state,0);
   }
#else
   //打开需要的库
   luaL_openlibs(lua_state);
#endif
   //注册自定义模块
   luaL_requiref(lua_state,"mylib",luaopen_mylib,1);

//   //注册函数为Lua的全局变量
//   lua_register(lua_state,"GetCurTxAccount",ExGetCurTxAccountFunc);
#if 1
   //4.往lua脚本传递合约内容
//   luaopen_array(lua_state);
//   lua_pop(lua_state,1);
	lua_newtable(lua_state);    //新建一个表,压入栈顶
	lua_pushnumber(lua_state,-1);
	lua_rawseti(lua_state,-2,0);
	memcpy(&count,m_ExRam,  2);
    for(int16_t n = 0;n < count;n++)
    {
        lua_pushinteger(lua_state,m_ExRam[2 + n]);// value值放入
        lua_rawseti(lua_state,-2,n+1);  //set table at key 'n + 1'
    }
    lua_setglobal(lua_state,"contact");
#endif
   //5.加载脚本        //<待确认，解析器脚本最大是否256个byte????? m_ExeFile
    step = maxstep;
//   if(luaL_loadbuffer(lua_state,(char *)m_ExeFile,strlen((char *)m_ExeFile),"line") || lua_pcall(lua_state,0,0,0))
    if(luaL_loadbuffer(lua_state,(char *)m_ExeFile,strlen((char *)m_ExeFile),"line") || lua_pcallk(lua_state,0,0,0,0,NULL,&step))
   {
	   LogPrint("vm", "luaL_loadbuffer fail:%s\n", lua_tostring(lua_state,-1));
	   step = -1;
   }else{
//	   step = 111;
#if 0
	//6.执行脚本  ,调用lua的函数
	   int ret = -1;
       lua_getglobal(lua_state,"main"); //the function name
       lua_pushnumber(lua_state,11);     //push 1st argument
       lua_pushnumber(lua_state,12);     //push 2nd argument
       ret = lua_pcall(lua_state,2,1,0); //要求有一个返回值
       LogPrint("vm", "run ret:%d\n", ret);
	   if(ret != LUA_OK)
       {
    	   LogPrint("vm", "lua_pcall err1:%s\n", lua_tostring(lua_state,-1));
    	   step = -1;
       }else{
    	    //!<校验返回值
           if(!lua_isnumber(lua_state,-1))
           {
        	   LogPrint("vm", "lua_pcall must return a number\n");
        	   step = -1;
           }else{
        	   ret = lua_tonumber(lua_state,-1);
        	   lua_pop(lua_state,1);             //清空栈里的返回值
        	   LogPrint("vm", "run state rslt:%d\n", ret);
			   step = 111;
           }
       }
#endif
   }

    //5.关闭Lua虚拟机
	lua_close(lua_state);
    LogPrint("vm", "run step=%d\n",step);
	return step;
}



