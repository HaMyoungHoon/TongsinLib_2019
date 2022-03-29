#include "FSocket_S.h"
#include <stdio.h>
#include <WinSock2.h>
#include <Windows.h>
#include <process.h>
#pragma comment(lib, "ws2_32")

void* _serverThreadHandle[2];
bool _serverThreadAliveFlag[2];
unsigned int _serverThreadID[2];
unsigned long _serverInterval[2];

bool _isServerOpen;
bool _isServerConnect;
unsigned short _serverPort;
char _serverIp[256];
char _serverBuf[BUFFER_SIZE];
SOCKET _server;
SOCKET _child;
WSADATA _serverWsaData;

FSocket_S::FSocket_S()
{
	_lib = Fmhha();
	sprintf_s(_serverIp, 256, "127.0.0.1");
	_serverPort = 9999;
	_serverInterval[0] = 10;
	_serverInterval[1] = 10;
}
FSocket_S::~FSocket_S()
{

}

static unsigned int __stdcall AcceptThread(void* param)
{
	FSocket_S* pthis = static_cast<FSocket_S*>(param);
	while (_isServerOpen)
	{
		Sleep(_serverInterval[0]);
		sockaddr_in addr_c = {};
		int clientSize = sizeof(addr_c);
		_child = accept(_server, (sockaddr*)&addr_c, &clientSize);
		if (_child == INVALID_SOCKET)
		{
			continue;
		}

		_isServerConnect = true;
		pthis->OnConnect();

		while (_isServerConnect)
		{
			Sleep(_serverInterval[0]);
			if (recv(_child, _serverBuf, BUFFER_SIZE, 0) == SOCKET_ERROR)
			{
				_isServerConnect = false;
				pthis->OnDisConnect();
				continue;
			}

			pthis->RecvMessage(_serverBuf);
		}
	}
	_serverThreadAliveFlag[0] = false;
	CloseHandle(_serverThreadHandle[0]);
	return 0;
}

void FSocket_S::SetConfig(char* ip, int port)
{
	if (_lib.LibraryPermit() == false)
	{
		return;
	}

	sprintf_s(_serverIp, 256, "%s", ip);
	_serverPort = port;
}
void FSocket_S::SetConfig(char* ip, char* port)
{
	if (_lib.LibraryPermit() == false)
	{
		return;
	}

	sprintf_s(_serverIp, 256, "%s", ip);
	_serverPort = atoi(port);
}
void FSocket_S::SetConfigW(wchar_t* ip, int port)
{
	if (_lib.LibraryPermit() == false)
	{
		return;
	}

	int nTemp = WideCharToMultiByte(CP_ACP, 0, ip, -1, NULL, 0, NULL, NULL);
	char* pszTemp = new char[nTemp];
	WideCharToMultiByte(CP_ACP, 0, ip, -1, pszTemp, nTemp, NULL, NULL);
	sprintf_s(_serverIp, 256, "%s", pszTemp);
	_serverPort = port;
	delete[] pszTemp;
}
void FSocket_S::SetConfigW(wchar_t* ip, wchar_t* port)
{
	if (_lib.LibraryPermit() == false)
	{
		return;
	}

	int nTemp = WideCharToMultiByte(CP_ACP, 0, ip, -1, NULL, 0, NULL, NULL);
	char* pszTemp = new char[nTemp];
	WideCharToMultiByte(CP_ACP, 0, ip, -1, pszTemp, nTemp, NULL, NULL);
	sprintf_s(_serverIp, 256, "%s", pszTemp);
	delete[] pszTemp;

	nTemp = WideCharToMultiByte(CP_ACP, 0, port, -1, NULL, 0, NULL, NULL);
	pszTemp = new char[nTemp];
	WideCharToMultiByte(CP_ACP, 0, port, -1, pszTemp, nTemp, NULL, NULL);
	_serverPort = atoi(pszTemp);
	delete[] pszTemp;
}
bool FSocket_S::IsServerOpen()
{
	if (_lib.LibraryPermit() == false)
	{
		return false;
	}

	return _isServerOpen;
}
bool FSocket_S::IsServerConnected()
{
	if (_lib.LibraryPermit() == false)
	{
		return false;
	}

	return _isServerConnect;
}

void FSocket_S::OpenServer()
{
	if (_lib.LibraryPermit() == false)
	{
		return;
	}
	if (_isServerOpen == true || _isServerConnect == true)
	{
		return;
	}
	if (_serverThreadAliveFlag[0] == true)
	{
		return;
	}

	if (WSAStartup(MAKEWORD(2, 2), &_serverWsaData) != 0)
	{
		return;
	}

	_server = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	sockaddr_in addr_s = {};
	addr_s.sin_family = AF_INET;
	addr_s.sin_port = htons(_serverPort);
	addr_s.sin_addr.s_addr = inet_addr(_serverIp);

	if (bind(_server, (sockaddr*)&addr_s, sizeof(addr_s)) == SOCKET_ERROR)
	{
		NotifySocketError("bind Fail");
		WSACleanup();
		return;
	}
	if (listen(_server, SOMAXCONN) == SOCKET_ERROR)
	{
		NotifySocketError("Listen Fail");
		WSACleanup();
		return;
	}

	_isServerOpen = true;
	
	_serverThreadAliveFlag[0] = true;
	_serverThreadHandle[0] = (void*)_beginthreadex(NULL, 0, AcceptThread, this, 0, &(_serverThreadID[0]));
}
void FSocket_S::CloseServer()
{
	if (_lib.LibraryPermit() == false)
	{
		return;
	}

	_isServerOpen = false;
	_isServerConnect = false;
	closesocket(_server);
	closesocket(_child);
	WSACleanup();
}
int FSocket_S::SendCommand(char* data)
{
	if (_lib.LibraryPermit() == false)
	{
		return -1;
	}

	return send(_child, data, strlen(data), 0);
}