using Google.Protobuf.Protocol;
using Packet;
using UnityEngine;
using UnityEngine.EventSystems;
using UnityEngine.Rendering;

public class HUDManager : MonoBehaviour
{
    public static HUDManager Instance { get; private set; }
    [SerializeField] private GameObject _systemUIPanel; // 시스템 UI 오브젝트
    [SerializeField] private bool _isSystemUIActive = false;
    [SerializeField] private GameObject _playerInfoPanel; // 플레이어 정보 UI 오브젝트
    [SerializeField] private GameObject _inventoryPanel; // 인벤토리 UI 오브젝트    
    [SerializeField] private GameObject _chatPanel; // 채팅창 UI 오브젝트
    [SerializeField] private GameObject _deathPanel;
    void Awake()
    {
        // 중복 인스턴스 방지
        if (Instance == null)
        {
            Instance = this;
            DontDestroyOnLoad(gameObject); 
        }
        else
        {
            Destroy(gameObject);
        }
    }
    private void Start()
    {
        _systemUIPanel.SetActive(false);
        _playerInfoPanel.SetActive(false);
        _inventoryPanel.SetActive(false);   
        _deathPanel.SetActive(false);
        _chatPanel.SetActive(false);
    }
    public void SetPlayerInfo(PlayerStatInfo info)
    {
        _playerInfoPanel.GetComponent<UI_PlayerInfo>().InitInfo(info);
    }
    public void ShowChattingPanel()
    {
        if (Input.GetKeyDown(KeyCode.Return))
        {
            // 포커스가 다른 UI에 있으면 열지 않음
            if (EventSystem.current != null && EventSystem.current.currentSelectedGameObject != null)
                return;

            if (!_chatPanel.activeSelf)
            {
                _chatPanel.SetActive(true);
                _chatPanel.GetComponent<UI_Chat>().OpenChat();
            }
        }
    }
    public void OnLoadingInventoryUI(bool show)
    {
        if (show)
        {
            _inventoryPanel.SetActive(true);
            return;
        }
        else
        {
            _inventoryPanel.SetActive(false);
            return;
        }
    }
    public void ShowInventory_UI()
    {
        if (Input.GetKeyDown(KeyCode.I))
        {
            if (!_inventoryPanel.activeSelf)
            {
                _inventoryPanel.SetActive(true);
                InventoryManager.Instance.RequestInventory();   
                return;
            }
            else 
            {
                _inventoryPanel.SetActive(false);
                return;
            }
        }
    }
    public void OnLoadingOffPlayerInfoUI(bool show)
    {
        if (show)
        {
            _playerInfoPanel.SetActive(true);
            return;
        }
        else
        {
            _playerInfoPanel.SetActive(false);
            return;
        }
    }

    public void OnLoadingOffSystemUI(bool show)
    {
        if (show)
        {
            _isSystemUIActive = true;
            _systemUIPanel.SetActive(_isSystemUIActive);
            return;
        }
        else
        {
            _isSystemUIActive = false;
            _systemUIPanel.SetActive(_isSystemUIActive);
            return;
        }
    }
    public void ShowDeathPanel(bool show)
    {
        if (show )
        {
            _deathPanel.SetActive(true);
        }
        else
        {
            _deathPanel.SetActive(false);
        }
    }
    public void ShowSystemUI()
    {
        if (Input.GetKeyDown(KeyCode.Escape))
        {
            if (_chatPanel.activeSelf) return;
            if (_inventoryPanel.activeSelf)
            {
                _inventoryPanel.SetActive(false);
                return;
            }
            if (_isSystemUIActive == false)
            {
                _isSystemUIActive = true;
                _systemUIPanel.SetActive(_isSystemUIActive);
                return;
            }
            else if (_isSystemUIActive == true)
            {
                _isSystemUIActive = false;
                _systemUIPanel.SetActive(_isSystemUIActive);
                return;
            }
        }
    }
    private void Update()
    {
        if (!_chatPanel.activeSelf)
        {
            ShowSystemUI();
            ShowInventory_UI();
        }
        ShowChattingPanel();
    }
}
