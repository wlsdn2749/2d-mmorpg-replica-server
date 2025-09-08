using Google.Protobuf.Protocol;
using Packet;
using System.Collections.Generic;
using TMPro;
using UnityEngine;
using UnityEngine.UI;

public class CharacterList_UI : MonoBehaviour
{
    public static CharacterList_UI Instance { get; private set; }
    [SerializeField] private Transform _contentParent;         // 리스트 부모(Grid)
    [SerializeField] private CharacterSlot_UI _slotPrefab;  // 캐릭터 슬롯 프리팹
    [SerializeField] private TMP_Text _emptyListText;
    [SerializeField] private TMP_Text _selectedCharacterName;
    [SerializeField] private Button _createBtn;
    [SerializeField] private Button _characterDeleteBtn;
    [SerializeField] private Button _startBtn;
    [SerializeField] private GameObject _createCharacterPanel;
    private readonly List<CharacterSlot_UI> _slots = new();
    [SerializeField] private int _selectedIndex = -1;
    private void Awake()
    {
        if (Instance == null)
        {
            Instance = this;
        }
        if (_createBtn) _createBtn.onClick.AddListener(OnClickCreateCharacter);
        if (_characterDeleteBtn) _characterDeleteBtn.onClick.AddListener(OnClickShowCharacterDeleteUI);
        if (_startBtn) _startBtn.onClick.AddListener(OnClickStartGame);
        _selectedCharacterName.text = "";
        gameObject.SetActive(false);
    }
    private void OnEnable()
    {
        RequestCharacterList();
    }
    public void RequestCharacterList()
    {
        var req = new C_CharacterListRequest();
        var send = ServerPacketManager.MakeSendBuffer(req);
        NetworkManager.Instance.Send(send);
        UnityEngine.Debug.Log($"[UI] 캐릭터 리스트 전송 요청: playerIndex={0}, len={send.Count}");
    }
    public void SetCharacterList(IList<CharacterSummaryInfo> list)
    {
        if (list.Count == 0)
        {
            EmptyListTextEnable(true);
        }
        else
        {
            EmptyListTextEnable(false);
        }
        foreach (Transform c in _contentParent) Destroy(c.gameObject);
        _slots.Clear();
        _selectedIndex = -1;
        _startBtn.interactable = false;
        _characterDeleteBtn.interactable = false;

        _selectedCharacterName.text = $"선택한 캐릭터 : ";
        for (int i = 0; i < list.Count; i++)
        {
            var slot = Instantiate(_slotPrefab, _contentParent);
            slot.SetupSlot(list[i], i, OnSlotSelected);
            _slots.Add(slot);
        }
    }
    private void OnSlotSelected(int index,string name)
    {
        _selectedIndex = index;
        _startBtn.interactable = true;
        _characterDeleteBtn.interactable = true;
        _selectedCharacterName.text = $"선택한 캐릭터 : {name}";
        UpdateHighlights();
    }
    
    private void UpdateHighlights()
    {
        for (int i = 0; i < _slots.Count; i++)
        {
             _slots[i].SetSelected(i == _selectedIndex);
        }
    }
    private void OnClickStartGame()
    {
        if (_selectedIndex < 0) return;

        // 여기서만 입장 패킷 전송
        var req = new C_EnterGame { PlayerIndex = _selectedIndex };
        var sendBuffer = ServerPacketManager.MakeSendBuffer(req);
        NetworkManager.Instance.Send(sendBuffer);
        Debug.Log($"캐릭터 접속 패킷 전송 \n캐릭터명 : {_selectedCharacterName}");
        Debug.Log($"[UI] 게임 접속 요청 전송: playerIndex={0}, len={sendBuffer.Count}");
        AuthNotice_UI.Instance.gameObject.SetActive(true);
        AuthNotice_UI.Instance.ShowNotice(NoticeCode.EnterGame);
        // 선택: 로딩 표시
        _startBtn.interactable = false;
        // ShowLoading(true);
    }
    private void OnClickShowCharacterDeleteUI()
    {
        if (_selectedIndex < 0) return;
        AuthNotice_UI.Instance.gameObject.SetActive(true);
        AuthNotice_UI.Instance.ShowNotice(NoticeCode.CharacterDelete);
    }
    public void DeleteCharacter()
    {
        var req = new C_DeleteCharacterRequest { CharacterIndex = _selectedIndex };
        var sendBuffer = ServerPacketManager.MakeSendBuffer(req);
        NetworkManager.Instance.Send(sendBuffer);
    }
    void OnClickCreateCharacter()
    {
        _createCharacterPanel.SetActive(true);
        gameObject.SetActive(false);
    }
    void EmptyListTextEnable(bool enable)
    {
        _emptyListText.enabled = enable;
    }
}
