#pragma once


/*----------------
 *	RecvBuffer
 ---------------*/


class RecvBuffer
{
	// read랑 write 사이에 껴 있는게 유효한 데이터임
	enum { BUFFER_COUNT = 10}; // 크게 잡아서, 언젠가 rw가 만나도록
public:
	RecvBuffer(int32 bufferSize);
	~RecvBuffer();

	void Clean();
	bool OnRead(int32 numOfBytes);
	bool OnWrite(int32 numOfBytes);

	BYTE* ReadPos() { return &_buffer[_readPos]; }
	BYTE* WritePos() { return &_buffer[_writePos]; }
	int32 DataSize() { return _writePos - _readPos; }
	int32 FreeSize() { return _capacity - _writePos;}

private:
	int32 _capacity = 0;
	int32 _bufferSize = 0;
	int32 _readPos = 0;
	int32 _writePos = 0;
	vector<BYTE> _buffer;

};

