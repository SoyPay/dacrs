/*
 * VmScript.h
 *
 *  Created on: Sep 2, 2014
 *      Author: ranger.shi
 */

#ifndef VMSCRIPT_H_
#define VMSCRIPT_H_

#include "serialize.h"
using namespace std;

/**
 * @brief Load script binary code class
 */
class CVmScript {

public:
	vector<unsigned char> Rom;      		//!< Binary code
	vector<unsigned char> ScriptExplain;	// !<explain the binary code action

public:
	/**
	 * @brief
	 * @return
	 */
	bool IsValid()
	{
		///Binary code'size less 64k
		if((Rom.size() > 64*1024) || (Rom.size() <= 0))
			return false;
		if(Rom[0] != 0x02)
			return false;

		//!<指定版本的SDK以上，才去校验 账户平衡开关的取值
		if(memcmp(&Rom[0x0004],"\x00\x02\x02",3) >= 0){
           if(!((Rom[0x0014] == 0x00) || (Rom[0x0014] == 0x01))){
        	   return false;
           }
		}
		return true;
	}

	bool IsCheckAccount(void){
		//!<指定版本的SDK以上，才去读取 账户平衡开关的取值
		if(memcmp(&Rom[0x0004],"\x00\x02\x02",3) >= 0)
		{
	        if(Rom[0x0014] == 0x01){
	        	return true;
	        }
		}
        return false;
	}
	CVmScript();

	 IMPLEMENT_SERIALIZE
	(
		READWRITE(Rom);
		READWRITE(ScriptExplain);
	)
	virtual ~CVmScript();
};

#endif /* VMSCRIPT_H_ */
