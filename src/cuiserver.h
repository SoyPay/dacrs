#include <ctime>
#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/thread/thread.hpp>
#include <boost/shared_ptr.hpp>
#include "chainparams.h"
using namespace boost;
using namespace std;
using boost::asio::ip::tcp;
using boost::system::error_code;

class CUIServer {
 public:
	typedef boost::shared_ptr<tcp::socket> sock_pt;

 public:
	static void StartServer();
	static void Send(const string& strData);
	static bool HasConnection();
	static void StopServer();
	static void PackageData(string &strData);

 public:
	static bool m_bIsInitalEnd;

 private:
	CUIServer() :
		m_Acceptor(m_IoSev, tcp::endpoint(tcp::v4(), SysCfg().GetArg("-uiport", SysCfg().GetUIPort()))) {
		m_bConnect = false;
		m_bRunFlag = true;
	}
	static CUIServer* getInstance();
	static void RunThreadPorc(CUIServer* pThis);
	void SendData(const std::string& strData);
	void Accept();
	void Accept_handler(sock_pt sock);
	void read_handler(const system::error_code& ec, char* pstr, sock_pt sock);
	void write_handler() {/*std::cout << "send msg complete." << std::endl;*/
	}
	void RunServer();

 private:
	static const int m_sPort = 18999;
	static boost::thread_group m_sThreadGroup;
	static CUIServer* m_sInstance;
	asio::io_service m_IoSev;
	tcp::acceptor m_Acceptor;
	sock_pt m_Socket;
	bool m_bConnect;
	enum {
		EM_MAX_LENGTH = 1024
	};
	char m_chData_[EM_MAX_LENGTH];
	bool m_bRunFlag;
};
