using Google.Protobuf.Protocol;
using UnityEngine;
using UnityEngine.Rendering;

public class HUDManager : MonoBehaviour
{
    public static HUDManager Instance { get; private set; }
    [SerializeField] private GameObject _systemUIPanel; // 시스템 UI 오브젝트
    [SerializeField] private bool _isSystemUIActive = false;
    [SerializeField] private GameObject _playerInfoPanel; // 플레이어 정보 UI 오브젝트
    void Awake()
    {
        // 중복 인스턴스 방지
        if (Instance == null)
        {
            Instance = this;
            DontDestroyOnLoad(gameObject); // 씬 전환 시에도 파괴되지 않도록 설정
            Debug.Log("HUDManager Awake() 호출됨");
        }
        else
        {
            Destroy(gameObject); // 이미 인스턴스가 존재하면 자신을 파괴합니다.
        }
    }
    private void Start()
    {
        _systemUIPanel.SetActive(false);
    }
    public void SetPlayerInfo(PlayerStatInfo info)
    {
        _playerInfoPanel.GetComponent<UI_PlayerInfo>().InitInfo(info);
    }
    public void OnOffPlayerInfoUI(bool on)
    {
        if (on)
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
    public void OnOffSystemUI(bool on)
    {
        if (on)
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
    public void ShowSystemUI()
    {
        if (Input.GetKeyDown(KeyCode.Escape))
        {
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
        ShowSystemUI();
    }
}
