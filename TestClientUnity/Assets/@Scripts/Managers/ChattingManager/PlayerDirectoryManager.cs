// PlayerDirectory.cs
using System.Collections.Generic;
using UnityEngine;

public class PlayerDirectoryManager : MonoBehaviour
{
    public static PlayerDirectoryManager Instance { get; private set; }

    private readonly Dictionary<int, string> _idToName = new();

    void Awake()
    {
        if (Instance && Instance != this) { Destroy(gameObject); return; }
        Instance = this;
        DontDestroyOnLoad(gameObject);
    }

    public void SetName(int id, string name)
    {
        if (id <= 0 || string.IsNullOrEmpty(name)) return;
        _idToName[id] = name;
    }

    public bool TryGetName(int id, out string name) => _idToName.TryGetValue(id, out name);

    // 선택: 초기화/로그아웃 시
    public void Clear() => _idToName.Clear();
}
