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
                var monster = GameObject.Find("@Monsters") ?? new GameObject("@Monsters");
                _container = monster.transform;
            }
            return _container;
        }
    }

    public static bool Exists(int id) => _spawned.ContainsKey(id);
    public static GameObject Get(int id) => _spawned.TryGetValue(id, out var go) ? go : null;
    
    // 스냅샷/스폰 패킷 통합 진입점
    public static GameObject SpawnDirect(int id, int typeId, Vector3 pos, EDirection dir)
    {
        if (_spawned.TryGetValue(id, out var exist))
        {
            // 이미 있으면 위치/방향만 동기화
            var av = exist.GetComponent<MonsterAvatar>() ?? exist.AddComponent<MonsterAvatar>();
            av.HardSnap(pos);
            av.SetDirection(dir);
            return exist;
        }

        var prefab = MonsterResourceLoader.GetPrefab(typeId);
        if (!prefab)
        {
            Debug.LogError($"[MonsterSpawner] prefab not found for type {typeId}, id={id}");
            return null;
        }

        var go = Object.Instantiate(prefab, pos, Quaternion.identity, Container);
        go.name = $"Monster_{id}";

        var idc = go.GetComponent<MonsterIdentity>() ?? go.AddComponent<MonsterIdentity>();
        idc.Init(id, typeId, dir);

        var avNew = go.GetComponent<MonsterAvatar>() ?? go.AddComponent<MonsterAvatar>();
        avNew.HardSnap(pos);
        avNew.SetDirection(dir);

        _spawned[id] = go;
        Debug.Log("스냅샷 몬스터 생성 완료");
        return go;
    }

    public static void UpdateMove(S_BroadcastMonsterMove msg)
    {
        if (!_spawned.TryGetValue(msg.MonsterId, out var go) || go == null)
        {
            Debug.LogWarning($"[MonsterSpawner] Move update for missing {msg.MonsterId}");
            return;
        }

        var av = go.GetComponent<MonsterAvatar>();
        if (av == null)
        {
            Debug.LogWarning($"[MonsterSpawner] Avatar missing for {msg.MonsterId}");
            return;
        }

        av.SmoothMoveTo(new Vector3(msg.X, msg.Y, 0));
        av.SetDirection(msg.Dir);
    }
    public static Vector2 MonsterPos(S_BroadcastPlayerAttack msg)
    {
        if(!_spawned.TryGetValue(msg.TargetId, out var go) || go == null)
        {
            Debug.LogWarning($"[MonsterSpawner] Move update for missing {msg.TargetId}");
            return Vector2.zero ;
        }
        var mu = go.GetComponent<MonsterUI>();
        if (mu == null)
        {
            Debug.LogWarning($"[MonsterSpawner] Avatar missing for {msg.TargetId}");
            return Vector2.zero;
        }
        return go.transform.position + mu.DamageTextOffset;
    }
    public static void UpdateAttack(S_BroadcastMonsterAttack msg)
    {
        if (!_spawned.TryGetValue(msg.MonsterId, out var go) || go == null)
        {
            Debug.LogWarning($"[MonsterSpawner] Attack update for missing {msg.MonsterId}");
            return;
        }
        var av = go.GetComponent<MonsterAvatar>();
        if (av == null)
        {
            Debug.LogWarning($"[MonsterSpawner] Avatar missing for {msg.MonsterId}");
            return;
        }
        av.OnAttack();
    }

    public static void Despawn(S_DespawnMonster msg)
    {
        if (_spawned.TryGetValue(msg.MonsterId, out var go))
        {
            Object.Destroy(go);
            _spawned.Remove(msg.MonsterId);
        }
    }
    public static void OnHit(S_BroadcastPlayerAttack msg)
    {
        if (!_spawned.TryGetValue(msg.TargetId, out var go) || go == null)
        {
            Debug.LogWarning($"[MonsterSpawner] Hit update for missing {msg.TargetId}");
            return;
        }
        var mh = go.GetComponent<MonsterHealth>();
        if (mh == null)
        {
            Debug.LogWarning($"[MonsterSpawner] Avatar missing for {msg.TargetId}");
            return;
        }
        //mh.AfterHp(msg.HpAfter);
    }
    public static void Despawn(S_BroadcastMonsterDeath msg)
    {
        if (_spawned.TryGetValue(msg.MonsterId, out var go))
        {
            Object.Destroy(go);
            _spawned.Remove(msg.MonsterId);
        }
    }
    public static void DespawnAll()
    {
        foreach (var kv in _spawned) if (kv.Value) Object.Destroy(kv.Value);
        _spawned.Clear();
        foreach (var kv in _spawned)
        {
            if (kv.Value != null)
                Object.Destroy(kv.Value);
        }
        _spawned.Clear(); 
    }
}