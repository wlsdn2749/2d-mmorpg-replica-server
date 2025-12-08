using Google.Protobuf.Protocol;
using TMPro;
using UnityEngine;
using UnityEngine.UI;

public class PartyJoinRequesterSlot : PoolAble
{
    [SerializeField] TMP_Text _levelText;      // Text_RequesterLevel
    [SerializeField] TMP_Text _nameText;       // Text_RequesterName
    [SerializeField] Button _buttonAccept;     // Button_Accept
    [SerializeField] Button _buttonReject;     // Button_Reject

    int _partyId;
    int _playerId;

    public void Bind(int partyId, PartyJoinRequesterInfo info)
    {
        _partyId = partyId;
        _playerId = info.PlayerId;

        _levelText.text = $"Lv.{info.Level}";
        _nameText.text = info.PlayerName;

        _buttonAccept.onClick.RemoveAllListeners();
        _buttonReject.onClick.RemoveAllListeners();

        _buttonAccept.onClick.AddListener(OnClickAccept);
        _buttonReject.onClick.AddListener(OnClickReject);
    }

    void OnClickAccept()
    {
        PartyNet.RespondJoinAsLeader(_partyId, _playerId, true);
        // 일단 버튼 잠궈두기 (서버 브로드캐스트 오면 실제 파티 상태 갱신됨)
        _buttonAccept.interactable = false;
        _buttonReject.interactable = false;

        ReleaseObject();
    }

    void OnClickReject()
    {
        PartyNet.RespondJoinAsLeader(_partyId, _playerId, false);
        _buttonAccept.interactable = false;
        _buttonReject.interactable = false;
        ReleaseObject();
    }
}
