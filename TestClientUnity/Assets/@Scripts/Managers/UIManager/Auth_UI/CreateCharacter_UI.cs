using Google.Protobuf.Protocol;
using Packet;
using System.Collections;
using System.Text.RegularExpressions;
using TMPro;
using UnityEngine;
using UnityEngine.EventSystems;
using UnityEngine.UI;
public enum Gender
{
    NONE = 0,
    MALE,
    FEMALE
}
public enum Region
{
    NONE = 0,
    GOGURYEO,
    BAEKJE
}

public class CreateCharacter_UI : MonoBehaviour
{
    public static CreateCharacter_UI Instance { get; private set; }
    /// <summary>
    /// 서버에 보낼 패킷 
    /// 1. 유저 이름
    /// 2. 성별 (0 = NONE, 1 = MALE, 2 = FEMALE)
    /// 3. 지역 (0 = NONE, 1 = GOGURYEO, 2 = BAEKJE)
    /// </summary>
    [SerializeField] private Gender _selectedGender;
    [SerializeField] private Region _selectedRegion;
    #region Images
    [SerializeField] private Image _maleCharacterImg;
    [SerializeField] private Image _femaleCharacterImg;
    [Tooltip("0 = 남자, 1 = 여자")]
    [SerializeField] private Image[] _selectedGenderSealImg; 
    [Tooltip("0 = 고구려, 1 = 백제")]
    [SerializeField] private Image[] _selectedRegionSealImg; 
    #endregion
    #region Buttons
    [Tooltip("0 = 남자, 1 = 여자")]
    [SerializeField] private Button _selectMaleBtn;
    [SerializeField] private Button _selectFemaleBtn;
    [Tooltip("0 = 고구려, 1 = 백제")]
    [SerializeField] private Button _selectGoguryeoBtn;
    [SerializeField] private Button _selectBaekjeBtn;
    [SerializeField] private Button _tryCreateCharacterBtn;
    #endregion
    #region InputFields
    [SerializeField] TMP_InputField _userNameField;
    #endregion
    #region Variables
    #endregion
    private void OnEnable()
    {
        // 패널이 켜질 때 ID 필드에 자동 포커스
        StartCoroutine(FocusIdNextFrame());
    }
    private IEnumerator FocusIdNextFrame()
    {
        // UI 그려지는 프레임 한두 번 기다렸다가 포커스
        yield return null;
        yield return null;

        if (_userNameField == null || EventSystem.current == null)
            yield break;

        EventSystem.current.SetSelectedGameObject(_userNameField.gameObject);
        _userNameField.Select();
        _userNameField.ActivateInputField();
    }
    private void Awake()
    {
        if (Instance == null)
        {
            Instance = this;
        }
        if (_selectMaleBtn) _selectMaleBtn.onClick.AddListener(OnClickSelectMale);
        if (_selectFemaleBtn) _selectFemaleBtn.onClick.AddListener(OnClickSelectFemale);

        if (_selectGoguryeoBtn) _selectGoguryeoBtn.onClick.AddListener(OnClickSelectGoguryeo);
        if(_selectBaekjeBtn) _selectBaekjeBtn.onClick.AddListener(OnClickSelectBaekje);

        if (_tryCreateCharacterBtn) _tryCreateCharacterBtn.onClick.AddListener(OnClickTryCreateCharacter);

        _maleCharacterImg.enabled = false;
        _femaleCharacterImg.enabled = false;
        for (int i = 0; i < _selectedGenderSealImg.Length; i++)
        {
            _selectedGenderSealImg[i].enabled = false;
        }
        for (int i = 0; i < _selectedRegionSealImg.Length; i++)
        {
            _selectedRegionSealImg[i].enabled = false;
        }
    }
    private void OnClickSelectMale()
    {
        _maleCharacterImg.enabled = true;
        _femaleCharacterImg.enabled = false;
        _selectedGenderSealImg[0].enabled = true;
        _selectedGenderSealImg[1].enabled = false;
        _selectedGender = Gender.MALE;
    }
    private void OnClickSelectFemale()
    {
        _maleCharacterImg.enabled = false;
        _femaleCharacterImg.enabled = true;
        _selectedGenderSealImg[0].enabled = false;
        _selectedGenderSealImg[1].enabled = true;
        _selectedGender = Gender.FEMALE;
    }
    private void OnClickSelectGoguryeo()
    {
        _selectedRegionSealImg[0].enabled = true;
        _selectedRegionSealImg[1].enabled = false;
        _selectedRegion = Region.GOGURYEO;
    }
    private void OnClickSelectBaekje()
    {
        _selectedRegionSealImg[0].enabled = false;
        _selectedRegionSealImg[1].enabled = true;
        _selectedRegion = Region.BAEKJE;
    }
    public bool IsValidNickname(string userName)
    {
        string name = userName.Trim();
        return Regex.IsMatch(name, @"^[가-힣]{2,6}$");
    }
    private void OnClickTryCreateCharacter()
    {
        if (string.IsNullOrEmpty(_userNameField.text))
        {
            AuthNotice_UI.Instance.gameObject.SetActive(true);
            AuthNotice_UI.Instance.ShowNotice(NoticeCode.FailCreateCharacterName);
            return;
        }
        if (IsValidNickname(_userNameField.text) == false)
        {
            AuthNotice_UI.Instance.gameObject.SetActive(true);
            AuthNotice_UI.Instance.ShowNotice(NoticeCode.FailCreateCharacterNameWrong);
            return;
        }
        if (_selectedGender == Gender.NONE)
        {
            AuthNotice_UI.Instance.gameObject.SetActive(true);
            AuthNotice_UI.Instance.ShowNotice(NoticeCode.FailCreateCharacterGender);
            return;
        }
        if (_selectedRegion == Region.NONE)
        {
            AuthNotice_UI.Instance.gameObject.SetActive(true);
            AuthNotice_UI.Instance.ShowNotice(NoticeCode.FailCreateCharacterRegion);
            return;
        }
        var req = new C_CreateCharacterRequest { Username = _userNameField.text, Gender = (EGender)_selectedGender, Region = (ERegion)_selectedRegion };
        var sendBuffer = ServerPacketManager.MakeSendBuffer(req);

        NetworkManager.Instance.Send(sendBuffer);
        Debug.Log($"캐릭터 생성 요청 전송\n캐릭터 이름 : {_userNameField.text}\n 성별 : {_selectedGender}\n 지역 : {_selectedRegion} ");

    }
    void Start()
    {
        gameObject.SetActive(false);
    }
}
