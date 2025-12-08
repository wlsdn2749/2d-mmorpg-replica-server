using TMPro;
using UnityEngine;
using UnityEngine.UI;
using Google.Protobuf.Protocol;

public class PartyInvitePopupUI : MonoBehaviour
{
    [Header("UI")]
    [SerializeField] private GameObject _panelBackground; // Panel_Background
    [SerializeField] private TMP_Text _messageText;     // Text_Message
    [SerializeField] private Button _buttonAccept;      // Button_Accept
    [SerializeField] private Button _buttonDecline;     // Button_Decline

    int _currentPartyId = -1;
    int _inviterPid;
    string _inviterName;

    void Awake()
    {
        if (_buttonAccept != null)
            _buttonAccept.onClick.AddListener(OnClickAccept);

        if (_buttonDecline != null)
            _buttonDecline.onClick.AddListener(OnClickDecline);

        
    }
    private void Start()
    {
        _panelBackground.SetActive(false);
    }

    void OnEnable()
    {
        // 초대 받았을 때
        PartyState.Instance.OnInvite += HandleInviteNotify;
        // 초대 응답 결과 (성공/실패 메시지)
        PartyState.Instance.OnInviteResult += HandleInviteResult;
    }

    void OnDisable()
    {
        if (PartyState.Instance == null) return;

        PartyState.Instance.OnInvite -= HandleInviteNotify;
        PartyState.Instance.OnInviteResult -= HandleInviteResult;
    }

    // 서버에서 S_PartyInviteNotify 들어오면 PartyState가 여기로 던져줌
    void HandleInviteNotify(int inviterPid, string inviterName, int partyId)
    {
        _inviterPid = inviterPid;
        _inviterName = inviterName;
        _currentPartyId = partyId;

        if (_messageText != null)
        {
            // 원하는 문구로 바꿔도 됨
            _messageText.text = $"{_inviterName} 님이 파티에 초대했습니다.\n\n수락하시겠습니까?";
        }

        SetButtonsInteractable(true);
        _panelBackground.SetActive(true);
    }

    void SetButtonsInteractable(bool value)
    {
        if (_buttonAccept != null) _buttonAccept.interactable = value;
        if (_buttonDecline != null) _buttonDecline.interactable = value;
    }

    void OnClickAccept()
    {
        if (_currentPartyId < 0) return;

        Debug.Log($"[PartyInvitePopupUI] Accept invite. partyId={_currentPartyId}, inviter={_inviterName}");
        PartyNet.RespondInvite(_currentPartyId, true);

        // 중복 클릭 방지
        SetButtonsInteractable(false);
        Close();
    }

    void OnClickDecline()
    {
        if (_currentPartyId < 0) { Close(); return; }

        Debug.Log($"[PartyInvitePopupUI] Decline invite. partyId={_currentPartyId}, inviter={_inviterName}");
        PartyNet.RespondInvite(_currentPartyId, false);

        SetButtonsInteractable(false);
        // 거절은 굳이 결과 기다리지 않고 바로 닫아도 됨
        Close();
    }

    // S_PartyInviteReply 들어왔을 때
    void HandleInviteResult(bool success, string errorMessage)
    {
        if (_currentPartyId < 0)
        {
            // 현재 처리 중인 초대가 없으면 무시
            return;
        }

        string msg;
        if (success)
            msg = "파티 초대를 수락했습니다.";
        else
            msg = string.IsNullOrEmpty(errorMessage) ? "파티 초대 처리에 실패했습니다." : errorMessage;

        Debug.Log($"[PartyInvitePopupUI] Invite result: success={success}, msg={msg}");

        // 여기서 Party_StatusWindow 같은 데로 넘겨서 보여주고 싶으면:
        // var partyWindow = FindAnyObjectByType<PartyWindowUI>();
        // partyWindow?.ShowStatus(msg);

        Close();
    }

    void Close()
    {
        _currentPartyId = -1;
        _inviterPid = -1;
        _inviterName = null;
        _panelBackground.SetActive(false);
    }
}
