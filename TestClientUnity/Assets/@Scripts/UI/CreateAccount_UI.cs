using Cysharp.Net.Http;
using Google.Protobuf.Protocol;
using Grpc.Core;
using Grpc.Net.Client;
using Mmorpg2d.Auth;
using Packet;
using System;
using System.Collections;
using System.Net.Sockets;
using System.Security;
using System.Threading;
using System.Threading.Tasks;
using TMPro;
using UnityEngine;
using UnityEngine.UI;
public static class Authenticate
{
    public static async Task<bool> DoCreateAccountAsync(Auth.AuthClient client, string id, string password)
    {
        try
        {
            var reply = await client.RegisterAsync(new RegisterRequest
            {
                Email = (id ?? "").Trim().ToLowerInvariant(),
                Password = password ?? ""
            });
            Debug.Log($"[가입 결과] {reply.Success} / {reply.Detail}");
            return reply.Success;
        }
        catch (RpcException ex)
        {
            Debug.LogError($"[가입 RPC 오류] {ex.StatusCode} / {ex.Status.Detail}");
            return false;
        }
        catch (Exception ex)
        {
            Debug.LogError($"[가입 예외] {ex.Message}");
            return false;
        }
    }
}
    
public class CreateAccount_UI : MonoBehaviour
{
    private YetAnotherHttpHandler _handler;
    private GrpcChannel _channel;
    private Auth.AuthClient _client;
    private bool _checkID =false;
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
        //if (_idDuplicateCheckBtn) _idDuplicateCheckBtn.onClick.AddListener(OnClickIdDuplicateCheck);
    }
    void Start()
    {
        _handler = new YetAnotherHttpHandler { Http2Only = true };
        _channel = GrpcChannel.ForAddress("http://182.231.5.187:8080",
            new GrpcChannelOptions 
            { 
                HttpHandler = _handler,
                DisposeHttpClient = true 
            });
        _client = new Auth.AuthClient(_channel);
        InitializePanel();
        this.gameObject.SetActive(false);
    }
    private async void OnClickCreateAccount()
    {
        var id = _idField.text?.Trim();
        var pw = _pwField.text ?? "";
        var pw2 = _pwCheckField.text ?? "";

        // 입력 검증
        if (string.IsNullOrEmpty(id) || string.IsNullOrEmpty(pw) || string.IsNullOrEmpty(pw2))
        {
            ShowNotice(NoticeCode.CreateAccountFail);
            return;
        }
        if (pw != pw2)
        {
            _pwRecheckText.text = "비밀번호가 일치하지 않습니다.";
            ShowNotice(NoticeCode.CreateAccountFail);
            return;
        }
        if (_checkID == false)
        {
            _idUseableText.text = "아이디 중복 확인이 필요합니다.";
        }
        _createAccountBtn.interactable = false;

        // 실제 가입 호출
        var ok = await Authenticate.DoCreateAccountAsync(_client, id, pw);

        // UI 업데이트
        ShowNotice(ok ? NoticeCode.CreateAccountSucess : NoticeCode.CreateAccountFail);

        _createAccountBtn.interactable = true;
    }
    private void ShowNotice(NoticeCode code)
    {
        _noticePanel.SetActive(true);
        _noticePanel.GetComponent<Notice_UI>()?.ChangeNoticeCode(code);
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
    //private async void OnClickIdDuplicateCheck() //아이디 중복확인 버튼 이벤트
    //{
    //}
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
