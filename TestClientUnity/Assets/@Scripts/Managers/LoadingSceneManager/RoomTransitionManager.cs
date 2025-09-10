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
        Debug.Log($"[RTM] Begin tid={msg.TransitionId}, map={msg.MapId}");
        // 1) 입력 잠금 + 전체 제거
        ToggleLocalInput(false);
        PlayerSpawner.DespawnAll();
        MonsterSpawner.DespawnAll();   // 🔸 이 줄 추가
        // 2) 로딩 시작: 네 매핑 규칙에 맞게 이름을 결정
        string nextSceneName = MapIdToSceneName(CurrentMapId);

        // 3) 새 씬 활성화 직후에 Ready 보내도록 콜백 등록
        LoadingSceneManager.OnSceneActivated = () =>
        {
            if (CurrentState != State.BeginReceived && CurrentState != State.Loading)
            {
                Debug.LogWarning($"[Room] OnSceneActivated ignored: state={CurrentState}, TransitionId={CurrentTransitionId}");
                return;
            }
            Debug.Log($"[Room] Sending Ready for TransitionId={CurrentTransitionId}, MapId={CurrentMapId} ({nextSceneName})");  
            CurrentState = State.ReadySent;
            var ready = new C_ChangeRoomReady { TransitionId = CurrentTransitionId };
            var send = ServerPacketManager.MakeSendBuffer(ready);
            NetworkManager.Instance.Send(send);
        };

        // 4) 로딩씬 통해 비동기 로드
        LoadingSceneManager.LoadScene(nextSceneName);
        CurrentState = State.Loading;
    }

    public void OnChangeRoomCommit(S_ChangeRoomCommit msg)
    {
        if (msg.TransitionId != CurrentTransitionId)
        {
            Debug.LogWarning($"[Room] Stale commit ignored: got={msg.TransitionId}, want={CurrentTransitionId}");
            return;
        }
        Debug.Log($"[RTM] Commit tid={msg.TransitionId}, map={msg.MapId}, count={msg.Snapshots?.Players?.Count ?? -1}");

        // 5) 스냅샷 재생성(로컬/리모트 전부 새로 생성)
        PlayerSpawner.EnsureExists();
        PlayerSpawner.ApplySnapshot(msg.Snapshots);
        MonsterSync.OnMapActivated(msg.MapId); // ✅ 맵 전환 시 갱신
        // 6) 복구
        ToggleLocalInput(true);
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
