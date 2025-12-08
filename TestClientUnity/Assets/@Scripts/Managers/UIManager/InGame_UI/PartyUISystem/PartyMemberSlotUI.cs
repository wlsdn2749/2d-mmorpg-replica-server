using Google.Protobuf.Protocol;
using Packet;
using TMPro;
using UnityEngine;
using UnityEngine.UI;

public class PartyMemberSlotUI : MonoBehaviour
{
    [SerializeField] TMP_Text _levelText;          // Text_MemberLevel
    [SerializeField] TMP_Text _nameText;           // Text_MemberName
    [SerializeField] Image _leaderIcon;           // 왕관 이미지 (없으면 null 가능)
    [SerializeField] Button _button;               // 슬롯 버튼

    int _pid;
    bool _isSelf;
    MyPartyViewUI _owner;

    public void SetOwner(MyPartyViewUI owner)
    {
        _owner = owner;
        if (_button != null)
        {
            _button.onClick.RemoveAllListeners();
            _button.onClick.AddListener(OnClick);
        }
    }

    public void SetEmpty()
    {
        _pid = -1;
        _isSelf = false;

        _levelText.text = "";
        _nameText.text = "- 빈 슬롯 -";

        if (_leaderIcon)
            _leaderIcon.gameObject.SetActive(false);

        if (_button)
            _button.interactable = false;
    }

    public void Bind(PartyMemberStatusInfo info, bool iAmLeader, int leaderId)
    {
        _pid = info.PlayerId;
        _isSelf = (info.PlayerId == NetDebug.MyPlayerId);

        // 
        Debug.Log($"[PartySlot] Bind: pid={info.PlayerId}, name='{info.PlayerName}', lv={info.Level}, isLeader={info.IsLeader}");

        // 이름
        if (string.IsNullOrEmpty(info.PlayerName))
        {
            // 이름이 비어 있으면 일단 디버깅용으로 PID라도,,
            _nameText.text = $"Player {info.PlayerId}";
        }
        else
        {
            _nameText.text = info.PlayerName;
        }

        // 레벨
        _levelText.text = $"Lv.{info.Level}";

        // 파티장 아이콘
        if (_leaderIcon)
            _leaderIcon.gameObject.SetActive(info.PlayerId == leaderId);

        // 버튼 인터랙션 (리더가 자기 제외 멤버만 클릭 가능)
        if (_button)
            _button.interactable = iAmLeader && !_isSelf;
    }

    public void Refresh()
    {
        if (!PartyState.Instance.Members.TryGetValue(_pid, out var m))
            return;

        _levelText.text = $"Lv.{m.Level}";
        // HP 등을 나중에 추가하면 여기서 갱신
    }

    void OnClick()
    {
        if (_owner != null && _pid >= 0)
            _owner.OnMemberSlotClicked(_pid, _isSelf);
    }
}
