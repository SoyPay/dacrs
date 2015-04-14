/*
 * CycleTestBase.h
 *
 *  Created on: 2014Äê12ÔÂ30ÈÕ
 *      Author: ranger.shi
 */

#ifndef CYCLETESTBASE_H_
#define CYCLETESTBASE_H_
#include "../test/systestbase.h"
enum TEST_STATE{
	this_state,
	next_state,
	end_state,
};

class CycleTestBase {
protected:
	SysTestBase basetest;
	static int totalsend;
public:
	CycleTestBase();
	bool IncSentTotal(){
		   cout<< "send total:" << ++totalsend<<endl;
		   return true;
	}
	virtual TEST_STATE run();
	virtual ~CycleTestBase();
};



#endif /* CYCLETESTBASE_H_ */
