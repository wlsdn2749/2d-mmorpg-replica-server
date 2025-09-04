 // ← Cinemachine 사용
using System.Collections.Generic;
using UnityEngine;
using Google.Protobuf.Protocol;
using Unity.Cinemachine;
using System.Collections;
using UnityEngine.PlayerLoop;
/// <summary>
/// 서버에서 내려오는 PlayerInfo로 플레이어 프리팹을 스폰/제거.
/// 로컬 플레이어가 스폰되면 Cinemachine vcam의 Follow/LookAt을 자동 연결.
/// </summary>
public class PlayerSpawner : MonoBehaviour
{
    [Header("References")]
    [SerializeField] private GameObject playerPrefab;                 // 스폰할 프리팹
    [SerializeField] private CinemachineCamera vcam;           // 씬에 배치한 vcam (없으면 런타임에 찾아옴)

    public static PlayerSpawner Instance { get; private set; }
    private readonly Dictionary<long, GameObject> _spawned = new();   // playerId → GO
    private static readonly List<(PlayerInfo info, bool isLocal)> _pending = new();
    void Awake()
    {
        if (Instance != null && Instance != this) { Destroy(gameObject); return; }
        Instance = this;

        // 필요하면 유지
        // DontDestroyOnLoad(gameObject);

        FlushPending();
        if (vcam == null)
           vcam = FindFirstObjectByType<CinemachineCamera>(FindObjectsInactive.Include);
    }

    public static bool HasInstance => Instance != null;

    /// <summary>이미 스폰돼 있는지 확인</summary>
    public static bool IsSpawned(int playerId)
        => HasInstance && Instance._spawned.ContainsKey(playerId);

    /// <summary>스폰 (중복 스폰 방지 포함)</summary>
    public void SpawnNow(PlayerInfo info, bool isLocal)
    {
        if (!playerPrefab) { Debug.LogError("[PlayerSpawner] playerPrefab 미할당"); return; }
        if (_spawned.ContainsKey(info.Id)) return;
        
        
        var pos = new Vector3(info.Pos.X, info.Pos.Y, 0);
        Debug.Log($"{pos}, {info.Pos.X} {info.Pos.Y}");
        var go = Instantiate(playerPrefab, pos, Quaternion.identity);
        go.name = (isLocal ? "LocalPlayer_" : "Player_") + info.Id;

        var identity = go.GetComponent<PlayerIdentity>() ?? go.AddComponent<PlayerIdentity>();
        identity.Init(info, isLocal);

        _spawned[info.Id] = go;
        Debug.Log($"[PlayerSpawner] Spawn OK: {info.Username} (ID:{info.Id}) {(isLocal ? "(ME)" : "")} at {pos}");

        // 등록
        Instance._spawned[info.Id] = go;

        // 로컬 플레이어라면 시네머신 vcam 연결
        if (isLocal)
            Instance.AttachCinemachineTo(go.transform);
        
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
        Instance.SpawnNow(info, isLocal);
    }
    /// <summary>제거</summary>
    public static void RemovePlayer(int playerId)
    {
        if (!HasInstance) return;

        if (Instance._spawned.TryGetValue(playerId, out var go))
        {
            Object.Destroy(go);
            Instance._spawned.Remove(playerId);
        }
    }
    private void FlushPending()
    {
        if (_pending.Count == 0) return;
        foreach (var p in _pending)
            SpawnNow(p.info, p.isLocal);
        _pending.Clear();
    }

    /// <summary>전체 제거(맵 갈아탈 때 등)</summary>
    public static void DespawnAll()
    {
        if (!HasInstance) return;

        foreach (var kv in Instance._spawned)
            if (kv.Value != null) Object.Destroy(kv.Value);

        Instance._spawned.Clear();
    }

    // --- 내부 유틸 ---

    private void AttachCinemachineTo(Transform target)
    {
        if (vcam == null)
            vcam = FindFirstObjectByType<CinemachineCamera>(FindObjectsInactive.Include);

        if (vcam == null)
        {
            Debug.LogWarning("[PlayerSpawner] CinemachineCamera를 찾지 못했습니다. 씬에 vcam을 배치하세요.");
            return;
        }

        // CM3에서도 Follow/LookAt은 동일
        vcam.Follow = target;
        vcam.LookAt = target; // 2D면 LookAt은 생략 가능

        // (선택) 첫 프레임 튐 방지: PositionComposer의 Damping을 잠깐 0으로
        var composer = vcam.GetComponent<CinemachinePositionComposer>();
        if (composer != null)
            StartCoroutine(SnapCameraNextFrame(composer, new Vector3(0.5f, 0.5f, 0f))); // 복구값은 프로젝트에 맞게
    }
    IEnumerator SnapCameraNextFrame(CinemachinePositionComposer composer, Vector3 restoreDamping)
    {
        // 현재 Damping 저장
        var prev = composer.Damping;
        // 스폰 프레임 즉시 스냅
        composer.Damping = Vector3.zero;

        yield return null; // 한 프레임 양보 후

        // 원래 감쇠값 복구
        composer.Damping = restoreDamping;
    }

    static void ApplyDirection(Transform t, EDirection dir)
    {
        switch (dir)
        {
            case EDirection.DirLeft: 
                t.localScale = new Vector3(-1, 1, 1); 
                break;
            case EDirection.DirRight:
                t.localScale = new Vector3(1, 1, 1);
                break;
            case EDirection.DirUp:
                // Animator 파라미터로 처리 권장
                break;
                case EDirection.DirDown:
                // Animator 파라미터로 처리 권장
                break;
                // UP/DOWN은 Animator 파라미터로 처리 권장
        }
    }
    
    //System.Collections.IEnumerator AttachCameraNextFrame(Transform target)
    //{
    //    yield return null;
    //    var cf = Object.FindObjectOfType<CameraFollow>(true);
    //    if (cf != null) cf.Follow(target);
    //}
}