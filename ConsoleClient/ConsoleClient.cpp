#include <iostream>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/thread/thread.hpp>
#include "ClientSession.h"

using boost::asio::ip::tcp;
using namespace std;

CClientSession* g_pClient = NULL;
boost::thread*  g_pThread = NULL;

wstring g_wszUserName = L"gary";
string g_szIp = "127.0.0.1";
string g_szPort = "8000";

// wstring->string
string ws2s(const wstring& ws)
{
	string curLocal = setlocale(LC_ALL, NULL);
	setlocale(LC_ALL, "chs");
	const wchar_t* source = ws.c_str();
	size_t size = ws.size() * sizeof(wchar_t) + 1;
	char* dest = new char[size];
	memset(dest, 0, size);
	wcstombs(dest, source, size);
	string result = dest;
	delete[] dest;
	setlocale(LC_ALL, curLocal.c_str());
	return result;
}

bool ConnectSever(CClientSession* pClientSession, wstring& user, string& ip, string& port, boost::thread** ppThread)
{
	if(pClientSession == NULL)
	{
		return false;
	}
	pClientSession->Start(user, ip, port);
	//为什么要用新增线程，直接用主线程不行吗
	if(*ppThread)
	{
		delete *ppThread;
		*ppThread = NULL;
	}
	*ppThread = new boost::thread(boost::bind(&CClientSession::RunService, pClientSession));
	if(ppThread == NULL)
	{
		cout<<"创建线程失败"<<endl;
		return false;
	}
	return true;
	
}

void disconnect()
{
	if (g_pThread)
	{
		g_pClient->Stop();				
	}
}

int main()
{

	g_pClient = new CClientSession;
	if(!ConnectSever(g_pClient, g_wszUserName, g_szIp, g_szPort, &g_pThread))
	{
		cout<<"连接服务器失败"<<endl;
	}
	g_pThread->join();
	wstring wszMsg;
	int a;
	while(wcin >> wszMsg)
	{

	}
	delete g_pThread;
	g_pThread = NULL;
	delete g_pClient;
	g_pClient = NULL;
	system("pause");
	
}