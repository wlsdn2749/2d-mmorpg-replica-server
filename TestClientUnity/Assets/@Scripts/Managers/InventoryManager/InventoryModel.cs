using Google.Protobuf.Protocol;
using System;
using System.Collections.Generic;
using UnityEngine;


public class InventoryModel
{
    // key = slotIndex
    private readonly Dictionary<int, InventorySlot> _slots = new();
    public event Action OnBulkRefresh;                    // 전체 새로고침
    public event Action<int, InventorySlot> OnSlotChanged; // 개별 슬롯 변경
    public event Action<IReadOnlyDictionary<int, InventorySlot>> OnChanged;


    public bool TryGet(int index, out InventorySlot s) => _slots.TryGetValue(index, out s);
    public int GetMaxIndex() => _slots.Count == 0 ? -1 : Math.Max(-1, MaxKey());


    public void ApplySnapshot(IList<InventorySlotInfo> fromServer)
    {
        _slots.Clear();
        foreach (var s in fromServer)
            _slots[s.SlotIndex] = FromInfo(s);

        OnBulkRefresh?.Invoke();
        OnChanged?.Invoke(_slots);
    }
    public void ApplySnapshot(IList<InventorySlot> fromServer)
    {
        _slots.Clear();
        foreach (var s in fromServer)
            _slots[s.slotIndex] = Clone(s);
        OnBulkRefresh?.Invoke();
    }
    public void ApplyDelta(IEnumerable<InventorySlotInfo> changed)
    {
        foreach (var s in changed)
        {
            if (s.Count > 0)
            {
                var slot = FromInfo(s);
                _slots[slot.slotIndex] = slot;
                OnSlotChanged?.Invoke(slot.slotIndex, slot);
            }
            else
            {
                _slots.Remove(s.SlotIndex);
                OnSlotChanged?.Invoke(s.SlotIndex, null);
            }
        }
        OnChanged?.Invoke(_slots);
    }
    public void Upsert(InventorySlot s)
    {
        _slots[s.slotIndex] = Clone(s);
        OnSlotChanged?.Invoke(s.slotIndex, _slots[s.slotIndex]);
    }
    private static InventorySlot FromInfo(InventorySlotInfo s) =>
      new InventorySlot
      {
          slotIndex = s.SlotIndex,
          itemId = s.ItemId,
          count = s.Count,
          isQuickslot = s.IsQuickslot
      };
    private InventorySlot Clone(InventorySlot s) =>
        new InventorySlot 
        { 
            slotIndex = s.slotIndex, 
            itemId = s.itemId, 
            count = s.count, 
            isQuickslot = s.isQuickslot 
        };

    private int MaxKey()
    {
        int max = -1;
        foreach (var k in _slots.Keys) if (k > max) max = k;
        return max;
    }
}
