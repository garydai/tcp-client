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


//8000�˿�
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
	if(error) //����ʧ��,�ش�
	{
		cout<<"����ʧ��"<<endl;
	}
	else
	{
		cout<<"���ͳɹ�"<<endl;
		m_Socket.async_read_some(boost::asio::buffer(m_RecvBuf, BUFF_SIZE),
			boost::bind(&CClientSession::OnRecvOk, this,
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred));

	}

}
void CClientSession::OnRecvOk(const boost::system::error_code& error,
    size_t bytes_transferred)
{
	if(error) //����ʧ��,�ش�
	{
		cout<<"����ʧ��"<<endl;
	}
	else
	{
		cout<<"���ճɹ�"<<endl;
		if(m_bWaitForMsgHead)//����Ϣͷ
		{
			if(bytes_transferred >= sizeof(NetMsgHead))//��Ϣͷ���ճɹ�
			{	
				NetMsgHead* pHead = (NetMsgHead*)m_RecvBuf;
				if(bytes_transferred >= sizeof(NetMsgHead) + pHead->DataLen)//�������ݶ��յ�
				{
					m_bWaitForMsgHead = true;//��������һ����ͷ
					m_pDataBegin = m_RecvBuf;
					HandleMsg(m_RecvBuf);
					memset(m_RecvBuf, 0, sizeof(m_RecvBuf));
					return ;
				}
				else
				{
					//���岿��δȫ������
					int TotalLen = pHead->DataLen + sizeof(NetMsgHead);
					m_pData = new char[TotalLen];
					memset(m_pData, 0, TotalLen);
					memcpy(m_pData, m_RecvBuf, bytes_transferred);
					m_pDataBegin = m_pData + bytes_transferred;
					m_WaitLen = pHead->DataLen - (bytes_transferred - sizeof(NetMsgHead));
					m_bWaitForMsgHead = false;

				}
			}
			else//��Ϣͷ����ʧ��
			{

			}
		}
		else//�Ȱ���
		{
			memcpy(m_pDataBegin, m_RecvBuf, min(bytes_transferred, m_WaitLen));
			//���ݻ�δȫ������
			if (bytes_transferred < m_WaitLen)
			{
				m_pDataBegin += bytes_transferred;
				m_WaitLen -= bytes_transferred;
			}
			else
			{
				//���е����ݶ����յ�
				//S3_LOG_DEBUG(m_logger,L"All data is received, [Session:" << m_SessionID << "]");
				HandleMsg(m_pData);
				delete[] m_pData;
				memset(m_RecvBuf, 0, sizeof(m_RecvBuf));
				return ;
				m_pData = NULL;
				m_bWaitForMsgHead = true;//��������һ�����ݰ�
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
		cout<<"����ʧ��"<<endl;//����
	}
	else
	{
		cout<<"���ӳɹ�"<<endl;

	//	m_Socket.async_handshake(boost::asio::ssl::stream_base::client,
		//	boost::bind(&CClientSession::HandleShandshake, this,
	//		boost::asio::placeholders::error));
		wstring wszContent;
		cout<<"�����������˺�:";
		wcin>>wszContent;
		SetMsgToBuf(MSG_C_REQ_LOGIN, wszContent);
		cout<<"�����������Ϣ: ";
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
		cout<<"����ʧ��"<<endl;
	}
	else
	{
		cout<<"���ֳɹ�"<<endl;

		m_Socket.async_read_some(boost::asio::buffer(m_RecvBuf, BUFF_SIZE),
			boost::bind(&CClientSession::OnRecvOk, this,
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred));

	}


}
void CClientSession::ReqLogIn(char* pData)
{

	wstring wszContent = (TCHAR*)(sizeof(NetMsgHead) + pData);
	cout<<"������Ϣ���ݣ�";
	wcout<<wszContent<<endl;

	cout<<"�����������˺�:";
	wcin>>wszContent;
	SetMsgToBuf(MSG_C_REQ_LOGIN, wszContent);
	cout<<"�����������Ϣ: ";
	wcout<<wszContent<<endl;
	boost::asio::async_write(m_Socket, boost::asio::buffer(m_SendBuf, BUFF_SIZE),
		boost::bind(&CClientSession::OnSendOk, this,
		boost::asio::placeholders::error,
		boost::asio::placeholders::bytes_transferred));


}

void CClientSession::TalkWithServer(char* pData)
{
	wstring wszContent = (TCHAR*)(sizeof(NetMsgHead) + pData);
	cout<<"������Ϣ���ݣ�";
	wcout<<wszContent<<endl;

	cout<<"����˵ʲô:";
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