using TMPro;
using UnityEngine;
using UnityEngine.UI;

public class UI_InvetorySlot : MonoBehaviour
{
    [SerializeField] private ItemDatabase itemDB;  // ✅ 주입
    [SerializeField] private Image icon;
    [SerializeField] private TMP_Text countText;
    [SerializeField] private Button _useButton;

    private int _index;

    void Awake()
    {
        if (_useButton) _useButton.onClick.AddListener(OnClickUseItem);
    }

    public void BindIndex(int idx) => _index = idx;

    public void SetData(InventorySlot s)
    {
        if (s == null || s.count <= 0 || s.itemId <= 0)
        {
            Clear();
            return;
        }

        
        Sprite sp = null;
        if (itemDB != null && itemDB.TryGetIcon(s.itemId, out sp))
        {
            icon.sprite = sp;
            icon.enabled = true;
        }
        else
        {
            icon.enabled = false; // or 기본 스프라이트 지정
        }

        countText.text = (s.count > 1) ? s.count.ToString() : "";
        // quickBadge.SetActive(s.isQuickslot);
    }

    public void OnClickUseItem()
    {
        if (_index < 0) return;

        // 슬롯 조회
        if (!InventoryManager.Instance.Model.TryGet(_index, out var slot))
            return; // 슬롯 자체 없음 → 요청 안 보냄

        if (slot.itemId <= 0 || slot.count <= 0)
            return; // 빈 슬롯 → 요청 안 보냄
        InventoryManager.Instance?.RequestUseItem(_index);
    }

    public void Clear()
    {
        icon.enabled = false;
        countText.text = "";
    }

    // 선택: 외부에서 주입하고 싶으면
    public void InjectDB(ItemDatabase db) => itemDB = db;
}
