using System.Collections.Generic;
using UnityEngine;
using Google.Protobuf.Protocol;

public static class MonsterSync
{
    // 현재 맵
    static int? _currentMapId = null;
    static S_MonsterList _pendingSnapshot;

    public static int CurrentMapId
    {
        get => _currentMapId ?? -1;
        private set => _currentMapId = value;
    }

    // 버퍼/레지스트리
    static readonly Dictionary<int, MonsterInfo> pendingInfos = new();          // id -> (pos,dir)
    static readonly Dictionary<int, int> typeRegistry = new();           // id -> typeId
    static readonly Dictionary<int, S_BroadcastMonsterMove> pendingMoves = new(); // id -> last move
    public static void OnMapActivated(int mapId)
    {
        CurrentMapId = mapId;
        Debug.Log($"[MonsterSync] Map activated: {mapId}");

        if (_pendingSnapshot != null && _pendingSnapshot.MapId == mapId)
        {
            var snap = _pendingSnapshot; _pendingSnapshot = null;
            ApplySnapshot(snap); // 실제 적용 함수
        }
    }
    // ===== 스냅샷 적용 =====
    public static void ApplySnapshot(S_MonsterList snap)
    {
        if (_currentMapId == null)
        {
            Debug.Log($"[MonsterSync] Snapshot queued (map={snap.MapId}) — waiting for map activation");
            _pendingSnapshot = snap;
            return;
        }

        if (snap.MapId != _currentMapId.Value)
        {
            Debug.LogWarning($"[MonsterSync] Ignore snapshot: map mismatch {snap.MapId}!={_currentMapId.Value}");
            // 원하면 다른 맵 스냅샷도 큐에 1개만 보관 가능
            return;
        }

        ApplySnapshotInternal(snap);
    }
    static void ApplySnapshotInternal(S_MonsterList snap)
    {
        MonsterSpawner.DespawnAll();
        pendingInfos.Clear();
        foreach (var m in snap.Monsters)
            pendingInfos[m.MonsterId] = m;

        // 이미 타입을 알고 있는 개체는 지금 바로 스폰
        foreach (var kv in pendingInfos)
        {
            var id = kv.Key;
            if (typeRegistry.TryGetValue(id, out var typeId))
                SpawnWithAllData(id, typeId, kv.Value);
            Debug.Log("몬스터 스냅샷 처리 완료");

        }
    }

    // ===== 개별 스폰 수신 =====
    public static void OnSpawn(S_SpawnMonster msg)
    {
        typeRegistry[msg.Monster.MonsterTypeId] = msg.Monster.MonsterTypeId;

        // 스냅샷에서 정보가 이미 왔으면 그 좌표/방향으로 스폰
        if (pendingInfos.TryGetValue(msg.Monster.MonsterId, out var info))
        {
            SpawnWithAllData(msg.Monster.MonsterId, msg.Monster.MonsterTypeId, info);
            pendingInfos.Remove(msg.Monster.MonsterId);
        }
        else
        {
            // 스냅샷이 아직 없으면 스폰 패킷 좌표로 일단 만들고, 나중에 스냅샷/이동이 오면 덮어씀
            var pos = new Vector3(msg.Monster.Pos.X, msg.Monster.Pos.Y, 0);
            var go = MonsterSpawner.SpawnDirect(msg.Monster.MonsterId, msg.Monster.MonsterTypeId, pos, msg.Monster.Direction);
            // 스폰 직후 대기 중 이동 적용
            if (pendingMoves.TryGetValue(msg.Monster.MonsterId, out var mv))
            {
                pendingMoves.Remove(msg.Monster.MonsterId);
                MonsterSpawner.UpdateMove(mv);
            }
        }
    }

    // ===== 이동 수신 =====
    public static void OnMove(S_BroadcastMonsterMove msg)
    {
        // 아직 스폰 전이면 일단 버퍼
        if (!MonsterSpawner.Exists(msg.MonsterId))
        {
            pendingMoves[msg.MonsterId] = msg;
            return;
        }
        MonsterSpawner.UpdateMove(msg);
    }

    // ===== 디스폰 수신 =====
    public static void OnDespawn(S_DespawnMonster msg)
    {
        pendingInfos.Remove(msg.MonsterId);
        pendingMoves.Remove(msg.MonsterId);
        typeRegistry.Remove(msg.MonsterId);
        MonsterSpawner.Despawn(msg);
    }

    // ===== 헬퍼 =====
    static void SpawnWithAllData(int id, int typeId, MonsterInfo info)
    {
        Debug.Log("SpawnWithAllData");
        var pos = new Vector3(info.Pos.X, info.Pos.Y, 0);
        var go = MonsterSpawner.SpawnDirect(id, typeId, pos, info.Direction);
        // 스폰 직후 대기 이동 적용
        if (pendingMoves.TryGetValue(id, out var mv))
        {
            pendingMoves.Remove(id);
            MonsterSpawner.UpdateMove(mv);
        }
    }
}
