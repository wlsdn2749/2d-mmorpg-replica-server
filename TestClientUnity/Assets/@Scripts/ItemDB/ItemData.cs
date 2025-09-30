// ItemDatabase.cs
using System;
using System.Collections.Generic;
using UnityEngine;

[CreateAssetMenu(menuName = "GameData/ItemDatabase")]
public class ItemDatabase : ScriptableObject
{
    [Serializable]
    public class Entry
    {
        public int id;
        public Sprite icon;
        public string displayName;   // 선택
        // public EItemType type;    // 선택
    }

    public List<Entry> entries = new List<Entry>();

    Dictionary<int, Entry> _map;

    void OnEnable()
    {
        _map = new Dictionary<int, Entry>(entries.Count);
        foreach (var e in entries)
            _map[e.id] = e;
    }

    public bool TryGetIcon(int id, out Sprite sprite)
    {
        if (_map != null && _map.TryGetValue(id, out var e) && e.icon != null)
        {
            sprite = e.icon;
            return true;
        }
        sprite = null;
        return false;
    }

    public string GetNameOrNull(int id) =>
        (_map != null && _map.TryGetValue(id, out var e)) ? e.displayName : null;
}
