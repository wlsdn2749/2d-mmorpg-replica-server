using System;
using System.Collections.Generic;
using UnityEngine;
//[Serializable]
//public class ItemDef   // 아이템 DB 엔트리
//{
//    public int itemId;
//    public string displayName;
//    public bool stackable;
//    public int maxStack;
//    public string iconPath; // Resources/Icon/xxx
//    public string type;     // "consumable", "equip", etc.
//}

//[CreateAssetMenu(menuName = "DB/ItemDatabase")]
//public class ItemDatabase : ScriptableObject
//{
//    public List<ItemDef> items = new();
//    private Dictionary<int, ItemDef> _map;

//    public void Init()
//    {
//        _map = new Dictionary<int, ItemDef>(items.Count);
//        foreach (var it in items) _map[it.itemId] = it;
//    }

//    public ItemDef Get(int id) => (_map != null && _map.TryGetValue(id, out var def)) ? def : null;
//}

public class InventoryModel
{
    // key = slotIndex
    private readonly Dictionary<int, InventorySlot> _slots = new();

    public event Action OnBulkRefresh;                    // 전체 새로고침
    public event Action<int, InventorySlot> OnSlotChanged; // 개별 슬롯 변경

    public bool TryGet(int index, out InventorySlot s) => _slots.TryGetValue(index, out s);
    public int GetMaxIndex() => _slots.Count == 0 ? -1 : Math.Max(-1, MaxKey());

    public void ApplySnapshot(IList<InventorySlot> fromServer)
    {
        _slots.Clear();
        foreach (var s in fromServer)
            _slots[s.slotIndex] = Clone(s);
        OnBulkRefresh?.Invoke();
    }

    public void Upsert(InventorySlot s)
    {
        _slots[s.slotIndex] = Clone(s);
        OnSlotChanged?.Invoke(s.slotIndex, _slots[s.slotIndex]);
    }

    private InventorySlot Clone(InventorySlot s) =>
        new InventorySlot { slotIndex = s.slotIndex, itemId = s.itemId, count = s.count, isQuickslot = s.isQuickslot };

    private int MaxKey()
    {
        int max = -1;
        foreach (var k in _slots.Keys) if (k > max) max = k;
        return max;
    }
}
