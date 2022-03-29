#include "FSocket_C.h"
#include <stdio.h>
#include <WinSock2.h>
#include <Windows.h>
#include <process.h>
#pragma comment(lib, "ws2_32")

void* _clientThreadHandle[2];
bool _clientThreadAliveFlag[2];
unsigned int _clientThreadID[2];
unsigned long _clientInterval[2];

bool _isClientConnect;
unsigned short _clientPort;
char _clientIp[256];
char _clientBuf[BUFFER_SIZE];
SOCKET _client;
WSADATA _clientWsaData;

FSocket_C::FSocket_C()
{
	_lib = Fmhha();
	sprintf_s(_clientIp, 256, "127.0.0.1");
	_clientPort = 9999;
	_clientInterval[0] = 10;
	_clientInterval[1] = 10;
}
FSocket_C::~FSocket_C()
{

}

static unsigned int __stdcall ConnectThread(void* param)
{
	FSocket_C* pthis = static_cast<FSocket_C*>(param);
	while (_isClientConnect)
	{
		Sleep(_clientInterval[0]);
		if (recv(_client, _clientBuf, BUFFER_SIZE, 0) == SOCKET_ERROR)
		{
			_isClientConnect = false;
			pthis->OnDisConnect();
			continue;
		}

		pthis->RecvMessage(_clientBuf);
	}
	_clientThreadAliveFlag[0] = false;
	CloseHandle(_clientThreadHandle[0]);
	return 0;
}

void FSocket_C::SetConfig(char* ip, int port)
{
	if (_lib.LibraryPermit() == false)
	{
		return;
	}

	sprintf_s(_clientIp, 256, "%s", ip);
	_clientPort = port;
}
void FSocket_C::SetConfig(char* ip, char* port)
{
	if (_lib.LibraryPermit() == false)
	{
		return;
	}

	sprintf_s(_clientIp, 256, "%s", ip);
	_clientPort = atoi(port);
}
void FSocket_C::SetConfigW(wchar_t* ip, int port)
{
	if (_lib.LibraryPermit() == false)
	{
		return;
	}

	int nTemp = WideCharToMultiByte(CP_ACP, 0, ip, -1, NULL, 0, NULL, NULL);
	char* pszTemp = new char[nTemp];
	WideCharToMultiByte(CP_ACP, 0, ip, -1, pszTemp, nTemp, NULL, NULL);
	sprintf_s(_clientIp, 256, "%s", pszTemp);
	_clientPort = port;
	delete[] pszTemp;
}
void FSocket_C::SetConfigW(wchar_t* ip, wchar_t* port)
{
	if (_lib.LibraryPermit() == false)
	{
		return;
	}

	int nTemp = WideCharToMultiByte(CP_ACP, 0, ip, -1, NULL, 0, NULL, NULL);
	char* pszTemp = new char[nTemp];
	WideCharToMultiByte(CP_ACP, 0, ip, -1, pszTemp, nTemp, NULL, NULL);
	sprintf_s(_clientIp, 256, "%s", pszTemp);
	delete[] pszTemp;

	nTemp = WideCharToMultiByte(CP_ACP, 0, port, -1, NULL, 0, NULL, NULL);
	pszTemp = new char[nTemp];
	WideCharToMultiByte(CP_ACP, 0, port, -1, pszTemp, nTemp, NULL, NULL);
	_clientPort = atoi(pszTemp);
	delete[] pszTemp;
}
bool FSocket_C::IsClientConnected()
{
	if (_lib.LibraryPermit() == false)
	{
		return false;
	}

	return _isClientConnect;
}

bool FSocket_C::ConnectClient()
{
	if (_lib.LibraryPermit() == false)
	{
		return false;
	}
	if (_isClientConnect == true)
	{
		return false;
	}
	if (_clientThreadAliveFlag[0] == true)
	{
		return false;
	}

	if (WSAStartup(MAKEWORD(2, 2), &_clientWsaData) != 0)
	{
		return false;
	}

	_client = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	sockaddr_in addr_c = {};
	addr_c.sin_family = AF_INET;
	addr_c.sin_port = htons(_clientPort);
	addr_c.sin_addr.s_addr = inet_addr(_clientIp);

	if (connect(_client, (sockaddr*)&addr_c, sizeof(addr_c)) != 0)
	{
		NotifySocketError("Connect fail");
		WSACleanup();
		return false;
	}

	_isClientConnect = true;
	OnConnect();

	_clientThreadAliveFlag[0] = true;
	_clientThreadHandle[0] = (void*)_beginthreadex(NULL, 0, ConnectThread, this, 0, &(_clientThreadID[0]));

	return true;
}
void FSocket_C::DisConnectClient()
{
	if (_lib.LibraryPermit() == false)
	{
		return;
	}

	_isClientConnect = false;
	closesocket(_client);
	WSACleanup();
}
int FSocket_C::SendCommand(char* data)
{
	if (_lib.LibraryPermit() == false)
	{
		return -1;
	}

	return send(_client, data, strlen(data), 0);
}