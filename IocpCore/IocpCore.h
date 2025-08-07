#pragma once

/*------------------
 *	IocpObject
 -----------------*/
class IocpObject : public std::enable_shared_from_this<IocpObject>
{
public:
	virtual HANDLE GetHandle() abstract;
	virtual void Dispatch(class IocpEvent* iocpEvent, int32 numOfBytes = 0) abstract;
	// IocpEvent의 type을 확인해서 Dispatch가 일을 진행함.
};



/*------------------
 *	IocpCore
 -----------------*/


class IocpCore
{
public:
	IocpCore();
	~IocpCore();

	HANDLE		GetHandle() { return _iocpHandle; }
		
	bool		Register(IocpObjectRef iocpObject);
	bool		Dispatch(uint32 timeousMs = INFINITE);
private:
	HANDLE		_iocpHandle;
};
