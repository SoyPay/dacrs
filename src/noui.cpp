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
#include "main.h"
#include "rpc/rpctx.h"
#include "wallet/wallet.h"
#include "init.h"
#include "util.h"
#include "miner.h"
using namespace json_spirit;
#include "cuiserver.h"
#include "net.h"

CCriticalSection g_cs_Sendbuffer;
deque<string> g_dSendBuffer;

void ThreadSendMessageToUI() {
	RenameThread("send-message-to-ui");
	while (true) {
		{
			LOCK(g_cs_Sendbuffer);
			if (!g_dSendBuffer.empty()) {
				if (CUIServer::HasConnection()) {
					string message = g_dSendBuffer.front();
					g_dSendBuffer.pop_front();
					CUIServer::Send(message);
				}
//				else {
//					if(GetTime() - nCurTime > 120) {   //2 minutes no connection exit ThreadSendMessageToUI thread
//						g_dSendBuffer.clear();
//						break;
//					}
//				}
			}
		}
		MilliSleep(10);
	}
}

void AddMessageToDeque(const std::string &strMessage) {
	if (SysCfg().GetBoolArg("-ui", false)) {
		LOCK(g_cs_Sendbuffer);
		g_dSendBuffer.push_back(strMessage);
		LogPrint("msgdeque", "AddMessageToDeque %s\n", strMessage.c_str());
	} else {
		LogPrint("msgdeque", "AddMessageToDeque %s\n", strMessage.c_str());
	}
}

static bool noui_ThreadSafeMessageBox(const std::string& message, const std::string& caption, unsigned int style) {
	Object obj;
	obj.push_back(Pair("type", "MessageBox"));
	obj.push_back(Pair("BoxType", message));

	std::string strCaption;
	// Check for usage of predefined caption
	switch (style) {
	case CClientUIInterface::MSG_ERROR: {
		obj.push_back(Pair("BoxType", "Error"));
		break;
	}
	case CClientUIInterface::MSG_WARNING: {
		obj.push_back(Pair("BoxType", "Warning"));
		break;
	}
	case CClientUIInterface::MSG_INFORMATION: {
		obj.push_back(Pair("BoxType", "Information"));
		break;
	}
	default: {
		obj.push_back(Pair("BoxType", "unKown"));
	}
	}

	AddMessageToDeque(write_string(Value(std::move(obj)), true));

	fprintf(stderr, "%s: %s\n", strCaption.c_str(), message.c_str());
	return false;
}

static bool noui_SyncTx() {
	Array arrayObj;
	int nTipHeight = g_cChainActive.Tip()->m_nHeight;
	int nSyncTxDeep = SysCfg().GetArg("-synctxdeep", 100);
	if (nSyncTxDeep >= nTipHeight) {
		nSyncTxDeep = nTipHeight;
	}
	CBlockIndex *pStartBlockIndex = g_cChainActive[nTipHeight - nSyncTxDeep];

	Object objStartHeight;
	objStartHeight.push_back(Pair("syncheight", pStartBlockIndex->m_nHeight));
	Object objMsg;
	objMsg.push_back(Pair("type", "SyncTxHight"));
	objMsg.push_back(Pair("msg", objStartHeight));
	AddMessageToDeque(write_string(Value(std::move(objMsg)), true));

	while (pStartBlockIndex != NULL) {
		if ((g_pwalletMain->m_mapInBlockTx).count(pStartBlockIndex->GetBlockHash()) > 0) {
			Object objTx;
			CAccountTx cAcctTx = g_pwalletMain->m_mapInBlockTx[pStartBlockIndex->GetBlockHash()];
			map<uint256, std::shared_ptr<CBaseTransaction> >::iterator iterTx = cAcctTx.m_mapAccountTx.begin();
			for (; iterTx != cAcctTx.m_mapAccountTx.end(); ++iterTx) {
				objTx = iterTx->second->ToJSON(*g_pAccountViewTip);
				objTx.push_back(Pair("blockhash", iterTx->first.GetHex()));
				objTx.push_back(Pair("confirmHeight", pStartBlockIndex->m_nHeight));
				objTx.push_back(Pair("confirmedtime", (int) pStartBlockIndex->m_unTime));
				Object obj;
				obj.push_back(Pair("type", "SyncTx"));
				obj.push_back(Pair("msg", objTx));
				AddMessageToDeque(write_string(Value(std::move(obj)), true));
			}
		}
		pStartBlockIndex = g_cChainActive.Next(pStartBlockIndex);
	}
	/*
	 map<uint256, CAccountTx>::iterator iterAccountTx = pwalletMain->mapInBlockTx.begin();
	 for(; iterAccountTx != pwalletMain->mapInBlockTx.end(); ++iterAccountTx)
	 {
	 Object objTx;
	 map<uint256, std::shared_ptr<CBaseTransaction> >::iterator iterTx = iterAccountTx->second.mapAccountTx.begin();
	 for(;iterTx != iterAccountTx->second.mapAccountTx.end(); ++iterTx) {
	 objTx = iterTx->second.get()->ToJSON(*pAccountViewTip);
	 objTx.push_back(Pair("blockhash", iterAccountTx->first.GetHex()));
	 if(mapBlockIndex.count(iterAccountTx->first) && chainActive.Contains(mapBlockIndex[iterAccountTx->first])) {
	 objTx.push_back(Pair("confirmHeight", mapBlockIndex[iterAccountTx->first]->nHeight));
	 objTx.push_back(Pair("confirmedtime", (int)mapBlockIndex[iterAccountTx->first]->nTime));
	 }
	 else {
	 LogPrint("NOUI", "block hash=%s in wallet map invalid\n", iterAccountTx->first.GetHex());
	 continue;
	 }
	 Object obj;
	 obj.push_back(Pair("type",     "SyncTx"));
	 obj.push_back(Pair("msg",  objTx));// write_string(Value(arrayObj),true)));
	 AddMessageToDeque(write_string(Value(std::move(obj)),true));
	 }
	 }
	 */
	map<uint256, std::shared_ptr<CBaseTransaction> >::iterator iterTx = g_pwalletMain->m_mapUnConfirmTx.begin();
	for (; iterTx != g_pwalletMain->m_mapUnConfirmTx.end(); ++iterTx) {
		Object objTx = iterTx->second.get()->ToJSON(*g_pAccountViewTip);
		arrayObj.push_back(objTx);

		Object obj;
		obj.push_back(Pair("type", "SyncTx"));
		obj.push_back(Pair("msg", objTx));
		AddMessageToDeque(write_string(Value(std::move(obj)), true));
	}
	return true;
}

static void noui_InitMessage(const std::string &message) {
	if (message == "initialize end") {
		CUIServer::m_bIsInitalEnd = true;
	}
	if ("Sync Tx" == message) {
		noui_SyncTx();
		return;
	}
	Object obj;
	obj.push_back(Pair("type", "init"));
	obj.push_back(Pair("msg", message));
	AddMessageToDeque(write_string(Value(std::move(obj)), true));
}

static void noui_BlockChanged(int64_t time, int64_t high, const uint256 &hash) {
	Object obj;
	obj.push_back(Pair("type", "blockchanged"));
	obj.push_back(Pair("tips", g_nSyncTipHeight));
	obj.push_back(Pair("high", (int) high));
	obj.push_back(Pair("time", (int) time));
	obj.push_back(Pair("hash", hash.ToString()));
	obj.push_back(Pair("connections", (int) g_vNodes.size()));
	obj.push_back(Pair("fuelrate", GetElementForBurn(g_cChainActive.Tip())));
	AddMessageToDeque(write_string(Value(std::move(obj)), true));
}

extern Object GetTxDetailJSON(const uint256& cTxHash);

static bool noui_RevTransaction(const uint256 &cHash) {
	Object obj;
	obj.push_back(Pair("type",     "revtransaction"));
	obj.push_back(Pair("transation",     GetTxDetailJSON(cHash)));
	AddMessageToDeque(write_string(Value(std::move(obj)),true));
	return true;
}

static bool noui_RevAppTransaction(const CBlock *pBlock ,int nIndex) {
	Object obj;
	obj.push_back(Pair("type",     "rev_app_transaction"));
	Object objTx = pBlock->vptx[nIndex].get()->ToJSON(*g_pAccountViewTip);
	objTx.push_back(Pair("blockhash", pBlock->GetHash().GetHex()));
	objTx.push_back(Pair("confirmHeight", (int) pBlock->GetHeight()));
	objTx.push_back(Pair("confirmedtime", (int) pBlock->GetTime()));
	obj.push_back(Pair("transation",     objTx));
	AddMessageToDeque(write_string(Value(std::move(obj)),true));
	return true;
}

static void noui_NotifyMessage(const std::string &message) {
	Object obj;
	obj.push_back(Pair("type", "notify"));
	obj.push_back(Pair("msg", message));
	AddMessageToDeque(write_string(Value(std::move(obj)), true));
}

static bool noui_ReleaseTransaction(const uint256 &cHash) {
	Object obj;
	obj.push_back(Pair("type",     "releasetx"));
	obj.push_back(Pair("hash",   cHash.ToString()));
	AddMessageToDeque(write_string(Value(std::move(obj)),true));
	return true;
}

static bool noui_RemoveTransaction(const uint256 &hash) {
	Object obj;
	obj.push_back(Pair("type",     "rmtx"));
	obj.push_back(Pair("hash",   hash.ToString()));
	AddMessageToDeque(write_string(Value(std::move(obj)),true));
	return true;
}

void noui_connect() {
	// Connect Dacrsd signal handlers
	g_cUIInterface.RevTransaction.connect(noui_RevTransaction);
	g_cUIInterface.RevAppTransaction.connect(noui_RevAppTransaction);
	g_cUIInterface.ThreadSafeMessageBox.connect(noui_ThreadSafeMessageBox);
	g_cUIInterface.InitMessage.connect(noui_InitMessage);
	g_cUIInterface.NotifyBlocksChanged.connect(noui_BlockChanged);
	g_cUIInterface.NotifyMessage.connect(noui_NotifyMessage);
	g_cUIInterface.ReleaseTransaction.connect(noui_ReleaseTransaction);
	g_cUIInterface.RemoveTransaction.connect(noui_RemoveTransaction);
}
