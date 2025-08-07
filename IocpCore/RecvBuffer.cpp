#include "pch.h"
#include "RecvBuffer.h"

RecvBuffer::RecvBuffer(int32 bufferSize)
	:	_capacity(bufferSize * BUFFER_COUNT),
		_bufferSize(bufferSize)
{
	_buffer.resize(_capacity);
}

RecvBuffer::~RecvBuffer()
{
}

void RecvBuffer::Clean()
{
	int32 dataSize = DataSize();
	if (dataSize == 0)
	{
		/* 읽기, 쓰기 커서가 동일한 위치 ? 둘다 리셋*/
		// 모든 데이터를 다 처리했다는 의미임
		_readPos = _writePos = 0;
	}
	else
	{
		// 여유 공간이 버퍼 한개 크기 미만, 데이터 앞으로 떙김
		if (FreeSize() < _bufferSize)
		{
			::memcpy(&_buffer, &_buffer[_readPos], dataSize);
			_readPos = 0;
			_writePos = dataSize;
		}
	}
}

bool RecvBuffer::OnRead(int32 numOfBytes)
{
	if (numOfBytes > DataSize()) // 만약 읽으려는게 현재 있는 DataSize보다 크다면?
		return false; // 실패

	_readPos += numOfBytes;
	return true;

}

bool RecvBuffer::OnWrite(int32 numOfBytes)
{
	if (numOfBytes > FreeSize()) // 만약 쓰려는게 현재 FreeSize보다 크다면
		return false; // 실패

	_writePos += numOfBytes;
	return true;
}
