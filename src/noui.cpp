// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Dacrs developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "noui.h"

#include "ui_interface.h"
#include "util.h"
#include <stdint.h>
#include <string>
#include "json/json_spirit_utils.h"
#include "json/json_spirit_value.h"
#include "json/json_spirit_writer_template.h"
using namespace json_spirit;
#include "cuiserve.h"
static bool noui_ThreadSafeMessageBox(const std::string& message, const std::string& caption, unsigned int style)
{

	Object obj;
	obj.push_back(Pair("type",     "MessageBox"));
	obj.push_back(Pair("BoxType",     message));

    std::string strCaption;
    // Check for usage of predefined caption
    switch (style) {
    case CClientUIInterface::MSG_ERROR:
      	obj.push_back(Pair("BoxType",     "Error"));
        break;
    case CClientUIInterface::MSG_WARNING:
       	obj.push_back(Pair("BoxType",     "Warning"));
        break;
    case CClientUIInterface::MSG_INFORMATION:
    	obj.push_back(Pair("BoxType",     "Information"));
        break;
    default:
    	obj.push_back(Pair("BoxType",     "unKown"));
//        strCaption += caption; // Use supplied caption (can be empty)
    }

	if (CUIServer::HasConnection()) {
		CUIServer::Send(write_string(Value(std::move(obj)),true));
	} else {
		LogPrint("NOUI", "%s\n", write_string(Value(std::move(obj)),true));
	}

    fprintf(stderr, "%s: %s\n", strCaption.c_str(), message.c_str());
    return false;
}

static void noui_InitMessage(const std::string &message)
{
	if(message =="initialize end")
	{
		CUIServer::IsInitalEnd = true;
	}
	if(CUIServer::HasConnection()){
		Object obj;
		obj.push_back(Pair("type",     "init"));
		obj.push_back(Pair("msg",     message));
		CUIServer::Send(write_string(Value(std::move(obj)),true));
	}else{
		LogPrint("NOUI","init message: %s\n", message);
	}
}

static void noui_BlockChanged(int64_t time,int64_t high,const uint256 &hash) {
	if (CUIServer::HasConnection()) {
		Object obj;
		obj.push_back(Pair("type",     "blockchanged"));
		obj.push_back(Pair("time",     (int)time));
		obj.push_back(Pair("high",     (int)high));
		obj.push_back(Pair("hash",     hash.ToString()));
		CUIServer::Send(write_string(Value(std::move(obj)),true));
	}
}
extern Object GetTxDetailJSON(const uint256& txhash);

static bool noui_RevTransaction(const uint256 &hash){
	Object obj;
	obj.push_back(Pair("type",     "revtransaction"));
	obj.push_back(Pair("transation",     GetTxDetailJSON(hash)));

	if (CUIServer::HasConnection()) {
		CUIServer::Send(write_string(Value(std::move(obj)),true));
	} else {
		LogPrint("NOUI", "%s\n", write_string(Value(std::move(obj)),true));
	}
	return true;
}

static bool noui_RevAppTransaction(const uint256 &hash){
	Object obj;
	obj.push_back(Pair("type",     "rev_app_transaction"));
	obj.push_back(Pair("transation",     GetTxDetailJSON(hash)));

	if (CUIServer::HasConnection()) {
		CUIServer::Send(write_string(Value(std::move(obj)),true));
	} else {
		LogPrint("NOUI", "%s\n", write_string(Value(std::move(obj)),true));
	}
	return true;
}
void noui_connect()
{
    // Connect Dacrsd signal handlers
	uiInterface.RevTransaction.connect(noui_RevTransaction);
	uiInterface.RevAppTransaction.connect(noui_RevAppTransaction);
    uiInterface.ThreadSafeMessageBox.connect(noui_ThreadSafeMessageBox);
    uiInterface.InitMessage.connect(noui_InitMessage);
    uiInterface.NotifyBlocksChanged.connect(noui_BlockChanged);
}
