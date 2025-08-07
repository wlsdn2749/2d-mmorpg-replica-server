#include "pch.h"
#include "Service.h"
#include "Session.h"
#include "Listener.h"
#include <random>

/*---------------------
		Service
---------------------*/


Service::Service(ServiceType type, NetAddress address, IocpCoreRef core, SessionFactory sessionFactory,
	int32 maxSessionCount)
	: _type(type), _netAddress(address), _iocpCore(core), _sessionFactory(sessionFactory), _maxSessionCount(maxSessionCount)
{
}

Service::~Service()
{
}

void Service::CloseService()
{
	//TODO
}

void Service::Broadcast(SendBufferRef sendBuffer)
{
	WRITE_LOCK;
	for (const auto& session : _sessions)
	{
		session->Send(sendBuffer);
	}
}

SessionRef Service::CreateSession()
{
	SessionRef session = _sessionFactory();
	session->SetService(shared_from_this());
	if (_iocpCore->Register(session) == false)
		return nullptr;

	return session;
}

void Service::AddSession(SessionRef session)
{
	{
		WRITE_LOCK;
		_sessionCount++;
		_sessions.insert(session);
	}

	// 여기서 SendPlayerListTo(session) (GameRoom::Enter부분)
}

void Service::ReleaseSession(SessionRef session)
{
	WRITE_LOCK;
	ASSERT_CRASH(_sessions.erase(session) != 0);
	_sessionCount--;
}

ClientService::ClientService(NetAddress targetAddress, IocpCoreRef core, SessionFactory sessionFactory, int32 maxSessionCount)
	:Service(ServiceType::Client, targetAddress, core, sessionFactory, maxSessionCount)
{
}

bool ClientService::Start()
{
	if (CanStart() == false)
		return false;

	const int32 sessionCount = GetMaxSessionCount();
	for (int32 i = 0; i < sessionCount; i++)
	{
		SessionRef session = CreateSession();
		if (session->Connect() == false)
			return false;
	}

	return true;
}

ServerService::ServerService(NetAddress address, IocpCoreRef core, SessionFactory sessionFactory, int32 maxSessionCount)
	:Service(ServiceType::Server, address, core, sessionFactory, maxSessionCount)
{
}

bool ServerService::Start()
{
	if (CanStart() == false)
		return false;

	_listener = MakeShared<Listener>();
	if (_listener == nullptr)
		return false;

	ServerServiceRef service = static_pointer_cast<ServerService>(shared_from_this());

	if (_listener->StartAccept(service) == false)
		return false;

	return true;
}

void ServerService::CloseService()
{
	//TODO

	Service::CloseService();
}
