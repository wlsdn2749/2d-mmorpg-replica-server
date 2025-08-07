using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace ServerCore
{
    public abstract class PacketSession : Session
    {
        private static readonly int _headerSize = 4; // size(2) + id(2)

        // 패킷 헤더 구조체 (Server와 동일하게 관리)
        private readonly struct PacketHeader
        {
            public readonly ushort Size;  // 전체 패킷 크기 (Header + Body)
            public readonly ushort Id;    // 패킷 ID

            public PacketHeader(ushort size, ushort id)
            {
                Size = size;
                Id = id;
            }

            public static PacketHeader FromBuffer(ArraySegment<byte> buffer)
            {
                if (buffer.Count < _headerSize)
                    throw new ArgumentException("Buffer too small for PacketHeader");

                ushort size = BitConverter.ToUInt16(buffer.Array, buffer.Offset);
                ushort id   = BitConverter.ToUInt16(buffer.Array, buffer.Offset + 2);
                return new PacketHeader(size, id);
            }
        }

        public sealed override int OnRecv(ArraySegment<byte> buffer) // buffer를 통해 얼마만큼의 데이터를 처리했는지 반환
        {
            int processLen = 0;
            int packetCount = 0;
            while (true)
            {
                // 최소한 헤더는 파싱할 수 있는지 확인
                if (buffer.Count < _headerSize)
                    break;

                // 헤더를 읽어온다
                PacketHeader header = PacketHeader.FromBuffer(buffer);

                // 패킷이 완전체로 도착했는지 확인
                if (buffer.Count < header.Size)
                    break;

                // 패킷 하나가 완성 → 상위 핸들러로 전달
                OnRecvPacket(new ArraySegment<byte>(buffer.Array, buffer.Offset, header.Size));
                packetCount++;

                processLen += header.Size;
                buffer = new ArraySegment<byte>(buffer.Array, buffer.Offset + header.Size, buffer.Count - header.Size);
            }

            return processLen;
        }

        public abstract void OnRecvPacket(ArraySegment<byte> buffer);
    }

    public abstract class Session
    {
        Socket _socket;
        int _disConnected = 0;

        RecvBuffer _recvBuffer = new RecvBuffer(65535);

        object _lock = new object();
        Queue<ArraySegment<byte>> _sendQueue = new Queue<ArraySegment<byte>>();
        List<ArraySegment<byte>> _pendingList = new List<ArraySegment<byte>>();

        SocketAsyncEventArgs _sendArgs = new SocketAsyncEventArgs();
        SocketAsyncEventArgs _recvArgs = new SocketAsyncEventArgs();

        public abstract void OnConnected(EndPoint endPoint);
        public abstract int OnRecv(ArraySegment<byte> buffer); // buffer를 통해 얼마만큼의 데이터를 처리했는지 반환
        public abstract void OnSend(int numOfBytes);
        public abstract void OnDisconnected(EndPoint endPoint); 

        void Clear()
        {
            lock (_lock)
            {
                _sendQueue.Clear();
                _pendingList.Clear();
            }
        }
        public void Start(Socket socket)
        {
            _socket = socket;
            
            _recvArgs.Completed += new EventHandler<SocketAsyncEventArgs>(OnRecvCompleted);
            _sendArgs.Completed += new EventHandler<SocketAsyncEventArgs>(OnSendCompleted);

            RegisterReceive();
        }
        public void Send(List<ArraySegment<byte>> sendBuffList)
        {
            if (sendBuffList.Count == 0)
                return;

            lock (_lock)
            {
                foreach(ArraySegment<byte> sendBuff in sendBuffList)
                    _sendQueue.Enqueue(sendBuff);

                if (_pendingList.Count == 0)
                    RegisterSend();
                
            }
        }
        public void Send(ArraySegment<byte> sendBuff)
        {
            lock (_lock)
            {
                // 안에 이미 있으면 그냥 넣기만
                _sendQueue.Enqueue(sendBuff);
                // 1빠로 들어왔으면 보내기
                if (_pendingList.Count == 0)
                {
                    RegisterSend();
                }
            }
            //

        }
        void RegisterSend()
        {
            if (_disConnected == 1 || _socket == null)
                return;

            while(_sendQueue.Count > 0)
            {
                ArraySegment<byte> buff = _sendQueue.Dequeue();
                _pendingList.Add(buff);
            
            }
            _sendArgs.BufferList = _pendingList;

            try
            {
                bool pending = _socket.SendAsync(_sendArgs);
                if (pending == false)
                {
                    OnSendCompleted(null, _sendArgs);
                }
            }
            catch(Exception e)
            {
                Console.WriteLine(e.ToString());
            }
        }

        void OnSendCompleted(object sender, SocketAsyncEventArgs args)
        {
            lock (_lock)
            {
                if (args.BytesTransferred > 0 && args.SocketError == SocketError.Success)
                {
                    try
                    {
                        // 여기서 이미 성공적으로 모두 보냈기에, Buffer 초기화
                        _sendArgs.BufferList = null;
                        _pendingList.Clear();
                        OnSend(_sendArgs.BytesTransferred);

                        
                        if(_sendQueue.Count > 0)
                        {
                            RegisterSend();
                        }
                        
                        

                    }
                    catch (Exception e)
                    {
                        Console.WriteLine($"OnSendCompleted Failed {e}");
                    }
                }
            }
        }


        public void Disconnect()
        {
            // disConnected를 1로 바꿀건데, 이미 1이였으면 failure 
            if (Interlocked.Exchange(ref _disConnected, 1) == 1)
                return;

            OnDisconnected(_socket.RemoteEndPoint);
            _socket.Shutdown(SocketShutdown.Both);
            _socket.Close();
            Clear();
        }

        #region 네트워크 통신
        void RegisterReceive()
        {
            if (_disConnected == 1)
                return;

            _recvBuffer.Clean();
            ArraySegment<byte> segment = _recvBuffer.WriteSegment; // RecvBuffer의 FreeSize만큼 가져옴
            _recvArgs.SetBuffer(segment.Array, segment.Offset, segment.Count); 
            
            try 
            {
                bool pending = _socket.ReceiveAsync(_recvArgs);
                if (pending == false)
                {
                    OnRecvCompleted(null, _recvArgs);
                }
            }
            catch (Exception e)
            {
                Console.WriteLine(e.ToString());
            }
            
        }

        void OnRecvCompleted(object sender, SocketAsyncEventArgs args)
        {
            if(args.BytesTransferred > 0 && args.SocketError == SocketError.Success)
            {
                //TODO
                try
                {
                    // Write 커서 이동
                    if(_recvBuffer.OnWrite(args.BytesTransferred) == false) // 커서 이동했는데 이동 실패 (FreeSize보다 WriteSize가 큰 경우) 했다?
                    {
                        Disconnect();
                        return;
                    }

                    // 컨텐츠 쪽으로 데이터를 넘겨주고 얼마나 처리했는지 받는다.
                    int processLen = OnRecv(_recvBuffer.ReadSegment); // DataSize는 processLen과 같아야함
                    if (processLen < 0 || _recvBuffer.DataSize < processLen)
                    {
                        Disconnect();
                        return;
                    }

                    // Read 커서 이동
                    if(_recvBuffer.OnRead(processLen) == false) // 이동했는데, processLen이 DataSize보다 큰 경우 실패
                    {
                        Disconnect();
                        return;
                    }
                    RegisterReceive();
                }
                catch(Exception e)
                {
                    Console.WriteLine($"OnReceived Failed {e}");
                }
            }

            else
            {
                Disconnect();
            }
        }
        #endregion
    }
}
