#include "cuiserver.h"
#include "json/json_spirit_utils.h"
#include "json/json_spirit_value.h"
#include "json/json_spirit_writer_template.h"
using namespace json_spirit;

boost::thread_group CUIServer::m_sThreadGroup;
bool CUIServer::m_bIsInitalEnd = false;
CUIServer* CUIServer::m_sInstance = NULL;

bool CUIServer::HasConnection() {
	if (NULL == m_sInstance) {
		return false;
	}

	return m_sInstance->m_bConnect;
}

void CUIServer::StartServer() {
	m_sThreadGroup.create_thread(boost::bind(&CUIServer::RunThreadPorc, CUIServer::getInstance()));
}

void CUIServer::StopServer() {
	if (NULL == m_sInstance) {
		return;
	}
	m_sInstance->m_IoSev.stop();
	m_sInstance->m_bRunFlag = false;
	m_sThreadGroup.interrupt_all();
	m_sThreadGroup.join_all();
}

void CUIServer::RunThreadPorc(CUIServer* pThis) {
	if (NULL == pThis) {
		return;
	}
	pThis->RunServer();
}

CUIServer* CUIServer::getInstance() {
	if (NULL == m_sInstance) {
		m_sInstance = new CUIServer();
	}

	return m_sInstance;
}

void CUIServer::Send(const string& strData) {
	if(NULL == m_sInstance) {
		return ;
	}
//	LogPrint("TOUI","send message: %s\n", strData);
	string strSendData(strData);
	PackageData(strSendData);
	LogPrint("TOUI","send message: %s\n", strSendData);
	m_sInstance->SendData(strSendData);
}

void CUIServer::SendData(const string& strData) {
	system::error_code ignored_error;
	if(m_bConnect&&m_Socket.get() ){
		m_Socket->write_some(asio::buffer(strData), ignored_error);
	}
}

void CUIServer::Accept() {
	sock_pt sock(new tcp::socket(m_IoSev));
	m_Acceptor.async_accept(*sock, bind(&CUIServer::Accept_handler, this, sock));
}

void CUIServer::Accept_handler(sock_pt sock) {
	if (m_bConnect) {
		//only save one connection
		Accept();
		return;
	}
	m_Socket = sock;
	m_bConnect = true;
	Object obj;
	if (CUIServer::m_bIsInitalEnd == true) {
		obj.push_back(Pair("type", "init"));
		obj.push_back(Pair("msg", "initialize end"));
	} else {
		obj.push_back(Pair("type", "hello"));
		obj.push_back(Pair("msg", "hello asio"));
	}

	Accept();
	string strSendData = write_string(Value(std::move(obj)), true);
	PackageData(strSendData);
	sock->async_write_some(asio::buffer(strSendData), bind(&CUIServer::write_handler, this));
	std::shared_ptr<vector<char> > str(new vector<char>(100, 0));
	memset(m_chData_, 0, EM_MAX_LENGTH);
	sock->async_read_some(asio::buffer(m_chData_, EM_MAX_LENGTH),
			bind(&CUIServer::read_handler, this, asio::placeholders::error, m_chData_, sock));
}

void CUIServer::read_handler(const system::error_code& ec, char* pstr, sock_pt sock) {
	if (ec) {
		sock->shutdown(boost::asio::ip::tcp::socket::shutdown_both, const_cast<system::error_code&>(ec));
		sock->close(const_cast<system::error_code&>(ec));
		m_bConnect = false;
		return;
	}
}

void CUIServer::RunServer(){
	Accept();
	m_IoSev.run();
	if(m_sInstance != NULL && !m_sInstance->m_bRunFlag) {
		delete m_sInstance;
		m_sInstance = NULL;
	}
}

void CUIServer::PackageData(string &strData) {
	unsigned char arruchDataTemp[65536] = {0};
	unsigned short usDataLen = strData.length();
	if(0 == usDataLen) {
		return;
	}
	arruchDataTemp[0] = '<';
	memcpy(arruchDataTemp+1, &usDataLen, 2);
	memcpy(arruchDataTemp+3, strData.c_str(), usDataLen);
	arruchDataTemp[usDataLen+3] = '>';
	LogPrint("TOUI","send message length: %d\n", usDataLen);
	strData.assign(arruchDataTemp, arruchDataTemp+usDataLen+4);
//	cout << "Send Data len " << nDataLen + 4 << ":";
//	for(int i=0; i< nDataLen+4; ++i)
//		printf("%02X", cDataTemp[i]);
//	cout << endl;
//	strData = strprintf("<%s%s>", cLen, strData.c_str());
}
