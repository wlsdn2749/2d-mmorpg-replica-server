using System;
using System.Collections.Generic;
using Google.Protobuf.Protocol;

public sealed class PartyState
{
    public static PartyState Instance { get; } = new PartyState();

    public int? PartyId { get; private set; }
    public string PartyName { get; private set; }
    public int PartyLeaderId { get; private set; }
    public int MaxMemberCount { get; private set; }
    public int CurMemberCount => _members.Count;

    public bool InParty => _members.Count > 0;
    // playerId -> PartyMemberStatusInfo
    private readonly Dictionary<int, PartyMemberStatusInfo> _members = new();

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

    private PartyState() { }

    public IReadOnlyDictionary<int, PartyMemberStatusInfo> Members => _members;

    // ===== 서버 브로드캐스트 반영 =====
    public void ApplyBroadcast(S_BroadcastPartyUpdate pkt)
    {
        if (pkt == null) return;

        switch (pkt.UpdateType)
        {
            case EPartyUpdateType.PartyUpdateMemberJoin:
                foreach (var m in pkt.Members)
                    _members[m.PlayerId] = m;
                RecomputeLeaderAndEmitChanged();
                break;

            case EPartyUpdateType.PartyUpdateMemberLeave:
                foreach (var m in pkt.Members)
                    _members.Remove(m.PlayerId);
                if (_members.Count == 0)
                    ClearPartyInternal();
                else
                    RecomputeLeaderAndEmitChanged();
                break;

            case EPartyUpdateType.PartyUpdateStatus:
                foreach (var m in pkt.Members)
                {
                    if (_members.TryGetValue(m.PlayerId, out var cur))
                    {
                        cur.Hp = m.Hp; cur.MaxHp = m.MaxHp; cur.Level = m.Level; cur.IsLeader = m.IsLeader;
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
        }
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

        OnPartyChanged?.Invoke();
    }

    public void ConfirmJoined(int partyId)
    {
        PartyId = partyId;
        // 멤버/리더 정보는 이후 Broadcast나 리스트로 들어온 걸로 채워짐
    }

    public void ClearParty()
    {
        ClearPartyInternal();
    }

    // ===== 서버 → 클라 알림 진입점 =====
    public void HandleInviteNotify(S_PartyInviteNotify n)
        => OnInvite?.Invoke(n.InviterPid, n.InviterName, n.PartyId);

    public void HandleInviteReply(S_PartyInviteReply r)
        => OnInviteResult?.Invoke(r.Success, r.ErrorMessage);

    public void HandleCreateReply(S_PartyCreateReply r)
    {
        OnCreateResult?.Invoke(r.Success, r.Message);
    }
    
    public void HandleJoinReply(S_PartyJoinReply r, int requestedPartyId)
    {
        OnJoinResult?.Invoke(r.Success, r.Message);
        if (r.Success) 
        { 
            ConfirmJoined(requestedPartyId);
        }
    }

    public void HandleJoinNotifyToLeader(S_PartyJoinNotify n)
        => OnJoinNotifyToLeader?.Invoke(n.JoinPlayerId, n.PartyId, n.LeaderId);

    public void HandlePartyList(S_PartyList list)
        => OnPartyList?.Invoke(new List<PartyInfo>(list.PartyInfos));

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
        OnPartyChanged?.Invoke();
    }
}
