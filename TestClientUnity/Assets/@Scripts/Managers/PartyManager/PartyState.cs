using Google.Protobuf.Protocol;
using Packet;
using System;
using System.Collections.Generic;
using System.Linq;
using UnityEngine;

public sealed class PartyState
{
    public static PartyState Instance { get; } = new PartyState();

    public int? PartyId { get; private set; }
    public string PartyName { get; private set; }
    public int PartyLeaderId { get; private set; }
    public int MaxMemberCount { get; private set; }

    public int CurMemberCount => _members.Count;

    //  파티 여부는 멤버 수로 판단
    public bool InParty => _members.Count > 0;

    // playerId -> PartyMemberStatusInfo
    private readonly Dictionary<int, PartyMemberStatusInfo> _members = new();
    public IReadOnlyDictionary<int, PartyMemberStatusInfo> Members => _members;

    // 편의용 정렬 멤버 리스트 (리더 먼저)
    public IReadOnlyList<PartyMemberStatusInfo> MembersSorted =>
        _members.Values
            .OrderByDescending(m => m.IsLeader)
            .ThenBy(m => m.PlayerId)
            .ToList();

    // ===== 외부(UI 등) 구독용 이벤트 =====
    public event Action OnPartyChanged;                              // 멤버 증감/리더 변경/해산 등
    public event Action OnPartyStatusUpdated;                        // HP/레벨 등 상태값 갱신
    public event Action<int, string, int> OnInvite;                  // inviterPid, inviterName, partyId
    public event Action<bool, string> OnInviteResult;                // success, errorMessage
    public event Action<bool, string> OnCreateResult;                // success, message
    public event Action<bool, string> OnJoinResult;                  // success, message
    public event Action<int, int, int> OnJoinNotifyToLeader;         // joinPlayerId, partyId, leaderId
    public event Action<List<PartyInfo>> OnPartyList;                // 전체 파티 리스트
    public event Action<int, List<PartyJoinRequesterInfo>> OnJoinRequestList; // partyId, 요청자들
    public event Action OnKicked;                           // 강퇴(또는 타인에 의해 파티에서 제거)당했을 때 알림용 이벤트
    
    private bool _selfLeaveRequested = false;                        // 내가 직접 탈퇴 버튼을 눌렀는지 여부
    private PartyState() { }

    // ===== 서버 브로드캐스트 반영 =====
    public void ApplyBroadcast(S_BroadcastPartyUpdate pkt)
    {
        if (pkt == null) return;
        Debug.Log($"[PartyState] Broadcast updateType={pkt.UpdateType}, membersCount={pkt.Members.Count}");

        switch (pkt.UpdateType)
        {
            case EPartyUpdateType.PartyUpdateMemberJoin:
                bool iJoined = false;

                foreach (var m in pkt.Members)
                {
                    _members[m.PlayerId] = m;

                    // 내가 포함되어 있는지 체크
                    if (m.PlayerId == NetDebug.MyPlayerId)
                        iJoined = true;
                }

                RecomputeLeaderAndEmitChanged();

                //  내가 파티에 들어왔다고 브로드캐스트에서 확정된 시점인데,
                // 아직 PartyId/PartyName 메타가 비어있다면, 이제 PartyList를 요청해서 메타를 채운다.
                if (iJoined && (!PartyId.HasValue || string.IsNullOrEmpty(PartyName)))
                {
                    Debug.Log("[PartyState] I joined party but meta not set yet. Requesting PartyList.");
                    PartyNet.RequestPartyList();
                }
                break;

            case EPartyUpdateType.PartyUpdateMemberLeave:
                Debug.Log($"[PartyState] MemberLeave broadcast. membersCount={pkt.Members.Count}");

                int myId = NetDebug.MyPlayerId;
                bool stillInParty = false;

                // 서버가 보내준 members를 "현재 남아 있는 파티원 스냅샷"으로 사용
                _members.Clear();

                foreach (var m in pkt.Members)
                {
                    _members[m.PlayerId] = m;
                    if (m.PlayerId == myId)
                        stillInParty = true;

                    Debug.Log($"[PartyState]   member pid={m.PlayerId}, name={m.PlayerName}, isLeader={m.IsLeader}");
                }

                // 나는 더 이상 목록에 없거나, 아예 멤버가 0명이면 → 이 클라 기준에선 파티 끝
                if (!stillInParty || _members.Count == 0)
                {
                    Debug.Log("[PartyState] I am not in party anymore or no members left. ClearPartyInternal.");
                    ClearPartyInternal();
                }
                else
                {
                    // 남아 있는 멤버들 기준으로 리더 재계산 + OnPartyChanged 이벤트
                    RecomputeLeaderAndEmitChanged();
                }

                break;

            case EPartyUpdateType.PartyUpdateStatus:
                foreach (var m in pkt.Members)
                {
                    if (_members.TryGetValue(m.PlayerId, out var cur))
                    {
                        cur.Hp = m.Hp;
                        cur.MaxHp = m.MaxHp;
                        cur.Level = m.Level;
                        cur.IsLeader = m.IsLeader;
                        _members[m.PlayerId] = cur;
                    }
                    else
                    {
                        _members[m.PlayerId] = m;
                    }
                }
                RecomputeLeader();
                OnPartyStatusUpdated?.Invoke();
                break;

            case EPartyUpdateType.PartyUpdateDisbanded:
                ClearPartyInternal();
                break;

            case EPartyUpdateType.PartyUpdateDelegateLeader:
                foreach (var m in pkt.Members)
                {
                    if (_members.TryGetValue(m.PlayerId, out var cur))
                    {
                        cur.IsLeader = m.IsLeader;
                        _members[m.PlayerId] = cur;
                    }
                    else
                    {
                        // 안전빵: 혹시 목록에 없던 멤버가 오면 통째로 등록
                        _members[m.PlayerId] = m;
                    }
                }

                // 내부 LeaderId 재계산 + UI 갱신 이벤트
                RecomputeLeaderAndEmitChanged();
                break;
        }
    }

    // 파티강퇴 알림 처리   
    public void HandleKickedNotify(S_PartyKickedNotify n)
    {
        Debug.Log($"[PartyState] HandleKickedNotify: partyId={n.PartyId}, kickedPid={n.KickedPlayerId}, leaderId={n.LeaderId}");

        // 이 클라가 강퇴 대상인지 한 번 확인 (안전용)
        if (n.KickedPlayerId != NetDebug.MyPlayerId)
        {
            Debug.LogWarning($"[PartyState] KickedNotify for other player (myId={NetDebug.MyPlayerId})");
            return;
        }

        // 파티 상태 정리 → InParty=false, _members Clear, OnPartyChanged 호출
        ClearPartyInternal();

        // UI에 알림
        OnKicked?.Invoke();
    }

    // 파티 메타 채우기(리스트/가입 직후 등 필요 시)
    public void SetPartyMetaFromInfo(PartyInfo info)
    {
        if (info == null) return;

        PartyId = info.PartyId;
        PartyName = info.PartyName;
        MaxMemberCount = info.MaxMemberCount;
        PartyLeaderId = info.PartyLeaderId;

        _members.Clear();
        foreach (var m in info.Members)
            _members[m.PlayerId] = m;

        Debug.Log($"[PartyState] SetPartyMeta: partyId={PartyId}, name={PartyName}, members={_members.Count}");
        OnPartyChanged?.Invoke();
    }
    public void ConfirmJoined(int partyId)
    {
        PartyId = partyId;
        // 멤버/리더 정보는 이후 Broadcast나 리스트로 들어온 걸로 채워짐
        Debug.Log($"[PartyState] ConfirmJoined: partyId={partyId}");
    }

    public void ClearParty()
    {
        ClearPartyInternal();
    }

    // ===== 서버 → 클라 알림 진입점 =====
    public void HandleInviteNotify(S_PartyInviteNotify n)
    {
        Debug.Log(
            $"[PartyState] HandleInviteNotify: inviterPid={n.InviterPid}, " +
            $"inviterName={n.InviterName}, partyId={n.PartyId}, " +
            $"hasListener={OnInvite != null}"
        );

        OnInvite?.Invoke(n.InviterPid, n.InviterName, n.PartyId);
    }

    public void HandleInviteReply(S_PartyInviteReply r)
        => OnInviteResult?.Invoke(r.Success, r.ErrorMessage);

    public void HandleCreateReply(S_PartyCreateReply r)
    {
        OnInviteResult?.Invoke(r.Success, r.Message);

        if (r.Success)
        {
            PartyNet.RequestPartyList();
        }
        else
        {
            Debug.Log($"[PartyState] Party creation failed: {r.Message}");
        }
    }
    public void SetLocalPartyName(string partyName)
    {
        PartyName = partyName;
        OnPartyChanged?.Invoke();
    }
    public void HandleJoinReply(S_PartyJoinReply r, int requestedPartyId)
    {
        OnJoinResult?.Invoke(r.Success, r.Message);

        if (r.Success)
        {
            // 내가 어떤 파티에 조인 요청을 보낸 건지 기억해두기
            ConfirmJoined(requestedPartyId);

            // 조인 성공 시, 리더/파티원 구분 없이 무조건 최신 파티 메타 요청
            // 여기서 받은 S_PartyList 안에서 내 pid를 찾아서 SetPartyMetaFromInfo(...)가 호출되면서
            // PartyName / PartyId / MaxMemberCount / PartyLeaderId / Members 가 모두 채워짐.
            PartyNet.RequestPartyList();
        }
    }

    public void HandleJoinNotifyToLeader(S_PartyJoinNotify n)
        => OnJoinNotifyToLeader?.Invoke(n.JoinPlayerId, n.PartyId, n.LeaderId);

    //  핵심: 여기서 내 파티를 찾아서 메타 세팅
    public void HandlePartyList(S_PartyList list)
    {
        var infos = new List<PartyInfo>(list.PartyInfos);
        OnPartyList?.Invoke(infos);

        int myId = NetDebug.MyPlayerId;
        if (myId < 0)
        {
            Debug.Log("[PartyState] HandlePartyList: MyPlayerId < 0, skip self-party detection");
            return;
        }

        PartyInfo myParty = null;

        foreach (var info in infos)
        {
            foreach (var m in info.Members)
            {
                if (m.PlayerId == myId)
                {
                    myParty = info;
                    break;
                }
            }
            if (myParty != null)
                break;
        }

        if (myParty != null)
        {
            Debug.Log($"[PartyState] My party found in list. partyId={myParty.PartyId}, name={myParty.PartyName}, memberCount={myParty.Members.Count}");
            SetPartyMetaFromInfo(myParty);
        }
        else
        {
            // ❗ 여기서 바로 ClearPartyInternal() 해버리면,
            // 타이밍 문제로 리스트에 아직 내 파티가 안 잡혔을 때
            // 파티 상태를 전부 날려버리는 레이스가 생김.
            Debug.Log("[PartyState] My party NOT found in list. Keep current party state for now.");
            // 아무것도 안 하고 현재 상태 유지
        }
    }
    public void MarkSelfLeaveRequested()
    {
        _selfLeaveRequested = true;
    }
    public void HandleJoinRequestList(S_PartyJoinRequestList list)
        => OnJoinRequestList?.Invoke(list.PartyId, new List<PartyJoinRequesterInfo>(list.Requesters));

    // ===== 내부 유틸 =====
    private void ClearPartyInternal()
    {
        PartyId = null;
        PartyName = null;
        MaxMemberCount = 0;
        PartyLeaderId = 0;
        _members.Clear();
        Debug.Log("[PartyState] ClearPartyInternal");
        OnPartyChanged?.Invoke();
    }

    private void RecomputeLeader()
    {
        foreach (var kv in _members)
        {
            if (kv.Value.IsLeader)
            {
                PartyLeaderId = kv.Key;
                return;
            }
        }
    }

    private void RecomputeLeaderAndEmitChanged()
    {
        RecomputeLeader();
        Debug.Log($"[PartyState] RecomputeLeader: leaderId={PartyLeaderId}, memberCount={_members.Count}");
        OnPartyChanged?.Invoke();
    }
}
