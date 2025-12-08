using TMPro;
using UnityEngine;
using UnityEngine.UI;

public class PlayerContextMenuUI : MonoBehaviour
{
    public static PlayerContextMenuUI Instance { get; private set; }

    [Header("UI Refs")]
    [SerializeField] private TMP_Text _nameText;                 // Text_PlayerName
    [SerializeField] private Button _buttonInviteParty;          // Button_Player_InviteParty
    [SerializeField] private Button _buttonWhisper;              // Button_Player_Whisper
    [SerializeField] private Button _buttonTrade;                // Button_Player_Trade
    [SerializeField] private Button _buttonInviteGuild;          // Button_Player_InviteGuild
    [SerializeField] private Button _buttonClose;                // Button_Close (선택 사항)

    int _targetPid = -1;
    string _targetName;
    Vector3 _worldPos;

    void Awake()
    {
        if (Instance != null && Instance != this)
        {
            Destroy(gameObject);
            return;
        }
        Instance = this;

        gameObject.SetActive(false);

        if (_buttonInviteParty != null)
            _buttonInviteParty.onClick.AddListener(OnClickInviteParty);

        if (_buttonWhisper != null)
            _buttonWhisper.onClick.AddListener(OnClickWhisper);

        if (_buttonTrade != null)
            _buttonTrade.onClick.AddListener(OnClickTrade);

        if (_buttonInviteGuild != null)
            _buttonInviteGuild.onClick.AddListener(OnClickInviteGuild);

        if (_buttonClose != null)
            _buttonClose.onClick.AddListener(Close);
    }

    /// <summary>
    /// 다른 플레이어를 클릭했을 때 호출.
    /// worldPos는 플레이어 머리 위 정도 위치를 넣어주면 좋다.
    /// </summary>
    public void Open(int targetPid, string targetName, Vector3 worldPos)
    {
        _targetPid = targetPid;
        _targetName = targetName;
        _worldPos = worldPos;

        if (_nameText != null)
            _nameText.text = targetName;

        UpdateScreenPosition();
        gameObject.SetActive(true);
    }

    public void Close()
    {
        gameObject.SetActive(false);
        _targetPid = -1;
        _targetName = null;
    }

    void UpdateScreenPosition()
    {
        if (Camera.main == null)
            return;

        Vector3 screenPos = Camera.main.WorldToScreenPoint(_worldPos);
        transform.position = screenPos;

        // 필요하면 화면 밖으로 나가지 않도록 클램프 로직 추가 가능
    }

    void OnClickInviteParty()
    {
        if (_targetPid < 0) return;

        Debug.Log($"[PlayerContextMenuUI] 파티 초대: pid={_targetPid}, name={_targetName}");
        PartyNet.SendInvite(_targetPid);
        Close();
    }

    void OnClickWhisper()
    {
        // TODO: 귓속말 시스템 붙일 때 구현
        Debug.Log($"[PlayerContextMenuUI] 귓속말 TODO: name={_targetName}");
        Close();
    }

    void OnClickTrade()
    {
        // TODO: 거래 시스템 붙일 때 구현
        Debug.Log($"[PlayerContextMenuUI] 교환 신청 TODO: name={_targetName}");
        Close();
    }

    void OnClickInviteGuild()
    {
        // TODO: 길드 초대 시스템 붙일 때 구현
        Debug.Log($"[PlayerContextMenuUI] 길드 초대 TODO: name={_targetName}");
        Close();
    }
    private void Update()
    {
        if (gameObject.activeSelf)
        {
            if (Input.GetKeyDown(KeyCode.Escape))
            {
                Close();
            }
        }
    }
}

