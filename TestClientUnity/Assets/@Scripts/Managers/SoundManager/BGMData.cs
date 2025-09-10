using UnityEngine;

[CreateAssetMenu(menuName = "GameData/BGMData", fileName = "BGMData")]
public class BGMData : ScriptableObject
{
    [System.Serializable]
    public class Entry
    {
        public string sceneName;   // 트리거: 씬 이름
        public AudioClip clip;     // 재생할 브금
    }

    public Entry[] entries;

    public AudioClip GetClip(string sceneName)
    {
        foreach (var e in entries)
            if (e.sceneName == sceneName)
                return e.clip;
        return null;
    }
}
