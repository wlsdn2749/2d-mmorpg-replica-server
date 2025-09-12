using Google.Protobuf.Protocol;
using System.Collections.Generic;
using UnityEditor;
using UnityEngine;
public static class MonsterSync
{
    static int _currentMapId = -1;
    static bool _sceneReady = false;

    static S_MonsterList _pendingSnapshot;
    static Vector2 _monsterPos;
    static readonly Dictionary<int, MonsterInfo> _pendingSpawns = new();
    static readonly Dictionary<int, S_BroadcastMonsterMove> _pendingMoves = new();
    static readonly Dictionary<int, S_BroadcastMonsterAttack> _pendingAttacks = new();
    static readonly Dictionary<int, S_BroadcastPlayerAttack> _pendingPlayerAttacks = new();
    // 맵이 바뀐다(또는 최초 접속으로 이 맵 진입한다)
    public static void OnMapActivated(int mapId)
    {
        _currentMapId = mapId;
        _sceneReady = false;              // 중요: 새 씬 준비 전 상태
        _pendingMoves.Clear();
        // 스냅샷/스폰은 큐에 쌓이게 둔다
        Debug.Log($"[MonsterSync] Map activated: {mapId}");
    }

    // 씬 로드 시작 직전에 (선택) 부르면 명시적
    public static void OnGameplaySceneWillLoad() => _sceneReady = false;

    // 씬이 ‘활성’된 바로 그 프레임에 반드시 호출
    public static void OnGameplaySceneActivated()
    {
        _sceneReady = true;
        Debug.Log("[MonsterSync] Scene ready → flush pending");

        // 1) 스냅샷 먼저
        if (_pendingSnapshot != null && _pendingSnapshot.MapId == _currentMapId)
        {
            var snap = _pendingSnapshot; _pendingSnapshot = null;
            ApplySnapshotInternal(snap);
        }

        // 2) 개별 스폰들
        if (_pendingSpawns.Count > 0)
        {
            foreach (var kv in _pendingSpawns) SpawnNow(kv.Value);
            _pendingSpawns.Clear();
        }
    }

    public static void ApplySnapshot(S_MonsterList snap)
    {
        if (_currentMapId < 0) _currentMapId = snap.MapId;
        if (snap.MapId != _currentMapId) { Debug.LogWarning($"[MonsterSync] Ignore snapshot: {snap.MapId}!={_currentMapId}"); return; }

        if (!_sceneReady)
        {
            _pendingSnapshot = snap;
            Debug.Log($"[MonsterSync] Snapshot queued until scene ready (count={snap.Monsters.Count})");
            return;
        }
        ApplySnapshotInternal(snap);
    }

    static void ApplySnapshotInternal(S_MonsterList snap)
    {
        Debug.Log($"[MonsterSync] ApplySnapshotInternal: count={snap.Monsters.Count}");
        MonsterSpawner.DespawnAll();

        foreach (var info in snap.Monsters) SpawnNow(info);

        Debug.Log("[MonsterSync] snapshot applied");
    }

    public static void OnSpawn(MonsterInfo info)
    {
        if (!_sceneReady)
        {
            _pendingSpawns[info.MonsterId] = info;
            Debug.Log($"[MonsterSync] Spawn queued id={info.MonsterId} (scene not ready)");
            return;
        }
        SpawnNow(info);
    }

    static void SpawnNow(MonsterInfo info)
    {
        if (MonsterSpawner.Exists(info.MonsterId)) return;

        var pos = new Vector3(info.Pos.X, info.Pos.Y, 0);
        var go = MonsterSpawner.SpawnDirect(info.MonsterId, info.MonsterTypeId, pos, info.Direction);
        if (go == null) { Debug.LogError($"[MonsterSync] spawn failed id={info.MonsterId}"); return; }

        if (_pendingMoves.TryGetValue(info.MonsterId, out var mv))
        {
            _pendingMoves.Remove(info.MonsterId);
            MonsterSpawner.UpdateMove(mv);
        }
    }

    public static void OnMove(S_BroadcastMonsterMove msg)
    {
        if (!MonsterSpawner.Exists(msg.MonsterId)) { _pendingMoves[msg.MonsterId] = msg; return; }
        MonsterSpawner.UpdateMove(msg);
    }
    public static Vector2 MonsterPos(S_BroadcastPlayerAttack msg)
    {
        if (!MonsterSpawner.Exists(msg.TargetId)) { _pendingPlayerAttacks[msg.TargetId] = msg; return _monsterPos; }
        _monsterPos = MonsterSpawner.MonsterPos(msg);
        return _monsterPos;
    }
    public static void OnHit(S_BroadcastPlayerAttack msg)
    {
        if (!MonsterSpawner.Exists(msg.TargetId)) { _pendingPlayerAttacks[msg.TargetId] = msg; return; }
        MonsterSpawner.OnHit(msg);
    }
    public static void OnAttack(S_BroadcastMonsterAttack msg)
    {
        if (!MonsterSpawner.Exists(msg.MonsterId)) { _pendingAttacks[msg.MonsterId] = msg; return; }
        MonsterSpawner.UpdateAttack(msg);
    }

    public static void OnDespawn(S_DespawnMonster msg)
    {
        _pendingMoves.Remove(msg.MonsterId);
        _pendingSpawns.Remove(msg.MonsterId);
        MonsterSpawner.Despawn(msg);
    }
    public static void OnDespawn(S_BroadcastMonsterDeath msg)
    {
        _pendingMoves.Remove(msg.MonsterId);
        _pendingSpawns.Remove(msg.MonsterId);
        MonsterSpawner.Despawn(msg);
    }
}
