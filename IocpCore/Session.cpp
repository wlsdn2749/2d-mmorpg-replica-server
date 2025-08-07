#include "pch.h"
#include "Session.h"
#include "SocketUtils.h"
#include "Service.h"

Session::Session() : _recvBuffer(BUFFER_SIZE)
{
	_socket = SocketUtils::CreateSocket();
}

Session::~Session()
{
	SocketUtils::Close(_socket);
}


void Session::Send(SendBufferRef sendBuffer)
{
	if (IsConnected() == false)
		return;

	bool registerSend = false;
	// 현재 RegisterSend가 걸리지 않은 상태라면 걸어준다
	{
		WRITE_LOCK;

		_sendQueue.push(sendBuffer);

		// 지금 false면 true로 하고 RegisterSend하고, 아니면 그냥 통과
		if (_sendRegistered.exchange(true) == false)
			registerSend = true;
	}


	if (registerSend)
		RegisterSend();
	
	
}

// 서버끼리 연결할때 사용
bool Session::Connect()
{
	RegisterConnect();
	return true;
}

void Session::Disconnect(const WCHAR* cause)
{
	// 원래 False였으면, False -> 리턴
	// 원래 True였으면 False로 변경 -> 진행
	if (_connected.exchange(false) == false)
		return;

	// TEMP
	wcout << "Disconnect : " << cause << endl;

	RegisterDisconnect();

}

HANDLE Session::GetHandle()
{
	return reinterpret_cast<HANDLE>(_socket);
}

void Session::Dispatch(class IocpEvent* iocpEvent, int32 numOfBytes)
{
	switch (iocpEvent->eventType)
	{
	case EventType::Connect:
		ProcessConnect();
		break;
	case EventType::Disconnect:
		ProcessDisconnect();
		break;
	case EventType::Recv:
		ProcessRecv(numOfBytes);
		break;
	case EventType::Send:
		ProcessSend(numOfBytes);
		break;
	default:
		break;
	}
}

bool Session::RegisterConnect()
{
	if (IsConnected())
		return false;

	if (GetService()->GetServiceType() != ServiceType::Client)
		return false;

	if (SocketUtils::SetReuseAddress(_socket, true) == false)
		return false;

	if (SocketUtils::BindAnyAddress(_socket, 0/*남는거*/) == false)
		return false;

	_connectEvent.Init();
	_connectEvent.owner = shared_from_this(); // ADD_REF

	DWORD numOfBytes = 0;
	SOCKADDR_IN sockAddr = GetService()->GetNetAddress().GetSockAddr(); // ServiceType::Client면, 이 주소는 Client
	if (false == SocketUtils::ConnectEx(_socket, reinterpret_cast<SOCKADDR*>(&sockAddr), sizeof(sockAddr),
		nullptr, 0, &numOfBytes, &_connectEvent))
	{
		int32 errorCode = ::WSAGetLastError();
		if (errorCode != WSA_IO_PENDING)
		{
			_connectEvent.owner = nullptr; // RELEASE_REF
			return false;
		}
	}

	return true;


}

bool Session::RegisterDisconnect()
{
	_disconnectEvent.Init();
	_disconnectEvent.owner = shared_from_this(); // ADD_REF

	if (false == SocketUtils::DisconnectEx(_socket, &_disconnectEvent, TF_REUSE_SOCKET, 0))
	{
		int32 errorCode = ::WSAGetLastError();
		if (errorCode != WSA_IO_PENDING)
		{
			_disconnectEvent.owner = nullptr; // RELEASE_REF
			return false;
		}
	}
	return true;
}

void Session::RegisterRecv()
{
	if (IsConnected() == false) // 연결이 끊겼으면 리턴
		return;
	
	// 이런식으로 매번 만들어도 되고
	// RecvEvent* recvEvent = xnew<RecvEvent>;
	// recvEvent->owner = shared_from_this();


	_recvEvent.Init();
	_recvEvent.owner = shared_from_this(); // ADD_REF 


	WSABUF wsaBuf;
	wsaBuf.buf = reinterpret_cast<char*>(_recvBuffer.WritePos());
	wsaBuf.len = _recvBuffer.FreeSize(); // 최대로 받을 수 있는 크기는 얼마인가? ( 현재 가지고 있는 데이터 크기 X)

	DWORD numOfBytes = 0; // 실제로 복사해온 데이터 <- 이게 DataSize()가 될 것?
	DWORD flags = 0;

	if (SOCKET_ERROR == ::WSARecv(_socket, &wsaBuf, 1, OUT &numOfBytes, OUT &flags, &_recvEvent, nullptr))
	{
		int32 errorCode = ::WSAGetLastError();
		if (errorCode != WSA_IO_PENDING)
		{
			HandleError(errorCode);
			_recvEvent.owner = nullptr; // Release Ref
			_sendEvent.sendBuffers.clear();
			_sendRegistered.store(false);
		}
	}
}

void Session::RegisterSend()
{
	if (IsConnected() == false)
		return;

	_sendEvent.Init();
	_sendEvent.owner = shared_from_this(); // ADD_REF

	// 보낼 데이터를 sendEvent에 등록
	{
		WRITE_LOCK;

		int32 writeSize = 0;
		while (_sendQueue.empty() == false)
		{
			SendBufferRef sendBuffer = _sendQueue.front();

			writeSize += sendBuffer->WriteSize();
			// TODO : 예외 체크

			_sendQueue.pop();
			_sendEvent.sendBuffers.push_back(sendBuffer);
		}

	}

	Vector<WSABUF> wsaBufs;
	wsaBufs.reserve(_sendEvent.sendBuffers.size());
	for (SendBufferRef sendBuffer : _sendEvent.sendBuffers)
	{
		WSABUF wsaBuf;
		wsaBuf.buf = reinterpret_cast<char*>(sendBuffer->Buffer());
		wsaBuf.len = static_cast<LONG>(sendBuffer->WriteSize());
		wsaBufs.push_back(wsaBuf);
	}

	DWORD numOfBytes = 0;
	if (SOCKET_ERROR == ::WSASend(_socket, wsaBufs.data(), static_cast<DWORD>(wsaBufs.size()), OUT & numOfBytes, 0, &_sendEvent, nullptr))
	{
		int32 errorCode = ::WSAGetLastError();
		if (errorCode != WSA_IO_PENDING)
		{
			HandleError(errorCode);
			_sendEvent.owner = nullptr; // RELEASE REF
		}
	}
}

void Session::ProcessConnect()
{
	_connectEvent.owner = nullptr; // RELEASE_REF
	_connected.store(true);

	// 세션 등록
	GetService()->AddSession(GetSessionRef());

	// 컨텐츠 코드에서 오버라이딩
	OnConnected();

	// 수신 등록
	RegisterRecv();
}

void Session::ProcessDisconnect()
{
	_disconnectEvent.owner = nullptr; // RELEASE_REF

	OnDisconnected(); // 콘텐츠 코드에서 오버로딩
	GetService()->ReleaseSession(GetSessionRef());
}

void Session::ProcessRecv(int32 numOfBytes)
{
	_recvEvent.owner = nullptr; // Release Ref 성공했고, 더이상 예약 안걸려있으니 Ref줄여줌

	if (numOfBytes == 0)
	{
		Disconnect(L"Recv 0");
		return;
	}

	if (_recvBuffer.OnWrite(numOfBytes) == false)
	{
		Disconnect(L"OnWrite Overflow");
		return;
	}

	int32 dataSize = _recvBuffer.DataSize();

	int32 processLen = OnRecv(_recvBuffer.ReadPos(), dataSize);

	if (processLen < 0 || dataSize < processLen || _recvBuffer.OnRead(processLen) == false)
	{
		// 처리한 데이터가 0보다 작거나
		// 처리할 데이터가 가 처리한 데이터보다 작거나
		// OnRead에 실패했으면
		Disconnect(L"OnRecv Overflow");
		return;
	}

	// 커서 정리
	_recvBuffer.Clean();

	// 수신 등록
	RegisterRecv();
}

void Session::ProcessSend(int32 numOfBytes)
{
	_sendEvent.owner == nullptr; // RELEASE REF
	_sendEvent.sendBuffers.clear(); // RELEASE_REF 버퍼 사용했으니까 밀 수 있음

	if (numOfBytes == 0)
	{
		Disconnect(L"Send 0");
		return;
	}

	// 컨텐츠 오버로딩
	OnSend(numOfBytes);

	WRITE_LOCK;
	if (_sendQueue.empty()) // 호출한 스레드에서 현재 sendQueue가 비어있다면
		_sendRegistered.store(false); // send예약을 종료하고 끝
	else
		RegisterSend();  // 남아있다면 그 스레드가 다시 예약을 검
}

void Session::HandleError(int32 errorCode)
{
	switch (errorCode)
	{
	case WSAECONNRESET:
	case WSAECONNABORTED:
		Disconnect(L"HandleError");
		break;
	default:
		// TODO : Log
		cout << "Handle Error" << errorCode << endl;
		break;
	}

}

PacketSession::PacketSession()
{
}

PacketSession::~PacketSession()
{
}

// [size(2)][id(2)][data...] [size(2)][id(2)[data...]
// Header를 먼저 까본다
// size를 추출
// 지금까지 받은 데이터가 최소 size보다 큰지?
// 작으면, 기다려서 합침

// buffer는 여러 패킷이 들어있을 수 있음

int32 PacketSession::OnRecv(BYTE* buffer, int32 len)
{
	int32 processLen = 0; // ① 이번 호출에서 이미 처리한 총 바이트
	while (true) 
	{
		int32 dataSize = len - processLen; // ② 아직 안 읽은 바이트 수
		// 최소한 헤더는 파싱할 수 있는지?

		if (dataSize < sizeof(PacketHeader))
			break;							// ③ 헤더(4B)조차 못 읽으면 다음 recv 때 이어서

		// ④ 현재 오프셋(processLen)에서 헤더 구조체를 읽는다
		PacketHeader header = *(reinterpret_cast<PacketHeader*>(&buffer[processLen])); // 여기부분 실수;;

		
		// ⑤ 헤더가 알려 준 ‘전체 패킷 길이’(size) 만큼 모두 도착했는가?
		if (dataSize < header.size)
			break;

		// ⑥ 한 패킷(헤더+본문)이 완성 → 상위 핸들러로 넘김
		OnRecvPacket(&buffer[processLen], header.size);

		processLen += header.size;
	}

	return processLen;  // ⑧ 이번에 소비한 바이트 수 리턴
}
