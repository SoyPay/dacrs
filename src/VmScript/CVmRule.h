/*
 * CVmRule.h
 *
 *  Created on: Sep 2, 2014
 *      Author: ranger.shi
 */

#ifndef CVMRULE_H_
#define CVMRULE_H_
#include "serialize.h"
class CVmRule {
public:
	uint64_t maxReSv;
	uint64_t maxPay;
	unsigned int vpreOutHeihgt;
	unsigned int vNextOutHeight;
public:
	bool IsValid()
	{
		return true;
	}
	CVmRule();
	 IMPLEMENT_SERIALIZE
	(
		READWRITE(maxReSv);
		READWRITE(maxPay);
		READWRITE(vpreOutHeihgt);
		READWRITE(vNextOutHeight);
	)
	virtual ~CVmRule();
};

#endif /* CVMRULE_H_ */
