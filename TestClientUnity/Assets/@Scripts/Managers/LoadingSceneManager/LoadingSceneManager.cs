using System.Collections;
using UnityEngine;
using UnityEngine.SceneManagement;
using UnityEngine.UI;
using TMPro;
using System;

public class LoadingSceneManager : MonoBehaviour
{
    // === 로딩씬 이름 고정 ===
    public const string LOADING_SCENE_NAME = "LoadingScene";
    public static Action OnSceneActivated; // 새 씬이 '활성화'된 직후 1회 호출

    // === 외부에서 세팅되는 다음 씬 정보(정적) ===
    public static string nextSceneName = null;
    public static int nextSceneIndex = -1;
    // === UI 레퍼런스 ===
    [Header("UI")]
    [SerializeField] private Image progressBar;     // 0~1 범위 Fill
    [SerializeField] private TMP_Text loadingText;  // "Loading..." 등
    [SerializeField] private string[] loadingTextVariations; // ["Loading", "Preparing", "Almost there"] 등

    [Header("옵션")]
    [Tooltip("최소 로딩 화면 노출 시간(초)")]
    [SerializeField] private float minDisplayTime = 5.0f;

    [Tooltip("씬 준비(0.9)에 도달하면 자동으로 씬을 활성화할지 여부")]
    [SerializeField] private bool autoActivateOnReady = true;

    [Tooltip("로딩 텍스트를 점(.) 애니메이션으로 보완할지 여부 (Variations 비어있을 때만 동작)")]
    [SerializeField] private bool animateDotsWhenNoVariations = true;

    private Coroutine _textCo;

    // =========================
    // 외부 진입 API
    // =========================
    public static void LoadScene(string sceneName)
    {
        HUDManager.Instance.OnLoadingOffSystemUI(false); // 로딩씬에서는 HUD 숨기기
        HUDManager.Instance.OnLoadingOffPlayerInfoUI(false); // 로딩씬에서는 플레이어 정보 UI 숨기기
        nextSceneName = sceneName;
        nextSceneIndex = -1;
        SceneManager.LoadScene(LOADING_SCENE_NAME);
    }

    public static void LoadScene(int sceneBuildIndex)
    {
        HUDManager.Instance.OnLoadingOffSystemUI(false); // 로딩씬에서는 HUD 숨기기
        HUDManager.Instance.OnLoadingOffPlayerInfoUI(false); // 로딩씬에서는 플레이어 정보 UI 숨기기
        nextSceneIndex = sceneBuildIndex;
        nextSceneName = null;
        SceneManager.LoadScene(LOADING_SCENE_NAME);
    }

    // =========================
    // 로딩씬 수명주기
    // =========================
    private void Start()
    {
        // UI 초기화
        if (progressBar != null) progressBar.fillAmount = 0f;

        // 로딩 텍스트 코루틴
        _textCo = StartCoroutine(CoLoadingText());

        // 실제 씬 로드
        StartCoroutine(CoLoadNextScene());
    }

    private void OnDestroy()
    {
        if (_textCo != null) StopCoroutine(_textCo);
    }

    // =========================
    // 로딩 텍스트 순환/애니메이션
    // =========================
    private IEnumerator CoLoadingText()
    {
        // 1) variations가 있으면 순환
        if (loadingText != null && loadingTextVariations != null && loadingTextVariations.Length > 0)
        {
            int i = 0;
            while (true)
            {
                loadingText.text = loadingTextVariations[i];
                i = (i + 1) % loadingTextVariations.Length;
                yield return new WaitForSeconds(1.0f);
            }
        }
        // 2) 없으면 점 애니메이션
        else if (loadingText != null && animateDotsWhenNoVariations)
        {
            var baseText = "Loading";
            int dots = 0;
            while (true)
            {
                dots = (dots + 1) % 4; // 0..3
                loadingText.text = baseText + new string('.', dots);
                yield return new WaitForSeconds(0.5f);
            }
        }
        else
        {
            yield break;
        }
    }

    // =========================
    // 비동기 씬 로드
    // =========================
    private IEnumerator CoLoadNextScene()
    {
        yield return null; // 한 프레임 양보 (UI 렌더)

        // 대상 결정
        AsyncOperation op = null;
        if (IsValidIndex(nextSceneIndex))
        {
            op = SceneManager.LoadSceneAsync(nextSceneIndex);
        }
        else if (!string.IsNullOrEmpty(nextSceneName))
        {
            op = SceneManager.LoadSceneAsync(nextSceneName);
        }
        else
        {
            Debug.LogError("[LoadingSceneManager] 다음 씬 정보가 없습니다. LoadScene(int) 또는 LoadScene(string)으로 먼저 호출하세요.");
            yield break;
        }

        // 자동 활성화는 일단 막아두고, 0.9 도달시 UI/연출 마무리 후 활성화
        op.allowSceneActivation = false;

        float shownTime = 0f;
        while (!op.isDone)
        {
            shownTime += Time.unscaledDeltaTime;

            // Unity는 0.9까지 로딩, 이후 allowSceneActivation=true가 되면 1.0로 점프
            float progress = Mathf.Clamp01(op.progress / 0.9f);
            if (progressBar != null)
                progressBar.fillAmount = progress;

            // 준비 완료 구간
            if (op.progress >= 0.9f)
            {
                if (autoActivateOnReady && shownTime >= minDisplayTime)
                {
                    //  새 씬이 '활성화'되는 그 프레임을 포착하기 위해 이벤트 구독
                    void HandleSceneLoaded(Scene s, LoadSceneMode m)
                    {
                        // 다음 씬 판단(이름/인덱스 어느쪽으로 호출했는지에 따라)
                        bool isTarget = (!string.IsNullOrEmpty(nextSceneName) && s.name == nextSceneName)
                                        || (IsValidIndex(nextSceneIndex) && s.buildIndex == nextSceneIndex);

                        if (isTarget)
                        {
                            SceneManager.sceneLoaded -= HandleSceneLoaded;
                            // 정적 상태 초기화(다음 로딩 대비)
                            nextSceneName = null;
                            nextSceneIndex = -1;
                            // 🔽 외부에 "활성화 완료" 알림
                            try { OnSceneActivated?.Invoke(); }
                            finally { OnSceneActivated = null; }
                        }
                    }

                    SceneManager.sceneLoaded += HandleSceneLoaded;
                    op.allowSceneActivation = true; // 이때 다음 씬으로 스위칭
                    HUDManager.Instance.OnLoadingOffPlayerInfoUI(true);
                }
            }
            
            yield return null;
        }

        // 씬이 활성화되고 로딩씬이 빠져나간 시점
        // 정적 상태 초기화(다음 로딩 대비)
        nextSceneName = null;
        nextSceneIndex = -1;
    }

    private static bool IsValidIndex(int idx)
    {
        return idx >= 0 && idx < SceneManager.sceneCountInBuildSettings;
    }
}