// Copyright (c) 2012 Pieter Wuille
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "addrman.h"

#include "hash.h"
#include "serialize.h"

using namespace std;

int CAddrInfo::GetTriedBucket(const vector<unsigned char> &nKey) const {
	CDataStream cDataStream1(SER_GETHASH, 0);
	vector<unsigned char> vchKey = GetKey();
	cDataStream1 << nKey << vchKey;
	uint64_t ullHash1 = Hash(cDataStream1.begin(), cDataStream1.end()).GetCheapHash();

	CDataStream cDataStream2(SER_GETHASH, 0);
	vector<unsigned char> vchGroupKey = GetGroup();
	cDataStream2 << nKey << vchGroupKey << (ullHash1 % ADDRMAN_TRIED_BUCKETS_PER_GROUP);
	uint64_t ullHash2 = Hash(cDataStream2.begin(), cDataStream2.end()).GetCheapHash();

	return ullHash2 % ADDRMAN_TRIED_BUCKET_COUNT;
}

int CAddrInfo::GetNewBucket(const vector<unsigned char> &nKey, const CNetAddr& src) const {
	CDataStream cDataStream1(SER_GETHASH, 0);
	vector<unsigned char> vchGroupKey = GetGroup();
	vector<unsigned char> vchSourceGroupKey = src.GetGroup();
	cDataStream1 << nKey << vchGroupKey << vchSourceGroupKey;
	uint64_t ullHash1 = Hash(cDataStream1.begin(), cDataStream1.end()).GetCheapHash();

	CDataStream cDataStream2(SER_GETHASH, 0);
	cDataStream2 << nKey << vchSourceGroupKey << (ullHash1 % ADDRMAN_NEW_BUCKETS_PER_SOURCE_GROUP);
	uint64_t ullHash2 = Hash(cDataStream2.begin(), cDataStream2.end()).GetCheapHash();

	return ullHash2 % ADDRMAN_NEW_BUCKET_COUNT;
}

bool CAddrInfo::IsTerrible(int64_t nNow) const {
	// never remove things tried the last minute
	if (nLastTry && nLastTry >= nNow - 60) {
		return false;
	}

	// came in a flying DeLorean
	if (nTime > nNow + 10 * 60) {
		return true;
	}

	// not seen in over a month
	if (nTime == 0 || nNow - nTime > ADDRMAN_HORIZON_DAYS * 86400) {
		return true;
	}

	// tried three times and never a success
	if (m_llLastSuccess == 0 && m_nAttempts >= ADDRMAN_RETRIES) {
		return true;
	}

	// 10 successive failures in the last week
	if (nNow - m_llLastSuccess > ADDRMAN_MIN_FAIL_DAYS * 86400 && m_nAttempts >= ADDRMAN_MAX_FAILURES) {
		return true;
	}

	return false;
}

double CAddrInfo::GetChance(int64_t nNow) const {
	double dChance = 1.0;

	int64_t llSinceLastSeen = nNow - nTime;
	int64_t llSinceLastTry = nNow - nLastTry;

	if (llSinceLastSeen < 0) {
		llSinceLastSeen = 0;
	}
	if (llSinceLastTry < 0) {
		llSinceLastTry = 0;
	}

	dChance *= 600.0 / (600.0 + llSinceLastSeen);

	// deprioritize very recent attempts away
	if (llSinceLastTry < 60 * 10) {
		dChance *= 0.01;
	}

	// deprioritize 50% after each failed attempt
	for (int n = 0; n < m_nAttempts; n++) {
		dChance /= 1.5;
	}

	return dChance;
}

CAddrInfo* CAddrMan::Find(const CNetAddr& addr, int *pnId) {
	map<CNetAddr, int>::iterator iter1 = m_mapAddr.find(addr);
	if (iter1 == m_mapAddr.end()) {
		return NULL;
	}
	if (pnId) {
		*pnId = (*iter1).second;
	}

	map<int, CAddrInfo>::iterator iter2 = m_mapInfo.find((*iter1).second);
	if (iter2 != m_mapInfo.end()) {
		return &(*iter2).second;
	}

	return NULL;
}

CAddrInfo* CAddrMan::Create(const CAddress &addr, const CNetAddr &addrSource, int *pnId) {
	int nId = m_nIdCount++;
	m_mapInfo[nId] = CAddrInfo(addr, addrSource);
	m_mapAddr[addr] = nId;
	m_mapInfo[nId].m_nRandomPos = m_vnRandom.size();
	m_vnRandom.push_back(nId);
	if (pnId) {
		*pnId = nId;
	}

	return &m_mapInfo[nId];
}

void CAddrMan::SwapRandom(unsigned int nRndPos1, unsigned int nRndPos2) {
	if (nRndPos1 == nRndPos2) {
		return;
	}

	assert(nRndPos1 < m_vnRandom.size() && nRndPos2 < m_vnRandom.size());

	int nId1 = m_vnRandom[nRndPos1];
	int nId2 = m_vnRandom[nRndPos2];

	assert(m_mapInfo.count(nId1) == 1);
	assert(m_mapInfo.count(nId2) == 1);

	m_mapInfo[nId1].m_nRandomPos = nRndPos2;
	m_mapInfo[nId2].m_nRandomPos = nRndPos1;

	m_vnRandom[nRndPos1] = nId2;
	m_vnRandom[nRndPos2] = nId1;
}

int CAddrMan::SelectTried(int nKBucket) {
	vector<int> &vTried = m_vvnTried[nKBucket];

	// random shuffle the first few elements (using the entire list)
	// find the least recently tried among them
	int64_t llOldest = -1;
	int nOldestPos = -1;
	for (unsigned int i = 0; i < ADDRMAN_TRIED_ENTRIES_INSPECT_ON_EVICT && i < vTried.size(); i++) {
		int nPos = GetRandInt(vTried.size() - i) + i;
		int nTemp = vTried[nPos];
		vTried[nPos] = vTried[i];
		vTried[i] = nTemp;
		assert(llOldest == -1 || m_mapInfo.count(nTemp) == 1);
		if (llOldest == -1 || m_mapInfo[nTemp].m_llLastSuccess < m_mapInfo[llOldest].m_llLastSuccess) {
			llOldest = nTemp;
			nOldestPos = nPos;
		}
	}

	return nOldestPos;
}

int CAddrMan::ShrinkNew(int nUBucket) {
	assert(nUBucket >= 0 && (unsigned int )nUBucket < m_vvnNew.size());
	set<int> &vNew = m_vvnNew[nUBucket];

	// first look for deletable items
	for (set<int>::iterator it = vNew.begin(); it != vNew.end(); it++) {
		assert(m_mapInfo.count(*it));
		CAddrInfo &info = m_mapInfo[*it];
		if (info.IsTerrible()) {
			if (--info.m_nRefCount == 0) {
				SwapRandom(info.m_nRandomPos, m_vnRandom.size() - 1);
				m_vnRandom.pop_back();
				m_mapAddr.erase(info);
				m_mapInfo.erase(*it);
				m_nNew--;
			}
			vNew.erase(it);
			return 0;
		}
	}

	// otherwise, select four randomly, and pick the oldest of those to replace
	int nRandom[4] = { GetRandInt(vNew.size()), GetRandInt(vNew.size()), GetRandInt(vNew.size()), GetRandInt(vNew.size()) };
	int nValue = 0;
	int nOldest = -1;
	for (set<int>::iterator it = vNew.begin(); it != vNew.end(); it++) {
		if (nValue == nRandom[0] || nValue == nRandom[1] || nValue == nRandom[2] || nValue == nRandom[3]) {
			assert(nOldest == -1 || m_mapInfo.count(*it) == 1);
			if (nOldest == -1 || m_mapInfo[*it].nTime < m_mapInfo[nOldest].nTime)
				nOldest = *it;
		}
		nValue++;
	}
	assert(m_mapInfo.count(nOldest) == 1);
	CAddrInfo &info = m_mapInfo[nOldest];
	if (--info.m_nRefCount == 0) {
		SwapRandom(info.m_nRandomPos, m_vnRandom.size() - 1);
		m_vnRandom.pop_back();
		m_mapAddr.erase(info);
		m_mapInfo.erase(nOldest);
		m_nNew--;
	}
	vNew.erase(nOldest);

	return 1;
}

void CAddrMan::MakeTried(CAddrInfo& info, int nId, int nOrigin) {
	assert(m_vvnNew[nOrigin].count(nId) == 1);

	// remove the entry from all new buckets
	for (vector<set<int> >::iterator it = m_vvnNew.begin(); it != m_vvnNew.end(); it++) {
		if ((*it).erase(nId))
			info.m_nRefCount--;
	}
	m_nNew--;

	assert(info.m_nRefCount == 0);

	// what tried bucket to move the entry to
	int nKBucket = info.GetTriedBucket(m_vchKey);
	vector<int> &vTried = m_vvnTried[nKBucket];

	// first check whether there is place to just add it
	if (vTried.size() < ADDRMAN_TRIED_BUCKET_SIZE) {
		vTried.push_back(nId);
		m_nTried++;
		info.m_bInTried = true;
		return;
	}

	// otherwise, find an item to evict
	int nPos = SelectTried(nKBucket);

	// find which new bucket it belongs to
	assert(m_mapInfo.count(vTried[nPos]) == 1);
	int nUBucket = m_mapInfo[vTried[nPos]].GetNewBucket(m_vchKey);
	set<int> &vNew = m_vvnNew[nUBucket];

	// remove the to-be-replaced tried entry from the tried set
	CAddrInfo& infoOld = m_mapInfo[vTried[nPos]];
	infoOld.m_bInTried = false;
	infoOld.m_nRefCount = 1;
	// do not update nTried, as we are going to move something else there immediately

	// check whether there is place in that one,
	if (vNew.size() < ADDRMAN_NEW_BUCKET_SIZE) {
		// if so, move it back there
		vNew.insert(vTried[nPos]);
	} else {
		// otherwise, move it to the new bucket nId came from (there is certainly place there)
		m_vvnNew[nOrigin].insert(vTried[nPos]);
	}
	m_nNew++;

	vTried[nPos] = nId;
	// we just overwrote an entry in vTried; no need to update nTried
	info.m_bInTried = true;
	return;
}

void CAddrMan::Good_(const CService &addr, int64_t nTime)
{
    int nId;
    CAddrInfo *pAddrInfo = Find(addr, &nId);

    // if not found, bail out
    if (!pAddrInfo) {
    	return;
    }

    CAddrInfo &cAddrInfo = *pAddrInfo;

    // check whether we are talking about the exact same CService (including same port)
    if (cAddrInfo != addr) {
    	return;
    }

    // update info
    cAddrInfo.m_llLastSuccess = nTime;
    cAddrInfo.nLastTry = nTime;
    cAddrInfo.nTime = nTime;
    cAddrInfo.m_nAttempts = 0;

    // if it is already in the tried set, don't do anything else
    if (cAddrInfo.m_bInTried) {
    	return;
    }

    // find a bucket it is in now
    int nRnd = GetRandInt(m_vvnNew.size());
    int nUBucket = -1;
	for (unsigned int n = 0; n < m_vvnNew.size(); n++) {
		int nB = (n + nRnd) % m_vvnNew.size();
		set<int> &vNew = m_vvnNew[nB];
		if (vNew.count(nId)) {
			nUBucket = nB;
			break;
		}
	}

    // if no bucket is found, something bad happened;
    // TODO: maybe re-add the node, but for now, just bail out
    if (nUBucket == -1) {
    	return;
    }

    LogPrint("addrman", "Moving %s to tried\n", addr.ToString());

    // move nId to the tried tables
    MakeTried(cAddrInfo, nId, nUBucket);
}

bool CAddrMan::Add_(const CAddress &addr, const CNetAddr& source, int64_t nTimePenalty) {
	if (!addr.IsRoutable()) {
		return false;
	}

	bool bNew = false;
	int nId;
	CAddrInfo *pAddrInfo = Find(addr, &nId);

	if (pAddrInfo) {
		// periodically update nTime
		bool bCurrentlyOnline = (GetAdjustedTime() - addr.nTime < 24 * 60 * 60);
		int64_t llUpdateInterval = (bCurrentlyOnline ? 60 * 60 : 24 * 60 * 60);
		if (addr.nTime && (!pAddrInfo->nTime || pAddrInfo->nTime < addr.nTime - llUpdateInterval - nTimePenalty)) {
			pAddrInfo->nTime = max((int64_t) 0, addr.nTime - nTimePenalty);
		}

		// add services
		pAddrInfo->nServices |= addr.nServices;

		// do not update if no new information is present
		if (!addr.nTime || (pAddrInfo->nTime && addr.nTime <= pAddrInfo->nTime)) {
			return false;
		}

		// do not update if the entry was already in the "tried" table
		if (pAddrInfo->m_bInTried) {
			return false;
		}

		// do not update if the max reference count is reached
		if (pAddrInfo->m_nRefCount == ADDRMAN_NEW_BUCKETS_PER_ADDRESS) {
			return false;
		}

		// stochastic test: previous nRefCount == N: 2^N times harder to increase it
		int nFactor = 1;
		for (int n = 0; n < pAddrInfo->m_nRefCount; n++) {
			nFactor *= 2;
		}

		if (nFactor > 1 && (GetRandInt(nFactor) != 0)) {
			return false;
		}
	} else {
		pAddrInfo = Create(addr, source, &nId);
		pAddrInfo->nTime = max((int64_t) 0, (int64_t) pAddrInfo->nTime - nTimePenalty);
		m_nNew++;
		bNew = true;
	}

	int nUBucket = pAddrInfo->GetNewBucket(m_vchKey, source);
	set<int> &vNew = m_vvnNew[nUBucket];
	if (!vNew.count(nId)) {
		pAddrInfo->m_nRefCount++;
		if (vNew.size() == ADDRMAN_NEW_BUCKET_SIZE) {
			ShrinkNew(nUBucket);
		}
		m_vvnNew[nUBucket].insert(nId);
	}
	return bNew;
}

void CAddrMan::Attempt_(const CService &addr, int64_t nTime) {
	CAddrInfo *pAddrInfo = Find(addr);

	// if not found, bail out
	if (!pAddrInfo) {
		return;
	}

	CAddrInfo &cAddrInfo = *pAddrInfo;

	// check whether we are talking about the exact same CService (including same port)
	if (cAddrInfo != addr) {
		return;
	}

	// update info
	cAddrInfo.nLastTry = nTime;
	cAddrInfo.m_nAttempts++;
}

CAddress CAddrMan::Select_(int nUnkBias) {
	if (size() == 0) {
		return CAddress();
	}

	double dCorTried = sqrt(m_nTried) * (100.0 - nUnkBias);
	double dCorNew = sqrt(m_nNew) * nUnkBias;

	if ((dCorTried + dCorNew) * GetRandInt(1 << 30) / (1 << 30) < dCorTried) {
		// use a tried node
		double dChanceFactor = 1.0;
		while (1) {
			int nKBucket = GetRandInt(m_vvnTried.size());
			vector<int> &vTried = m_vvnTried[nKBucket];
			if (vTried.size() == 0) {
				continue;
			}

			int nPos = GetRandInt(vTried.size());
			assert(m_mapInfo.count(vTried[nPos]) == 1);
			CAddrInfo &cAddrInfo = m_mapInfo[vTried[nPos]];
			if (GetRandInt(1 << 30) < dChanceFactor * cAddrInfo.GetChance() * (1 << 30)) {
				return cAddrInfo;
			}
			dChanceFactor *= 1.2;
		}
	} else {
		// use a new node
		double dChanceFactor = 1.0;
		while (1) {
			int nUBucket = GetRandInt(m_vvnNew.size());
			set<int> &vNew = m_vvnNew[nUBucket];
			if (vNew.size() == 0) {
				continue;
			}
			int nPos = GetRandInt(vNew.size());
			set<int>::iterator iter = vNew.begin();
			while (nPos--) {
				iter++;
			}
			assert(m_mapInfo.count(*iter) == 1);
			CAddrInfo &info = m_mapInfo[*iter];
			if (GetRandInt(1 << 30) < dChanceFactor * info.GetChance() * (1 << 30)) {
				return info;
			}
			dChanceFactor *= 1.2;
		}
	}
}

#ifdef DEBUG_ADDRMAN
int CAddrMan::Check_()
{
    set<int> setTried;
    map<int, int> mapNew;

    if (vRandom.size() != nTried + nNew) return -7;

    for (map<int, CAddrInfo>::iterator it = mapInfo.begin(); it != mapInfo.end(); it++)
    {
        int n = (*it).first;
        CAddrInfo &info = (*it).second;
        if (info.fInTried)
        {

            if (!info.nLastSuccess) return -1;
            if (info.nRefCount) return -2;
            setTried.insert(n);
        } else {
            if (info.nRefCount < 0 || info.nRefCount > ADDRMAN_NEW_BUCKETS_PER_ADDRESS) return -3;
            if (!info.nRefCount) return -4;
            mapNew[n] = info.nRefCount;
        }
        if (mapAddr[info] != n) return -5;
        if (info.nRandomPos<0 || info.nRandomPos>=vRandom.size() || vRandom[info.nRandomPos] != n) return -14;
        if (info.nLastTry < 0) return -6;
        if (info.nLastSuccess < 0) return -8;
    }

    if (setTried.size() != nTried) return -9;
    if (mapNew.size() != nNew) return -10;

    for (int n=0; n<vvTried.size(); n++)
    {
        vector<int> &vTried = vvTried[n];
        for (vector<int>::iterator it = vTried.begin(); it != vTried.end(); it++)
        {
            if (!setTried.count(*it)) return -11;
            setTried.erase(*it);
        }
    }

    for (int n=0; n<vvNew.size(); n++)
    {
        set<int> &vNew = vvNew[n];
        for (set<int>::iterator it = vNew.begin(); it != vNew.end(); it++)
        {
            if (!mapNew.count(*it)) return -12;
            if (--mapNew[*it] == 0)
                mapNew.erase(*it);
        }
    }

    if (setTried.size()) return -13;
    if (mapNew.size()) return -15;

    return 0;
}
#endif

void CAddrMan::GetAddr_(vector<CAddress> &vAddr) {
	int nNodes = ADDRMAN_GETADDR_MAX_PCT * m_vnRandom.size() / 100;
	if (nNodes > ADDRMAN_GETADDR_MAX) {
		nNodes = ADDRMAN_GETADDR_MAX;
	}

	// perform a random shuffle over the first nNodes elements of vRandom (selecting from all)
	for (int n = 0; n < nNodes; n++) {
		int nRndPos = GetRandInt(m_vnRandom.size() - n) + n;
		SwapRandom(n, nRndPos);
		assert(m_mapInfo.count(m_vnRandom[n]) == 1);
		vAddr.push_back(m_mapInfo[m_vnRandom[n]]);
	}
}

void CAddrMan::Connected_(const CService &addr, int64_t nTime) {
	CAddrInfo *pAddrInfo = Find(addr);

	// if not found, bail out
	if (!pAddrInfo) {
		return;
	}

	CAddrInfo &cAddrInfo = *pAddrInfo;

	// check whether we are talking about the exact same CService (including same port)
	if (cAddrInfo != addr) {
		return;
	}

	// update info
	int64_t llUpdateInterval = 20 * 60;
	if (nTime - cAddrInfo.nTime > llUpdateInterval) {
		cAddrInfo.nTime = nTime;
	}
}
