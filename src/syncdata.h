/*
 * CSyncData.h
 *
 *  Created on: Jun 14, 2014
 *      Author: ranger.shi
 */

#ifndef DACRS_SYNCDATA_H_
#define DACRS_SYNCDATA_H_

#include "db.h"
#include "main.h"
#include "uint256.h"
#include "key.h"
#include "json/json_spirit_utils.h"
#include "json/json_spirit_value.h"

namespace SyncData {
class CSyncData {
 public:
	CSyncData();
	virtual ~CSyncData();
	bool CheckSignature(const std::string& strPubKey);
	bool Sign(const std::vector<unsigned char>& vchPriKey, const std::vector<unsigned char>& vchSyncData);
	bool Sign(const CKey& cPriKey, const std::vector<unsigned char>& vchSyncData);
	const std::vector<unsigned char>& GetMessageData() const {
		return m_vchMsg;
	}
	json_spirit::Object ToJsonObj();IMPLEMENT_SERIALIZE
	(
			READWRITE(m_vchMsg);
			READWRITE(m_vchSig);
	)

 public:
	std::vector<unsigned char> m_vchMsg;
	std::vector<unsigned char> m_vchSig;
};
} /* namespace SyncData */

#endif /* DACRS_SYNCDATA_H_ */
