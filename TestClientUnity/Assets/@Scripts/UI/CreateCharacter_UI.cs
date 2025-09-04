using Google.Protobuf.Protocol;
using Mono.Cecil.Cil;
using Packet;
using System.Text.RegularExpressions;
using TMPro;
using UnityEngine;
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
    /// ¼­¹ö¿¡ º¸³¾ ÆÐÅ¶ 
    /// 1. À¯Àú ÀÌ¸§
    /// 2. ¼ºº° (0 = NONE, 1 = MALE, 2 = FEMALE)
    /// 3. Áö¿ª (0 = NONE, 1 = GOGURYEO, 2 = BAEKJE)
    /// </summary>
    [SerializeField] private Gender _selectedGender;
    [SerializeField] private Region _selectedRegion;
    #region Images
    [SerializeField] private Image _maleCharacterImg;
    [SerializeField] private Image _femaleCharacterImg;
    [Tooltip("0 = ³²ÀÚ, 1 = ¿©ÀÚ")]
    [SerializeField] private Image[] _selectedGenderSealImg; 
    [Tooltip("0 = °í±¸·Á, 1 = ¹éÁ¦")]
    [SerializeField] private Image[] _selectedRegionSealImg; 
    #endregion
    #region Buttons
    [Tooltip("0 = ³²ÀÚ, 1 = ¿©ÀÚ")]
    [SerializeField] private Button _selectMaleBtn;
    [SerializeField] private Button _selectFemaleBtn;
    [Tooltip("0 = °í±¸·Á, 1 = ¹éÁ¦")]
    [SerializeField] private Button _selectGoguryeoBtn;
    [SerializeField] private Button _selectBaekjeBtn;
    [SerializeField] private Button _tryCreateCharacterBtn;
    #endregion
    #region InputFields
    [SerializeField] TMP_InputField _userNameField;
    #endregion
    #region Variables
    #endregion
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
        return Regex.IsMatch(name, @"^[°¡-ÆR]{2,6}$");
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
        Debug.Log($"Ä³¸¯ÅÍ »ý¼º ¿äÃ» Àü¼Û\nÄ³¸¯ÅÍ ÀÌ¸§ : {_userNameField.text}\n ¼ºº° : {_selectedGender}\n Áö¿ª : {_selectedRegion} ");

    }
    void Start()
    {
        gameObject.SetActive(false);
    }
}
