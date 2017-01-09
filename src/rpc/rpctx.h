/*
 * rpctx.h
 *
 *  Created on: Sep 3, 2014
 *      Author: leo
 */

#ifndef DACRS_RPC_RPCTX_H_
#define DACRS_RPC_RPCTX_H_

#include <boost/assign/list_of.hpp>
#include "json/json_spirit_utils.h"
#include "json/json_spirit_value.h"

using namespace std;
using namespace boost;
using namespace boost::assign;
using namespace json_spirit;

class CBaseTransaction;

extern Value registaccounttx(const Array& params, bool bHelp);
extern Value sendtoaddressraw(const Array& params, bool bHelp);
extern Value createcontracttx(const Array& params, bool bHelp);
extern Value signcontracttx(const Array& params, bool bHelp);
extern Value createfreezetx(const Array& params, bool bHelp);
extern Value registerapptx(const Array& params, bool bHelp);

extern Value listaddr(const Array& params, bool bHelp);
extern Value listtx(const Array& params, bool bHelp);

extern Value getaddrinfo(const Array& params, bool bHelp);
extern Value listunconfirmedtx(const Array& params, bool bHelp);
extern Value gettxdetail(const Array& params, bool bHelp);
extern Value listregid(const Array& params, bool bHelp);
extern Value sign(const Array& params, bool bHelp);
extern Value getaccountinfo(const Array& params, bool bHelp);


extern Value disconnectblock(const Array& params, bool bHelp);
extern Value listapp(const Array& params, bool bHelp);
extern Value getappinfo(const Array& params, bool bHelp);

extern Value getaddrbalance(const Array& params, bool bHelp);
extern Value generateblock(const Array& params, bool bHelp);
//extern Value getpublickey(const Array& params, bool bHelp);
extern Value listtxcache(const Array& params, bool bHelp);
extern Value reloadtxcache(const Array& params, bool bHelp);
extern Value getscriptdata(const Array& params, bool bHelp);
extern Value getscriptvalidedata(const Array& params, bool bHelp);
extern Value saveblocktofile(const Array& params, bool bHelp);
extern Value getscriptdbsize(const Array& params, bool bHelp);
extern Value getalltxinfo(const Array& params, bool bHelp);
extern Value listauthor(const Array& params, bool bHelp);
extern Value  getappaccinfo(const Array& params, bool bHelp);
extern Value  gethash(const Array& params, bool bHelp);
extern Value  getappkeyvalue(const Array& params, bool bHelp);
extern Value  gencheckpoint(const Array& params, bool bHelp);
extern Value  setcheckpoint(const Array& params, bool bHelp);
extern Value validateaddress(const Array& params, bool bHelp);
extern Object TxToJSON(CBaseTransaction *pTx);
extern Value gettotalcoin(const Array& params, bool bHelp);
extern Value gettotalassets(const Array& params, bool bHelp);
extern Value gettxhashbyaddress(const Array& params, bool bHelp);
extern Value getrawtx(const Array& params, bool bHelp);
extern Value gettransaction(const Array& params, bool bHelp);
extern Value contractreckon(const Array& params, bool bHelp);//π¿À„∫œ‘º
#endif /* DACRS_RPC_RPCTX_H_ */
