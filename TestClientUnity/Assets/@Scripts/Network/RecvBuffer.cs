using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ServerCore
{
    public class RecvBuffer
    {
        // [rw][ ][ ][ ][ ][ ][ ]
        ArraySegment<byte> _buffer;
        int _readPos;
        int _writePos;

        public RecvBuffer(int bufferSize)
        {
            _buffer = new ArraySegment<byte>(new byte[bufferSize], 0, bufferSize);
        }

        public int DataSize { get { return _writePos - _readPos; } } // DataSize를 리턴하면 기록된 데이터의 크기를 가져올 수 있음
        public int FreeSize { get { return _buffer.Count - _writePos; } } // 전체 버퍼에서 얼마 썼는지

        public ArraySegment<byte> ReadSegment
        {
            get { return new ArraySegment<byte>(_buffer.Array, _buffer.Offset + _readPos, DataSize); }
        }

        public ArraySegment<byte> WriteSegment
        {
            get { return new ArraySegment<byte>(_buffer.Array, _buffer.Offset + _writePos, FreeSize); }
        }

        public void Clean()
        {
            int dataSize = DataSize;
            if(dataSize == 0) 
            {
                // 데이터를 다 처리한 상태, 복사 하지 않고 처리
                _readPos = _writePos = 0;
            }

            else
            {
                // 남은 찌끄레기가 있으면 시작 위치로 복사
                Array.Copy(_buffer.Array, _buffer.Offset + _readPos, _buffer.Array, _buffer.Offset, dataSize);
                _readPos = 0;
                _writePos = dataSize;
            }
        }

        public bool OnRead(int numOfBytes) // numOfBytes는 처리하는 양
        {
            if (numOfBytes > DataSize)
                return false;

            _readPos += numOfBytes;
            return true;
        }
        public bool OnWrite(int numOfBytes)
        {
            if (numOfBytes > FreeSize)
                return false;

            _writePos += numOfBytes;
            return true;
        }
    }
}
