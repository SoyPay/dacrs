/*
 * rpctx.h
 *
 *  Created on: Sep 3, 2014
 *      Author: leo
 */

#ifndef RPCTX_H_
#define RPCTX_H_

#include <boost/assign/list_of.hpp>
#include "json/json_spirit_utils.h"
#include "json/json_spirit_value.h"

using namespace std;
using namespace boost;
using namespace boost::assign;
using namespace json_spirit;



extern Value registersecuretx(const Array& params, bool fHelp);
extern Value createnormaltx(const Array& params, bool fHelp);
extern Value createappealtx(const Array& params, bool fHelp);
extern Value signappealtx(const Array& params, bool fHelp);
extern Value createfreezetx(const Array& params, bool fHelp);
extern Value registerscripttx(const Array& params, bool fHelp);
extern Value createsecuretx(const Array& params, bool fHelp);
extern Value signsecuretx(const Array& params, bool fHelp);

extern Value listaddr(const Array& params, bool fHelp);
extern Value listaddrtx(const Array& params, bool fHelp);

extern Value getaddramount(const Array& params, bool fHelp);
extern Value listunconfirmedtx(const Array& params, bool fHelp);
extern Value gettxdetail(const Array& params, bool fHelp);
extern Value listscriptregid(const Array& params, bool fHelp);
extern Value listregid(const Array& params, bool fHelp);
extern Value sign(const Array& params, bool fHelp);
extern Value getaccountinfo(const Array& params, bool fHelp);


extern Value testnormaltx(const Array& params, bool fHelp);
extern Value testminer(const Array& params, bool fHelp);
extern Value disconnectblock(const Array& params, bool fHelp);
extern Value listregscript(const Array& params, bool fHelp);

extern Value getoneaddr(const Array& params, bool fHelp);
extern Value getaddrbalance(const Array& params, bool fHelp);
extern Value generateblock(const Array& params, bool fHelp);
extern Value getpublickey(const Array& params, bool fHelp);
extern Value listtxcache(const Array& params, bool fHelp);
extern Value reloadtxcache(const Array& params, bool fHelp);
#endif /* RPCTX_H_ */
