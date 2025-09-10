using UnityEngine;
using System.Collections.Generic;

public static class MonsterResourceLoader
{
    static Dictionary<int, GameObject> _cache = new();
    static GameObject _fallback;
    public static GameObject GetPrefab(int typeId)
    {
        if (_cache.TryGetValue(typeId, out var p)) return p;

        // 경로 규칙: Monsters/Monster_{typeId}
        var path = $"Monsters/Monster_{typeId}";
        var prefab = Resources.Load<GameObject>(path);
        if (!prefab) Debug.LogError($"[MonsterRes] Resources.Load failed at 'Resources/{path}.prefab' (typeId={typeId})");
        if (!prefab)
        {
            if (!_fallback)
            {
                _fallback = GameObject.CreatePrimitive(PrimitiveType.Cube);
                _fallback.name = "Monster_FALLBACK";
                _fallback.transform.localScale = new Vector3(0.8f, 0.8f, 0.8f);
                _fallback.AddComponent<MonsterAvatar>();
                _fallback.AddComponent<MonsterIdentity>();
                _fallback.SetActive(false); // 템플릿처럼 사용
            }
            Debug.LogError($"[MonsterRes] Prefab NOT found for typeId={typeId} at 'Resources/{path}.prefab' — using FALLBACK.");
            prefab = _fallback;
        }

        _cache[typeId] = prefab;
        return prefab;
    }
}
