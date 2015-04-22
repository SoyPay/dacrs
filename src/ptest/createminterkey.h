/*
 * createminterkey.h
 *
 *  Created on: 2015Äê4ÔÂ13ÈÕ
 *      Author: ranger.shi
 */

#ifndef CREATEMINTERKEY_H_
#define CREATEMINTERKEY_H_

class CCreateMinerkey :public SysTestBase{
public:
	void CreateAccount();
	bool SelectAccounts();
	string GetOneAccount();
//	CCreateMinerkey():argc(framework::master_test_suite().argc), argv(framework::master_test_suite().argv){};
	CCreateMinerkey(){};
	virtual ~CCreateMinerkey();
//	int argc;
//	char **argv;
private:
	vector<string> vAccount;
	map<string, uint64_t> mapSendValue;

};

#endif /* CREATEMINTERKEY_H_ */
