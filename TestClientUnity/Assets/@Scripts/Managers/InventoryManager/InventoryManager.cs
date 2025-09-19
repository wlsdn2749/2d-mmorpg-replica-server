using Google.Protobuf.Protocol;
using Packet;
using System.Collections.Generic;
using UnityEngine;

public class InventoryManager : MonoBehaviour
{
    public static InventoryManager Instance { get; private set; }
    public InventoryModel Model { get; private set; } = new();

    void Awake()
    {
        if (Instance && Instance != this) { Destroy(gameObject); return; }
        Instance = this;
    }

    // --- 요청: 스냅샷 조회 ---
    public void RequestInventory()
    {
        var req = new C_InventoryRequest();
        var send = ServerPacketManager.MakeSendBuffer(req);
        NetworkManager.Instance.Send(send);
        Debug.Log("[Inv] C_InventoryRequest sent");
    }
}