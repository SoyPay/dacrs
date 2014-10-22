#include <ctime>
#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/thread/thread.hpp>
#include <boost/shared_ptr.hpp>

using namespace boost;
using namespace std;
using boost::asio::ip::tcp;
using boost::system::error_code;

class CUIServer {
public:
	typedef boost::shared_ptr<tcp::socket> sock_pt;

public:
	static void StartServer(boost::thread_group & group);
	static void Send(const string& strData);
	static bool HasConnection();
	static void StopServer();

private:
	CUIServer() :m_acceptor(m_iosev, tcp::endpoint(tcp::v4(), PORT)) {
		m_bConnect = false;
	}
	static CUIServer* getInstance();
	static void RunThreadPorc(CUIServer* pThis);
	void SendData(const std::string& strData);
	void Accept();
	void Accept_handler(sock_pt sock);
	void read_handler(const system::error_code& ec, char* pstr, sock_pt sock);
	void write_handler() {/*std::cout << "send msg complete." << std::endl;*/}
	void RunServer();

private:
	static const int PORT=3200;
	static CUIServer* instance;
	asio::io_service m_iosev;
	tcp::acceptor m_acceptor;
	sock_pt m_socket;
	bool m_bConnect;
	enum { max_length = 1024 };
	char data_[max_length];
};
