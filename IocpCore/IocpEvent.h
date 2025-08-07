#pragma once
#include "Types.h"


class Session;

enum class EventType : uint8
{
	Connect,
	Disconnect,
	Accept,
	//PreRecv, 0 bytes Recv
	Recv,
	Send
};


/*-----------------------
		IocpEvent
------------------------*/


class IocpEvent: public OVERLAPPED
{
public:
	IocpEvent(EventType type);

	void		Init();

public:
	EventType	eventType;
	IocpObjectRef owner;

private:

};

/*-----------------
	ConnectEvent
-----------------*/

class ConnectEvent : public IocpEvent
{
public:
	ConnectEvent() : IocpEvent(EventType::Connect) {}
};

/*-----------------
	DisConnectEvent
-----------------*/

class DisConnectEvent : public IocpEvent
{
public:
	DisConnectEvent() : IocpEvent(EventType::Disconnect) {}
};


/*-----------------
	AcceptEvent
-----------------*/

class AcceptEvent : public IocpEvent
{
public:
	AcceptEvent() : IocpEvent(EventType::Accept) {}

public:
	SessionRef session = nullptr;
};

/*-----------------
	RecvEvent
-----------------*/

class RecvEvent : public IocpEvent
{
public:
	RecvEvent() : IocpEvent(EventType::Recv) {}
};


/*-----------------
	SendEvent
-----------------*/

class SendEvent : public IocpEvent
{
public:
	SendEvent() : IocpEvent(EventType::Send) {}

	Vector<SendBufferRef> sendBuffers;
};



