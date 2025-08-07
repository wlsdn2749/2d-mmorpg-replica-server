#include "pch.h"
#include "SocketUtils.h"

/*------------------
	SocketUtils
 -----------------*/

LPFN_CONNECTEX		SocketUtils::ConnectEx = nullptr;
LPFN_DISCONNECTEX	SocketUtils::DisconnectEx = nullptr;
LPFN_ACCEPTEX		SocketUtils::AcceptEx = nullptr;

void SocketUtils::Init()
{
	WSADATA wsaData;
	ASSERT_CRASH(::WSAStartup(MAKEWORD(2, 2), OUT & wsaData) == 0);
	// winsock 초기화 WSAStartup은 정상적으로 호출되면 0이므로, 그 이외의 값은 크래시

	/* 런타임에 주소 얻어오는 API */
	SOCKET dummySocket = CreateSocket();
	ASSERT_CRASH(BindWindowsFunction(dummySocket, WSAID_CONNECTEX, reinterpret_cast<LPVOID*>(&ConnectEx)));
	ASSERT_CRASH(BindWindowsFunction(dummySocket, WSAID_DISCONNECTEX, reinterpret_cast<LPVOID*>(&DisconnectEx)));
	ASSERT_CRASH(BindWindowsFunction(dummySocket, WSAID_ACCEPTEX, reinterpret_cast<LPVOID*>(&AcceptEx)));
}

void SocketUtils::Clear()
{
	::WSACleanup();
}

bool SocketUtils::BindWindowsFunction(SOCKET socket, GUID guid, LPVOID* fn)
{
	// GUID는 WSAID_CONNECTEX같은 전역 고유 식별자, (각 함수마다 GUID값이 할당되어있음)
	// fn은 함수 주소값
	// SIO_GET_EXTENSION_FUNCTION_POINTER로
	// mswsock.lib에 있는 AcceptEx()의 포인터를 받아서, 이를 fn에 등록함
	// 다음부터 ConnectEx를 호출하면, 매번 WSAloctl을 통해 호출하는 오버헤드를 줄일 수 있음
	DWORD bytes = 0;
	return SOCKET_ERROR != ::WSAIoctl(socket, SIO_GET_EXTENSION_FUNCTION_POINTER, &guid, sizeof(guid), fn, sizeof(*fn), OUT & bytes, NULL, NULL);
	// Connect, Disconnect, Accept Ex들을 런타임에 불러오기 위한 함수
}

SOCKET SocketUtils::CreateSocket()
{
	// 일반 socket()과 다른점은 WSA_FLAG_OVERLAPPED인데, 기본값이 True라, socket에서도
	// Overlapped 함수 호출이 가능했음
	return ::WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
}

bool SocketUtils::SetLinger(SOCKET socket, uint16 onoff, uint16 linger)
{
	LINGER _linger;
	_linger.l_onoff = onoff;
	_linger.l_linger = linger;
	return SetSockOpt(socket, SOL_SOCKET, SO_LINGER, _linger);
}

bool SocketUtils::SetReuseAddress(SOCKET socket, bool flag)
{
	return SetSockOpt(socket, SOL_SOCKET, SO_REUSEADDR, flag);
}

bool SocketUtils::SetRecvBufferSize(SOCKET socket, int32 size)
{
	return SetSockOpt(socket, SOL_SOCKET, SO_RCVBUF, size);
}

bool SocketUtils::SetSendBufferSize(SOCKET socket, int32 size)
{
	return SetSockOpt(socket, SOL_SOCKET, SO_SNDBUF, size);
}

bool SocketUtils::SetTcpNoDelay(SOCKET socket, bool flag)
{
	// FALSE가 사용하지 않도록 설정
	return SetSockOpt(socket, IPPROTO_TCP, TCP_NODELAY, flag);
}

bool SocketUtils::SetUpdateAcceptSocket(SOCKET socket, SOCKET listenSocket)
{
	return SetSockOpt(socket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, listenSocket);
}

bool SocketUtils::Bind(SOCKET socket, NetAddress netAddr)
{
	return SOCKET_ERROR != ::bind(socket, reinterpret_cast<const SOCKADDR*>(&netAddr.GetSockAddr()), sizeof(SOCKADDR_IN));
}

bool SocketUtils::BindAnyAddress(SOCKET socket, uint16 port)
{
	SOCKADDR_IN myAddress;
	myAddress.sin_family = AF_INET;
	myAddress.sin_addr.s_addr = ::htonl(INADDR_ANY);
	myAddress.sin_port = ::htons(port);

	return SOCKET_ERROR != ::bind(socket, reinterpret_cast<const SOCKADDR*>(&myAddress), sizeof(myAddress));
}

bool SocketUtils::Listen(SOCKET socket, int32 backlog)
{
	return SOCKET_ERROR != ::listen(socket, backlog); 
}

void SocketUtils::Close(SOCKET& socket)
{
	if (socket != INVALID_SOCKET)
		::closesocket(socket);

	socket = INVALID_SOCKET;

}
