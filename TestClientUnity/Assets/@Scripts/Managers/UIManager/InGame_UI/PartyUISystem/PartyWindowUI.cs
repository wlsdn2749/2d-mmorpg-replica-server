using System.Collections.Generic;
using Google.Protobuf.Protocol;
using TMPro;
using UnityEngine;
using UnityEngine.UI;

public class PartyWindowUI : MonoBehaviour
{
    [Header("List")]
    [SerializeField] Transform _content;                // Scroll View_Party_Window/Viewport/Content
    [SerializeField] PartyListRowUI _rowPrefab;         // Slot_Party 프리팹

    [Header("Top Buttons")]
    [SerializeField] Button _buttonOpenCreateWindow;    // Button_IndicateCreateWindow
    [SerializeField] Button _buttonJoinSelected;        // Button_JoinParty
    [SerializeField] Button _buttonRefreshList;        // Button_RefreshPartyList

    [Header("Create Window")]
    [SerializeField] GameObject _createWindowRoot;      // Party_CreateWindow
    [SerializeField] TMP_InputField _createNameInput;   // InputField (TMP)
    [SerializeField] Button _buttonCreateParty;         // Button_CreateParty
    [SerializeField] Button _buttonExitCreate1;         // Button_ExitCreateWindow
    [SerializeField] Button _buttonExitCreate2;         // Button_ExitCreateWindow2

    [Header("Status Window")]
    [SerializeField] GameObject _statusWindowRoot;      // Party_StatusWindow
    [SerializeField] TMP_Text _statusText;              // Text_PartyStatus
    [SerializeField] Button _buttonStatusOk;            // Button_OK

    private readonly List<PartyListRowUI> _rows = new();
    private PartyListRowUI _selectedRow;
    private int _selectedPartyId = -1;

    private void Awake()
    {
        // 버튼 리스너
        _buttonOpenCreateWindow.onClick.AddListener(OpenCreateWindow);
        _buttonJoinSelected.onClick.AddListener(JoinSelectedParty);
        _buttonRefreshList.onClick.AddListener(RefreshPartyList);

        _buttonCreateParty.onClick.AddListener(OnClickCreateParty);
        _buttonExitCreate1.onClick.AddListener(CloseCreateWindow);
        _buttonExitCreate2.onClick.AddListener(CloseCreateWindow);

        _buttonStatusOk.onClick.AddListener(() => _statusWindowRoot.SetActive(false));

        _createWindowRoot.SetActive(false);
        _statusWindowRoot.SetActive(false);
    }

    private void OnEnable()
    {
        PartyState.Instance.OnPartyList += HandlePartyList;
        PartyState.Instance.OnCreateResult += HandleCreateResult;
        PartyState.Instance.OnJoinResult += HandleJoinResult;

        // 켜질 때마다 최신 리스트 요청
        PartyNet.RequestPartyList();
    }

    private void OnDisable()
    {
        if (PartyState.Instance == null) return;

        PartyState.Instance.OnPartyList -= HandlePartyList;
        PartyState.Instance.OnCreateResult -= HandleCreateResult;
        PartyState.Instance.OnJoinResult -= HandleJoinResult;
    }

    private void ClearRows()
    {
        foreach (var r in _rows)
        {
            if (r)
                Destroy(r.gameObject);
        }
        _rows.Clear();
        _selectedRow = null;
        _selectedPartyId = -1;
    }

    private void HandlePartyList(List<PartyInfo> infos)
    {
        ClearRows();

        if (infos == null || infos.Count == 0)
        {
            ShowStatus("현재 생성된 파티가 없습니다.");
            return;
        }

        foreach (var info in infos)
        {
            var row = Instantiate(_rowPrefab, _content);
            row.Bind(this, info);
            _rows.Add(row);
        }
    }

    // 행에서 호출됨
    public void OnRowSelected(PartyListRowUI row, int partyId)
    {
        _selectedRow = row;
        _selectedPartyId = partyId;
        Debug.Log($"[PartyWindowUI] Selected partyId={partyId}");

        foreach (var r in _rows)
            r.SetSelected(r == row);
    }

    private void JoinSelectedParty()
    {
        if (_selectedPartyId < 0)
        {
            ShowStatus("가입할 파티를 먼저 선택해주세요.");
            return;
        }

        PartyNet.RequestJoin(_selectedPartyId);
        ShowStatus("파티 가입 요청을 보냈습니다.");
    }

    private void OpenCreateWindow()
    {
        _createWindowRoot.SetActive(true);
        _createNameInput.text = string.Empty;
    }

    private void CloseCreateWindow()
    {
        _createWindowRoot.SetActive(false);
    }

    private void OnClickCreateParty()
    {
        string name = _createNameInput.text;
        if (string.IsNullOrWhiteSpace(name))
            name = null;

        PartyNet.Create(name);
        ShowStatus("파티 생성 요청을 보냈습니다.");
        _createWindowRoot.SetActive(false);
    }

    private void HandleCreateResult(bool success, string message)
    {
        ShowStatus(message ?? (success ? "파티 생성 성공" : "파티 생성 실패"));

        if (success)
        {
            PartyNet.RequestPartyList(); // 생성 후 리스트 새로고침
        }
    }

    private void HandleJoinResult(bool success, string message)
    {
        ShowStatus(message ?? (success ? "파티 가입 성공" : "파티 가입 실패"));
        if (success)
        {
            // 실제 파티 HUD 등은 PartyState 브로드캐스트로 갱신될 거라 여기선 메시지 정도만
        }
    }
    private void RefreshPartyList()
    {
        ShowStatus("파티 목록을 새로고침합니다.");
        PartyNet.RequestPartyList();
    }
    private void ShowStatus(string msg)
    {
        if (string.IsNullOrEmpty(msg)) return;
        _statusText.text = msg;
        _statusWindowRoot.SetActive(true);
    }
}
