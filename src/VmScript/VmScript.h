/*
 * VmScript.h
 *
 *  Created on: Sep 2, 2014
 *      Author: ranger.shi
 */

#ifndef VMSCRIPT_H_
#define VMSCRIPT_H_

#include "serialize.h"
#include "CVmRule.h"
using namespace std;
class CVmScript {

public:
	CVmRule rule;
	vector<unsigned char> Rom;
	vector<unsigned char> ScriptExplain;

public:
	bool IsValid()
	{
		if(Rom.size() > 64*1024 || Rom.size()<=0)
			return false;
		if(Rom[0] != 0x02)
			return false;
		return rule.IsValid();
	}

	CVmScript();

	 IMPLEMENT_SERIALIZE
	(
		READWRITE(rule);
		READWRITE(Rom);
		READWRITE(ScriptExplain);
	)
	virtual ~CVmScript();
};

#endif /* VMSCRIPT_H_ */
