#pragma once
#include "IocpCore.h"
#include "IocpEvent.h"
#include "NetAddress.h"
#include "RecvBuffer.h"

class Service;

/*--------==-----------
		Session
---------------------*/


// Client의 정보를 담는 Session 클래스

class Session : public IocpObject
{
	friend class Listener; // Listener는 Session에 마음대로 접근 가능
	friend class IocpCore; // IocpCore는 Session에 마음대로 접근 가능
	friend class Service; // Service는 Session에 마음대로 접근 가능

	enum
	{
		BUFFER_SIZE = 0x10000 // 64KB
	};
public:
	Session();
	virtual ~Session();

public:
	/* 외부에서 사용 */

	void				Send(SendBufferRef sendBuffer);
	bool				Connect();
	void				Disconnect(const WCHAR* cause);
	shared_ptr<Service> GetService() { return _service.lock(); } // weak_ptr은 .lock으로 획득 가능
	void				SetService(shared_ptr<Service> service) {_service = service; }

public:
	/* 정보 관련 */
	void				SetNetAddress(NetAddress address) { _netAddress = address; }
	NetAddress			GetAddress() { return _netAddress; }
	SOCKET				GetSocket() {return _socket;}
	bool				IsConnected() { return _connected; }
	SessionRef			GetSessionRef() { return static_pointer_cast<Session>(shared_from_this()); }	

private:
	/* IocpObject 인터페이스 구현 */
	virtual HANDLE		GetHandle() override;
	virtual void		Dispatch(class IocpEvent* iocpEvent, int32 numOfBytes = 0) override;

private:
	/* 전송 관련 */

	bool RegisterConnect();
	bool RegisterDisconnect();
	void RegisterRecv();
	void RegisterSend();

	void ProcessConnect();
	void ProcessDisconnect();
	void ProcessRecv(int32 numOfBytes);
	void ProcessSend(int32 numOfBytes);

	void HandleError(int32 errorCode);

protected:
	/* 컨텐츠 코드에서 오버라이딩 */
	virtual void	OnConnected() {} // 연결 성공시 호출
	virtual int32	OnRecv(BYTE* buffer, int32 len) {return len;}
	virtual void	OnDisconnected() {} // 연결 해제시 호출
	virtual void	OnSend(int32 len) {} // 보낼때 호출


private:
	weak_ptr<Service> _service;
	SOCKET			_socket = INVALID_SOCKET;
	NetAddress		_netAddress = {};
	Atomic<bool>	_connected = false;

private:
	USE_LOCK;

	/* 수신 관련 */
	RecvBuffer		_recvBuffer;

	/* 송신 관련 */
	Queue<SendBufferRef>	_sendQueue; // RegisterSend가 진행중일때, 잠시 들고 있음.
	Atomic<bool>			_sendRegistered = false; // RegisterSend가 진행중인지 여부
private:
	/* iocpEvent 재사용 */
	ConnectEvent	_connectEvent;
	DisConnectEvent _disconnectEvent;
	RecvEvent		_recvEvent;
	SendEvent		_sendEvent;
};

/*---------------------------
		PacketSession
---------------------------*/

// PacketSession은 받는 쪽이랑 관련이 있음

// PakcetHeader로 
struct PacketHeader
{
	uint16 size; // 전체 패킷의 사이즈 기입
	uint16 id; // Protocol ID ( ex. 1=Login, 2=MoveRequest )
};

// [size(2)][id(2)][data...] [size(2)][id(2)[data...]
// Header를 먼저 까본다
// size를 추출
// 지금까지 받은 데이터가 최소 size보다 큰지?
// 작으면, 기다려서 합침

class PacketSession : public Session
{
public:
	PacketSession();
	virtual ~PacketSession();

	PacketSessionRef GetPacketSessionRef() {return static_pointer_cast<PacketSession>(shared_from_this());}

protected:
	virtual int32 OnRecv(BYTE* buffer, int32 len) sealed; // OnRecv의 base코드는 그대로 두고, OnRecvPacket으로 사용
	virtual void OnRecvPacket(BYTE* buffer, int32 len) abstract; // 이를 반드시 구현해서 사용해야함
};