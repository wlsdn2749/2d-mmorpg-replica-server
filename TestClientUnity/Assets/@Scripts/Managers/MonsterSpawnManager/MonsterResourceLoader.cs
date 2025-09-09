using UnityEngine;
using System.Collections.Generic;

public static class MonsterResourceLoader
{
    static Dictionary<int, GameObject> _cache = new();

    public static GameObject GetPrefab(int typeId)
    {
        if (_cache.TryGetValue(typeId, out var prefab)) return prefab;

        // 경로 규칙은 프로젝트에 맞게 수정
        prefab = Resources.Load<GameObject>($"Prefab/Monsters/Monster_{typeId}");
        if (prefab != null) _cache[typeId] = prefab;

        return prefab;
    }
}
