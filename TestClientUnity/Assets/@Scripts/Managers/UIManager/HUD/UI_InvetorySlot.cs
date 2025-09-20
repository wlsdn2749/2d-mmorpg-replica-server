using TMPro;
using UnityEngine;
using UnityEngine.UI;

public class UI_InvetorySlot : MonoBehaviour
{
    [SerializeField] private Image icon;
    [SerializeField] private TMP_Text countText;
    //[SerializeField] private GameObject quickBadge;

    private int _index;

    public void BindIndex(int idx) => _index = idx;

    public void SetData(InventorySlot s)
    {
        // 프로젝트에 맞게 아이콘 로딩 로직 연결(여기선 간단 처리)
        icon.enabled = !s.IsEmpty;
        countText.text = (s.count > 1) ? s.count.ToString() : "";
        //quickBadge.SetActive(s.isQuickslot);
    }

    public void Clear()
    {
        icon.enabled = false;
        countText.text = "";
        //quickBadge.SetActive(false);
    }
}
