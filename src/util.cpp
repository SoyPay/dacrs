// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Dacrs developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "util.h"

#include "chainparams.h"
#include "netbase.h"
#include "sync.h"
#include "ui_interface.h"
#include "uint256.h"
#include "version.h"

#include <stdarg.h>

#include <boost/date_time/posix_time/posix_time.hpp>

#ifndef WIN32
// for posix_fallocate
#ifdef __linux_

#ifdef _POSIX_C_SOURCE
#undef _POSIX_C_SOURCE
#endif

#define _POSIX_C_SOURCE 200112L
#include <sys/prctl.h>

#endif

#include <algorithm>
#include <fcntl.h>
#include <sys/resource.h>
#include <sys/stat.h>

#else

#ifdef _MSC_VER
#pragma warning(disable:4786)
#pragma warning(disable:4804)
#pragma warning(disable:4805)
#pragma warning(disable:4717)
#endif

#ifdef _WIN32_WINNT
#undef _WIN32_WINNT
#endif
#define _WIN32_WINNT 0x0501

#ifdef _WIN32_IE
#undef _WIN32_IE
#endif
#define _WIN32_IE 0x0501

#define WIN32_LEAN_AND_MEAN 1
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <io.h> /* for _commit */
#include <shlobj.h>
#endif

#include <boost/algorithm/string/case_conv.hpp> // for to_lower()#include <boost/algorithm/string/join.hpp>#include <boost/algorithm/string/predicate.hpp> // for startswith() and endswith()#include <boost/filesystem.hpp>#include <boost/filesystem/fstream.hpp>#include <boost/program_options/detail/config_file.hpp>#include <boost/program_options/parsers.hpp>#include <openssl/crypto.h>#include <openssl/rand.h>// Work around clang compilation problem in Boost 1.46:// /usr/include/boost/program_options/detail/config_file.hpp:163:17: error: call to function 'to_internal' that is neither visible in the template definition nor found by argument-dependent lookup// See also: http://stackoverflow.com/questions/10020179/compilation-fail-in-boost-librairies-program-options//           http://clang.debian.net/status.php?version=3.0&key=CANNOT_FIND_FUNCTIONnamespace boost {namespace program_options {string to_internal(const string&);}
}

using namespace std;

bool g_bDaemon = false;
string g_strMiscWarning;
bool g_bNoListen = false;
volatile bool g_bReopenDebugLog = false;

CClientUIInterface g_cUIInterface;
CClientUIInterface * g_pUIInterface = &g_cUIInterface;

// Init OpenSSL library multithreading support
static CCriticalSection** m_sppMutexOpenSSL;
void locking_callback(int mode, int i, const char* file, int line) {
if (mode & CRYPTO_LOCK) {
ENTER_CRITICAL_SECTION(*m_sppMutexOpenSSL[i]);
} else {
LEAVE_CRITICAL_SECTION(*m_sppMutexOpenSSL[i]);
}
}

// Init
class CInit {
 public:
	CInit() {
		// Init OpenSSL library multithreading support
		m_sppMutexOpenSSL = (CCriticalSection**) OPENSSL_malloc(CRYPTO_num_locks() * sizeof(CCriticalSection*));
		for (int i = 0; i < CRYPTO_num_locks(); i++) {
		m_sppMutexOpenSSL[i] = new CCriticalSection();
		}
	CRYPTO_set_locking_callback(locking_callback);

#ifdef WIN32
// Seed random number generator with screen scrape and other hardware sources
	RAND_screen();
#endif

	// Seed random number generator with performance counter
	RandAddSeed();
}

	~CInit() {
		// Shutdown OpenSSL library multithreading support
		CRYPTO_set_locking_callback(NULL);
		for (int i = 0; i < CRYPTO_num_locks(); i++) {
			delete m_sppMutexOpenSSL[i];
		}
		OPENSSL_free(m_sppMutexOpenSSL);
}
} instance_of_cinit;

//void RandAddSeed() {
//// Seed with CPU performance counter
//int64_t nCounter = GetPerformanceCounter();
//RAND_add(&nCounter, sizeof(nCounter), 1.5);
//memset(&nCounter, 0, sizeof(nCounter));
//}

//void RandAddSeedPerfmon() {
//RandAddSeed();
//
//// This can take up to 2 seconds, so only do it every 10 minutes
//static int64_t nLastPerfmon;
//if (GetTime() < nLastPerfmon + 10 * 60)
//return;
//nLastPerfmon = GetTime();
//
//#ifdef WIN32
//// Don't need this on Linux, OpenSSL automatically uses /dev/urandom
//// Seed with the entire set of perfmon data
//unsigned char pdata[250000];
//memset(pdata, 0, sizeof(pdata));
//unsigned long nSize = sizeof(pdata);
//long ret = RegQueryValueExA(HKEY_PERFORMANCE_DATA, "Global", NULL, NULL, pdata, &nSize);
//RegCloseKey(HKEY_PERFORMANCE_DATA);
//if (ret == ERROR_SUCCESS) {
//RAND_add(pdata, nSize, nSize / 100.0);
//OPENSSL_cleanse(pdata, nSize);
//LogPrint("rand", "RandAddSeed() %lu bytes\n", nSize);
//}
//#endif
//}

//uint64_t GetRand(uint64_t nMax) {
//if (nMax == 0)
//return 0;
//
//// The range of the random source must be a multiple of the modulus
//// to give every possible output value an equal possibility
//uint64_t nRange = (numeric_limits<uint64_t>::max() / nMax) * nMax;
//uint64_t nRand = 0;
//do
//RAND_bytes((unsigned char*) &nRand, sizeof(nRand));
//while (nRand >= nRange);
//return (nRand % nMax);
//}

//int GetRandInt(int nMax) {
//return GetRand(nMax);
//}

//uint256 GetRandHash() {
//uint256 hash;
//RAND_bytes((unsigned char*) &hash, sizeof(hash));
//return hash;
//}

// LogPrint("INFO",) has been broken a couple of times now
// by well-meaning people adding mutexes in the most straightforward way.
// It breaks because it may be called by global destructors during shutdown.
// Since the order of destruction of static/global objects is undefined,
// defining a mutex as a global object doesn't work (the mutex gets
// destroyed, and then some later destructor calls OutputDebugStringF,
// maybe indirectly, and you get a core dump at shutdown trying to lock
// the mutex).

static boost::once_flag g_tDebugPrintInitFlag = BOOST_ONCE_INIT;
// We use boost::call_once() to make sure these are initialized in
// in a thread-safe manner the first time it is called:
//static FILE* fileout = NULL;
//static boost::mutex* mutexDebugLog = NULL;

struct ST_DebugLogFile {
	ST_DebugLogFile():bNewLine(true), pFileout(NULL), MutexDebugLog(NULL) {}
	~ST_DebugLogFile() {
		if(pFileout) {
			fclose(pFileout);
			pFileout = NULL;
		}
		if(MutexDebugLog) {
			delete MutexDebugLog;
			MutexDebugLog = NULL;
		}
	}
	bool bNewLine;
	FILE* pFileout;
	boost::mutex* MutexDebugLog;
};

static map<string,ST_DebugLogFile> g_DebugLogs;

static void DebugPrintInit() {
	shared_ptr<vector<string>> ptrTemp = SysCfg().GetMultiArgsMap("-debug");
	const vector<string>& vstrCategories = *(ptrTemp.get());
	set<string> vstrLogfiles(vstrCategories.begin(), vstrCategories.end());

	shared_ptr<vector<string>> ptrTemp1 = SysCfg().GetMultiArgsMap("-nodebug");
	const vector<string>& vstrNoCategories = *(ptrTemp1.get());
	set<string> vstrNoLogfiles(vstrNoCategories.begin(), vstrNoCategories.end());

	if (SysCfg().IsDebugAll()) {
		vstrLogfiles.clear();
		vstrLogfiles = vstrNoLogfiles;
		vstrLogfiles.insert("debug");

		for (auto& tmp : vstrLogfiles) {
			g_DebugLogs[tmp];
		}

		{
			FILE* pFileout = NULL;
			boost::filesystem::path cPathDebug;
			string strFile = "debug.log";
			cPathDebug = GetDataDir() / strFile;
			pFileout = fopen(cPathDebug.string().c_str(), "a");
			if (pFileout) {
				ST_DebugLogFile& log = g_DebugLogs["debug"];
				setbuf(pFileout, NULL); // unbuffered
				log.pFileout = pFileout;
				log.MutexDebugLog = new boost::mutex();
			}
		}

	} else {
		for (auto& auTemp : vstrNoLogfiles) {
			vstrLogfiles.erase(auTemp);
		}
		vstrLogfiles.insert("debug");

		for (auto& auCat : vstrLogfiles) {
			FILE* pFileout = NULL;
			boost::filesystem::path pathDebug;
			string strFile = auCat + ".log";
			pathDebug = GetDataDir() / strFile;
			pFileout = fopen(pathDebug.string().c_str(), "a");
			if (pFileout) {
				ST_DebugLogFile& tDebugLogFile = g_DebugLogs[auCat];
				setbuf(pFileout, NULL); // unbuffered
				tDebugLogFile.pFileout = pFileout;
				tDebugLogFile.MutexDebugLog = new boost::mutex();
			}
		}
	}
}

int LogPrintStr(const string &strLog) {
	return LogPrintStr(NULL, strLog);
}

string GetLogHead(int nLine, const char* pszFile, const char* pszCategory) {
	string strTemp(pszCategory != NULL ? pszCategory : "");
	if (SysCfg().IsDebug()) {
		if (SysCfg().IsLogPrintLine()) {
			return tfm::format("[%s:%d]%s: ", pszFile, nLine, strTemp);
		}
	}
	return string("");
}

/**
 *  日志文件预处理。写日志文件前被调用，检测文件A是否超长。
 *  当超长则先将原文件A重命名为Abak，再打开并创建A文件，删除重命名文件Abak，返回。
 * @param path  文件路径
 * @param len  写入数据的长度
 * @param stream  文件的句柄
 * @return
 */
int LogFilePreProcess(const char *pszPath, size_t unLength, FILE** ppStream) {
	if ((NULL == pszPath) || (unLength <= 0) || (NULL == *ppStream)) {
		return -1;
	}
	int nSize = ftell(*ppStream); 									//当前文件长度
	if (nSize + unLength > (size_t) SysCfg().GetLogMaxSize()) {   	//文件超长，关闭，删除，再创建
		FILE *pFileout = NULL;
		fclose(*ppStream);
		string strBkFile = strprintf("%sbak", pszPath);
		rename(pszPath, strBkFile.c_str());  						//原文件重命名
		pFileout = fopen(pszPath, "a+");   							//重新打开， 类似于删除文件.
		if (pFileout) {
			*ppStream = pFileout;
			if (remove(strBkFile.c_str()) != 0) {  					//删除重命名文件
				return -1;
			}
		} else {
			return -1;
		}
	}
	return 1;
}

int LogPrintStr(const char* pszCategory, const string &strLog) {
	if (!SysCfg().IsDebug()) {
		return 0;
	}
	int nRet = 0; 													// Returns total number of characters written
	boost::call_once(&DebugPrintInit, g_tDebugPrintInitFlag);

	map<string, ST_DebugLogFile>::iterator it;
	if (SysCfg().IsDebugAll()) {
		if (NULL != pszCategory) {
			it = g_DebugLogs.find(pszCategory);
			if (it != g_DebugLogs.end()) {
				return 0;
			}
		}
		it = g_DebugLogs.find("debug");
	} else {
		it = g_DebugLogs.find((NULL == pszCategory) ? ("debug") : (pszCategory));
		if (it == g_DebugLogs.end()) {
			return 0;
		}
	}

	if (SysCfg().IsPrint2Console()) {
		nRet = fwrite(strLog.data(), 1, strLog.size(), stdout);
	}
	if (SysCfg().IsPrintToFile()) {
		ST_DebugLogFile& tLog = it->second;
		boost::mutex::scoped_lock scoped_lock(*tLog.MutexDebugLog);

		boost::filesystem::path cPathDebug;
		string strFile = it->first + ".tLog";
		cPathDebug = GetDataDir() / strFile;   // /home/share/bille/dacrs_test/regtest/INFO.log
		// Debug print useful for profiling
		if (SysCfg().IsLogTimestamps() && tLog.bNewLine) {
			string strTimeFormat = DateTimeStrFormat("%Y-%m-%d %H:%M:%S", GetTime());
			LogFilePreProcess(cPathDebug.string().c_str(), strTimeFormat.length() + 1 + strLog.size(), &tLog.pFileout);
			nRet += fprintf(tLog.pFileout, "%s ", strTimeFormat.c_str());
		}
		if (!strLog.empty() && strLog[strLog.size() - 1] == '\n') {
			tLog.bNewLine = true;
		} else {
			tLog.bNewLine = false;
		}
		LogFilePreProcess(cPathDebug.string().c_str(), strLog.size(), &tLog.pFileout);
		nRet = fwrite(strLog.data(), 1, strLog.size(), tLog.pFileout);
	}
	return nRet;
}

string FormatMoney(int64_t llFormat, bool bPlus) {
	// Note: not using straight sprintf here because we do NOT want
	// localized number formatting.
	int64_t llAbs = (llFormat > 0 ? llFormat : -llFormat);
	int64_t llQuotient = llAbs / COIN;
	int64_t llRemainder = llAbs % COIN;
	string strString = strprintf("%d.%08d", llQuotient, llRemainder);

	// Right-trim excess zeros before the decimal point:
	int nTrim = 0;
	for (int i = strString.size() - 1; (strString[i] == '0' && isdigit(strString[i - 2])); --i) {
		++nTrim;
	}
	if (nTrim) {
		strString.erase(strString.size() - nTrim, nTrim);
	}
	if (llFormat < 0) {
		strString.insert((unsigned int) 0, 1, '-');
	} else if (bPlus && llFormat > 0) {
		strString.insert((unsigned int) 0, 1, '+');
	}
	return strString;
}

bool ParseMoney(const string& strContent, int64_t& llRet) {
	return ParseMoney(strContent.c_str(), llRet);
}

bool ParseMoney(const char* pszIn, int64_t& llRet) {
	string strWhole;
	int64_t llUnits = 0;
	const char* pTemp = pszIn;

	while (isspace(*pTemp)) {
		pTemp++;
	}

	for (; *pTemp; pTemp++) {
		if (*pTemp == '.') {
			pTemp++;
			int64_t llMult = CENT * 10;
			while (isdigit(*pTemp) && (llMult > 0)) {
				llUnits += llMult * (*pTemp++ - '0');
				llMult /= 10;
			}
			break;
		}
		if (isspace(*pTemp)) {
			break;
		}
		if (!isdigit(*pTemp)) {
			return false;
		}

		strWhole.insert(strWhole.end(), *pTemp);
	}
	for (; *pTemp; pTemp++) {
		if (!isspace(*pTemp)) {
			return false;
		}
	}

	if (strWhole.size() > 10) {				// guard against 63 bit overflow
		return false;
	}
	if (llUnits < 0 || llUnits > COIN) {
		return false;
	}

	int64_t llWhole = atoi64(strWhole);
	int64_t llValue = llWhole * COIN + llUnits;
	llRet = llValue;

	return true;
}

// safeChars chosen to allow simple messages/URLs/email addresses, but avoid anything
// even possibly remotely dangerous like & or >
static string safeChars("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ01234567890 .,;_/:?@");
string SanitizeString(const string& strContent) {
	string strResult;
	for (string::size_type i = 0; i < strContent.size(); i++) {
		if (safeChars.find(strContent[i]) != string::npos) {
			strResult.push_back(strContent[i]);
		}
	}
	return strResult;
}

const signed char g_util_hexdigit[256] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, -1, -1, -1, -1, -1, -1, -1, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0xa, 0xb, 0xc, 0xd, 0xe,
		0xf, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, };

bool IsHex(const string& strContent) {
	for (char c : strContent) {
		if (HexDigit(c) < 0) {
			return false;
		}
	}
	return (strContent.size() > 0) && (strContent.size() % 2 == 0);
}

vector<unsigned char> ParseHex(const char* pszContent) {
	// convert hex dump to vector
	vector<unsigned char> vchContent;
	while (true) {
		while (isspace(*pszContent)) {
			pszContent++;
		}
		signed char chValue = HexDigit(*pszContent++);
		if (chValue == (signed char) -1) {
			break;
		}
		unsigned char uchValue = (chValue << 4);
		chValue = HexDigit(*pszContent++);
		if (chValue == (signed char) -1) {
			break;
		}
		uchValue |= chValue;
		vchContent.push_back(uchValue);
	}
	return vchContent;
}

vector<unsigned char> ParseHex(const string& strContent) {
	return ParseHex(strContent.c_str());
}

static void InterpretNegativeSetting(string strName, map<string, string>& mapSettingsRet) {
	// interpret -nofoo as -foo=0 (and -nofoo=0 as -foo=1) as long as -foo not set
	if (strName.find("-no") == 0) {
		string strPositive("-");
		strPositive.append(strName.begin() + 3, strName.end());
		if (mapSettingsRet.count(strPositive) == 0) {
			bool bValue = !SysCfg().GetBoolArg(strName, false);
			mapSettingsRet[strPositive] = (bValue ? "1" : "0");
		}
	}
}

string EncodeBase64(const unsigned char* pszContent, size_t unLength) {
	static const char *pBase64 = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	string strRet = "";
	strRet.reserve((unLength + 2) / 3 * 4);

	int nMode = 0, nLeft = 0;
	const unsigned char *pchEnd = pszContent + unLength;

	while (pszContent < pchEnd) {
		int nEnc = *(pszContent++);
		switch (nMode) {
		case 0: // we have no bits
			strRet += pBase64[nEnc >> 2];
			nLeft = (nEnc & 3) << 4;
			nMode = 1;
			break;
		case 1: // we have two bits
			strRet += pBase64[nLeft | (nEnc >> 4)];
			nLeft = (nEnc & 15) << 2;
			nMode = 2;
			break;
		case 2: // we have four bits
			strRet += pBase64[nLeft | (nEnc >> 6)];
			strRet += pBase64[nEnc & 63];
			nMode = 0;
			break;
		}
	}

	if (nMode) {
		strRet += pBase64[nLeft];
		strRet += '=';
		if (nMode == 1) {
			strRet += '=';
		}
	}

	return strRet;
}

string EncodeBase64(const string& strContent) {
	return EncodeBase64((const unsigned char*) strContent.c_str(), strContent.size());
}

vector<unsigned char> DecodeBase64(const char* pszContent, bool* pbInvalid) {
	static const int decode64_table[256] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62, -1, -1,
			-1, 63, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, -1, -1, -1, -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
			10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1, -1, 26, 27, 28, 29, 30,
			31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1, -1, -1, -1,
			-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };

	if (pbInvalid) {
		*pbInvalid = false;
	}

	vector<unsigned char> vchRet;
	vchRet.reserve(strlen(pszContent) * 3 / 4);

	int nMode = 0;
	int nLeft = 0;
	while (1) {
		int nDec = decode64_table[(unsigned char) *pszContent];
		if (nDec == -1) {
			break;
		}
		pszContent++;
		switch (nMode) {
		case 0: // we have no bits and get 6
			nLeft = nDec;
			nMode = 1;
			break;
		case 1: // we have 6 bits and keep 4
			vchRet.push_back((nLeft << 2) | (nDec >> 4));
			nLeft = nDec & 15;
			nMode = 2;
			break;
		case 2: // we have 4 bits and get 6, we keep 2
			vchRet.push_back((nLeft << 4) | (nDec >> 2));
			nLeft = nDec & 3;
			nMode = 3;
			break;
		case 3: // we have 2 bits and get 6
			vchRet.push_back((nLeft << 6) | nDec);
			nMode = 0;
			break;
		}
	}

	if (pbInvalid)
		switch (nMode) {
		case 0: // 4n base64 characters processed: ok
			break;
		case 1: // 4n+1 base64 character processed: impossible
			*pbInvalid = true;
			break;
		case 2: // 4n+2 base64 characters processed: require '=='
			if (nLeft || pszContent[0] != '=' || pszContent[1] != '=' || decode64_table[(unsigned char) pszContent[2]] != -1) {
				*pbInvalid = true;
			}
			break;
		case 3: // 4n+3 base64 characters processed: require '='
			if (nLeft || pszContent[0] != '=' || decode64_table[(unsigned char) pszContent[1]] != -1) {
				*pbInvalid = true;
			}
			break;
		}
	return vchRet;
}

string DecodeBase64(const string& strContent) {
	vector<unsigned char> vchRet = DecodeBase64(strContent.c_str());
	return string((const char*) &vchRet[0], vchRet.size());
}

string EncodeBase32(const unsigned char* pszContent, size_t unlength) {
	static const char *pBase32 = "abcdefghijklmnopqrstuvwxyz234567";

	string strRet = "";
	strRet.reserve((unlength + 4) / 5 * 8);

	int nMode = 0, nLeft = 0;
	const unsigned char *pchEnd = pszContent + unlength;

	while (pszContent < pchEnd) {
		int nEnc = *(pszContent++);
		switch (nMode) {
		case 0: // we have no bits
			strRet += pBase32[nEnc >> 3];
			nLeft = (nEnc & 7) << 2;
			nMode = 1;
			break;
		case 1: // we have three bits
			strRet += pBase32[nLeft | (nEnc >> 6)];
			strRet += pBase32[(nEnc >> 1) & 31];
			nLeft = (nEnc & 1) << 4;
			nMode = 2;
			break;
		case 2: // we have one bit
			strRet += pBase32[nLeft | (nEnc >> 4)];
			nLeft = (nEnc & 15) << 1;
			nMode = 3;
			break;
		case 3: // we have four bits
			strRet += pBase32[nLeft | (nEnc >> 7)];
			strRet += pBase32[(nEnc >> 2) & 31];
			nLeft = (nEnc & 3) << 3;
			nMode = 4;
			break;
		case 4: // we have two bits
			strRet += pBase32[nLeft | (nEnc >> 5)];
			strRet += pBase32[nEnc & 31];
			nMode = 0;
		}
	}

	static const int nPadding[5] = { 0, 6, 4, 3, 1 };
	if (nMode) {
		strRet += pBase32[nLeft];
		for (int n = 0; n < nPadding[nMode]; n++) {
			strRet += '=';
		}
	}
	return strRet;
}

string EncodeBase32(const string& strContent) {
	return EncodeBase32((const unsigned char*) strContent.c_str(), strContent.size());
}

vector<unsigned char> DecodeBase32(const char* pszContent, bool* pbInvalid) {
	static const int g_nDecode32_table[256] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			-1, -1, -1, -1, 26, 27, 28, 29, 30, 31, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
			10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1, -1, 0, 1, 2, 3, 4, 5, 6,
			7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };

	if (pbInvalid) {
		*pbInvalid = false;
	}

	vector<unsigned char> vchRet;
	vchRet.reserve((strlen(pszContent)) * 5 / 8);

	int nMode = 0;
	int nLeft = 0;

	while (1) {
		int nDec = g_nDecode32_table[(unsigned char) *pszContent];
		if (nDec == -1) {
			break;
		}
		pszContent++;
		switch (nMode) {
		case 0: // we have no bits and get 5
			nLeft = nDec;
			nMode = 1;
			break;
		case 1: // we have 5 bits and keep 2
			vchRet.push_back((nLeft << 3) | (nDec >> 2));
			nLeft = nDec & 3;
			nMode = 2;
			break;
		case 2: // we have 2 bits and keep 7
			nLeft = nLeft << 5 | nDec;
			nMode = 3;
			break;
		case 3: // we have 7 bits and keep 4
			vchRet.push_back((nLeft << 1) | (nDec >> 4));
			nLeft = nDec & 15;
			nMode = 4;
			break;
		case 4: // we have 4 bits, and keep 1
			vchRet.push_back((nLeft << 4) | (nDec >> 1));
			nLeft = nDec & 1;
			nMode = 5;
			break;
		case 5: // we have 1 bit, and keep 6
			nLeft = nLeft << 5 | nDec;
			nMode = 6;
			break;
		case 6: // we have 6 bits, and keep 3
			vchRet.push_back((nLeft << 2) | (nDec >> 3));
			nLeft = nDec & 7;
			nMode = 7;
			break;
		case 7: // we have 3 bits, and keep 0
			vchRet.push_back((nLeft << 5) | nDec);
			nMode = 0;
			break;
		}
	}

	if (pbInvalid)
		switch (nMode) {
		case 0: // 8n base32 characters processed: ok
			break;
		case 1: // 8n+1 base32 characters processed: impossible
		case 3: //   +3
		case 6: //   +6
			*pbInvalid = true;
			break;
		case 2: // 8n+2 base32 characters processed: require '======'
			if (nLeft || pszContent[0] != '=' || pszContent[1] != '=' || pszContent[2] != '=' || pszContent[3] != '='
					|| pszContent[4] != '=' || pszContent[5] != '='
					|| g_nDecode32_table[(unsigned char) pszContent[6]] != -1) {
				*pbInvalid = true;
			}
			break;
		case 4: // 8n+4 base32 characters processed: require '===='
			if (nLeft || pszContent[0] != '=' || pszContent[1] != '=' || pszContent[2] != '=' || pszContent[3] != '='
					|| g_nDecode32_table[(unsigned char) pszContent[4]] != -1) {
				*pbInvalid = true;
			}
			break;
		case 5: // 8n+5 base32 characters processed: require '==='
			if (nLeft || pszContent[0] != '=' || pszContent[1] != '=' || pszContent[2] != '='
					|| g_nDecode32_table[(unsigned char) pszContent[3]] != -1) {
				*pbInvalid = true;
			}
			break;
		case 7: // 8n+7 base32 characters processed: require '='
			if (nLeft || pszContent[0] != '=' || g_nDecode32_table[(unsigned char) pszContent[1]] != -1) {
				*pbInvalid = true;
			}
			break;
		}
	return vchRet;
}

string DecodeBase32(const string& strContent) {
	vector<unsigned char> vchRet = DecodeBase32(strContent.c_str());
	return string((const char*) &vchRet[0], vchRet.size());
}

bool WildcardMatch(const char* pszContent, const char* pszMask) {
	while (true) {
		switch (*pszMask) {
		case '\0':
			return (*pszContent == '\0');
		case '*':
			return WildcardMatch(pszContent, pszMask + 1) || (*pszContent && WildcardMatch(pszContent + 1, pszMask));
		case '?':
			if (*pszContent == '\0')
				return false;
			break;
		default:
			if (*pszContent != *pszMask)
				return false;
			break;
		}
		pszContent++;
		pszMask++;
	}
	return false;
}

bool WildcardMatch(const string& strContent, const string& strMask) {
	return WildcardMatch(strContent.c_str(), strMask.c_str());
}

static string FormatException(exception* pExcep, const char* pszThread) {
#ifdef WIN32
	char pszModule[MAX_PATH] = "";
	GetModuleFileNameA(NULL, pszModule, sizeof(pszModule));
#else
	const char* pszModule = "Dacrs";
#endif
	if (pExcep)
		return strprintf(
		"EXCEPTION: %s       \n%s       \n%s in %s       \n", typeid(*pExcep).name(), pExcep->what(), pszModule, pszThread);
		else
		return strprintf(
		"UNKNOWN EXCEPTION       \n%s in %s       \n", pszModule, pszThread);
	}

void LogException(exception* pExcep, const char* pszThread) {
	string message = FormatException(pExcep, pszThread);
	LogPrint("INFO", "\n%s", message);
}

void PrintExceptionContinue(exception* pExcep, const char* pszThread) {
	string strMessage = FormatException(pExcep, pszThread);
	LogPrint("INFO", "\n\n************************\n%s\n", strMessage);
	fprintf(stderr, "\n\n************************\n%s\n", strMessage.c_str());
	g_strMiscWarning = strMessage;
}

boost::filesystem::path GetDefaultDataDir() {
	namespace fs = boost::filesystem;
	// Windows < Vista: C:\Documents and Settings\Username\Application Data\Dacrs
	// Windows >= Vista: C:\Users\Username\AppData\Roaming\Dacrs
	// Mac: ~/Library/Application Support/Dacrs
	// Unix: ~/.Dacrs
#ifdef WIN32
	// Windows
	return GetSpecialFolderPath(CSIDL_APPDATA) / "Dacrs";
#else
	fs::path pathRet;
	char* pszHome = getenv("HOME");
	if (pszHome == NULL || strlen(pszHome) == 0)
	pathRet = fs::path("/");
	else
	pathRet = fs::path(pszHome);
#ifdef MAC_OSX
	// Mac
	pathRet /= "Library/Application Support";
	TryCreateDirectory(pathRet);
	return pathRet / "Bitcoin";
#else
	// Unix
	return pathRet / ".Dacrs";
#endif
#endif
}

static boost::filesystem::path g_sPathCached[CBaseParams::EM_MAX_NETWORK_TYPES + 1];
static CCriticalSection g_sCSPathCached;

const boost::filesystem::path &GetDataDir(bool bNetSpecific) {
	namespace fs = boost::filesystem;

	LOCK(g_sCSPathCached);

	int nNet = CBaseParams::EM_MAX_NETWORK_TYPES;
	if (bNetSpecific) {
		nNet = SysCfg().NetworkID();
	}

	fs::path &cPath = g_sPathCached[nNet];

	// This can be called during exceptions by LogPrint("INFO",), so we cache the
	// value so we don't have to do memory allocations after that.
	if (!cPath.empty()) {
		return cPath;
	}
	if (CBaseParams::IsArgCount("-datadir")) {
		std::string strCfgDataDir = CBaseParams::GetArg("-datadir", "");
		if("cur" == strCfgDataDir) {
			strCfgDataDir = fs::initial_path<boost::filesystem::path>().string();
		}
		cPath = fs::system_complete(strCfgDataDir);
		if (!fs::is_directory(cPath)) {
			cPath = "";
			return cPath;
		}
	} else {
		cPath = GetDefaultDataDir();
	}
	if (bNetSpecific) {
		cPath /= SysCfg().DataDir();
	}
	fs::create_directories(cPath);

	return cPath;
}

void ClearDatadirCache() {
	fill(&g_sPathCached[0], &g_sPathCached[CBaseParams::EM_MAX_NETWORK_TYPES + 1], boost::filesystem::path());
}

boost::filesystem::path GetConfigFile() {
	boost::filesystem::path cPathConfigFile(CBaseParams::GetArg("-conf", "Dacrs.conf"));
	if (!cPathConfigFile.is_complete()) {
		cPathConfigFile = GetDataDir(false) / cPathConfigFile;
	}
	return cPathConfigFile;
}

void ReadConfigFile(map<string, string>& mapSettingsRet, map<string, vector<string> >& mapMultiSettingsRet) {
	boost::filesystem::ifstream streamConfig(GetConfigFile());
	if (!streamConfig.good()) {
		return; // No Dacrs.conf file is OK
	}

	set<string> setOptions;
	setOptions.insert("*");

	for (boost::program_options::detail::config_file_iterator it(streamConfig, setOptions), end; it != end; ++it) {
		// Don't overwrite existing settings so command line settings override Dacrs.conf
		string strKey = string("-") + it->string_key;
		if (mapSettingsRet.count(strKey) == 0) {
			mapSettingsRet[strKey] = it->value[0];

			// interpret nofoo=1 as foo=0 (and nofoo=0 as foo=1) as long as foo not set)
			InterpretNegativeSetting(strKey, mapSettingsRet);
		}
		mapMultiSettingsRet[strKey].push_back(it->value[0]);
	}
	// If datadir is changed in .conf file:
	ClearDatadirCache();
}

boost::filesystem::path GetPidFile() {
	boost::filesystem::path cPathPidFile(SysCfg().GetArg("-pid", "bitcoind.pid"));
	if (!cPathPidFile.is_complete()) {
		cPathPidFile = GetDataDir() / cPathPidFile;
	}
	return cPathPidFile;
}

#ifndef WIN32
void CreatePidFile(const boost::filesystem::path &path, pid_t pid)
{
	FILE* file = fopen(path.string().c_str(), "w");
	if (file)
	{
		fprintf(file, "%d\n", pid);
		fclose(file);
	}
}
#endif

bool RenameOver(boost::filesystem::path cSrc, boost::filesystem::path cDest) {
#ifdef WIN32
	return MoveFileExA(cSrc.string().c_str(), cDest.string().c_str(),
	MOVEFILE_REPLACE_EXISTING);
#else
	int rc = rename(cSrc.string().c_str(), cDest.string().c_str());
	return (rc == 0);
#endif /* WIN32 */
}

// Ignores exceptions thrown by boost's create_directory if the requested directory exists.
//   Specifically handles case where path p exists, but it wasn't possible for the user to write to the parent directory.
bool TryCreateDirectory(const boost::filesystem::path& cPath) {
	try {
		return boost::filesystem::create_directory(cPath);
	} catch (boost::filesystem::filesystem_error) {
		if (!boost::filesystem::exists(cPath) || !boost::filesystem::is_directory(cPath))
			throw;
	}

	// create_directory didn't create the directory, it had to have existed already
	return false;
}

void FileCommit(FILE *pFileout) {
	fflush(pFileout); // harmless if redundantly called
#ifdef WIN32
	HANDLE hFile = (HANDLE) _get_osfhandle(_fileno(pFileout));
	FlushFileBuffers(hFile);
#else
#if defined(__linux__) || defined(__NetBSD__)
	fdatasync(fileno(pFileout));
#elif defined(__APPLE__) && defined(F_FULLFSYNC)
	fcntl(fileno(fileout), F_FULLFSYNC, 0);
#else
	fsync(fileno(fileout));
#endif
#endif
}

bool TruncateFile(FILE *pFile, unsigned int unLength) {
#if defined(WIN32)
	return _chsize(_fileno(pFile), unLength) == 0;
#else
	return ftruncate(fileno(pFile), unLength) == 0;
#endif
}

// this function tries to raise the file descriptor limit to the requested number.
// It returns the actual file descriptor limit (which may be more or less than nMinFD)
int RaiseFileDescriptorLimit(int nMinFD) {
#if defined(WIN32)
	return 2048;
#else
	struct rlimit limitFD;
	if (getrlimit(RLIMIT_NOFILE, &limitFD) != -1) {
		if (limitFD.rlim_cur < (rlim_t)nMinFD) {
			limitFD.rlim_cur = nMinFD;
			if (limitFD.rlim_cur > limitFD.rlim_max)
			limitFD.rlim_cur = limitFD.rlim_max;
			setrlimit(RLIMIT_NOFILE, &limitFD);
			getrlimit(RLIMIT_NOFILE, &limitFD);
		}
		return limitFD.rlim_cur;
	}
	return nMinFD; // getrlimit failed, assume it's fine
#endif
}

// this function tries to make a particular range of a file allocated (corresponding to disk space)
// it is advisory, and the range specified in the arguments will never contain live data
void AllocateFileRange(FILE *pFile, unsigned int unOffset, unsigned int unLength) {
#if defined(WIN32)
	// Windows-specific version
	HANDLE hFile = (HANDLE) _get_osfhandle(_fileno(pFile));
	LARGE_INTEGER nFileSize;
	int64_t nEndPos = (int64_t) unOffset + unLength;
	nFileSize.u.LowPart = nEndPos & 0xFFFFFFFF;
	nFileSize.u.HighPart = nEndPos >> 32;
	SetFilePointerEx(hFile, nFileSize, 0, FILE_BEGIN);
	SetEndOfFile(hFile);
#elif defined(MAC_OSX)
	// OSX specific version
	fstore_t fst;
	fst.fst_flags = F_ALLOCATECONTIG;
	fst.fst_posmode = F_PEOFPOSMODE;
	fst.fst_offset = 0;
	fst.fst_length = (off_t)offset + length;
	fst.fst_bytesalloc = 0;
	if (fcntl(fileno(pFile), F_PREALLOCATE, &fst) == -1) {
		fst.fst_flags = F_ALLOCATEALL;
		fcntl(fileno(pFile), F_PREALLOCATE, &fst);
	}
	ftruncate(fileno(pFile), fst.fst_length);
#elif defined(__linux__)
	// Version using posix_fallocate
	off_t nEndPos = (off_t)unOffset + unLength;
	posix_fallocate(fileno(pFile), 0, nEndPos);
#else
	// Fallback version
	// TODO: just write one byte per block
	static const char buf[65536] = {};
	fseek(file, offset, SEEK_SET);
	while (length > 0) {
		unsigned int now = 65536;
		if (length < now)
		now = length;
		fwrite(buf, 1, now, file); // allowed to fail; this function is advisory anyway
		length -= now;
	}
#endif
}

void ShrinkDebugFile() {
	// Scroll debug.log if it's getting too big
	boost::filesystem::path pathLog = GetDataDir() / "debug.log";
	FILE* file = fopen(pathLog.string().c_str(), "r");
	if (file && boost::filesystem::file_size(pathLog) > 10 * 1000000) {
		// Restart the file with some of the end
		char pch[200000];
		fseek(file, -sizeof(pch), SEEK_END);
		int nBytes = fread(pch, 1, sizeof(pch), file);
		fclose(file);

		file = fopen(pathLog.string().c_str(), "w");
		if (file) {
			fwrite(pch, 1, nBytes, file);
			fclose(file);
		}
	} else if (file != NULL)
		fclose(file);
}

//
// "Never go to sea with two chronometers; take one or three."
// Our three time sources are:
//  - System clock
//  - Median of other nodes clocks
//  - The user (asking the user to fix the system clock if the first two disagree)
//
static int64_t nMockTime = 0;  // For unit testing

int64_t GetTime() {
	if (nMockTime) {
		return nMockTime;
	}
	return time(NULL);
}

void SetMockTime(int64_t llMockTimeIn) {
	nMockTime = llMockTimeIn;
}

static CCriticalSection g_cs_nTimeOffset;
static int64_t g_sllTimeOffset = 0;

int64_t GetTimeOffset() {
	LOCK(g_cs_nTimeOffset);
	return g_sllTimeOffset;
}

int64_t GetAdjustedTime() {
	return GetTime() + GetTimeOffset();
}

void AddTimeData(const CNetAddr& cIp, int64_t llTime) {
	int64_t llOffsetSample = llTime - GetTime();

	LOCK(g_cs_nTimeOffset);
	// Ignore duplicates
	static set<CNetAddr> setKnown;
	if (!setKnown.insert(cIp).second) {
		return;
	}

	// Add data
	static CMedianFilter<int64_t> vTimeOffsets(200, 0);
	vTimeOffsets.input(llOffsetSample);
	LogPrint("INFO", "Added time data, samples %d, offset %+d (%+d minutes)\n", vTimeOffsets.size(), llOffsetSample,
			llOffsetSample / 60);
	if (vTimeOffsets.size() >= 5 && vTimeOffsets.size() % 2 == 1) {
		int64_t llMedian = vTimeOffsets.median();
		vector<int64_t> vllSorted = vTimeOffsets.sorted();
		// Only let other nodes change our time by so much
		if (abs64(llMedian) < 70 * 60) {
			g_sllTimeOffset = llMedian;
		} else {
			g_sllTimeOffset = 0;

			static bool bDone;
			if (!bDone) {
				// If nobody has a time different than ours but within 5 minutes of ours, give a warning
				bool bMatch = false;
				for (int64_t llOffset : vllSorted)
					if (llOffset != 0 && abs64(llOffset) < 5 * 60) {
						bMatch = true;
					}

				if (!bMatch) {
					bDone = true;
					string strMessage = _("Warning: Please check that your computer's date and time "
							"are correct! If your clock is wrong Dacrs will not work properly.");
					g_strMiscWarning = strMessage;
					LogPrint("INFO", "*** %s\n", strMessage);
					g_cUIInterface.ThreadSafeMessageBox(strMessage, "", CClientUIInterface::MSG_WARNING);
				}
			}
		}
		if (SysCfg().IsDebug()) {
			for (int64_t llValue : vllSorted) {
				LogPrint("INFO", "%+d  ", llValue);
			}
			LogPrint("INFO", "|  ");
		}
		LogPrint("INFO", "nTimeOffset = %+d  (%+d minutes)\n", g_sllTimeOffset, g_sllTimeOffset / 60);
	}
}

//uint32_t insecure_rand_Rz = 11;
//uint32_t insecure_rand_Rw = 11;
//void seed_insecure_rand(bool fDeterministic) {
//	//The seed values have some unlikely fixed points which we avoid.
//	if (fDeterministic) {
//		insecure_rand_Rz = insecure_rand_Rw = 11;
//	} else {
//		uint32_t tmp;
//		do {
//			RAND_bytes((unsigned char*) &tmp, 4);
//		} while (tmp == 0 || tmp == 0x9068ffffU);
//		insecure_rand_Rz = tmp;
//		do {
//			RAND_bytes((unsigned char*) &tmp, 4);
//		} while (tmp == 0 || tmp == 0x464fffffU);
//		insecure_rand_Rw = tmp;
//	}
//}

string FormatVersion(int nVersion) {
	if (nVersion % 100 == 0) {
		return strprintf("%d.%d.%d", nVersion/1000000, (nVersion/10000)%100, (nVersion/100)%100);
	} else {
		return strprintf("%d.%d.%d.%d", nVersion/1000000, (nVersion/10000)%100, (nVersion/100)%100, nVersion%100);
	}
}

string FormatFullVersion() {
	return g_strClientBuild;
}

// Format the subversion field according to BIP 14 spec (https://en.Dacrs.it/wiki/BIP_0014)
string FormatSubVersion(const string& strName, int nClientVersion, const vector<string>& vstrComments) {
	ostringstream ss;
	ss << "/";
	ss << strName << ":" << FormatVersion(nClientVersion);
	if (!vstrComments.empty()) {
		ss << ":" << "(" << boost::algorithm::join(vstrComments, "; ") << ")";
	}
	ss << "/";
	return ss.str();
}

void StringReplace(string &strBase, string strSrc, string strDes) {
	string::size_type nPos = 0;
	string::size_type nSrcLen = strSrc.size();
	string::size_type nDesLen = strDes.size();
	nPos = strBase.find(strSrc, nPos);
	while ((nPos != string::npos)) {
		strBase.replace(nPos, nSrcLen, strDes);
		nPos = strBase.find(strSrc, (nPos + nDesLen));
	}
}

#ifdef WIN32
boost::filesystem::path GetSpecialFolderPath(int nFolder, bool bCreate) {
	namespace fs = boost::filesystem;

	char pszPath[MAX_PATH] = "";

	if (SHGetSpecialFolderPathA(NULL, pszPath, nFolder, bCreate)) {
		return fs::path(pszPath);
	}

	LogPrint("INFO", "SHGetSpecialFolderPathA() failed, could not obtain requested path.\n");
	return fs::path("");
}
#endif

boost::filesystem::path GetTempPath() {
#if BOOST_FILESYSTEM_VERSION == 3
	return boost::filesystem::temp_directory_path();
#else
	// TODO: remove when we don't support filesystem v2 anymore
	boost::filesystem::path path;
#ifdef WIN32
	char pszPath[MAX_PATH] = "";

	if (GetTempPathA(MAX_PATH, pszPath))
	path = boost::filesystem::path(pszPath);
#else
	path = boost::filesystem::path("/tmp");
#endif
	if (path.empty() || !boost::filesystem::is_directory(path)) {
		LogPrint("INFO","GetTempPath(): failed to find temp path\n");
		return boost::filesystem::path("");
	}
	return path;
#endif
}

void runCommand(string strCommand) {
	int nErr = ::system(strCommand.c_str());
	if (nErr) {
		LogPrint("INFO", "runCommand error: system(%s) returned %d\n", strCommand, nErr);
	}
}

void RenameThread(const char* pszName) {
#if defined(PR_SET_NAME)
	// Only the first 15 characters are used (16 - NUL terminator)
	::prctl(PR_SET_NAME, pszName, 0, 0, 0);
#elif 0 && (defined(__FreeBSD__) || defined(__OpenBSD__))
	// TODO: This is currently disabled because it needs to be verified to work
	//       on FreeBSD or OpenBSD first. When verified the '0 &&' part can be
	//       removed.
	pthread_set_name_np(pthread_self(), pszName);

#elif defined(MAC_OSX) && defined(__MAC_OS_X_VERSION_MAX_ALLOWED)

// pthread_setname_np is XCode 10.6-and-later
#if __MAC_OS_X_VERSION_MAX_ALLOWED >= 1060
	pthread_setname_np(name);
#endif

#else
	// Prevent warnings for unused parameters...
	(void) pszName;
#endif
}

void SetupEnvironment() {
#ifndef WIN32
try
{
#if BOOST_FILESYSTEM_VERSION == 3
	boost::filesystem::path::codecvt(); // Raises runtime error if current locale is invalid
#else				          // boost filesystem v2		locale();// Raises runtime error if current locale is invalid#endif} catch(runtime_error &e)	{	setenv("LC_ALL", "C", 1);// Force C locale}#endif}	string DateTimeStrFormat(const char* pszFormat, int64_t nTime) {// locale takes ownership of the pointer	locale loc(locale::classic(), new boost::posix_time::time_facet(pszFormat));	stringstream ss;	ss.imbue(loc);	ss << boost::posix_time::from_time_t(nTime);
	return ss.str();
}
