using UnityEngine;
using UnityEngine.SceneManagement;
using System.Collections;

public class BGMManager : MonoBehaviour
{
    public static BGMManager Instance { get; private set; }

    [SerializeField] private BGMData _bgmData;
    [SerializeField] private AudioSource _audioSource;

    [Header("옵션")]
    [SerializeField] private float _fadeTime = 1.0f;

    void Awake()
    {
        if (Instance != null && Instance != this) { Destroy(gameObject); return; }
        Instance = this;
        DontDestroyOnLoad(gameObject);

        if (!_audioSource) _audioSource = gameObject.AddComponent<AudioSource>();
        _audioSource.loop = true;

        SceneManager.sceneLoaded += OnSceneLoaded;
    }

    void OnDestroy()
    {
        SceneManager.sceneLoaded -= OnSceneLoaded;
    }

    private void OnSceneLoaded(Scene scene, LoadSceneMode mode)
    {
        PlayBGMForScene(scene.name);
    }

    public void PlayBGMForScene(string sceneName)
    {
        var clip = _bgmData.GetClip(sceneName);
        if (clip == null)
        {
            Debug.LogWarning($"[BGM] No BGM set for scene '{sceneName}'");
            return;
        }

        if (_audioSource.clip == clip) return; // 이미 같은 브금이면 무시
        StartCoroutine(PlayBgmAsync(_audioSource,clip,_fadeTime));
    }

    IEnumerator PlayBgmAsync(AudioSource src, AudioClip clip, float fade = 1f)
    {
        src.volume = 0f;
        src.clip = clip;
        // 필요하면 명시적으로 로드 시작
        if (!clip.preloadAudioData) clip.LoadAudioData();

        // 준비될 때까지 대기
        while (clip.loadState == AudioDataLoadState.Loading)
            yield return null;

        // 실패 방어
        if (clip.loadState != AudioDataLoadState.Loaded)
        {
            Debug.LogWarning("[BGM] clip load failed");
            yield break;
        }

        src.Play();

        // 페이드인
        float t = 0f;
        while (t < fade) { t += Time.unscaledDeltaTime; src.volume = Mathf.Lerp(0f, 1f, t / fade); yield return null; }
        src.volume = 1f;
    }
}