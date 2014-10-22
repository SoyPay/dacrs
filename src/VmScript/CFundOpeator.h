/*
 * CFundOpeator.h
 *
 *  Created on: Sep 3, 2014
 *      Author: ranger.shi
 */

#ifndef CFUNDOPEATOR_H_
#define CFUNDOPEATOR_H_
#include "serialize.h"
#include "tx.h"
class CFundOpeator {
	unsigned char Opeater;
	uint64_t values;
	int height;
	uint256 uTxHash;
public:
	CFundOpeator(OperType type,const uint256& hash,int height,uint64_t values);
	virtual ~CFundOpeator();
};

#endif /* CFUNDOPEATOR_H_ */
