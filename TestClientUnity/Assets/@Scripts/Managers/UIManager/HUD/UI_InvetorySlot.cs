using TMPro;
using UnityEngine;
using UnityEngine.UI;

public class UI_InvetorySlot : MonoBehaviour
{
    [SerializeField] private Image icon;
    [SerializeField] private TMP_Text countText;
    [SerializeField] private Button _useButton;
    [SerializeField] private ItemDatabase itemDB; // 아이콘 DB (ScriptableObject)

    private int _index = -1;

    void Awake()
    {
        if (_useButton) _useButton.onClick.AddListener(OnClickUseItem);
    }

    public void BindIndex(int idx) => _index = idx;

    public void SetData(InventorySlot s)
    {
        if (s == null || s.IsEmpty)
        {
            Clear();
            return;
        }

        // 아이콘
        if (itemDB != null && itemDB.TryGetIcon(s.itemId, out var sp))
        {
            icon.sprite = sp;
            icon.enabled = true;
        }
        else
        {
            icon.enabled = false;
        }

        // 수량
        countText.text = (s.count > 1) ? s.count.ToString() : "";
    }

    public void Clear()
    {
        icon.enabled = false;
        countText.text = "";
    }

    private void OnClickUseItem()
    {
        InventoryManager.Instance?.RequestUseItem(_index);
    }
}
