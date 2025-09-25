using Google.Protobuf.Protocol;
using TMPro;
using UnityEngine;
using UnityEngine.EventSystems;
using UnityEngine.UI;

public class UI_Chat : MonoBehaviour
{
    [Header("Refs")]
    [SerializeField] GameObject _panel;
    [SerializeField] TMP_InputField _input;
    [SerializeField] TMP_Text _chatTypeText;
    [SerializeField] ScrollRect _scrollRect;
    [SerializeField] Transform _content;
    [SerializeField] GameObject _linePrefab;

    [Header("옵션")]
    [SerializeField] EChatType _defaultType = EChatType.ChatRoom;
    [SerializeField] bool closeOnSend = true;

    private EChatType currentType;

    void Awake()
    {
        currentType = _defaultType;
        SetPanel(false);

        // ✅ 엔터로 제출되도록 보장
        if (_input)
        {
            // SingleLine 또는 MultiLineSubmit 권장
            _input.lineType = TMP_InputField.LineType.SingleLine;

            // ✅ onSubmit으로 전송 처리 (IME 안전)
            _input.onSubmit.RemoveListener(OnSubmit); // 중복 방지
            _input.onSubmit.AddListener(OnSubmit);
        }
    }
    private bool IsAlive(Object o) => o != null && !ReferenceEquals(o, null);

    void OnEnable()
    {
        // 구독
        if (ChatManager.Instance != null)
        {
            ChatManager.Instance.OnMessageAdded -= OnMessageAdded; // 중복 방지
            ChatManager.Instance.OnMessageAdded += OnMessageAdded;
        }

        // ✅ Content 살아있는지 점검 후 리바인딩
        if (!IsAlive(_content))
        {
            var canvas = GetComponentInParent<Canvas>();
            if (!canvas) canvas = FindObjectOfType<Canvas>();

            // 너의 계층 이름에 맞춰 경로 수정
            var t = canvas ? canvas.transform.Find("Panel_ChatLog/Scroll View/Viewport/Content") : null;
            if (t) _content = t;
            Debug.Log(_content
                ? "[ChatUI] Content re-bound after scene load"
                : "[ChatUI] Content NOT found. Please set UI_Chat._content in inspector.");
        }

        // 과거 로그 리빌드
        RebuildAll();
    }

    void OnDisable()
    {
        if (ChatManager.Instance != null)
            ChatManager.Instance.OnMessageAdded -= OnMessageAdded;
    }

    void Update()
    {
        // 패널 꺼져 있으면 무시
        if (!_panel.activeSelf) return;

        // Tab = 타입 전환
        if (_input.isFocused && (Input.GetKeyDown(KeyCode.Tab)))
        {
            ToggleType();
            return;
        }

        // Esc = 닫기
        if (Input.GetKeyDown(KeyCode.Escape))
        {
            CloseChat(clearText: true);
            return;
        }
    }

    // === onSubmit 콜백 (Enter/KeypadEnter 둘 다 처리됨, IME 조합 완료 후에만 호출) ===
    private void OnSubmit(string submittedText)
    {
        var msg = (submittedText ?? "").Trim();
        if (msg.Length > 0)
        {
            ChatManager.Instance.Send(msg, currentType);
        }

        if (closeOnSend) CloseChat(clearText: true);
        else
        {
            _input.text = "";
            _input.ActivateInputField();
            _input.MoveTextEnd(false);
        }
    }

    // === 열고/닫기 ===
    public void OpenChat()
    {
        SetPanel(true);
        _input.text = "";
        if (_chatTypeText)
            _chatTypeText.text = currentType == EChatType.ChatAll ? "전체" : "지역";

        //  열 때도 한 번 더 재빌드 (구독 전 들어온 메시지 반영)
        RebuildAll();

        EventSystem.current?.SetSelectedGameObject(_input.gameObject);
        _input.ActivateInputField();
        _input.MoveTextEnd(false);
    }
    private void RebuildAll()
    {
        if (_content == null || _linePrefab == null || ChatManager.Instance == null) return;

        // 자식 모두 제거
        for (int i = _content.childCount - 1; i >= 0; i--)
            Destroy(_content.GetChild(i).gameObject);

        foreach (var m in ChatManager.Instance.GetMessages())
            SpawnLine(m);

        ScrollToBottom();
    }
    private void CloseChat(bool clearText)
    {
        if (clearText) _input.text = "";
        SetPanel(false);

        if (EventSystem.current?.currentSelectedGameObject == _input.gameObject)
            EventSystem.current.SetSelectedGameObject(null);
    }

    private void SetPanel(bool on)
    {
        if (_panel) _panel.SetActive(on);
    }

    // === 타입 전환 ===
    private void ToggleType()
    {
        currentType = (currentType == EChatType.ChatRoom) ? EChatType.ChatAll : EChatType.ChatRoom;

        if (_chatTypeText)
            _chatTypeText.text = currentType == EChatType.ChatAll ? "전체" : "지역";
    }

    // === 로그 표시 ===
    private void OnMessageAdded(ChatMessage m)
    {
        if (_content == null || _linePrefab == null) return;
        SpawnLine(m);
        ScrollToBottom();
    }
    private void SpawnLine(ChatMessage m)
    {
        if (!IsAlive(_linePrefab)) { Debug.LogError("[ChatUI] _linePrefab is null"); return; }
        if (!IsAlive(_content)) { Debug.LogError("[ChatUI] _content is null or destroyed"); return; }

        Debug.Log($"[ChatUI] Spawning line for: {m.message}");

        // parent를 확실히 적용하기 위해 GameObject 오버로드 사용
        var go = Instantiate(_linePrefab.gameObject);
        go.transform.SetParent(_content, worldPositionStays: false);


        var line = go.GetComponent<UI_ChatLine>();
        if (!line) { Debug.LogError("[ChatUI] UI_ChatLine missing on prefab root"); return; }

        string typePrefix = m.chatType == EChatType.ChatAll ? "[전체]" : "[지역]";
        
        int playerID = m.playerId.Value;
        var player = PlayerSpawner.Get(playerID);
        string playerName = player.GetComponent<PlayerIdentity>().Username;
        line.SetText($"{typePrefix}{playerName}: {m.message}",m.chatType);

        LayoutRebuilder.ForceRebuildLayoutImmediate(_content as RectTransform);
    }

    private void ScrollToBottom()
    {
        if (_scrollRect == null) return;
        Canvas.ForceUpdateCanvases();
        _scrollRect.verticalNormalizedPosition = 0f;
        Canvas.ForceUpdateCanvases();
    }
}
