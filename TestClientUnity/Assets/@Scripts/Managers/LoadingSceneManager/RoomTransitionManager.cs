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
        ToggleLocalInput(false);
        PlayerSpawner.DespawnAll();
        MonsterSpawner.DespawnAll();

        WorldFlowState.ResetForBegin(msg.MapId, msg.TransitionId);

        // 몬스터 스냅샷 맵 일치용
        MonsterSync.OnMapActivated(msg.MapId);

        // 씬 로딩 (Begin이 유일한 로더)
        string sceneName = MapIdToSceneName(msg.MapId);
        LoadingSceneManager.OnSceneActivated = () =>
        {
            // Ready 전송
            var ready = new C_ChangeRoomReady { TransitionId = WorldFlowState.CurrentTransitionId ?? -1 };
            NetworkManager.Instance.Send(ServerPacketManager.MakeSendBuffer(ready));
            WorldFlowState.FinishSceneActivated();
            MonsterSync.OnGameplaySceneActivated();

        };
        LoadingSceneManager.LoadScene(sceneName);
    }
    public void UnlockInputAfterSpawn()
    {
        ToggleLocalInput(true);
        Debug.Log("[RTM] Local player spawned → input unlocked");
    }
    public void OnChangeRoomCommit(S_ChangeRoomCommit msg)
    {
        // 최초 접속 경로: Begin이 없었다면(CurrentTransitionId < 0) 비교하지 말고 받아들임
        if (CurrentTransitionId >= 0 && msg.TransitionId != CurrentTransitionId)
        {
            Debug.LogWarning($"[Room] stale commit ignored: got={msg.TransitionId}, want={CurrentTransitionId}");
            return;
        }

        // 최초 접속이면 여기서 세팅만
        if (CurrentTransitionId < 0)
            CurrentTransitionId = msg.TransitionId;

        Debug.Log($"[RTM] Commit tid={msg.TransitionId}, map={msg.MapId}");

        // 씬 로드는 여기서 하지 않음 (너 정책 유지)
        // 몬스터 스냅샷과의 맵 불일치 방지용
        MonsterSync.OnMapActivated(msg.MapId);

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
