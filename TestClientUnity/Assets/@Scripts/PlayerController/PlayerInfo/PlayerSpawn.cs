using Google.Protobuf.Protocol;
using NUnit.Framework;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using Unity.Cinemachine;
using UnityEngine;

public class PlayerSpawner : MonoBehaviour
{
    [Header("References")]
    [SerializeField] private GameObject playerPrefab;
    [SerializeField] private CinemachineCamera vcam;

    public static PlayerSpawner Instance { get; private set; }
    private readonly Dictionary<int, GameObject> _spawned = new(); // 🔁 int로 통일
    private static readonly List<(PlayerInfo info, bool isLocal)> _pending = new();

    void Awake()
    {
        if (Instance != null && Instance != this) { Destroy(gameObject); return; }
        Instance = this;

        FlushPending();
        if (vcam == null)
            vcam = FindFirstObjectByType<CinemachineCamera>(FindObjectsInactive.Include);
    }

    public static bool HasInstance => Instance != null;
    public static GameObject Get(int playerId)
        => HasInstance && Instance._spawned.TryGetValue(playerId, out var go) ? go : null;
    public static bool IsSpawned(int playerId)
        => HasInstance && Instance._spawned.ContainsKey(playerId);

    public void SpawnNow(PlayerInfo info, bool isLocal)
    {
        if (!playerPrefab) { Debug.LogError("[PlayerSpawner] playerPrefab 미할당"); return; }
        if (_spawned.ContainsKey(info.Id)) return;

        var pos = new Vector3(info.Pos.X, info.Pos.Y, 0);
        var go = Instantiate(playerPrefab, pos, Quaternion.identity);
        go.name = (isLocal ? "LocalPlayer_" : "Player_") + info.Id;
        Debug.Log($"[Spawner] spawn id={info.Id} name={info.Username} local={isLocal} pos=({info.Pos.X},{info.Pos.Y})");
        var identity = go.GetComponent<PlayerIdentity>() ?? go.AddComponent<PlayerIdentity>();
        identity.Init(info, isLocal);
        if (identity != null)
        {
            PlayerDirectoryManager.Instance?.SetName(info.Id, identity.Username);
        }
        _spawned[info.Id] = go; //  한 번만 등록

        if (isLocal)
            AttachCinemachineTo(go.transform);
    }

    public static void EnsureExists()
    {
        if (Instance != null) return;
        var go = new GameObject("PlayerSpawner");
        go.AddComponent<PlayerSpawner>();
        Debug.Log("[PlayerSpawner] 동적 생성");
    }

    public static void SafeSpawn(PlayerInfo info, bool isLocal)
    {
        if (Instance == null)
        {
            _pending.Add((info, isLocal));
            Debug.LogWarning("[PlayerSpawner] 인스턴스 없음 → 대기열에 보관");
            return;
        }
        SceneTransition.RunAfterGameplaySceneLoaded(() => Instance.SpawnNow(info, isLocal));
    }

    public static void RemovePlayer(int playerId)
    {
        if (!HasInstance)
        {
            SceneTransition.RunAfterGameplaySceneLoaded(() => Instance._spawned.Remove(playerId));
        }
        else if (Instance._spawned.TryGetValue(playerId, out var go))
        {
            Object.Destroy(go);
            Instance._spawned.Remove(playerId);
        }
    }

    public static void DespawnAll()
    {
        if (!HasInstance) return;
        foreach (var kv in Instance._spawned)
            if (kv.Value) Object.Destroy(kv.Value);
        Instance._spawned.Clear();
    }

    // 스냅샷 유틸: snap에 없는 애는 제거, 있는 애는 스폰(기존 정책: 모두 새 생성이지만, 안전상 있음)
    public static void ApplySnapshot(S_PlayerList snap)
    {
        if (snap == null) { Debug.LogWarning("[PlayerSpawner] snapshot is null"); return; }

        // 정책: 일단 전체 제거 후 재생성
        DespawnAll();

        int myId = snap.MyPlayerId;
        SceneTransition.RunAfterGameplaySceneLoaded(() =>
        {
            Debug.Log("씬 로딩 완료 후 플레이어 스폰 처리");
            foreach (var p in snap.Players)
            {
                Debug.Log($"[Spawner] apply snapshot myId={snap.MyPlayerId} total={snap.Players.Count}");
                SafeSpawn(p, p.Id == myId);
            }
        }); 
    }

    // (대안) 없애고 싶은 애만 제거하고 나머지 유지하고 싶을 때
    public static void DespawnExcept(HashSet<int> aliveIds)
    {
        if (!HasInstance) return;
        var toRemove = Instance._spawned.Keys.Where(id => !aliveIds.Contains(id)).ToList();
        foreach (var id in toRemove) RemovePlayer(id);
    }

    private void FlushPending()
    {
        if (_pending.Count == 0) return;
        foreach (var p in _pending)
            SpawnNow(p.info, p.isLocal);
        _pending.Clear();
    }

    private void AttachCinemachineTo(Transform target)
    {
        if (vcam == null)
            vcam = FindFirstObjectByType<CinemachineCamera>(FindObjectsInactive.Include);

        if (vcam == null || target == null)
        {
            Debug.LogWarning("[PlayerSpawner] vcam or target is null");
            return;
        }

        // 카메라를 먼저 타깃 위치로 순간이동
        vcam.transform.position = target.position + new Vector3(0, 0, -10f);

        // 이전 상태 무효화 → 첫 프레임부터 스냅
        vcam.PreviousStateIsValid = false;

        // Follow 바로 연결
        vcam.Follow = target;

        // damping 잠깐 0으로 만들고 한 프레임 후 복구
        if (vcam.TryGetComponent<CinemachinePositionComposer>(out var composer))
            StartCoroutine(ResetDampingNextFrame(composer));
    }

    private IEnumerator ResetDampingNextFrame(CinemachinePositionComposer composer)
    {
        var prev = composer.Damping;
        composer.Damping = Vector3.zero;
        yield return null;   // 한 프레임 유지
        composer.Damping = prev;
    }
}
