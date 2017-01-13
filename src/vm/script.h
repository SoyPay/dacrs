/*
 * VmScript.h
 *
 *  Created on: Sep 2, 2014
 *      Author: ranger.shi
 */

#ifndef DACRS_VM_SCRIPT_H_
#define DACRS_VM_SCRIPT_H_

#include "serialize.h"
using namespace std;

/**
 * @brief Load script binary code class
 */
class CVmScript {
 public:
	vector<unsigned char> m_vuchRom;      		//!< Binary code
	vector<unsigned char> m_vuchScriptExplain;	// !<explain the binary code action
	int  m_nScriptType = 0;                    //!<脚本的类型 0:8051,1:lua

 public:
	/**
	 * @brief
	 * @return
	 */
	bool IsValid() {
		///Binary code'size less 64k
		if ((m_vuchRom.size() > 64 * 1024) || (m_vuchRom.size() <= 0)) {
			return false;
		}
		if (m_vuchRom[0] != 0x02) {
			if (!memcmp(&m_vuchRom[0], "mylib = require", strlen("mylib = require"))) {
				m_nScriptType = 1;                    //lua脚本
				return true;                    //lua脚本，直接返回
			} else {
				return false;
			}
		} else {
			m_nScriptType = 0;                    //8051脚本
		}
		//!<指定版本的SDK以上，才去校验 账户平衡开关的取值
		if (memcmp(&m_vuchRom[0x0004], "\x00\x02\x02", 3) >= 0) {
			if (!((m_vuchRom[0x0014] == 0x00) || (m_vuchRom[0x0014] == 0x01))) {
				return false;
			}
		}
		return true;
	}

	bool IsCheckAccount(void) {
		if (m_nScriptType) {
			return false;                    //lua脚本，直接返回(关掉账户平衡)
		}

		//!<指定版本的SDK以上，才去读取 账户平衡开关的取值
		if (memcmp(&m_vuchRom[0x0004], "\x00\x02\x02", 3) >= 0) {
			if (m_vuchRom[0x0014] == 0x01) {
				return true;
			}
		}
		return false;
	}

	CVmScript();

	int getScriptType() {
		return m_nScriptType;
	}
	IMPLEMENT_SERIALIZE
	(
			READWRITE(m_vuchRom);
			READWRITE(m_vuchScriptExplain);
	)

	virtual ~CVmScript();
};

#endif /* DACRS_VM_SCRIPT_H_ */
