using Google.Protobuf.Protocol;
using Packet;
using System.Linq;
using TMPro;
using UnityEngine;
using UnityEngine.UI;

public class MyPartyViewUI : MonoBehaviour
{
    [Header("Basic Info UI")]
    [SerializeField] private TMP_Text _partyNameText;
    [SerializeField] private Transform _slotParent;   // Slot_PartyMember

    [Header("Member/Leader UI")]
    [SerializeField] private GameObject _viewNormalMemberUI;    // View_NormalMember_MyPartyUI
    [SerializeField] private GameObject _viewLeaderUI;          // View_PartyLeader_MyPartyUI

    [Header("Leader UI Buttons")]
    [SerializeField] private Button _buttonLeaderExitParty;
    [SerializeField] private Button _buttonOpenJoinRequestWindow;


    [Header("Normal Member UI Buttons")]
    [SerializeField] private Button _buttonNormalExitParty;

    [Header("Sub Windows")]
    [SerializeField] private PartyJoinRequestWindow _joinRequestWindow;      // Panel_JoinRequestWindow
    [SerializeField] private LeaderActionWindowUI _leaderActionWindow;       // Panel_LeaderActionWindow

    [Header("Member Slots")]
    [SerializeField] private PartyMemberSlotUI[] _memberSlots;
    void Awake()
    {
        // 모두 꺼두고 필요할 때 켜기
        _viewLeaderUI.SetActive(false);
        _viewNormalMemberUI.SetActive(false);


        // 버튼 연결
        if (_buttonLeaderExitParty)
            _buttonLeaderExitParty.onClick.AddListener(OnExitParty);

        if (_buttonNormalExitParty)
            _buttonNormalExitParty.onClick.AddListener(OnExitParty);

        if (_buttonOpenJoinRequestWindow)
            _buttonOpenJoinRequestWindow.onClick.AddListener(OpenJoinRequestWindow);

        foreach (var slot in _memberSlots)
            slot?.SetOwner(this);
    }

    void OnEnable()
    {
        // 파티 상태 갱신 이벤트
        PartyState.Instance.OnPartyChanged += Refresh;
        Refresh();
    }

    void OnDisable()
    {
        if (PartyState.Instance != null)
            PartyState.Instance.OnPartyChanged -= Refresh;
    }

    void Refresh()
    {
        var ps = PartyState.Instance;

        if (!ps.InParty)
        {
            Debug.Log("[MyPartyUI] No party, closing.");
            gameObject.SetActive(false);
            return;
        }
        Debug.Log($"[MyPartyUI] InParty. PartyId={ps.PartyId}, LeaderId={ps.PartyLeaderId}, MyId={NetDebug.MyPlayerId}");

        // 파티 이름
        _partyNameText.text = ps.PartyName;

        // 파티장/일반 UI 선택
        bool isLeader = (ps.PartyLeaderId == NetDebug.MyPlayerId);
        _viewLeaderUI.SetActive(isLeader);
        _viewNormalMemberUI.SetActive(!isLeader);

        // 파티원 슬롯들 업데이트
        UpdateMemberSlots();
    }

    void UpdateMemberSlots()
    {
        var ps = PartyState.Instance;

        // 리더 먼저, 그 다음 나머지
        var members = ps.Members.Values
            .OrderByDescending(m => m.IsLeader)
            .ThenBy(m => m.PlayerId)
            .ToList();

        Debug.Log($"[MyPartyUI] UpdateMemberSlots: memberCount={members.Count}");
        // 예시: _memberSlots 배열에 4개 슬롯을 인스펙터에서 넣어둔 상태라고 가정
        for (int i = 0; i < _memberSlots.Length; i++)
        {
            if (i < members.Count)
            {
                _memberSlots[i].Bind(members[i], ps.PartyLeaderId == NetDebug.MyPlayerId, ps.PartyLeaderId);
            }
            else
                _memberSlots[i].SetEmpty();
        }
    }

    void OnExitParty()
    {
        PartyNet.LeaveSelf();  
    }
    public void OnMemberSlotClicked(int pid, bool isSelf)
    {
        var ps = PartyState.Instance;

        // 리더가 아니면 클릭 동작 없음
        if (ps.PartyLeaderId != NetDebug.MyPlayerId)
            return;

        // 자기 자신은 액션 창 안 띄움
        if (isSelf)
            return;

        _leaderActionWindow.Open(pid);
    }
    void OpenJoinRequestWindow()
    {
        var ps = PartyState.Instance;
        if (!ps.InParty || !ps.PartyId.HasValue)
        {
            Debug.Log($"[MyPartyUI]{ps.InParty},{ps.PartyId.Value}");
            return; // 아직 partyId 모르면 그냥 무시
        }
        Debug.Log("[MyPartyUI] Opening Join Request Window");

        _joinRequestWindow.Open();
        PartyNet.RequestJoinRequestList(ps.PartyId.Value);
    }
}
