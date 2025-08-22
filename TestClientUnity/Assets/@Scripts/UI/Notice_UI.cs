using NUnit.Framework.Constraints;
using System.Collections;
using TMPro;
using UnityEngine;
using UnityEngine.UI;

public enum NoticeCode
{
    LoginFailNullID = 0,
    LoginFailNullPW,
    LoginFailNullAccount,
    LoginSuccess,
    OpenCreateAccountPanel ,
    CreateAccountFail  ,
    CreateAccountSucess 
}

public class Notice_UI : MonoBehaviour
{
    [SerializeField] private TMP_Text _noticeText;
    [SerializeField] private Button _checkBtn;
    [SerializeField] private Button _closePanelBtn;
    [SerializeField] private GameObject _createAccountPanel;
    [SerializeField] private GameObject _authorizePanel;
    [SerializeField] private NoticeCode _noticeCode;

    public TMP_Text ChangeNoticeCode(NoticeCode notiCode)
    {
        _noticeCode = notiCode;
        switch (_noticeCode)
        {
            case NoticeCode.LoginFailNullID:
                _noticeText.text = "아이디를 입력해주세요.";
                break;

            case NoticeCode.LoginFailNullPW:
                _noticeText.text = "비밀번호를 입력해주세요";
                break;
            case NoticeCode.LoginFailNullAccount:
                _noticeText.text = "로그인에 실패했습니다 " +
                    "존재하지 않는 계정이거나 로그인 정보를 다시 입력해주세요.";
                break;
            case NoticeCode.LoginSuccess:
                _noticeText.text = "로그인을 진행중입니다. 잠시만 기다려주세요.";
                break;

            case NoticeCode.OpenCreateAccountPanel:
                _noticeText.text = "회원가입을 진행하시겠습니까?";
                _closePanelBtn.enabled = true;
                _closePanelBtn.image.color = new Color(1, 1, 1, 1);
                break;

            case NoticeCode.CreateAccountSucess:
                _noticeText.text = "계정 생성이 완료되었습니다. 체크 버튼을 누르시면 로그인 화면으로 돌아갑니다.";
                break;
            case NoticeCode.CreateAccountFail:
                _noticeText.text = "계정 생성에 필요한 조건이 충족되지 않았습니다. 다시 시도해주세요.";
                break;
        }
        return _noticeText;
    }
    private void OnClickCheck()
    {
        if (_noticeCode == NoticeCode.LoginFailNullID)
        {
            _noticeText.text = "";
            this.gameObject.SetActive(false);
        }
        else if (_noticeCode == NoticeCode.LoginFailNullPW)
        {
            _noticeText.text = "";
            this.gameObject.SetActive(false);
        }
        else if (_noticeCode == NoticeCode.LoginFailNullAccount)
        {
            _noticeText.text = "";
            this.gameObject.SetActive(false);
        }
        else if (_noticeCode == NoticeCode.LoginSuccess)
        {
            _noticeText.text = "";
            this.gameObject.SetActive(false);
        }
        else if (_noticeCode == NoticeCode.OpenCreateAccountPanel)
        {
            _createAccountPanel.SetActive(true);
            _noticeText.text = "";
            this.gameObject.SetActive(false);
        }
        else if (_noticeCode == NoticeCode.CreateAccountSucess)
        {
            _createAccountPanel.SetActive(false);
            _authorizePanel.SetActive(true);
            _noticeText.text = "";
            this.gameObject.SetActive(false);
        }
        else if (_noticeCode == NoticeCode.CreateAccountFail)
        {
            _noticeText.text = "";
            this.gameObject.SetActive(false);
        }
    }
    private void OnClickClose()
    {
        _noticeText.text = "";
        _authorizePanel.SetActive(true);
        _closePanelBtn.enabled = false;
        _closePanelBtn.image.color = new Color(1, 1, 1, 0);
        this.gameObject.SetActive(false);
    }
    private void Awake()
    {
        if (_checkBtn) _checkBtn.onClick.AddListener(OnClickCheck);
        if (_closePanelBtn) _closePanelBtn.onClick.AddListener(OnClickClose);
    }
    private void Start()
    {
        _noticeText.text = "";
        _closePanelBtn.enabled = false;
        _closePanelBtn.image.color = new Color(1,1,1,0);
        this.gameObject.SetActive(false);   
    }
}
