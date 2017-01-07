#ifndef DACRS_VM_VMLUA_H_
#define DACRS_VM_VMLUA_H_

#include <cstdio>

#include <vector>
#include <string>
#include <memory>

using namespace std;
class CVmRunEvn;

class CVmlua {
 public:
	CVmlua(const vector<unsigned char> & vuchRom, const vector<unsigned char> &vuchInputData);
	~CVmlua();
	int64_t run(uint64_t ullMaxStep,CVmRunEvn *pcVmScriptRun);
    static tuple<bool,string> syntaxcheck(bool bFile, const char* pszFilePathOrContent, int nLen);
   
 private:
	unsigned char m_arruchExRam[65536];  //存放的是合约交易的contact内容
	unsigned char m_arruchExeFile[65536];//可执行文件 IpboApp.lua
};

#endif
