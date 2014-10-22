// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "noui.h"

#include "ui_interface.h"
#include "util.h"
#include <stdint.h>
#include <string>
#include "cuiserve.h"
static bool noui_ThreadSafeMessageBox(const std::string& message, const std::string& caption, unsigned int style)
{
    std::string strCaption;
    // Check for usage of predefined caption
    switch (style) {
    case CClientUIInterface::MSG_ERROR:
        strCaption += _("Error");
        break;
    case CClientUIInterface::MSG_WARNING:
        strCaption += _("Warning");
        break;
    case CClientUIInterface::MSG_INFORMATION:
        strCaption += _("Information");
        break;
    default:
        strCaption += caption; // Use supplied caption (can be empty)
    }

	if (CUIServer::HasConnection()) {
		std::string strMessage = "INFO ";
		strMessage += strprintf("%s: %s\n",strCaption, message);
		CUIServer::Send(strMessage);
	} else {
		LogPrint("INFO", "%s: %s\n", strCaption, message);
	}

    fprintf(stderr, "%s: %s\n", strCaption.c_str(), message.c_str());
    return false;
}

static void noui_InitMessage(const std::string &message)
{
	if(CUIServer::HasConnection()){
		CUIServer::Send(message);
	}else{
		LogPrint("INFO","init message: %s\n", message);
	}
}

void noui_connect()
{
    // Connect bitcoind signal handlers
    uiInterface.ThreadSafeMessageBox.connect(noui_ThreadSafeMessageBox);
    uiInterface.InitMessage.connect(noui_InitMessage);
}
