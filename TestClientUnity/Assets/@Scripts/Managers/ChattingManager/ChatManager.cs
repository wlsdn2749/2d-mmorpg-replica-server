using Google.Protobuf.Protocol;
using Packet;
using System;
using System.Collections.Generic;
using UnityEngine;

public class ChatMessage
{
    public int? playerId;        // 서버가 준 playerId (없으면 null)
    public string message;
    public string playerName;
    public EChatType chatType;   // CHAT_ROOM / CHAT_ALL
    public DateTime timeUtc;
}
public class ChatManager : MonoBehaviour
{
    public static ChatManager Instance { get; private set; }

    public event Action<ChatMessage> OnMessageAdded;

    private readonly Queue<ChatMessage> _messages = new();
    [SerializeField] private int maxMessages = 100;

    void Awake()
    {
        if (Instance != null && Instance != this) { Destroy(gameObject); return; }
        Instance = this;
        DontDestroyOnLoad(gameObject);
    }

    // === 송신 ===
    public void Send(string text, EChatType chatType)
    {
        if (string.IsNullOrWhiteSpace(text)) return;

        var info = new PlayerChatInfo
        {
            NonePlayer = true,   // oneof에 NonePlayer 세팅
            Message = text,
            ChatType = chatType
        };

        var req = new C_PlayerChat { PlayerChatInfo = info };
        var send = ServerPacketManager.MakeSendBuffer(req);
        NetworkManager.Instance.Send(send);

        Debug.Log($"[Chat] Sent: {text}");
    }

    // === 수신 ===
    public void HandleBroadcast(S_BroadcastPlayerChat pkt)
    {
        foreach (var p in pkt.PlayerChatInfos)
        {
            int? pid = null;
            // oneof에서 playerId가 세팅된 경우만 읽기
            if (p.PlayerIncludedCase == PlayerChatInfo.PlayerIncludedOneofCase.PlayerId)
                pid = p.PlayerId;

            var msg = new ChatMessage
            {
                playerId = pid,
                message = p.Message,
                chatType = p.ChatType,
                playerName = p.PlayerName,
                timeUtc = DateTime.UtcNow
            };

            _messages.Enqueue(msg);
            if (_messages.Count > maxMessages) _messages.Dequeue();

            OnMessageAdded?.Invoke(msg);
        }
    }

    public IEnumerable<ChatMessage> GetMessages() => _messages.ToArray();
}