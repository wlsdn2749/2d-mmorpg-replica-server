using UnityEngine;
using UnityEngine.UI;

public class PartyWindowToggle : MonoBehaviour
{
    [SerializeField] private GameObject partyListWindow; // Window_PartyList
    [SerializeField] private GameObject myPartyWindow;   // Window_MyParty
    [SerializeField] private Button CancleMyPartyUIButton;    

    void Awake()
    {
        if (partyListWindow) partyListWindow.SetActive(false);
        if (myPartyWindow) myPartyWindow.SetActive(false);
        if (CancleMyPartyUIButton)
            CancleMyPartyUIButton.onClick.AddListener(ToggleUI);
    }
    void OnEnable()
    {
        // 파티 상태가 바뀔 때마다 자동으로 창 전환을 결정
        PartyState.Instance.OnPartyChanged += HandlePartyChanged;
        PartyState.Instance.OnCreateResult += OpenMyPartyWindow;    
        PartyState.Instance.OnKicked += HandleKickedFromParty;   //  추가
        PartyState.Instance.OnInviteResult += OpenMyPartyWindow;
    }

    void OnDisable()
    {
        if (PartyState.Instance != null)
        {
            PartyState.Instance.OnPartyChanged -= HandlePartyChanged;
            PartyState.Instance.OnKicked -= HandleKickedFromParty; //  추가
            PartyState.Instance.OnCreateResult -= OpenMyPartyWindow;
            PartyState.Instance.OnInviteResult -= OpenMyPartyWindow;    
        }
    }
    void Update()
    {
        if (Input.GetKeyDown(KeyCode.P))
            ToggleUI();
    }
    void HandleKickedFromParty()
    {
        // 1) 내 파티창은 꺼지고
        if (myPartyWindow != null)
            myPartyWindow.SetActive(false);

        // 2) 파티 리스트 창은 켜준다
        if (partyListWindow != null)
            partyListWindow.SetActive(true);

        // 3) Panel_PartyWindow(root)도 활성화 (이 컴포넌트가 붙어있는 오브젝트)
        gameObject.SetActive(true);

        // 4) Party_StatusWindow에 "강퇴되었습니다" 메세지 표시
        PartyWindowUI partyUI = null;

        if (partyListWindow != null)
            partyUI = partyListWindow.GetComponentInChildren<PartyWindowUI>(true);

        if (partyUI == null)
            partyUI = FindAnyObjectByType<PartyWindowUI>();

        if (partyUI != null)
        {
            partyUI.ShowStatus("파티에서 강퇴되었습니다.");
        }
    }
    void ToggleUI()
    {
        bool inParty = PartyState.Instance.InParty;

        if (inParty)
        {
            bool newState = !myPartyWindow.activeSelf;
            myPartyWindow.SetActive(newState);
            if (partyListWindow) partyListWindow.SetActive(false);
        }
        else
        {
            bool newState = !partyListWindow.activeSelf;
            partyListWindow.SetActive(newState);
            if (myPartyWindow) myPartyWindow.SetActive(false);

            if (newState)
                PartyNet.RequestPartyList();
        }
    }
    public void OpenMyPartyWindow(bool success, string message)
    {
        if (!success)
        {
            Debug.Log($"[PartyWindow Toggle]Open My Party Window failed: {message}");
            return;
        }   
        if (myPartyWindow == null || partyListWindow == null)
        {
            Debug.Log("[PartyWindow Toggle]Open My Party Window error");
            return;
        }

        // 파티 생성 성공 → 바로 내 파티창으로 이동
        partyListWindow.SetActive(false);
        myPartyWindow.SetActive(true);
    }
    //  파티 상태 변경 시 호출되는 콜백
    void HandlePartyChanged()
    {
        var ps = PartyState.Instance;

        if (ps.InParty)
        {
            // 내가 파티에 없었다가 (혹은 리스트 보고 있다가) 이제 파티에 들어간 순간
            // 그리고 지금 파티 리스트 창이 떠 있는 상태라면 → MyParty로 전환
            if (partyListWindow != null && partyListWindow.activeSelf)
            {
                partyListWindow.SetActive(false);
                if (myPartyWindow != null)
                    myPartyWindow.SetActive(true);
            }
        }
        else
        {
            // 파티가 아예 없어졌을 때 MyParty 창이 켜져 있으면 꺼준다(안전빵)
            if (myPartyWindow != null && myPartyWindow.activeSelf)
            {
                myPartyWindow.SetActive(false);
                // 리스트를 자동으로 열지는 말고, 사용자가 P를 누를 때 열도록 유지
            }
        }
    }

}
