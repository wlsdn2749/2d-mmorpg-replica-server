using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Net.Sockets;
using System.Text;

namespace ServerCore
{
    public class Connector
    {
        Func<Session> _sessionFactory;
        public void Connect(IPEndPoint endPoint, Func<Session> sessionFactory, int count = 1)
        {

            for (int i = 0; i< count; i++)
            {
                // 휴대폰 설정
                Socket socket = new Socket(endPoint.AddressFamily, SocketType.Stream, ProtocolType.Tcp);
                _sessionFactory += sessionFactory;
                
               
                SocketAsyncEventArgs connArgs = new SocketAsyncEventArgs();
                connArgs.Completed += OnConnectCompleted;
                connArgs.RemoteEndPoint = endPoint;
                connArgs.UserToken = socket;
                
                RegisterConnect(connArgs);
            }
            
        }

        void RegisterConnect(SocketAsyncEventArgs args)
        {
            // as의 경우 Socket type이 아닌 경우 null 발생
            Socket socket = args.UserToken as Socket;
            if (socket == null)
                return;
            
            bool pending = socket.ConnectAsync(args);
            if(pending == false) // 바로 연결가능 한경우
            {
                OnConnectCompleted(null, args);
            }
        }

        void OnConnectCompleted(object sender, SocketAsyncEventArgs args)
        {
            if (args.SocketError == SocketError.Success)
            {
                Session session = _sessionFactory.Invoke();
                session.Start(args.ConnectSocket);
                session.OnConnected(args.RemoteEndPoint);
            }
            else
            {
                Console.WriteLine($"OnConnectCompleted Fail: {args.SocketError}");
            }
        }
    }
}
