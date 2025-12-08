using Google.Protobuf;
using Google.Protobuf.Protocol;
using Packet;
using UnityEngine;

public static class PartyNet
{
    private static int _lastRequestedJoinPartyId = -1;

    private static void Send<T>(T msg, PacketID id) where T : IMessage<T>
    {
        var send = ServerPacketManager.MakeSendBuffer(msg, (ushort)id);
        NetworkManager.Instance.Send(send);
    }

    public static void SendInvite(int targetPid)
    {
        Debug.Log($"[PartyNet] SendInvite() called. targetPid={targetPid}");
        var req = new C_PartyInviteRequest { TargetPid = targetPid };
        Send(req, PacketID.PKT_C_PartyInviteRequest);
        Debug.Log("[PartyNet] C_PartyInviteRequest sent to server.");
    }

    public static void RespondInvite(int partyId, bool accept)
    {
        Debug.Log($"[PartyNet] RespondInvite() partyId={partyId}, accept={accept}");
        var req = new C_PartyInviteResponse { PartyId = partyId, Accept = accept };
        Send(req, PacketID.PKT_C_PartyInviteResponse);
    }

    public static void LeaveSelf()
    {
        var req = new C_PartyLeave();
        req.SelfLeave = true; // oneof
        Send(req, PacketID.PKT_C_PartyLeave);
        Debug.Log ("Sent Party Leave Self");
    }

    public static void Kick(int targetPid)
    {
        var req = new C_PartyLeave();
        req.TargetPid = targetPid; // oneof
        //req.SelfLeave = false;
        Send(req, PacketID.PKT_C_PartyLeave);
        Debug.Log ("Sent Party Leave Kick for PID: " + targetPid);
    }

    public static void Create(string partyName)
    {
        var req = new C_PartyCreateRequest { PartyName = partyName ?? string.Empty };
        Send(req, PacketID.PKT_C_PartyCreateRequest);
    }

    public static void RequestJoin(int partyId)
    {
        _lastRequestedJoinPartyId = partyId;
        var req = new C_PartyJoinRequest { PartyId = partyId };
        Send(req, PacketID.PKT_C_PartyJoinRequest);
    }

    public static void RespondJoinAsLeader(int partyId, int requesterPid, bool accept)
    {
        var req = new C_PartyJoinResponse { PartyId = partyId, RequesterPid = requesterPid, Accept = accept };
        Send(req, PacketID.PKT_C_PartyJoinResponse);
    }

    public static void RequestPartyList()
    {
        Send(new C_PartyList(), PacketID.PKT_C_PartyList);
    }

    public static void RequestJoinRequestList(int partyId)
    {
        Send(new C_PartyJoinRequestList { PartyId = partyId }, PacketID.PKT_C_PartyJoinRequestList);
    }

    public static int ConsumeLastRequestedJoinPartyId()
    {
        var id = _lastRequestedJoinPartyId;
        _lastRequestedJoinPartyId = -1;
        return id;
    }
    public static void DelegateLeader(int targetPid)
    {
        var req = new C_PartyDelegateLeader { TargetPid = targetPid };
        Send(req, PacketID.PKT_C_PartyDelegateLeader);
    }
}
