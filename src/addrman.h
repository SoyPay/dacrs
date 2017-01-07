// Copyright (c) 2012 Pieter Wuille
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef DACRS_ADDRMAN_
#define DACRS_ADDRMAN_

#include "netbase.h"
#include "protocol.h"
#include "sync.h"
#include "util.h"

#include <map>
#include <set>
#include <stdint.h>
#include <vector>

#include <openssl/rand.h>

/** Extended statistics about a CAddress */
class CAddrInfo : public CAddress {
 public:
    IMPLEMENT_SERIALIZE(
        CAddress* pthis = (CAddress*)(this);
        READWRITE(*pthis);
        READWRITE(m_cSourceNetAddr);
        READWRITE(m_llLastSuccess);
        READWRITE(m_nAttempts);
    )

    void Init() {
    	m_llLastSuccess = 0;
    	m_llLastTry = 0;
        m_nAttempts = 0;
        m_nRefCount = 0;
        m_bInTried = false;
        m_nRandomPos = -1;
    }

    CAddrInfo(const CAddress &addrIn, const CNetAddr &addrSource) : CAddress(addrIn), m_cSourceNetAddr(addrSource) {
        Init();
    }

    CAddrInfo() : CAddress(), m_cSourceNetAddr() {
        Init();
    }

    // Calculate in which "tried" bucket this entry belongs
    int GetTriedBucket(const std::vector<unsigned char> &nKey) const;

    // Calculate in which "new" bucket this entry belongs, given a certain source
    int GetNewBucket(const std::vector<unsigned char> &nKey, const CNetAddr& src) const;

    // Calculate in which "new" bucket this entry belongs, using its default source
    int GetNewBucket(const std::vector<unsigned char> &nKey) const {
        return GetNewBucket(nKey, m_cSourceNetAddr);
    }

    // Determine whether the statistics about this entry are bad enough so that it can just be deleted
    bool IsTerrible(int64_t nNow = GetAdjustedTime()) const;

    // Calculate the relative chance this entry should be given when selecting nodes to connect to
    double GetChance(int64_t nNow = GetAdjustedTime()) const;

 private:
    // where knowledge about this address first came from
    CNetAddr m_cSourceNetAddr;

    // last successful connection by us
    int64_t m_llLastSuccess;

    // last try whatsoever by us:
    // int64_t CAddress::nLastTry

    // connection attempts since last successful attempt
    int m_nAttempts;

    // reference count in new sets (memory only)
    int m_nRefCount;

    // in tried set? (memory only)
    bool m_bInTried;

    // position in vRandom
    int m_nRandomPos;

    friend class CAddrMan;
};

// Stochastic address manager
//
// Design goals:
//  * Only keep a limited number of addresses around, so that cAddr.dat and memory requirements do not grow without bound.
//  * Keep the address tables in-memory, and asynchronously dump the entire to able in cAddr.dat.
//  * Make sure no (localized) attacker can fill the entire table with his nodes/addresses.
//
// To that end:
//  * Addresses are organized into buckets.
//    * Address that have not yet been tried go into 256 "new" buckets.
//      * Based on the address range (/16 for IPv4) of source of the information, 32 buckets are selected at random
//      * The actual bucket is chosen from one of these, based on the range the address itself is located.
//      * One single address can occur in up to 4 different buckets, to increase selection chances for addresses that
//        are seen frequently. The chance for increasing this multiplicity decreases exponentially.
//      * When adding a new address to a full bucket, a randomly chosen entry (with a bias favoring less recently seen
//        ones) is removed from it first.
//    * Addresses of nodes that are known to be accessible go into 64 "tried" buckets.
//      * Each address range selects at random 4 of these buckets.
//      * The actual bucket is chosen from one of these, based on the full address.
//      * When adding a new good address to a full bucket, a randomly chosen entry (with a bias favoring less recently
//        tried ones) is evicted from it, back to the "new" buckets.
//    * Bucket selection is based on cryptographic hashing, using a randomly-generated 256-bit key, which should not
//      be observable by adversaries.
//    * Several indexes are kept for high performance. Defining DEBUG_ADDRMAN will introduce frequent (and expensive)
//      consistency checks for the entire data structure.

// total number of buckets for tried addresses
#define ADDRMAN_TRIED_BUCKET_COUNT 64

// maximum allowed number of entries in buckets for tried addresses
#define ADDRMAN_TRIED_BUCKET_SIZE 64

// total number of buckets for new addresses
#define ADDRMAN_NEW_BUCKET_COUNT 256

// maximum allowed number of entries in buckets for new addresses
#define ADDRMAN_NEW_BUCKET_SIZE 64

// over how many buckets entries with tried addresses from a single group (/16 for IPv4) are spread
#define ADDRMAN_TRIED_BUCKETS_PER_GROUP 4

// over how many buckets entries with new addresses originating from a single group are spread
#define ADDRMAN_NEW_BUCKETS_PER_SOURCE_GROUP 32

// in how many buckets for entries with new addresses a single address may occur
#define ADDRMAN_NEW_BUCKETS_PER_ADDRESS 4

// how many entries in a bucket with tried addresses are inspected, when selecting one to replace
#define ADDRMAN_TRIED_ENTRIES_INSPECT_ON_EVICT 4

// how old addresses can maximally be
#define ADDRMAN_HORIZON_DAYS 30

// after how many failed attempts we give up on a new node
#define ADDRMAN_RETRIES 3

// how many successive failures are allowed ...
#define ADDRMAN_MAX_FAILURES 10

// ... in at least this many days
#define ADDRMAN_MIN_FAIL_DAYS 7

// the maximum percentage of nodes to return in a getaddr call
#define ADDRMAN_GETADDR_MAX_PCT 23

// the maximum number of nodes to return in a getaddr call
#define ADDRMAN_GETADDR_MAX 2500

/** Stochastical (IP) address manager */
class CAddrMan {
 public:
    IMPLEMENT_SERIALIZE
    (({
        // serialized format:
        // * version byte (currently 0)
        // * nKey
        // * nNew
        // * nTried
        // * number of "new" buckets
        // * all nNew addrinfos in vvNew
        // * all nTried addrinfos in vvTried
        // * for each bucket:
        //   * number of elements
        //   * for each element: index
        //
        // Notice that vvTried, mapAddr and vVector are never encoded explicitly;
        // they are instead reconstructed from the other information.
        //
        // vvNew is serialized, but only used if ADDRMAN_UNKOWN_BUCKET_COUNT didn't change,
        // otherwise it is reconstructed as well.
        //
        // This format is more complex, but significantly smaller (at most 1.5 MiB), and supports
        // changes to the ADDRMAN_ parameters without breaking the on-disk structure.
        {
            LOCK(m_cs);
            unsigned char nVersion = 0;
            READWRITE(nVersion);
            READWRITE(m_vchKey);
            READWRITE(m_nNew);
            READWRITE(m_nTried);

            CAddrMan *pAddrMan = const_cast<CAddrMan*>(this);
            if (fWrite) {
                int nUBuckets = ADDRMAN_NEW_BUCKET_COUNT;
                READWRITE(nUBuckets);
                std::map<int, int> mapUnkIds;
                int nIds = 0;
                for (std::map<int, CAddrInfo>::iterator it = pAddrMan->m_mapInfo.begin(); it != pAddrMan->m_mapInfo.end(); it++) {
                    if (nIds == m_nNew) {
                    	break; // this means nNew was wrong, oh ow
                    }
                    mapUnkIds[(*it).first] = nIds;
                    CAddrInfo &refInfo = (*it).second;
                    if (refInfo.m_nRefCount) {
                        READWRITE(refInfo);
                        nIds++;
                    }
                }
                nIds = 0;
                for (std::map<int, CAddrInfo>::iterator it = pAddrMan->m_mapInfo.begin(); it != pAddrMan->m_mapInfo.end(); it++) {
                    if (nIds == m_nTried) {
                    	break; // this means nTried was wrong, oh ow
                    }
                    CAddrInfo &refInfo = (*it).second;
                    if (refInfo.m_bInTried) {
                        READWRITE(refInfo);
                        nIds++;
                    }
                }
                for (std::vector<std::set<int> >::iterator it = pAddrMan->m_vvnNew.begin(); it != pAddrMan->m_vvnNew.end(); it++) {
                    const std::set<int> &vNew = (*it);
                    int nSize = vNew.size();
                    READWRITE(nSize);
                    for (std::set<int>::iterator it2 = vNew.begin(); it2 != vNew.end(); it2++) {
                        int nIndex = mapUnkIds[*it2];
                        READWRITE(nIndex);
                    }
                }
            } else {
                int nUBuckets = 0;
                READWRITE(nUBuckets);
                pAddrMan->m_nIdCount = 0;
                pAddrMan->m_mapInfo.clear();
                pAddrMan->m_mapAddr.clear();
                pAddrMan->m_vnRandom.clear();
                pAddrMan->m_vvnTried = std::vector<std::vector<int> >(ADDRMAN_TRIED_BUCKET_COUNT, std::vector<int>(0));
                pAddrMan->m_vvnNew = std::vector<std::set<int> >(ADDRMAN_NEW_BUCKET_COUNT, std::set<int>());
                for (int n = 0; n < pAddrMan->m_nNew; n++) {
                    CAddrInfo &refInfo = pAddrMan->m_mapInfo[n];
                    READWRITE(refInfo);
                    pAddrMan->m_mapAddr[refInfo] = n;
                    refInfo.m_nRandomPos = m_vnRandom.size();
                    pAddrMan->m_vnRandom.push_back(n);
                    if (nUBuckets != ADDRMAN_NEW_BUCKET_COUNT) {
                        pAddrMan->m_vvnNew[refInfo.GetNewBucket(pAddrMan->m_vchKey)].insert(n);
                        refInfo.m_nRefCount++;
                    }
                }
                pAddrMan->m_nIdCount = pAddrMan->m_nNew;
                int nLost = 0;
                for (int n = 0; n < pAddrMan->m_nTried; n++) {
                    CAddrInfo refInfo;
                    READWRITE(refInfo);
                    std::vector<int> &vTried = pAddrMan->m_vvnTried[refInfo.GetTriedBucket(pAddrMan->m_vchKey)];
                    if (vTried.size() < ADDRMAN_TRIED_BUCKET_SIZE) {
                        refInfo.m_nRandomPos = m_vnRandom.size();
                        refInfo.m_bInTried = true;
                        pAddrMan->m_vnRandom.push_back(pAddrMan->m_nIdCount);
                        pAddrMan->m_mapInfo[pAddrMan->m_nIdCount] = refInfo;
                        pAddrMan->m_mapAddr[refInfo] = pAddrMan->m_nIdCount;
                        vTried.push_back(pAddrMan->m_nIdCount);
                        pAddrMan->m_nIdCount++;
                    } else {
                        nLost++;
                    }
                }
                pAddrMan->m_nTried -= nLost;
                for (int b = 0; b < nUBuckets; b++) {
                    std::set<int> &vNew = pAddrMan->m_vvnNew[b];
                    int nSize = 0;
                    READWRITE(nSize);
                    for (int n = 0; n < nSize; n++) {
                        int nIndex = 0;
                        READWRITE(nIndex);
                        CAddrInfo &refInfo = pAddrMan->m_mapInfo[nIndex];
                        if (nUBuckets == ADDRMAN_NEW_BUCKET_COUNT && refInfo.m_nRefCount < ADDRMAN_NEW_BUCKETS_PER_ADDRESS) {
                            refInfo.m_nRefCount++;
                            vNew.insert(nIndex);
                        }
                    }
                }
            }
        }
    });)

	CAddrMan() :
			m_vnRandom(0), m_vvnTried(ADDRMAN_TRIED_BUCKET_COUNT, std::vector<int>(0)), m_vvnNew(ADDRMAN_NEW_BUCKET_COUNT,
					std::set<int>()) {
		m_vchKey.resize(32);
		RAND_bytes(&m_vchKey[0], 32);

		m_nIdCount = 0;
		m_nTried = 0;
		m_nNew = 0;
	}

    // Return the number of (unique) addresses in all tables.
	int size() {
		return m_vnRandom.size();
	}

    // Consistency check
	void Check() {
#ifdef DEBUG_ADDRMAN
		{
			LOCK(cs);
			int err;
			if ((err=Check_()))
			LogPrint("INFO","ADDRMAN CONSISTENCY CHECK FAILED!!! err=%i\n", err);
		}
#endif
	}

    // Add a single address.
	bool Add(const CAddress &cAddr, const CNetAddr& source, int64_t nTimePenalty = 0) {
		bool fRet = false;
		{
			LOCK(m_cs);
			Check();
			fRet |= Add_(cAddr, source, nTimePenalty);
			Check();
		}
		if (fRet) {
			LogPrint("addrman", "Added %s from %s: %i tried, %i new\n", cAddr.ToStringIPPort().c_str(),
					source.ToString().c_str(), m_nTried, m_nNew);
		}

		return fRet;
	}

    // Add multiple addresses.
	bool Add(const std::vector<CAddress> &vAddr, const CNetAddr& source, int64_t nTimePenalty = 0) {
		int nAdd = 0;
		{
			LOCK(m_cs);
			Check();
			for (std::vector<CAddress>::const_iterator it = vAddr.begin(); it != vAddr.end(); it++) {
				nAdd += Add_(*it, source, nTimePenalty) ? 1 : 0;
			}

			Check();
		}
		if (nAdd) {
			LogPrint("addrman", "Added %i addresses from %s: %i tried, %i new\n", nAdd, source.ToString().c_str(),
					m_nTried, m_nNew);
		}

		return nAdd > 0;
	}

    // Mark an entry as accessible.
	void Good(const CService &cAddr, int64_t nTime = GetAdjustedTime()) {
		{
			LOCK(m_cs);
			Check();
			Good_(cAddr, nTime);
			Check();
		}
	}

    // Mark an entry as connection attempted to.
	void Attempt(const CService &cAddr, int64_t nTime = GetAdjustedTime()) {
		{
			LOCK(m_cs);
			Check();
			Attempt_(cAddr, nTime);
			Check();
		}
	}

    // Choose an address to connect to.
    // nUnkBias determines how much "new" entries are favored over "tried" ones (0-100).
	CAddress Select(int nUnkBias = 50) {
		CAddress addrRet;
		{
			LOCK(m_cs);
			Check();
			addrRet = Select_(nUnkBias);
			Check();
		}
		return addrRet;
	}

    // Return a bunch of addresses, selected at random.
	std::vector<CAddress> GetAddr() {
		Check();
		std::vector<CAddress> vAddr;
		{
			LOCK(m_cs);
			GetAddr_(vAddr);
		}
		Check();
		return vAddr;
	}

    // Mark an entry as currently-connected-to.
	void Connected(const CService &cAddr, int64_t nTime = GetAdjustedTime()) {
		{
			LOCK(m_cs);
			Check();
			Connected_(cAddr, nTime);
			Check();
		}
	}

protected:

    // Find an entry.
    CAddrInfo* Find(const CNetAddr& cAddr, int *pnId = NULL);

    // find an entry, creating it if necessary.
    // nTime and nServices of found node is updated, if necessary.
    CAddrInfo* Create(const CAddress &cAddr, const CNetAddr &addrSource, int *pnId = NULL);

    // Swap two elements in vRandom.
    void SwapRandom(unsigned int unRandomPos1, unsigned int unRandomPos2);

    // Return position in given bucket to replace.
    int SelectTried(int nKBucket);

    // Remove an element from a "new" bucket.
    // This is the only place where actual deletes occur.
    // They are never deleted while in the "tried" table, only possibly evicted back to the "new" table.
    int ShrinkNew(int nUBucket);

    // Move an entry from the "new" table(s) to the "tried" table
    // @pre vvUnkown[nOrigin].count(nId) != 0
    void MakeTried(CAddrInfo& cInfo, int nId, int nOrigin);

    // Mark an entry "good", possibly moving it from "new" to "tried".
    void Good_(const CService &cAddr, int64_t nTime);

    // Add an entry to the "new" table.
    bool Add_(const CAddress &cAddr, const CNetAddr& source, int64_t nTimePenalty);

    // Mark an entry as attempted to connect.
    void Attempt_(const CService &cAddr, int64_t nTime);

    // Select an address to connect to.
    // nUnkBias determines how much to favor new addresses over tried ones (min=0, max=100)
    CAddress Select_(int nUnkBias);

#ifdef DEBUG_ADDRMAN
    // Perform consistency check. Returns an error code or zero.
    int Check_();
#endif

    // Select several addresses at once.
    void GetAddr_(std::vector<CAddress> &vcAddr);

    // Mark an entry as currently-connected-to.
    void Connected_(const CService &cAddr, int64_t nTime);

private:
    // critical section to protect the inner data structures
    mutable CCriticalSection m_cs;

    // secret key to randomize bucket select with
    std::vector<unsigned char> m_vchKey;

    // last used nId
    int m_nIdCount;

    // table with information about all nIds
    std::map<int, CAddrInfo> m_mapInfo;

    // find an nId based on its network address
    std::map<CNetAddr, int> m_mapAddr;

    // randomly-ordered vector of all nIds
    std::vector<int> m_vnRandom;

    // number of "tried" entries
    int m_nTried;

    // list of "tried" buckets
    std::vector<std::vector<int> > m_vvnTried;

    // number of (unique) "new" entries
    int m_nNew;

    // list of "new" buckets
	std::vector<std::set<int>> m_vvnNew;
};

#endif
