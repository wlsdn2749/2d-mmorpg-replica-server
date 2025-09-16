using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.Networking; // UnityWebRequest
using Newtonsoft.Json;

public class MonsterRegistry : MonoBehaviour
{
    public static MonsterRegistry Instance { get; private set; }

    [Header("Endpoints & Fallback")]
    [SerializeField] private TextAsset localFallbackJson; // Resources/… 에 넣고 인스펙터에 할당

    // 메모리 캐시
    private readonly Dictionary<int, MonsterDefDto> _byId = new();
    public int Version { get; private set; } = 0;
    public bool IsLoaded { get; private set; } = false;

    void Awake()
    {
        if (Instance != null && Instance != this) { Destroy(gameObject); return; }
        Instance = this;
        DontDestroyOnLoad(gameObject);
    }

    void Start()
    {
        StartCoroutine(LoadAsync());
    }

    public MonsterDefDto Get(int typeId)
        => _byId.TryGetValue(typeId, out var def) ? def : null;
    IEnumerator LoadAsync()
    {
        yield return null; // 한 프레임 쉬기

        // 2) 폴백 JSON
        if (localFallbackJson != null)
        {
            if (TryParse(localFallbackJson.text)) { IsLoaded = true; yield break; }
        }

        Debug.LogError("[MonsterRegistry] 폴백까지 실패. Monster DB 없음");
    }

    bool TryParse(string json)
    {
        try
        {
            var txt = json?.Trim();
            if (string.IsNullOrEmpty(txt))
            {
                Debug.LogError("[MonsterRegistry] JSON empty");
                return false;
            }

            // 1) 최상위가 배열이면: [ {...}, {...} ]
            if (txt[0] == '[')
            {
                var list = JsonConvert.DeserializeObject<List<MonsterDefDto>>(txt,
                    new JsonSerializerSettings
                    {
                        MissingMemberHandling = MissingMemberHandling.Ignore,
                        NullValueHandling = NullValueHandling.Ignore
                    });

                _byId.Clear();
                foreach (var m in list) _byId[m.Id] = m;
                Version = 0; // version 없으니 0으로
                Debug.Log($"[MonsterRegistry] Loaded {_byId.Count} monsters (array root)");
                return true;
            }

            // 2) 최상위가 객체이면: { "version":..., "monsters":[...] }
            var dto = JsonConvert.DeserializeObject<MonsterDbDto>(txt,
                new JsonSerializerSettings
                {
                    MissingMemberHandling = MissingMemberHandling.Ignore,
                    NullValueHandling = NullValueHandling.Ignore
                });

            if (dto?.Monsters == null)
            {
                Debug.LogError("[MonsterRegistry] 'monsters' not found in object root");
                return false;
            }

            _byId.Clear();
            foreach (var m in dto.Monsters) _byId[m.Id] = m;
            Version = dto.Version;
            Debug.Log($"[MonsterRegistry] Loaded {_byId.Count} monsters (v{Version})");
            return true;
        }
        catch (System.Exception e)
        {
            // 디버깅에 도움 되는 앞부분 로그
            var head = json != null && json.Length > 200 ? json.Substring(0, 200) : json;
            Debug.LogError($"[MonsterRegistry] Parse error: {e}\nJSON head: {head}");
            return false;
        }
    }
}
