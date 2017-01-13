// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The DACRS developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef DACRS_UTIL_H_
#define DACRS_UTIL_H_

#if defined(HAVE_CONFIG_H)
#include "dacrs-config.h"
#endif

#include "./compat/compat.h"
#include "serialize.h"
#include "tinyformat.h"

#include <cstdio>
#include <exception>
#include <map>
#include <stdarg.h>
#include <stdint.h>
#include <string>
#include <utility>
#include <vector>
#include "ui_interface.h"

#ifndef WIN32
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/types.h>
#endif

#include <boost/filesystem/path.hpp>
#include <boost/thread.hpp>

class CNetAddr;
class uint256;

static const int64_t COIN = 100000000;
static const int64_t CENT = 1000000;

#define BEGIN(a)            ((char*)&(a))
#define END(a)              ((char*)&((&(a))[1]))
#define UBEGIN(a)           ((unsigned char*)&(a))
#define UEND(a)             ((unsigned char*)&((&(a))[1]))
#define ARRAYLEN(array)     (sizeof(array)/sizeof((array)[0]))

// This is needed because the foreach macro can't get over the comma in pair<t1, t2>
#define PAIRTYPE(t1, t2)    pair<t1, t2>

// Align by increasing pointer, must have extra space at end of buffer
template<size_t nBytes, typename T>
T* alignup(T* p) {
	union {
		T* ptr;
		size_t n;
	} u;
	u.ptr = p;
	u.n = (u.n + (nBytes - 1)) & ~(nBytes - 1);
	return u.ptr;
}

#ifdef WIN32
#define MSG_DONTWAIT        0

#ifndef S_IRUSR
#define S_IRUSR             0400
#define S_IWUSR             0200
#endif
#else
#define MAX_PATH            1024
#endif
// As Solaris does not have the MSG_NOSIGNAL flag for send(2) syscall, it is defined as 0
#if !defined(HAVE_MSG_NOSIGNAL) && !defined(MSG_NOSIGNAL)
#define MSG_NOSIGNAL 0
#endif

inline void MilliSleep(int64_t n) {
// Boost's sleep_for was uninterruptable when backed by nanosleep from 1.50
// until fixed in 1.52. Use the deprecated sleep method for the broken case.
// See: https://svn.boost.org/trac/boost/ticket/7238
#if defined(HAVE_WORKING_BOOST_SLEEP_FOR)
	boost::this_thread::sleep_for(boost::chrono::milliseconds(n));
#elif defined(HAVE_WORKING_BOOST_SLEEP)
	boost::this_thread::sleep(boost::posix_time::milliseconds(n));
#else
//should never get here
#error missing boost sleep implementation
#endif
}

extern string g_strMiscWarning;
extern bool g_bNoListen;
extern volatile bool g_bReopenDebugLog;
extern CClientUIInterface * g_pUIInterface;

void RandAddSeed();
void RandAddSeedPerfmon();
void SetupEnvironment();

//bool GetBoolArg(const string& strArg, bool fDefault);

/* Return true if log accepts specified category */
bool LogAcceptCategory(const char* pszCategory);
/* Send a string to the log output */
int LogPrintStr(const string &strLog);
extern string GetLogHead(int nLine, const char* pszFile, const char* pszCategory);
int LogPrintStr(const char* pszCategory, const string &strLog);

#define strprintf tfm::format

#define ERRORMSG(...)  error2(__LINE__, __FILE__, __VA_ARGS__)
#define LogPrint(tag, ...) LogTrace(tag, __LINE__, __FILE__, __VA_ARGS__)

#define MAKE_ERROR_AND_TRACE_FUNC(n)                                        \
    /*   Print to debug.log if -debug=category switch is given OR category is NULL. */ \
    template<TINYFORMAT_ARGTYPES(n)>                                          \
    static inline int LogTrace(const char* category, int line, const char* file, const char* format, TINYFORMAT_VARARGS(n))  \
    {                                                                         \
        return LogPrintStr(category, GetLogHead(line,file,category) + tfm::format(format, TINYFORMAT_PASSARGS(n))); \
    }                                                                         \
    /*   Log error and return false */                                        \
    template<TINYFORMAT_ARGTYPES(n)>                                          \
    static inline bool error2(int line, const char* file,const char* format1, TINYFORMAT_VARARGS(n))     \
    {                                                                         \
    	LogPrintStr("ERROR", GetLogHead(line,file,"ERROR") + tfm::format(format1, TINYFORMAT_PASSARGS(n)) + "\n"); \
    	return false;                                                         \
    }


TINYFORMAT_FOREACH_ARGNUM(MAKE_ERROR_AND_TRACE_FUNC)

static inline bool error2(int nLine, const char* pszFile,const char* pszFormat) {
	LogPrintStr("ERROR",  GetLogHead(nLine, pszFile,"ERROR") + pszFormat + "\n");
	return false;
}

extern string GetLogHead(int nLine, const char* pszFile, const char* pszCategory);

static inline int LogTrace(const char* pszCategory, int nLine, const char* pszFile, const char* pszFormat) {
	return LogPrintStr(pszCategory,  GetLogHead(nLine, pszFile, pszCategory) + pszFormat);
}

void LogException(std::exception* pExcep, const char* pszThread);
void PrintExceptionContinue(std::exception* pExcep, const char* pszThread);
string FormatMoney(int64_t llFormat, bool bPlus = false);
bool ParseMoney(const string& strContent, int64_t& llRet);
bool ParseMoney(const char* pszIn, int64_t& llRet);
string SanitizeString(const string& strContent);
vector<unsigned char> ParseHex(const char* pszContent);
vector<unsigned char> ParseHex(const string& strContent);
bool IsHex(const string& strContent);
vector<unsigned char> DecodeBase64(const char* pszContent, bool* pbInvalid = NULL);
string DecodeBase64(const string& strContent);
string EncodeBase64(const unsigned char* pszContent, size_t unLength);
string EncodeBase64(const string& strContent);
vector<unsigned char> DecodeBase32(const char* pszContent, bool* pbInvalid = NULL);
string DecodeBase32(const string& strContent);
string EncodeBase32(const unsigned char* pszContent, size_t unlength);
string EncodeBase32(const string& strContent);
bool WildcardMatch(const char* pszContent, const char* pszMask);
bool WildcardMatch(const string& strContent, const string& strMask);
void FileCommit(FILE *pFileout);
bool TruncateFile(FILE *pFile, unsigned int unLength);
int RaiseFileDescriptorLimit(int nMinFD);
void AllocateFileRange(FILE *pFile, unsigned int unOffset, unsigned int unLength);
bool RenameOver(boost::filesystem::path cSrc, boost::filesystem::path cDest);
bool TryCreateDirectory(const boost::filesystem::path& cPath);
boost::filesystem::path GetDefaultDataDir();
const boost::filesystem::path &GetDataDir(bool bNetSpecific = true);
boost::filesystem::path GetConfigFile();
boost::filesystem::path GetPidFile();

#ifndef WIN32
void CreatePidFile(const boost::filesystem::path &path, pid_t pid);
#endif

void ReadConfigFile(map<string, string>& mapSettingsRet, map<string, vector<string> >& mapMultiSettingsRet);

#ifdef WIN32
boost::filesystem::path GetSpecialFolderPath(int nFolder, bool bCreate = true);
#endif

boost::filesystem::path GetTempPath();
void ShrinkDebugFile();
int GetRandInt(int nMax);
uint64_t GetRand(uint64_t ullMax);
uint256 GetRandHash();
int64_t GetTime();
void SetMockTime(int64_t llMockTimeIn);
int64_t GetAdjustedTime();
int64_t GetTimeOffset();
string FormatFullVersion();
string FormatSubVersion(const string& strName, int nClientVersion, const vector<string>& vstrComments);
void StringReplace(string &strBase, string strSrc, string strDes);
void AddTimeData(const CNetAddr& cIp, int64_t llTime);
void runCommand(string strCommand);

inline string i64tostr(int64_t llNum) {
	return strprintf("%d", llNum);
}

inline string itostr(int nNum) {
	return strprintf("%d", nNum);
}

inline int64_t atoi64(const char* pszString) {
#ifdef _MSC_VER
	return _atoi64(pszString);
#else
	return strtoll(pszString, NULL, 10);
#endif
}

inline int64_t atoi64(const string& strString) {
#ifdef _MSC_VER
	return _atoi64(strString.c_str());
#else
	return strtoll(strString.c_str(), NULL, 10);
#endif
}

inline int atoi(const string& strString) {
	return atoi(strString.c_str());
}

inline int roundint(double dNum) {
	return (int) (dNum > 0 ? dNum + 0.5 : dNum - 0.5);
}

inline int64_t roundint64(double dNum) {
	return (int64_t) (dNum > 0 ? dNum + 0.5 : dNum - 0.5);
}

inline int64_t abs64(int64_t llNum) {
	return (llNum >= 0 ? llNum : -llNum);
}

template<typename T>
string HexStr(const T itbegin, const T itend, bool bSpaces = false) {
	string rv;
	static const char hexmap[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };
	rv.reserve((itend - itbegin) * 3);
	for (T it = itbegin; it < itend; ++it) {
		unsigned char val = (unsigned char) (*it);
		if (bSpaces && it != itbegin) {
			rv.push_back(' ');
		}
		rv.push_back(hexmap[val >> 4]);
		rv.push_back(hexmap[val & 15]);
	}

	return rv;
}

template<typename T>
inline string HexStr(const T& vch, bool bSpaces = false) {
	return HexStr(vch.begin(), vch.end(), bSpaces);
}

template<typename T>
void PrintHex(const T pbegin, const T pend, const char* pszFormat = "%s", bool bSpaces = true) {
	LogPrint("INFO",pszFormat, HexStr(pbegin, pend, bSpaces).c_str());
}

inline void PrintHex(const vector<unsigned char>& vchContent, const char* pszFormat = "%s", bool bSpaces = true) {
	LogPrint("INFO",pszFormat, HexStr(vchContent, bSpaces).c_str());
}

inline int64_t GetTimeMillis() {
	return (boost::posix_time::ptime(boost::posix_time::microsec_clock::universal_time())
			- boost::posix_time::ptime(boost::gregorian::date(1970, 1, 1))).total_milliseconds();
}

inline int64_t GetTimeMicros() {
	return (boost::posix_time::ptime(boost::posix_time::microsec_clock::universal_time())
			- boost::posix_time::ptime(boost::gregorian::date(1970, 1, 1))).total_microseconds();
}

string DateTimeStrFormat(const char* pszFormat, int64_t llTime);

template<typename T>
void skipspaces(T& it) {
	while (isspace(*it)) {
		++it;
	}
}

inline bool IsSwitchChar(char c) {
#ifdef WIN32
	return c == '-' || c == '/';
#else
	return c == '-';
#endif
}

/**
 * MWC RNG of George Marsaglia
 * This is intended to be fast. It has a period of 2^59.3, though the
 * least significant 16 bits only have a period of about 2^30.1.
 *
 * @return random value
 */
//extern uint32_t insecure_rand_Rz;
//extern uint32_t insecure_rand_Rw;
//static inline uint32_t insecure_rand(void) {
//	insecure_rand_Rz = 36969 * (insecure_rand_Rz & 65535) + (insecure_rand_Rz >> 16);
//	insecure_rand_Rw = 18000 * (insecure_rand_Rw & 65535) + (insecure_rand_Rw >> 16);
//	return (insecure_rand_Rw << 16) + insecure_rand_Rz;
//}

/**
 * Seed insecure_rand using the random pool.
 * @param Deterministic Use a determinstic seed
 */
//void seed_insecure_rand(bool fDeterministic = false);

/**
 * Timing-attack-resistant comparison.
 * Takes time proportional to length
 * of first argument.
 */
template<typename T>
bool TimingResistantEqual(const T& a, const T& b) {
	if (b.size() == 0) {
		return a.size() == 0;
	}
	size_t unAccumulator = a.size() ^ b.size();
	for (size_t i = 0; i < a.size(); i++) {
		unAccumulator |= a[i] ^ b[i % b.size()];
	}
	return unAccumulator == 0;
}

/** Median filter over a stream of values.
 * Returns the median of the last N numbers
 */
template<typename T> class CMedianFilter {
 public:
	CMedianFilter(unsigned int size, T initial_value) :
			m_unSize(size) {
		m_vValues.reserve(size);
		m_vValues.push_back(initial_value);
		m_vSorted = m_vValues;
	}

	void input(T value) {
		if (m_vValues.size() == m_unSize) {
			m_vValues.erase(m_vValues.begin());
		}
		m_vValues.push_back(value);
		m_vSorted.resize(m_vValues.size());
		copy(m_vValues.begin(), m_vValues.end(), m_vSorted.begin());
		sort(m_vSorted.begin(), m_vSorted.end());
	}

	T median() const {
		int nSize = m_vSorted.size();
		assert(nSize > 0);
		if (nSize & 1) {		// Odd number of elements
			return m_vSorted[nSize / 2];
		} else { 				// Even number of elements
			return (m_vSorted[nSize / 2 - 1] + m_vSorted[nSize / 2]) / 2;
		}
	}

	int size() const {
		return m_vValues.size();
	}

	vector<T> sorted() const {
		return m_vSorted;
	}

 private:
 	vector<T> m_vValues;
 	vector<T> m_vSorted;
 	unsigned int m_unSize;
};

#ifdef WIN32
inline void SetThreadPriority(int nPriority) {
	SetThreadPriority(GetCurrentThread(), nPriority);
}
#else

// PRIO_MAX is not defined on Solaris
#ifndef PRIO_MAX
#define PRIO_MAX 20
#endif
#define THREAD_PRIORITY_LOWEST          PRIO_MAX
#define THREAD_PRIORITY_BELOW_NORMAL    2
#define THREAD_PRIORITY_NORMAL          0
#define THREAD_PRIORITY_ABOVE_NORMAL    (-2)

inline void SetThreadPriority(int nPriority)
{
	// It's unclear if it's even possible to change thread priorities on Linux,
	// but we really and truly need it for the generation threads.
#ifdef PRIO_THREAD
	setpriority(PRIO_THREAD, 0, nPriority);
#else
	setpriority(PRIO_PROCESS, 0, nPriority);
#endif
}
#endif

void RenameThread(const char* pszName);

inline uint32_t ByteReverse(uint32_t uValue) {
	uValue = ((uValue & 0xFF00FF00) >> 8) | ((uValue & 0x00FF00FF) << 8);
	return (uValue << 16) | (uValue >> 16);
}

// Standard wrapper for do-something-forever thread functions.
// "Forever" really means until the thread is interrupted.
// Use it like:
//   new boost::thread(boost::bind(&LoopForever<void (*)()>, "dumpaddr", &DumpAddresses, 900000));
// or maybe:
//    boost::function<void()> f = boost::bind(&FunctionWithArg, argument);
//    threadGroup.create_thread(boost::bind(&LoopForever<boost::function<void()> >, "nothing", f, milliseconds));
template<typename Callable> void LoopForever(const char* pszName, Callable callFunc, int64_t llMsecs) {
	string strContent = strprintf("bitcoin-%s", pszName);
	RenameThread(strContent.c_str());
	LogPrint("INFO","%s thread start\n", pszName);
	try {
		while (1) {
			MilliSleep(llMsecs);
			callFunc();
		}
	} catch (boost::thread_interrupted) {
		LogPrint("INFO","%s thread stop\n", pszName);
		throw;
	} catch (std::exception& e) {
		PrintExceptionContinue(&e, pszName);
		throw;
	} catch (...) {
		PrintExceptionContinue(NULL, pszName);
		throw;
	}
}

// .. and a wrapper that just calls func once
template<typename Callable> void TraceThread(const char* strName, Callable callFunc) {
	string strContent = strprintf("bitcoin-%s", strName);
	RenameThread(strContent.c_str());
	try {
		LogPrint("INFO","%s thread start\n", strName);
		callFunc();
		LogPrint("INFO","%s thread exit\n", strName);
	} catch (boost::thread_interrupted) {
		LogPrint("INFO","%s thread interrupt\n", strName);
		throw;
	} catch (std::exception& e) {
		PrintExceptionContinue(&e, strName);
		throw;
	} catch (...) {
		PrintExceptionContinue(NULL, strName);
		throw;
	}
}

#endif
