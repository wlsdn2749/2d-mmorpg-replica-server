using System.Collections.Generic;
using UnityEngine;
using Google.Protobuf.Protocol;

public static class MonsterSpawner
{
    static readonly Dictionary<int, GameObject> _spawned = new();
    static Transform _container;

    static Transform Container
    {
        get
        {
            if (_container == null)
            {
                var go = GameObject.Find("@Monsters") ?? new GameObject("@Monsters");
                _container = go.transform;
            }
            return _container;
        }
    }

    // === 스폰 ===
    public static GameObject Spawn(S_SpawnMonster msg)
    {
        if (_spawned.ContainsKey(msg.MonsterId))
        {
            Debug.LogWarning($"[MonsterSpawner] Already spawned monster {msg.MonsterId}");
            return _spawned[msg.MonsterId];
        }

        // MonsterTypeId를 Prefab으로 매핑
        var prefab = MonsterResourceLoader.GetPrefab(msg.MonsterTypeId);
        if (!prefab)
        {
            Debug.LogError($"[MonsterSpawner] prefab not found for type {msg.MonsterTypeId}");
            return null;
        }

        Vector3 pos = new Vector3(msg.X, msg.Y, 0);
        var go = Object.Instantiate(prefab, pos, Quaternion.identity, Container);
        go.name = $"Monster_{msg.MonsterId}";

        // 컴포넌트 초기화
        var id = go.GetComponent<MonsterIdentity>() ?? go.AddComponent<MonsterIdentity>();
        id.Init(msg.MonsterId, msg.MonsterTypeId, msg.Dir);

        _spawned[msg.MonsterId] = go;
        return go;
    }

    // === 디스폰 ===
    public static void Despawn(S_DespawnMonster msg)
    {
        if (_spawned.TryGetValue(msg.MonsterId, out var go))
        {
            Object.Destroy(go);
            _spawned.Remove(msg.MonsterId);
            Debug.Log($"[MonsterSpawner] Despawn {msg.MonsterId}, reason={msg.Reason}");
        }
    }

    // === 이동 업데이트 ===
    public static void UpdateMove(S_BroadcastMonsterMove msg)
    {
        if (!_spawned.TryGetValue(msg.MonsterId, out var go))
        {
            Debug.LogWarning($"[MonsterSpawner] Move update for missing {msg.MonsterId}");
            return;
        }

        var avatar = go.GetComponent<MonsterAvatar>();
        if (avatar == null) avatar = go.AddComponent<MonsterAvatar>();

        Vector3 dst = new Vector3(msg.X, msg.Y, 0);
        avatar.SmoothMoveTo(dst);
        avatar.SetDirection(msg.Dir);
    }
}