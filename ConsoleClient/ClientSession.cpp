#include "ClientSession.h"
#include <boost/lexical_cast.hpp>

using namespace std;


CClientSession::CClientSession():m_Socket(m_IoService)
{
//	m_sslContext.set_verify_mode(boost::asio::ssl::context::verify_peer);
 //   m_sslContext.load_verify_file("ca.pem");
	memset(m_RecvBuf, 0, sizeof(m_RecvBuf));
	memset(m_SendBuf, 0, sizeof(m_SendBuf));
	m_bWaitForMsgHead = true;
}

CClientSession::~CClientSession()
{

}
std::string make_daytime_string()
{
  using namespace std; // For time_t, time and ctime;
  time_t now = time(0);
  return ctime(&now);
}

//inline void CClientSession::RunService()
//{
//	m_IoService.run();
//}


//8000端口
void CClientSession::Start(const wstring& user, const string& ip, const string& port)
{

	m_wszUserName = user;
	m_szPort = port;
	m_szIp = ip;

	tcp::endpoint endpoint(boost::asio::ip::address_v4::from_string(ip), boost::lexical_cast<int>(port));
	m_Socket.lowest_layer().async_connect(endpoint,
    boost::bind(&CClientSession::OnConnect, this,
        boost::asio::placeholders::error));


}

void CClientSession::Stop()
{
	socket().close();
	m_IoService.stop();
}

void CClientSession::OnSendOk(const boost::system::error_code& error,
    size_t /*bytes_transferred*/)
{
	if(error) //发送失败,重传
	{
		cout<<"发送失败"<<endl;
	}
	else
	{
		cout<<"发送成功"<<endl;
		m_Socket.async_read_some(boost::asio::buffer(m_RecvBuf, BUFF_SIZE),
			boost::bind(&CClientSession::OnRecvOk, this,
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred));

	}

}
void CClientSession::OnRecvOk(const boost::system::error_code& error,
    size_t bytes_transferred)
{
	if(error) //发送失败,重传
	{
		cout<<"接收失败"<<endl;
	}
	else
	{
		cout<<"接收成功"<<endl;
		if(m_bWaitForMsgHead)//等消息头
		{
			if(bytes_transferred >= sizeof(NetMsgHead))//消息头接收成功
			{	
				NetMsgHead* pHead = (NetMsgHead*)m_RecvBuf;
				if(bytes_transferred >= sizeof(NetMsgHead) + pHead->DataLen)//所有数据都收到
				{
					m_bWaitForMsgHead = true;//继续等下一个包头
					m_pDataBegin = m_RecvBuf;
					HandleMsg(m_RecvBuf);
					memset(m_RecvBuf, 0, sizeof(m_RecvBuf));
					return ;
				}
				else
				{
					//包体部分未全部到达
					int TotalLen = pHead->DataLen + sizeof(NetMsgHead);
					m_pData = new char[TotalLen];
					memset(m_pData, 0, TotalLen);
					memcpy(m_pData, m_RecvBuf, bytes_transferred);
					m_pDataBegin = m_pData + bytes_transferred;
					m_WaitLen = pHead->DataLen - (bytes_transferred - sizeof(NetMsgHead));
					m_bWaitForMsgHead = false;

				}
			}
			else//消息头接收失败
			{

			}
		}
		else//等包体
		{
			memcpy(m_pDataBegin, m_RecvBuf, min(bytes_transferred, m_WaitLen));
			//数据还未全部到达
			if (bytes_transferred < m_WaitLen)
			{
				m_pDataBegin += bytes_transferred;
				m_WaitLen -= bytes_transferred;
			}
			else
			{
				//所有的数据都已收到
				//S3_LOG_DEBUG(m_logger,L"All data is received, [Session:" << m_SessionID << "]");
				HandleMsg(m_pData);
				delete[] m_pData;
				memset(m_RecvBuf, 0, sizeof(m_RecvBuf));
				return ;
				m_pData = NULL;
				m_bWaitForMsgHead = true;//继续等下一个数据包
				m_pDataBegin = NULL;
				m_WaitLen =0;

			}
		}
		
		m_Socket.async_read_some(boost::asio::buffer(m_RecvBuf,BUFF_SIZE),
		boost::bind(&CClientSession::OnRecvOk, shared_from_this(), boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));

	}

}

void CClientSession::OnConnect(const boost::system::error_code& error)
{
	if(error)
	{
		cout<<"连接失败"<<endl;//重连
	}
	else
	{
		cout<<"连接成功"<<endl;

	//	m_Socket.async_handshake(boost::asio::ssl::stream_base::client,
		//	boost::bind(&CClientSession::HandleShandshake, this,
	//		boost::asio::placeholders::error));
		wstring wszContent;
		cout<<"请输入您的账号:";
		wcin>>wszContent;
		SetMsgToBuf(MSG_C_REQ_LOGIN, wszContent);
		cout<<"向服务器发消息: ";
		wcout<<wszContent<<endl;
		boost::asio::async_write(m_Socket, boost::asio::buffer(m_SendBuf, BUFF_SIZE),
			boost::bind(&CClientSession::OnSendOk, this,
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred));



	}
}

void CClientSession::HandleShandshake(const boost::system::error_code& error)
{

	if(error)
	{
		cout<<"握手失败"<<endl;
	}
	else
	{
		cout<<"握手成功"<<endl;

		m_Socket.async_read_some(boost::asio::buffer(m_RecvBuf, BUFF_SIZE),
			boost::bind(&CClientSession::OnRecvOk, this,
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred));

	}


}
void CClientSession::ReqLogIn(char* pData)
{

	wstring wszContent = (TCHAR*)(sizeof(NetMsgHead) + pData);
	cout<<"接收消息内容：";
	wcout<<wszContent<<endl;

	cout<<"请输入您的账号:";
	wcin>>wszContent;
	SetMsgToBuf(MSG_C_REQ_LOGIN, wszContent);
	cout<<"向服务器发消息: ";
	wcout<<wszContent<<endl;
	boost::asio::async_write(m_Socket, boost::asio::buffer(m_SendBuf, BUFF_SIZE),
		boost::bind(&CClientSession::OnSendOk, this,
		boost::asio::placeholders::error,
		boost::asio::placeholders::bytes_transferred));


}

void CClientSession::TalkWithServer(char* pData)
{
	wstring wszContent = (TCHAR*)(sizeof(NetMsgHead) + pData);
	cout<<"接收消息内容：";
	wcout<<wszContent<<endl;

	cout<<"你想说什么:";
	wcin>>wszContent;
	SetMsgToBuf(MSG_C_TALK, wszContent);
	boost::asio::async_write(m_Socket, boost::asio::buffer(m_SendBuf, BUFF_SIZE),
		boost::bind(&CClientSession::OnSendOk, this,
		boost::asio::placeholders::error,
		boost::asio::placeholders::bytes_transferred));
}

void CClientSession::HandleMsg(char* pData)
{
	NetMsgHead* pHead = (NetMsgHead*)pData;
	switch(pHead->MessageID)
	{
	case MSG_S_REQ_USER_LOGIN:
		{
			ReqLogIn(pData);
			break;
		}
	case MSG_S_WAIT_USER_TALK:
		{
			TalkWithServer(pData);
			break;
		}
	}
}

void CClientSession::SetMsgToBuf(MSGID id, wstring wszcontent)
{
	memset(m_SendBuf, 0, sizeof(m_SendBuf));
	NetMsgHead msghead;
	msghead.Version = NET_VERSION;
	msghead.MessageID = id;
	msghead.DataLen = wszcontent.length()*sizeof(TCHAR);
	memcpy(m_SendBuf, &msghead, sizeof(msghead));
	memcpy(m_SendBuf + sizeof(msghead), wszcontent.c_str(), wszcontent.length()*sizeof(TCHAR));

}