using Google.Protobuf.Protocol;
using Packet;
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

    // === 요청 ===
    public void RequestInventory()
    {
        var req = new C_InventoryRequest();
        NetworkManager.Instance.Send(ServerPacketManager.MakeSendBuffer(req));
        Debug.Log("[Inv] C_InventoryRequest sent");
    }

    public void RequestUseItem(int slotIndex)
    {
        if (!Model.TryGet(slotIndex, out var slot) || slot == null || slot.IsEmpty)
        {
            Debug.LogWarning($"[Inv] Tried to use empty slot {slotIndex}");
            return;
        }

        var req = new C_ItemUseRequest { SlotIndex = slotIndex };
        NetworkManager.Instance.Send(ServerPacketManager.MakeSendBuffer(req));
        Debug.Log($"[Inv] C_ItemUseRequest sent: slot={slotIndex}");
    }
}
