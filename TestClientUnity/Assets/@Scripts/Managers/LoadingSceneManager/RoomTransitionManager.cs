using System;
using UnityEngine;
using Google.Protobuf.Protocol;
using Packet;

public class RoomTransitionManager : MonoBehaviour
{
    public static RoomTransitionManager Instance { get; private set; }

    public enum State { Idle, BeginReceived, Loading, ReadySent, Committed }
    public State CurrentState { get; private set; } = State.Idle;

    public int CurrentTransitionId { get; private set; } = -1;
    public int CurrentMapId { get; private set; } = -1;

    void Awake()
    {
        if (Instance != null && Instance != this) { Destroy(gameObject); return; }
        Instance = this;
        DontDestroyOnLoad(gameObject);
    }

    // === 서버 핸들러에서 호출 ===
    public void OnChangeRoomBegin(S_ChangeRoomBegin msg)
    {
        if (CurrentState != State.Idle && msg.TransitionId == CurrentTransitionId) return;

        CurrentTransitionId = msg.TransitionId;
        CurrentMapId = msg.MapId;
        CurrentState = State.BeginReceived;

        // 입력 잠금 + 기존 오브젝트 정리
        ToggleLocalInput(false);
        PlayerSpawner.DespawnAll();
        MonsterSpawner.DespawnAll();

        LoadingSceneManager.OnSceneActivated = () =>
        {
            var ready = new C_ChangeRoomReady { TransitionId = CurrentTransitionId };
            NetworkManager.Instance.Send(ServerPacketManager.MakeSendBuffer(ready));
        };

        LoadingSceneManager.LoadScene(MapIdToSceneName(CurrentMapId));
        CurrentState = State.Loading;
    }

    public void OnChangeRoomCommit(S_ChangeRoomCommit msg)
    {
        if (msg.TransitionId != CurrentTransitionId)
        {
            Debug.LogWarning($"[Room] Stale commit ignored: got={msg.TransitionId}, want={CurrentTransitionId}");
            return;
        }

        Debug.Log($"[RTM] Commit tid={msg.TransitionId}, map={msg.MapId}");

        // 1) 맵 확정(몬스터 스냅샷과의 맵 불일치 방지)
        MonsterSync.OnMapActivated(msg.MapId);

        // 2) (선택) 서버가 자동으로 스냅샷을 안 밀어줄 경우에만 요청
        //NetworkManager.Instance.Send(ServerPacketManager.MakeSendBuffer(new C_RequestPlayerList { MapId = msg.MapId }));
        //NetworkManager.Instance.Send(ServerPacketManager.MakeSendBuffer(new C_RequestMonsterList { MapId = msg.MapId }));

        // 3) 입력은 여기서 풀지 않는다! (내 플레이어가 스폰될 때까지 대기)
        CurrentState = State.Committed;
    }

    private void ToggleLocalInput(bool enabled)
    {
        // PlayerController의 모든 인스턴스를 가져와서 foreach로 순회
        foreach (var pc in FindObjectsByType<PlayerController>(FindObjectsSortMode.None))
        {
            var id = pc.GetComponent<PlayerIdentity>();
            pc.enabled = enabled && id != null && id.IsLocalPlayer;
        }
    }
    public void ResetState()
    {
        CurrentTransitionId = -1;
        CurrentMapId = -1;
        CurrentState = State.Idle;
    }
    private string MapIdToSceneName(int mapId)
    {
        return mapId switch
        {
            1 => "Map_Goguryeo",
            2 => "Map_Baekje",
            3 => "Map_HuntingField",
            _ => $"Map_{mapId}",
        };
    }
}
