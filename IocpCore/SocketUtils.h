#pragma once
#include "NetAddress.h"

/*------------------
	SocketUtils
 -----------------*/
class SocketUtils
{

public:
	// 이 부분은 처음에는 0인데, 런타임에 이를 초기화해야함
	static LPFN_CONNECTEX		ConnectEx;
	static LPFN_DISCONNECTEX	DisconnectEx;
	static LPFN_ACCEPTEX		AcceptEx;
public:
	static void Init(); // 초기화 
	static void Clear(); // 클리어

	static bool BindWindowsFunction(SOCKET socket, GUID guid, LPVOID* fn);
	static SOCKET CreateSocket();

	static bool SetLinger(SOCKET socket, uint16 onoff, uint16 linger); // 소켓이 닫힐 때 소켓에 남아있는 데이터 전송 여부
	static bool SetReuseAddress(SOCKET socket, bool flag); // 이미 사용중인 주소와 포트를 재사용 할 수 있게 함
	static bool SetRecvBufferSize(SOCKET socket, int32 size); // 수신 버퍼크기 설정
	static bool SetSendBufferSize(SOCKET socket, int32 size); // 송신 버퍼크기 설정
	static bool SetTcpNoDelay(SOCKET socket, bool flag); // Nagle 알고리즘 사용 여부 - 작은 패킷이 즉시 전송되게 할지 여부
	static bool SetUpdateAcceptSocket(SOCKET socket, SOCKET listenSocket); // AcceptEx 소켓에 대해 listenSocket의 옵션을 복사

	static bool Bind(SOCKET socket, NetAddress netAddr);
	static bool BindAnyAddress(SOCKET socket, uint16 port);
	static bool Listen(SOCKET socket, int32 backlog = SOMAXCONN);
	static void Close(SOCKET& socket);

};

template<typename T>
static inline bool SetSockOpt(SOCKET socket, int32 level, int32 optName, T optVal)
{
	return SOCKET_ERROR != ::setsockopt(socket, level, optName, reinterpret_cast<char*>(&optVal), sizeof(T));
}

