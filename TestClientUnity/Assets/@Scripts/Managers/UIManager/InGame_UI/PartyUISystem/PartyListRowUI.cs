using Google.Protobuf.Protocol;
using TMPro;
using UnityEngine;
using UnityEngine.EventSystems;
using UnityEngine.UI;

public class PartyListRowUI : MonoBehaviour, IPointerClickHandler
{
    [SerializeField] TMP_Text _partyNameText;   // Text_PartyName
    [SerializeField] TMP_Text _partySizeText;   // Text_PartySize
    [SerializeField] Image _selectedHighlight;  // 선택 표시용 (없으면 null로 두고 무시해도 됨)

    private PartyWindowUI _owner;
    private int _partyId;

    public void Bind(PartyWindowUI owner, PartyInfo info)
    {
        _owner = owner;
        _partyId = info.PartyId;

        _partyNameText.text = string.IsNullOrEmpty(info.PartyName)
            ? $"파티 {_partyId}"
            : info.PartyName;

        _partySizeText.text = $"{info.CurMemberCount}/{info.MaxMemberCount}";

        SetSelected(false);
    }
    public void OnPointerClick(PointerEventData eventData)
    {
        OnClickRow();
    }   
    public void OnClickRow()
    {
        _owner?.OnRowSelected(this, _partyId);
    }

    public void SetSelected(bool selected)
    {
        if (_selectedHighlight != null)
            _selectedHighlight.gameObject.SetActive(selected);
    }
}
