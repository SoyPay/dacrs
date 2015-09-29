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
		if(Rom.size() > 64*1024 || Rom.size()<=0)
			return false;
		if(Rom[0] != 0x02)
			return false;
		//!<校验SDK版本，账户平衡开关
		if(!memcmp(&Rom[0x0004],"\x00\x02\x02",3)){
           if(!((Rom[0x0014] == 0x00) || (Rom[0x0014] == 0x01))){
        	   cout<<"IsValid ROM0004 err"<<endl;
        	   return false;
           }
           cout<<"IsValid ROM0004 ok"<<endl;
		}
		return true;
	}

	bool IsCheckAccount(void){
		if(IsValid())
		{
	        if(Rom[0x0014] == 0x01){
	        	cout<<"IsCheckAccount true"<<endl;
	        	return true;
	        }
		}
		cout<<"IsCheckAccount false"<<endl;
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
