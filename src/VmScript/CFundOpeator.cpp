/*
 * CFundOpeator.cpp
 *
 *  Created on: Sep 3, 2014
 *      Author: ranger.shi
 */

#include "CFundOpeator.h"

CFundOpeator::CFundOpeator(OperType type,const uint256& hash,int height,uint64_t values) {
    this->Opeater = (unsigned char)type;
	this->values = values;
	this->uTxHash = hash ;
	this->height = height;
}

CFundOpeator::~CFundOpeator() {

}

