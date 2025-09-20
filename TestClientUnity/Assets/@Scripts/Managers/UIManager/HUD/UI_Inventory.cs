using System.Collections.Generic;
using UnityEngine;

public class UI_Inventory : MonoBehaviour
{
    [SerializeField] private Transform gridParent;    // GridLayoutGroup 달린 오브젝트
    [SerializeField] private UI_InvetorySlot slotPrefab;
    [SerializeField] private int minSlots = 20;       // 서버가 빈 슬롯은 안보내도 되니, 최소 표시 개수

    private readonly Dictionary<int, UI_InvetorySlot> _views = new();

    void OnEnable()
    {
        var m = InventoryManager.Instance.Model;
        m.OnBulkRefresh += Rebuild;
        m.OnSlotChanged += UpdateSlot;
        Rebuild();
    }

    void OnDisable()
    {
        var m = InventoryManager.Instance.Model;
        m.OnBulkRefresh -= Rebuild;
        m.OnSlotChanged -= UpdateSlot;
    }

    private void Rebuild()
    {
        foreach (Transform c in gridParent) Destroy(c.gameObject);
        _views.Clear();

        var model = InventoryManager.Instance.Model;
        int cap = Mathf.Max(minSlots, model.GetMaxIndex() + 1);

        for (int i = 0; i < cap; i++)
        {
            var v = Instantiate(slotPrefab, gridParent);
            v.BindIndex(i);
            _views[i] = v;

            if (model.TryGet(i, out var s)) UpdateSlot(i, s);
            else v.Clear(); // 비어있음
        }
    }

    private void UpdateSlot(int index, InventorySlot slot)
    {
        if (_views.TryGetValue(index, out var v))
            v.SetData(slot);
    }
}
