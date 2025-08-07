#include "pch.h"
#include "SendBuffer.h"

SendBuffer::SendBuffer(SendBufferChunkRef owner, BYTE* buffer, uint32 allocSize)
	: _owner(owner), _buffer(buffer), _allocSize(allocSize)
{
	
}

SendBuffer::~SendBuffer()
{
}

void SendBuffer::Close(uint32 writeSize)
{
	ASSERT_CRASH(_allocSize >= writeSize);
	_writeSize = writeSize;
	_owner->Close(writeSize);
}

SendBufferChunk::SendBufferChunk()
{
}

SendBufferChunk::~SendBufferChunk()
{
}

void SendBufferChunk::Reset()
{
	_open = false;
	_usedSize = 0;
}

SendBufferRef SendBufferChunk::Open(uint32 allocSize)
{
	ASSERT_CRASH(allocSize <= SEND_BUFFER_CHUNK_SIZE);
	ASSERT_CRASH(_open == false);

	if (allocSize > FreeSize())
		return nullptr;

	_open = true;
	return ObjectPool<SendBuffer>::MakeShared(shared_from_this(), Buffer(), allocSize); // allocSize만큼 버퍼 새로 만들어서 제공, ObjectPool로 관리

}

void SendBufferChunk::Close(uint32 writeSize)
{
	ASSERT_CRASH(_open == true) // open을 안했는데 close하는건 말이안되는 거임 (Single thread)
	_open = false;
	_usedSize += writeSize;
}


// 사용준비
SendBufferRef SendBufferManager::Open(uint32 size)
{
	if (LSendBufferChunk == nullptr) // 만약에 TLS data인 LSendBufferChunk가 nullptr이면
	{
		LSendBufferChunk = Pop(); // WRITE_LOCK
		LSendBufferChunk->Reset(); // 새로 사용하는 경우, 정보를 초기화해야함
	}


	ASSERT_CRASH(LSendBufferChunk->IsOpen() == false) // 이미 열려있으면 안됨

	// 다 썼으면 버리고 새거로 교체
	if (LSendBufferChunk->FreeSize() < size)
	{
		LSendBufferChunk = Pop(); // 여유분을 꺼내서 다시 사용
		LSendBufferChunk->Reset(); // 새로 사용하는 경우, 정보를 초기화해야함
	}

	// 필요한 버퍼청크가 준비되었음
	return LSendBufferChunk->Open(size);
}

SendBufferChunkRef SendBufferManager::Pop()
{
	{
		WRITE_LOCK;
		// _sendBufferChunks는 SendBufferChunkRef들을 모아놓은 vector이므로
		// 이는 SEndbufferManager인 Global적인 객체이므로 LOCk이 필요함
		if (_sendBufferChunks.empty() == false) // _sendBufferChunk에 여유 sendBufferChunkRef가 있다면
		{
			SendBufferChunkRef sendBufferChunk = _sendBufferChunks.back(); // 재사용
			_sendBufferChunks.pop_back();
			return sendBufferChunk;
		}
	}
	
	return SendBufferChunkRef( xnew<SendBufferChunk>(), PushGlobal );
	//새로 만들어서 제공, 삭제자로 다시 넣음 PushGlobal은 SendBufferChunkRef가 0이되어 소멸할때
	// 작동함
}

void SendBufferManager::Push(SendBufferChunkRef buffer)
{
	WRITE_LOCK;
	_sendBufferChunks.push_back(buffer); // 다써서 삭제할경우, 삭제하지말고 _sendBufferChunk에 다시 넣음
}

void SendBufferManager::PushGlobal(SendBufferChunk* buffer)
{
	//cout << "PUSHGLOBAL SENDBUFFERCHUNK" << endl;
	GSendBufferManager->Push(SendBufferChunkRef(buffer, PushGlobal)); // buffer를 그대로 받아서, 다시 넣음
}
