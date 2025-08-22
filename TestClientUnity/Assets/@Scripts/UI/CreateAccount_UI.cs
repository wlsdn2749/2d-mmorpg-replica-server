using System.Collections;
using System.Net.Sockets;
using System.Security;
using TMPro;
using Packet;
using UnityEngine;
using UnityEngine.UI;
using Google.Protobuf.Protocol;
using Mmorpg2d.Auth;

public class CreateAccount_UI : MonoBehaviour
{
    #region InputFields
    [SerializeField] private TMP_InputField _idField;
    [SerializeField] private TMP_InputField _pwField;
    [SerializeField] private TMP_InputField _pwCheckField;
    #endregion
    #region Buttons
    [SerializeField] private Button _createAccountBtn;
    [SerializeField] private Button _ExitCreateAccountBtn;
    [SerializeField] private Button _idDuplicateCheckBtn;
    #endregion
    #region Panels
    [SerializeField] private GameObject _authorizePanel;
    [SerializeField] private GameObject _noticePanel; 
    #endregion
    #region TMP_Texts
    [SerializeField] private TMP_Text _idUseableText;
    [SerializeField] private TMP_Text _pwRecheckText;
    #endregion
    private void Awake()
    {
        if (_createAccountBtn) _createAccountBtn.onClick.AddListener(OnClickCreateAccount);
        if (_ExitCreateAccountBtn) _ExitCreateAccountBtn.onClick.AddListener(OnClickExitCreateAccount);
        if (_idDuplicateCheckBtn) _idDuplicateCheckBtn.onClick.AddListener(OnClickIdDuplicateCheck);
    }
    void OnClickCreateAccount() // 계정 생성로직
    {
        // TODO >> 계정 생성 로직
        
        // 만약 계정 생성을 성공했다면  >>
        if (_idField.text == "" || _pwField.text == "" || _pwCheckField.text == "" /* bool=> id중복확인 실패시*/)
        {
            _noticePanel.SetActive(true);
            _noticePanel.GetComponent<Notice_UI>().ChangeNoticeCode(NoticeCode.CreateAccountFail);
        }
        else if(_idField.text == "" || _pwField.text == "" || _pwCheckField.text == ""/*bool=> id중복확인 성공시*/)
        {
            _noticePanel.SetActive(true);
            _noticePanel.GetComponent<Notice_UI>().ChangeNoticeCode(NoticeCode.CreateAccountSucess);
        }

    }
    void OnClickExitCreateAccount() // 회원가입 패널 나가기
    {
        // ex_1 >> 정말 창을 닫을거냐고 물어보고 닫기
        // ex_2 >> 바로 닫기

        // 일단 바로 닫고 다시 로그인 패널 띄우는 걸로 구현함.
        InitializePanel();
        _authorizePanel.SetActive(true);
        this.gameObject.SetActive(false); 
    }
    void OnClickIdDuplicateCheck() //아이디 중복확인 버튼 이벤트
    {
        if (_idField.text == "") // 아이디 입력필드가 비어있을때
        {
            _idUseableText.text = "아이디를 입력해주세요!";
            _idUseableText.color = Color.red;
        }
        else
        {
            string id = _idField.text.Trim();
            string password = _pwField.text;

            var pkt = new RegisterRequest
            {
                Email = id,
                Password = password
            };
            var sendBuffer = ServerPacketManager.MakeSendBuffer(pkt);
            NetworkManager.Instance.Send(sendBuffer);
        }
        // TODO >> 입력받은 아이디 사용가능 여부에 따라 아이디 중복 확인 텍스트 결과값 갱신 >> 서버에서 전송받아야할 데이터임.
    }
    void CheckPassword()
    {
        if (_pwField.text == _pwCheckField.text && _pwField.text != "") // 비밀번호와 비밀번호 재확인 필드의 텍스트가 모두 같고 비어있지 않다면
        {
            _pwRecheckText.text = "비밀번호가 일치합니다!";
            _pwRecheckText.color = Color.green;
        }
        else // 비밀번호와 비밀번호 재확인 필드의 값이 다를때
        {
            _pwRecheckText.text = "비밀번호가 일치하지 않습니다."; 
            _pwRecheckText.color = Color.red;
        }
    }
    void InitializePanel()
    {
        _pwRecheckText.text = "";
        _idUseableText.text = "";
        _idField.text = "";
        _pwField.text = "";
        _pwCheckField.text = "";
    }
    void Start()
    {
        InitializePanel();
        this.gameObject.SetActive(false);
    }
    private void Update()
    {
        if (_pwCheckField.text == "") // 비밀번호체크 입력필드가 비어있다면
        {
            if (_pwField.text == "")
            {
                _pwRecheckText.text = "비밀번호를 입력해주세요.";
                _pwRecheckText.color = Color.white;
            }
            else
            {
                _pwRecheckText.text = "비밀번호가 일치하지 않습니다.";
                _pwRecheckText.color = Color.red;
            }
        }
        else
        {
            if (_pwCheckField.text != "")
                CheckPassword();
        }
    }
}
