#pragma once
#include "NetAddress.h"
#include "IocpCore.h"
#include "Listener.h"
#include <functional>

enum class ServiceType : uint8
{
	Server,
	Client
};


/*---------------------
		Service
---------------------*/

using SessionFactory = function<SessionRef(void)>;

class Service : public enable_shared_from_this<Service>
{
public:
	Service(ServiceType type, NetAddress address, IocpCoreRef core, SessionFactory sessionFactory, int32 maxSessionCount = 1);
	virtual ~Service();

	virtual bool	Start() abstract;
	bool			CanStart() { return _sessionFactory != nullptr;}

	virtual void	CloseService();
	void			SetSessionFactory(SessionFactory func) { _sessionFactory = func; }

	void			Broadcast(SendBufferRef sendBuffer);
	SessionRef		CreateSession();
	void			AddSession(SessionRef session);
	void			ReleaseSession(SessionRef session);
	int32			GetCurrentSessionCount() { return _sessionCount; }
	int32			GetMaxSessionCount() { return _maxSessionCount; }
public:
	ServiceType		GetServiceType() { return _type; }
	NetAddress		GetNetAddress() { return _netAddress; }
	IocpCoreRef&	GetIocpCore() { return _iocpCore; }
protected:
	USE_LOCK;

	ServiceType			_type;
	NetAddress			_netAddress = {};
	IocpCoreRef			_iocpCore;

	Set<SessionRef>		_sessions;
	int32				_sessionCount = 0;
	int32				_maxSessionCount = 0;
	SessionFactory		_sessionFactory;
};

/*-----------------------------
		ClientService
------------------------------*/

class ClientService : public Service
{
public:
	ClientService(NetAddress address, IocpCoreRef core, SessionFactory sessionFactory, int32 maxSessionCount = 1);
	virtual ~ClientService() {}

	virtual bool Start() override;

};

/*-----------------------------
		ServerService
-----------------------------*/

class ServerService : public Service
{
public:
	// - address			: 서버가 바인딩할 IP와 포트
	// - core				: IOCP 코어 (Completion Port + Worker Thread)\
	// - sessionFactory		: 새 클라이언트가 접속하면 새 Session 객체를 생성해줄 팩토리
	// - maxSessionCount	: 동시에 허용할 최대 세션 수
	ServerService(NetAddress address, IocpCoreRef core, SessionFactory sessionFactory, int32 maxSessionCount = 1);
	virtual ~ServerService(){};

	virtual bool Start() override;
	virtual void CloseService() override;

private:
	ListenerRef		_listener = nullptr; // 서버는 리스너가 있어야함	
};