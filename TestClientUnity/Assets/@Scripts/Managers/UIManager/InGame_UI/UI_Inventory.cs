using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

public class InventoryUI : MonoBehaviour
{
    [SerializeField] private UI_InvetorySlot slotPrefab;
    [SerializeField] private Transform gridParent;

    [Header("Grid config")]
    [SerializeField] private int capacity = 30;   // 전체 슬롯 개수 (원하는 값)
    [SerializeField] private int columns = 5;     // 한 줄에 몇 칸

    private readonly Dictionary<int, UI_InvetorySlot> _uiSlots = new();

    void OnEnable()
    {
        InventoryManager.Instance.Model.OnBulkRefresh += RebuildAll;
        InventoryManager.Instance.Model.OnSlotChanged += UpdateSlot;
        RebuildAll(); // 켜질 때도 한 번
    }

    void OnDisable()
    {
        InventoryManager.Instance.Model.OnBulkRefresh -= RebuildAll;
        InventoryManager.Instance.Model.OnSlotChanged -= UpdateSlot;
    }

    private void RebuildAll()
    {
        // 기존 전부 제거
        foreach (Transform c in gridParent) Destroy(c.gameObject);
        _uiSlots.Clear();

        // 고정 개수로 슬롯 생성
        for (int i = 0; i < capacity; i++)
        {
            var slot = Instantiate(slotPrefab, gridParent);
            slot.BindIndex(i);
            _uiSlots[i] = slot;

            if (InventoryManager.Instance.Model.TryGet(i, out var data) && !data.IsEmpty)
                slot.SetData(data);
            else
                slot.Clear();
        }

        // Grid 설정 맞추기 (아래 B 섹션 참고)
        ApplyGridSettings();
    }

    private void UpdateSlot(int index, InventorySlot slot)
    {
        if (!_uiSlots.TryGetValue(index, out var ui)) return;

        if (slot == null || slot.IsEmpty) ui.Clear();
        else ui.SetData(slot);
    }

    private void ApplyGridSettings()
    {
        var grid = gridParent.GetComponent<GridLayoutGroup>();
        if (!grid) return;

        grid.constraint = GridLayoutGroup.Constraint.FixedColumnCount;
        grid.constraintCount = columns;
        // 셀 사이즈/스페이싱은 프리팹/디자인에 맞게 에디터에서 지정해둔 값 사용
    }
}
