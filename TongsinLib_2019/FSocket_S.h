#pragma once
#include "lib/Fmhha.h"

#define BUFFER_SIZE 1024

class __declspec(dllexport) FSocket_S
{
public:
	FSocket_S();
	virtual~FSocket_S();

private:
	Fmhha _lib;

public:
	void SetConfig(char* ip, int port);
	void SetConfig(char* ip, char* port);
	void SetConfigW(wchar_t* ip, int port);
	void SetConfigW(wchar_t* ip, wchar_t* port);
	bool IsServerOpen();
	bool IsServerConnected();

	void OpenServer();
	void CloseServer();
	int SendCommand(char* data);

	virtual void RecvMessage(char* data)			{ }
	virtual void OnConnect()						{ }
	virtual void OnDisConnect()						{ }
	virtual void NotifySocketError(char* data)		{ }
};