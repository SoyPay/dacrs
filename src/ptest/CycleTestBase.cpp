/*
 * CycleTestBase.cpp
 *
 *  Created on: 2014Äê12ÔÂ30ÈÕ
 *      Author: ranger.shi
 */

#include "CycleTestBase.h"
CycleTestBase::CycleTestBase() {

}

TEST_STATE CycleTestBase::Run() {
	return end_state;
}
int CycleTestBase::totalsend = 0;

CycleTestBase::~CycleTestBase() {

}
