using Google.Protobuf.Protocol;
using System;
using System.Collections.Generic;

public class InventoryModel
{
    private readonly Dictionary<int, InventorySlot> _slots = new();

    public event Action OnBulkRefresh;                     // 전체 리빌드
    public event Action<int, InventorySlot> OnSlotChanged; // 특정 슬롯만 갱신

    public bool TryGet(int index, out InventorySlot s) => _slots.TryGetValue(index, out s);
    public int GetMaxIndex() => _slots.Count == 0 ? -1 : MaxKey();

    // 서버에서 전체 스냅샷 내려줄 때
    public void ApplySnapshot(IList<InventorySlot> fromServer)
    {
        _slots.Clear();
        foreach (var s in fromServer)
            _slots[s.slotIndex] = Clone(s);

        OnBulkRefresh?.Invoke();
    }

    // 서버에서 일부 슬롯 변경 내려줄 때
    public void ApplyDelta(IEnumerable<InventorySlotInfo> changed)
    {
        foreach (var s in changed)
        {
            if (s.Count > 0)
            {
                _slots[s.SlotIndex] = new InventorySlot
                {
                    slotIndex = s.SlotIndex,
                    itemId = s.ItemId,
                    count = s.Count,
                    isQuickslot = s.IsQuickslot
                };
            }
            else
            {
                _slots.Remove(s.SlotIndex);
            }

            OnSlotChanged?.Invoke(
                s.SlotIndex,
                _slots.ContainsKey(s.SlotIndex) ? _slots[s.SlotIndex] : null
            );
        }
    }

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
        foreach (var k in _slots.Keys)
            if (k > max) max = k;
        return max;
    }
}
