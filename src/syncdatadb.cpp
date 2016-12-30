/*
 * CSyncDataDb.cpp
 *
 *  Created on: Jun 14, 2014
 *      Author: ranger.shi
 */

#include "syncdatadb.h"
#include "serialize.h"
#include <map>

namespace SyncData {

boost::shared_ptr<CLevelDBWrapper> CSyncDataDb::m_dbPoint;

bool CSyncDataDb::InitializeSyncDataDb(const boost::filesystem::path &path) {
	if (!m_dbPoint) {
		m_dbPoint.reset(new CLevelDBWrapper(path, 0));
	}
	return (NULL != m_dbPoint.get());
}

bool CSyncDataDb::WriteCheckpoint(int height, const CSyncData& data) {
	bool bRet = false;
	if (m_dbPoint) {
		bRet = m_dbPoint->Write(std::make_pair('c', height), data);
	}
	return bRet;
}

bool CSyncDataDb::ReadCheckpoint(int height, CSyncData& data) {
	bool bRet = false;
	if (m_dbPoint) {
		bRet = m_dbPoint->Read(std::make_pair('c', height), data);
	}
	return bRet;
}

bool CSyncDataDb::ExistCheckpoint(int height) {
	bool bRet = false;
	if (m_dbPoint) {
		bRet = m_dbPoint->Exists(height);
	}
	return bRet;
}

bool CSyncDataDb::LoadCheckPoint(std::map<int, uint256>& values) {
	bool bRet = false;
	if (m_dbPoint) {
		leveldb::Iterator *pCursor = m_dbPoint->NewIterator();
		CDataStream cDSKeySet(SER_DISK, g_sClientVersion);
		cDSKeySet << std::make_pair('c', 0);
		pCursor->Seek(cDSKeySet.str());

		while (pCursor->Valid()) {
			try {
				leveldb::Slice slKey = pCursor->key();
				CDataStream ssKey(slKey.data(), slKey.data() + slKey.size(), SER_DISK, g_sClientVersion);
				char chType = 0;
				int nHeight = 0;
				ssKey >> chType;
				if (chType == 'c') {
					ssKey >> nHeight;
					leveldb::Slice slValue = pCursor->value();
					CDataStream ssValue(slValue.data(), slValue.data() + slValue.size(), SER_DISK, g_sClientVersion);
					CSyncData cSyncData;
					ssValue >> cSyncData;
					CSyncCheckPoint cSyncCheckPoint;
					cSyncCheckPoint.SetData(cSyncData);
					values.insert(std::make_pair(nHeight, cSyncCheckPoint.m_hashCheckpoint));
					pCursor->Next();
				} else {
					break;
				}
			} catch (std::exception &e) {
				return ERRORMSG("%s : Deserialize or I/O error - %s", __PRETTY_FUNCTION__, e.what());
			}
		}
		delete pCursor;
		bRet = true;
	}
	return bRet;
}
} /* namespace SyncData */
