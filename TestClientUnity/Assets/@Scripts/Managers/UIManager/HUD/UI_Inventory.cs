using UnityEngine;
using System.Collections.Generic;

public class InventoryUI : MonoBehaviour
{
    [SerializeField] private UI_InvetorySlot slotPrefab;
    [SerializeField] private Transform gridParent;

    private readonly Dictionary<int, UI_InvetorySlot> _uiSlots = new();

    void OnEnable()
    {
        InventoryManager.Instance.Model.OnBulkRefresh += RebuildAll;
        InventoryManager.Instance.Model.OnSlotChanged += UpdateSlot;
    }

    void OnDisable()
    {
        InventoryManager.Instance.Model.OnBulkRefresh -= RebuildAll;
        InventoryManager.Instance.Model.OnSlotChanged -= UpdateSlot;
    }

    private void RebuildAll()
    {
        foreach (Transform child in gridParent)
            Destroy(child.gameObject);
        _uiSlots.Clear();

        var model = InventoryManager.Instance.Model;
        int maxIndex = model.GetMaxIndex();
        for (int i = 0; i <= maxIndex; i++)
        {
            var slot = Instantiate(slotPrefab, gridParent);
            slot.BindIndex(i);
            _uiSlots[i] = slot;

            if (model.TryGet(i, out var data))
                slot.SetData(data);
            else
                slot.Clear();
        }
    }

    private void UpdateSlot(int index, InventorySlot slot)
    {
        if (!_uiSlots.TryGetValue(index, out var ui)) return;

        if (slot == null || slot.IsEmpty)
            ui.Clear();
        else
            ui.SetData(slot);
    }
}
