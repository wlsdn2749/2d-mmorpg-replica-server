#include "pch.h"
#include "IocpCore.h"
#include "IocpEvent.h"


IocpCore::IocpCore()
{
	_iocpHandle = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
	ASSERT_CRASH(_iocpHandle != INVALID_HANDLE_VALUE);
}

IocpCore::~IocpCore()
{
	::CloseHandle(_iocpHandle);
}

bool IocpCore::Register(IocpObjectRef iocpObject)
{
	return ::CreateIoCompletionPort(iocpObject->GetHandle(), _iocpHandle, /*key*/ 0, 0);
}

bool IocpCore::Dispatch(uint32 timeousMs)
{
	DWORD numOfBytes = 0;
	
	IocpEvent* iocpEvent = nullptr; // iocpEvent는 크게 refCount가 필요없을지도?
	ULONG_PTR key = 0; // key는 사용하지 않음

	if (::GetQueuedCompletionStatus(_iocpHandle, OUT &numOfBytes,
		/*Key*/OUT &key,
		/*Overlapped*/OUT reinterpret_cast<LPOVERLAPPED*>(&iocpEvent), 
		timeousMs))
	{
		// 새로운 클라이언트가 접속했으면 -> AcceptEvent
		// 클라이언트가 데이터 보대면 -> RecvEvent
		IocpObjectRef iocpObject = iocpEvent->owner;
		iocpObject->Dispatch(iocpEvent, numOfBytes);
		// iocpEvent는 Listener혹은 Session이 될것임.
	}
	else
	{
		int32 errCode = ::WSAGetLastError();
		switch (errCode)
		{
		case WAIT_TIMEOUT:
			return false;
		default:
			// TODO 로그 찍기
			IocpObjectRef iocpObject = iocpEvent->owner;
			iocpObject->Dispatch(iocpEvent, numOfBytes);
			break;
		}
	}

	return true;


}
