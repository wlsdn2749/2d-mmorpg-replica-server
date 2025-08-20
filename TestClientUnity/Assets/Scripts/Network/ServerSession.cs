using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using Packet;
using ServerCore;
using UnityEngine;
using Google.Protobuf.Protocol;

class ServerSession : PacketSession
{
    public override void OnConnected(EndPoint endPoint)
    {
        Debug.Log($"onConnected : {endPoint}");
        Console.WriteLine($"onConnected : {endPoint}");

    }

    public override void OnDisconnected(EndPoint endPoint)
    {
        if (endPoint != null)
            Console.WriteLine($"onDisconnected : {endPoint}");
        else
            Console.WriteLine("onDisconnected : (null endpoint)");
    }

	
    public override void OnRecvPacket(ArraySegment<byte> buffer)
    {
        ServerPacketManager.Instance.OnRecvPacket(this, buffer, (s, p) => PacketQueue.Instance.Push(p));
    }

    public override void OnSend(int numOfBytes)
    {
        //Console.WriteLine($"Transfered bytes: {numOfBytes}");
    }
    
}
