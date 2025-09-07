using Google.Protobuf.Protocol;
using Packet;
using UnityEngine;

public static class LeaveGameContext
{
    public static ELeaveReason LastLeaveReason { get; private set; } = ELeaveReason.LeaveUnknown;

    public static void SendLeave(ELeaveReason reason)
    {
        LastLeaveReason = reason;

        var req = new C_LeaveGame { Reason = reason };
        var send = ServerPacketManager.MakeSendBuffer(req);
        NetworkManager.Instance.Send(send);

        Debug.Log($"[Leave] request sent. reason={reason}");
    }
}
