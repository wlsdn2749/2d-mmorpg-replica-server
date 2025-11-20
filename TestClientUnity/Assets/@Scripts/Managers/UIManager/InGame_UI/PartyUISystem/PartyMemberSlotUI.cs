using Google.Protobuf.Protocol;
using Packet;
using TMPro;
using UnityEngine;
using UnityEngine.UI;

public class PartyMemberSlotUI : MonoBehaviour
{
    [SerializeField] TMP_Text _levelText;
    [SerializeField] TMP_Text _nameText;
    [SerializeField] GameObject _leaderIcon; // 리더일 때만 켜기
    [SerializeField] Button _button;   // 리더일 때만 클릭 가능하게 쓰고 싶으면

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
        if (_button) _button.interactable = false;
    }
    /// <param name="iAmLeader">내가 파티장인지 여부</param>
    /// <param name="leaderId">현재 파티장 pid</param>
    public void Bind(PartyMemberStatusInfo info, bool iAmLeader, int leaderId)
    {
        _pid = info.PlayerId;
        _isSelf = (info.PlayerId == NetDebug.MyPlayerId);

        bool isLeader = (info.PlayerId == leaderId); // 리더면 
        if (_leaderIcon)                             // 왕관 이미지 on/off
            _leaderIcon.SetActive(isLeader);      


        _nameText.text =  $"{info.PlayerName}";
        _levelText.text = $"Lv.{info.Level}";

        if (_button)
            _button.interactable = iAmLeader && !_isSelf; // 리더가 자기 제외 멤버만 클릭 가능
    }

    public void Refresh()
    {
        if (!PartyState.Instance.Members.TryGetValue(_pid, out var m)) return;
        _levelText.text = $"Lv.{m.Level}";
        // HP바 같은 거 추가하면 여기서 갱신
    }

    void OnClick()
    {
        if (_owner != null && _pid >= 0)
            _owner.OnMemberSlotClicked(_pid, _isSelf);
    }
}
