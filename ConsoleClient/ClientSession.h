#pragma once
#include <ctime>
#include <iostream>
#include <string>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include "NetMsgDef.h"

using boost::asio::ip::tcp;
using namespace std;
#define BUFF_SIZE 1024
typedef boost::asio::ssl::stream<boost::asio::ip::tcp::socket> ssl_socket;

class CClientSession : public boost::enable_shared_from_this<CClientSession>
{
public:
	CClientSession();
	~CClientSession();
	void RunService()
	{
		m_IoService.run();
	}

	boost::asio::ip::tcp::socket& socket()
	{
		return m_Socket;
	}
	void Start(const wstring& user, const string& ip, const string& port);
	void Stop();
	void OnConnect(const boost::system::error_code& error);
	void OnSendOk(const boost::system::error_code& error, size_t bytes_transferred);
	void OnRecvOk(const boost::system::error_code& error, size_t bytes_transferred);
	void HandleShandshake(const boost::system::error_code& error);
	void HandleMsg(char* pData);
	void ReqLogIn(char* pData);
	void SetMsgToBuf(MSGID id, std::wstring wszcontent);
	void TalkWithServer(char* pData);
private:
	boost::asio::io_service m_IoService;
	boost::asio::ip::tcp::socket m_Socket;

	std::string m_MSG;
	bool m_bWaitForMsgHead;
	char m_RecvBuf[BUFF_SIZE];
	char m_SendBuf[BUFF_SIZE];
	char* m_pData;
	char* m_pDataBegin;
    size_t m_WaitLen;

	wstring m_wszUserName;
	string m_szPort;
	string m_szIp;
};
