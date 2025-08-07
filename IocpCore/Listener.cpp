#include "pch.h"
#include "Listener.h"

#include "SocketUtils.h"
#include "IocpEvent.h"
#include "Session.h"
#include "Service.h"

Listener::~Listener()
{
	SocketUtils::Close(_socket);

	for (AcceptEvent* acceptEvent : _acceptEvents)
	{
		// TODO

		xdelete(acceptEvent);
	}
}

bool Listener::StartAccept(ServerServiceRef service)
{
	_service = service; // Listener가 속한 서비스
	if (_service == nullptr)
		return false;

	_socket = SocketUtils::CreateSocket(); // Listen 소켓 만들기
	if (_socket == INVALID_SOCKET)
		return false;

	if (_service->GetIocpCore()->Register(shared_from_this()) == false) // Iocp가 Listener 소켓을 관찰 하고 있어야함 -> 소켓을 IOCP 큐에 등록
		return false;

	if (SocketUtils::SetReuseAddress(_socket, true) == false)
		return false;

	if (SocketUtils::SetLinger(_socket, 0, 0) == false)
		return false;

	if (SocketUtils::Bind(_socket, _service->GetNetAddress()) == false)
		return false;

	if (SocketUtils::Listen(_socket) == false)
		return false;

	//if (SocketUtils::SetTcpNoDelay(_socket, true) == false) // Nagle 끄기
	//	return false;

	const int32 acceptCount = service->GetMaxSessionCount();
	for (int32 i = 0; i < acceptCount; i++)
	{
		AcceptEvent* acceptEvent = xnew<AcceptEvent>();
		acceptEvent->owner = shared_from_this(); // Listener가 AcceptEvent의 owner가 된다. enable_shared_from_this<T>를 상속받아야함 <- // TODO 여기 순환참조 의심
		_acceptEvents.push_back(acceptEvent); // 나중에 삭제할 수 있도록
		RegisterAccept(acceptEvent);
		// 지금 당장 누군가 접속하면 좋은거고, 아니면 완료통지
		// IOCP에 등록하면, Worker Thread들이 관찰하다가 꺼내씀.
		// 이게 한번만 돌면 여러개가 한번에 도착했을때 처리를 못할 수 있으니까


		// 클라이언트가 접속하면, IOCP 큐에 완료 이벤트가 들어감
	}

	return true;

}

void Listener::CloseSocket()
{
	SocketUtils::Close(_socket);
}

HANDLE Listener::GetHandle()
{
	return reinterpret_cast<HANDLE>(_socket);
}

void Listener::Dispatch(class IocpEvent* iocpEvent, int32 numOfBytes)
{
	ASSERT_CRASH(iocpEvent->eventType == EventType::Accept);
	AcceptEvent* acceptEvent = static_cast<AcceptEvent*>(iocpEvent);
	ProcessAccept(acceptEvent);
}

void Listener::RegisterAccept(AcceptEvent* acceptEvent)
{
	SessionRef session = _service->CreateSession(); // 세션을 생성한다. (새로운 클라이언트가 접속할 때마다 세션이 필요함)	
	acceptEvent->Init();
	acceptEvent->session = session;

	DWORD bytesReceived = 0;
	BOOL ret = SocketUtils::AcceptEx(
		_socket,							// 리슨 소켓
		session->GetSocket(),				// 새 연결 받을 소켓
		session->_recvBuffer.WritePos(),	// 주소등 정보가 저장될 버퍼
		0,									// 실제로 수신할 데이터, 0이면 주소 정보만
		sizeof(SOCKADDR_IN) + 16,			// 로컬 주소 저장 공간
		sizeof(SOCKADDR_IN) + 16,			// 원격 주소 저장 공간
		OUT &bytesReceived,					// 실제 수신된 바이트
		static_cast<LPOVERLAPPED>(acceptEvent) // 비동기 구조체, acceptEvent가 OVERLAPPED를 상속하고 있음
	);
	// OS가 AcceptEx 완료시, IOCP큐에 AcceptEvent를 전달
	if (ret == false)
	{
		const int32 errorCode = ::WSAGetLastError();
		if (errorCode != WSA_IO_PENDING) // PENDING은 걸렸는데, 아무도 없어서 빠져 나온것
		{
			// 일단 다시 Accept 걸어준다. (낚시대를 걸었는데, 미끄러져 실패한것 -> 다시 걸어야함)
			RegisterAccept(acceptEvent);
		}
	}
}

void Listener::ProcessAccept(AcceptEvent* acceptEvent)
{
	SessionRef session = acceptEvent->session;

	if (SocketUtils::SetUpdateAcceptSocket(session->GetSocket(), _socket) == false)
	{
		RegisterAccept(acceptEvent);
		return;
	}

	SOCKADDR_IN sockAddress;
	int32 sizeOfSockAddr = sizeof(sockAddress);

	if (SOCKET_ERROR == ::getpeername(session->GetSocket(), OUT reinterpret_cast<SOCKADDR*>(&sockAddress), &sizeOfSockAddr))
	{
		RegisterAccept(acceptEvent); // 실패해도 다시 받기
		return;
	}

	session->SetNetAddress(NetAddress(sockAddress));
	session->ProcessConnect();
	cout << "Client Connected" << endl;

	//TODO

	RegisterAccept(acceptEvent); // 성공해도 다시 AcceptEx를 걸어야함
}
