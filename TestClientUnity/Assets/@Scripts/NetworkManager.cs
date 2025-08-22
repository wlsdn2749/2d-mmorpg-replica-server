using System;
using System.Collections;
using System.Collections.Generic;
using System.Net;
using Google.Protobuf.Protocol;
using Packet;
using ServerCore;
using Unity.Collections;
using UnityEngine;
using System.Net.Sockets;
using System.Threading.Tasks;
using Google.Protobuf;

public class NetworkManager : MonoBehaviour
{
    public static NetworkManager Instance { get; private set; }
    private ServerSession _session = new ServerSession();

    void Awake()
    {
        // 중복 인스턴스 방지
        if (Instance != null && Instance != this)
        {
            Destroy(gameObject);
            return;
        }
        
        // OnDestoryLoad ... 씬 바뀔때 사라지지 않게 처리하는 것도 필요할 수 있음.
        Instance = this;
        Application.targetFrameRate = 60;
    }

    

    public void Send(ArraySegment<byte> sendBuff)
    {
        _session.Send(sendBuff);
    }
    
    async Task Start()
    {
        string  host = "serotina.gyu.be";
        int     port = 6201;
        
        Debug.Log("NetworkSession Start");
        IPAddress ipAddr = (await Dns.GetHostAddressesAsync(host))[0];
        IPEndPoint endPoint = new IPEndPoint(ipAddr, port);

        Connector connector = new Connector();
        

        connector.Connect(endPoint,
            () => { return _session; },
            1);

    }



    // Update is called once per frame
    void Update()
    {
        List<IMessage> packetList = PacketQueue.Instance.PopAll();
        
        foreach(IMessage pkt in packetList)
            ServerPacketManager.Instance.HandlePacket(_session, pkt);
        
    }

    void OnApplicationQuit()
    {

        // 평문 TCP라면 바로 Close 하면 패킷이 유실될 수 있으므로
        // 100ms 정도 WaitForEndOfFrame 또는 Thread.Sleep 후 Close 추천
        _session.Disconnect();
    }

    
    // void OnDestroy()
    // {
    //     Debug.Log("NetworkManager Destroy - Cleaning up network resources");
    //     if (_session != null)
    //     {
    //         _session.Disconnect();
    //         _session = null;
    //     }
    // }
}
