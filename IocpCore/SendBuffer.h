#pragma once
#include "Container.h"

class SendBufferChunk;

/*---------------
 *	SendBuffer
 ---------------*/
class SendBuffer
{

public:
	SendBuffer(SendBufferChunkRef owner, BYTE* buffer, uint32 allocSize); //
	~SendBuffer();

	BYTE* Buffer(){return _buffer; }
	uint32 AllocSize(){return _allocSize;}
	uint32 WriteSize() { return _writeSize; }
	void Close(uint32 writeSize); 


private:
	BYTE*				_buffer; // 이 _buffer가 _owner가 살아있을 동안 유효함
	uint32				_allocSize = 0;
	uint32				_writeSize = 0;
	SendBufferChunkRef	_owner; // sendBuffer라는 공간을 제공한 Owner가 누구인가?
	// _owner는 ~SendBuffer에서 1줄어들고, 언젠가 이게 0이됨

};


/*--------------------
 *	SendBufferChunk
 --------------------*/

class SendBufferChunk : public enable_shared_from_this<SendBufferChunk>
{
public:
	enum
	{
		SEND_BUFFER_CHUNK_SIZE = 6000
	};

public:
	SendBufferChunk();
	~SendBufferChunk();

	void Reset();
	SendBufferRef Open(uint32 allocSize);
	void Close(uint32 writeSize); 

	bool IsOpen() {return _open;} // 지금 열려있고 Close를 잘 했는지?
	BYTE* Buffer() {return &_buffer[_usedSize]; } // usedSize까지 사용했으니까, 그 다음 사용할 포인터를 넘겨줌
	uint32 FreeSize() {return static_cast<uint32>(_buffer.size()) - _usedSize; }

private:
	Array<BYTE, SEND_BUFFER_CHUNK_SIZE> _buffer = {};
	bool								_open = false;
	uint32								_usedSize = 0;
};

 /*--------------------
  *	SendBufferManager
  --------------------*/


class SendBufferManager
{
public:
	SendBufferRef		Open(uint32 size);

private:
	SendBufferChunkRef	Pop();
	void				Push(SendBufferChunkRef buffer);
	static void			PushGlobal(SendBufferChunk* buffer);

private:

	USE_LOCK;

	Vector<SendBufferChunkRef> _sendBufferChunks;
};