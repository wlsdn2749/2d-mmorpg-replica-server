using Google.Protobuf.Protocol;
using Packet;
using System.Collections.Generic;
using TMPro;
using UnityEngine;
using UnityEngine.UI;

public class PartyJoinRequestWindow : MonoBehaviour
{
    [Header("List")]
    [SerializeField] Transform _content;                 // Scroll View/Viewport/Content
    [SerializeField] PartyJoinRequesterSlot _slotPrefab; // Slot_Requester 프리팹

    [Header("Optional UI")]
    [SerializeField] Button _buttonClose;                // 닫기 버튼 있으면 연결

    readonly List<PartyJoinRequesterSlot> _slots = new();

    void Awake()
    {
        if (_buttonClose != null)
            _buttonClose.onClick.AddListener(Close);

        // 기본은 꺼두고, MyParty에서 열어줄 것
        gameObject.SetActive(false);
    }

    void OnEnable()
    {
        PartyState.Instance.OnJoinRequestList += OnJoinRequestList;
        PartyState.Instance.OnJoinNotifyToLeader += OnJoinNotify;

        // 창이 열릴 때 내 파티 기준으로 최신 리스트 요청
        var ps = PartyState.Instance;
        if (ps.PartyId.HasValue)
        {
            PartyNet.RequestJoinRequestList(ps.PartyId.Value);
        }
    }

    void OnDisable()
    {
        if (PartyState.Instance == null) return;
        PartyState.Instance.OnJoinRequestList -= OnJoinRequestList;
        PartyState.Instance.OnJoinNotifyToLeader -= OnJoinNotify;
    }


    void Clear()
    {
        foreach (var s in _slots)
        {
            if (s) Destroy(s.gameObject);
        }
        _slots.Clear();
    }

    // 서버가 전체 요청 리스트 내려줄 때
    void OnJoinRequestList(int partyId, List<PartyJoinRequesterInfo> requesters)
    {
        Clear();

        foreach (var info in requesters)
        {
            var slot = Instantiate(_slotPrefab, _content);
            slot.Bind(partyId, info);
            _slots.Add(slot);
        }

        // 요청이 하나도 없으면 자동으로 창 닫을지 여부는 취향
        // if (requesters.Count == 0) Close();
    }

    // 누군가 새로 가입 요청을 했다는 알림 (리더만 받음)
    void OnJoinNotify(int joinPlayerId, int partyId, int leaderId)
    {
        // 내가 리더가 아니면 무시
        if (leaderId != NetDebug.MyPlayerId)
            return;

        // 창이 이미 열려 있으면 최신 리스트 다시 요청
        if (gameObject.activeSelf && PartyState.Instance.PartyId == partyId)
        {
            PartyNet.RequestJoinRequestList(partyId);
        }

        // 여기서 "새 가입 요청이 있습니다" 같은 작은 이펙트/아이콘 표시도 가능
    }

    public void Open()
    {
        var ps = PartyState.Instance;
        if (!ps.PartyId.HasValue)
            return;

        gameObject.SetActive(true);
        PartyNet.RequestJoinRequestList(ps.PartyId.Value);
    }

    public void Close()
    {
        gameObject.SetActive(false);
    }
}
