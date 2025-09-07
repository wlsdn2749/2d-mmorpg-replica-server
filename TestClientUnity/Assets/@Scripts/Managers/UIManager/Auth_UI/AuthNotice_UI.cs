using TMPro;
using UnityEngine;
using UnityEngine.UI;

public enum NoticeCode
{
    
    CheckExitCreateAccountPanel=0,
    CreateAccountFail,
    CreateAccountSucess, 
    LoginFailNullID ,
    LoginFailNullPW,
    LoginFailGrpcError,
    LoginFailNullAccount,
    DoLogin,
    LoginSuccess,
    RecvCharacterListSuccess,
    FailCreateCharacterName,
    FailCreateCharacterNameWrong,
    FailCreateCharacterGender,
    FailCreateCharacterRegion,
    FailCreateCharacterNameDuplicated,
    CreateCharacterSuccess,
    EnterGame,
    EnterGameFail,  
}

public class AuthNotice_UI : MonoBehaviour
{
    public static AuthNotice_UI Instance;
    [SerializeField] private TMP_Text _noticeText;
    [SerializeField] private Button _checkBtn;
    [SerializeField] private Button _closePanelBtn;
    [SerializeField] private GameObject _createAccountPanel;
    [SerializeField] private GameObject _authorizePanel;
    [SerializeField] private NoticeCode _noticeCode;
    private WaitForSeconds _interval = new WaitForSeconds(1f);
    public TMP_Text ChangeNoticeCode(NoticeCode notiCode)
    {
        _noticeCode = notiCode;
        switch (_noticeCode)
        {
            case NoticeCode.CheckExitCreateAccountPanel:
                _noticeText.text = "정말로 계정 생성을 멈추시고 로그인 화면으로\n돌아가시겠습니까?";
                _noticeText.color = Color.red;
                ShowOkButton();
                break;

            case NoticeCode.CreateAccountSucess:
                _noticeText.text = "계정 생성이 완료되었습니다.\n체크 버튼을 누르시면 로그인 화면으로 돌아갑니다.";
                _noticeText.color = Color.white;
                ShowOkButton();
                break;

            case NoticeCode.CreateAccountFail:
                _noticeText.text = "계정 생성에 필요한 조건이 충족되지 않았습니다.\n다시 시도해주세요.";
                _noticeText.color = Color.red;
                ShowOkButton();
                break;
            case NoticeCode.LoginFailNullID:
                _noticeText.text = "아이디를 입력해주세요.";
                _noticeText.color = Color.red;
                ShowOkButton();
                break;

            case NoticeCode.LoginFailNullPW:
                _noticeText.text = "비밀번호를 입력해주세요";
                _noticeText.color = Color.red;
                ShowOkButton();
                break;

            case NoticeCode.DoLogin:
                _noticeText.text = "로그인을 진행중입니다. 잠시만 기다려주세요.";
                _noticeText.color = Color.white;
                break;
            case NoticeCode.LoginFailGrpcError:
                _noticeText.text = "서버와 연결이 원활하지 않습니다.\n같은 현상이 반복되면 고객센터로 연락해주시기 바랍니다.";
                _noticeText.color = Color.red;
                ShowOkButton();
                break;
            case NoticeCode.LoginFailNullAccount:
                _noticeText.text = "존재하지 않는 계정입니다.\n회원가입을 진행하거나 로그인 정보를 다시 입력해주세요.";
                _noticeText.color = Color.red;
                ShowOkButton();
                break;

            case NoticeCode.LoginSuccess:
                _noticeText.text = "로그인에 성공하셨습니다.\n캐릭터 정보를 불러오고 있습니다.";
                _noticeText.color = Color.white;
                break;

            case NoticeCode.RecvCharacterListSuccess:
                _noticeText.text = "캐릭터 리스트를 성공적으로 불러왔습니다.";
                _noticeText.color = Color.white;
                ShowOkButton();
                break;

            case NoticeCode.FailCreateCharacterName:
                _noticeText.text = "캐릭터 이름을 입력해주세요.";
                _noticeText.color = Color.red;
                ShowOkButton();
                break;

            case NoticeCode.FailCreateCharacterNameWrong:
                _noticeText.text = "올바르지 못한 형식의 이름입니다.\n다시 설정해주세요.";
                _noticeText.color = Color.red;
                ShowOkButton();
                break;

            case NoticeCode.FailCreateCharacterGender:
                _noticeText.text = "성별을 설정해주세요.";
                _noticeText.color = Color.red;
                ShowOkButton();
                break;

            case NoticeCode.FailCreateCharacterRegion:
                _noticeText.text = "지역을 설정해주세요.";
                _noticeText.color = Color.red;
                ShowOkButton();
                break;

            case NoticeCode.FailCreateCharacterNameDuplicated:
                _noticeText.text = "중복된 닉네임입니다. 다시 설정해주세요.";
                _noticeText.color = Color.red;
                ShowOkButton();
                break;

            case NoticeCode.CreateCharacterSuccess:
                _noticeText.text = "캐릭터 생성에 성공하셨습니다!";
                _noticeText.color = Color.white;
                ShowOkButton();
                break;
            case NoticeCode.EnterGame:
                _noticeText.text = "게임에 접속중입니다...";
                _noticeText.color = Color.white;
                break;
            case NoticeCode.EnterGameFail:
                _noticeText.text = "게임에 접속하던중 문제가 발생하였습니다.";
                _noticeText.color = Color.red;
                ShowOkButton();
                break;
        }
        return _noticeText;
    }
    private void OnClickCheck()
    {
        if (_noticeCode == NoticeCode.LoginFailNullID)
        {
            OkButtonClose();
            this.gameObject.SetActive(false);
        }
        else if (_noticeCode == NoticeCode.LoginFailNullPW)
        {
            OkButtonClose();
            this.gameObject.SetActive(false);
        }
        else if (_noticeCode == NoticeCode.LoginFailNullAccount)
        {
            OkButtonClose();
            this.gameObject.SetActive(false);
        }
        else if (_noticeCode == NoticeCode.LoginSuccess)
        {
            OkButtonClose();
            this.gameObject.SetActive(false);
        }
        else if (_noticeCode == NoticeCode.CheckExitCreateAccountPanel)
        {
            _authorizePanel.SetActive(true);
            _createAccountPanel.SetActive(false);
            _createAccountPanel.GetComponent<CreateAccount_UI>().InitializePanel();
            OkButtonClose();
            this.gameObject.SetActive(false);
        }
        else if (_noticeCode == NoticeCode.CreateAccountSucess)
        {
            _createAccountPanel.SetActive(false);
            _authorizePanel.SetActive(true);
            _createAccountPanel.GetComponent<CreateAccount_UI>().InitializePanel();
            OkButtonClose();
            this.gameObject.SetActive(false);
        }
        else if (_noticeCode == NoticeCode.CreateAccountFail)
        {
            OkButtonClose();
            this.gameObject.SetActive(false);
        }
        else if (_noticeCode == NoticeCode.RecvCharacterListSuccess)
        {
            OkButtonClose();
            this.gameObject.SetActive(false);
        }
        else if (_noticeCode == NoticeCode.FailCreateCharacterGender ||
            _noticeCode == NoticeCode.FailCreateCharacterName ||
            _noticeCode == NoticeCode.FailCreateCharacterNameWrong||
            _noticeCode == NoticeCode.FailCreateCharacterRegion||
            _noticeCode == NoticeCode.FailCreateCharacterNameDuplicated)
        {
            OkButtonClose();
            this.gameObject.SetActive(false);
        }
        else if (_noticeCode == NoticeCode.CreateCharacterSuccess)
        {
            CreateCharacter_UI.Instance.gameObject.SetActive(false);
            CharacterList_UI.Instance.gameObject.SetActive(true);
            OkButtonClose() ;
            this.gameObject.SetActive(false);
        }
        else if (_noticeCode == NoticeCode.EnterGameFail)
        {
            OkButtonClose();
            this.gameObject.SetActive(false);
        }
    }
    public void ShowNotice(NoticeCode code)
    {
        ChangeNoticeCode(code);
    }
    #region Buttons
    private void ShowOkButton()
    {
        _closePanelBtn.enabled = true;
        _closePanelBtn.image.color = Color.white;
    }
    private void OkButtonClose()
    {
        _noticeText.text = "";
        _closePanelBtn.enabled = false;
        _closePanelBtn.image.color = new Color(0, 0, 0, 0);
    }
    private void OnClickClose()
    {
        _noticeText.text = "";
        _authorizePanel.SetActive(true);
        _closePanelBtn.enabled = false;
        _closePanelBtn.image.color = new Color(0, 0, 0, 0);
        this.gameObject.SetActive(false);
    }
    #endregion
    private void Awake()
    {
        if (Instance != null && Instance != this)
        {
            Destroy(gameObject);
            return;
        }
        Instance = this;
    }
    private void Start()
    {
        if (_checkBtn) _checkBtn.onClick.AddListener(OnClickCheck);
        if (_closePanelBtn) _closePanelBtn.onClick.AddListener(OnClickClose);
        _noticeText.text = "";
        _closePanelBtn.enabled = false;
        _closePanelBtn.image.color = new Color(0,0,0,0);
        this.gameObject.SetActive(false);   
    }
}
