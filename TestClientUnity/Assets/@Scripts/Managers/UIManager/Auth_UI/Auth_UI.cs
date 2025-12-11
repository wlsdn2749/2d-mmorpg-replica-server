using Cysharp.Net.Http;
using Google.Protobuf.Protocol;
using Grpc.Core;
using Grpc.Net.Client;
using Mmorpg2d.Auth;
using Packet;
using System;
using TMPro;
using UnityEngine;
using UnityEngine.UI;
using UnityEngine.EventSystems;
public class Auth_UI : MonoBehaviour
{
    private YetAnotherHttpHandler _handler;
    private GrpcChannel _channel;
    private Auth.AuthClient _client;
    
    #region InputFields
    [SerializeField] private TMP_InputField _idField;
    [SerializeField] private TMP_InputField _pwField;
    #endregion

    #region Buttons
    [SerializeField] private Button _createAccountBtn;
    [SerializeField] private Button _loginBtn;
    #endregion

    #region Panels 
    [SerializeField] private GameObject _createAccountPanel;
    #endregion
    private void Awake()
    {
        if (_createAccountBtn) _createAccountBtn.onClick.AddListener(OnClickCreateAccount);
        if (_loginBtn) _loginBtn.onClick.AddListener(OnClickLogin);
    }
    void Start()
    {
        _handler = new YetAnotherHttpHandler { Http2Only = true };
        // 진우씨 아이피 = http://14.36.6.179:8080
        // 로컬 아이피 = http://127.0.0.1:8080
        //_channel = GrpcChannel.ForAddress("http://14.36.6.179:8080",
        //    new GrpcChannelOptions
        //    {
        //        HttpHandler = _handler,
        //        DisposeHttpClient = true
        //    });
        _channel = GrpcChannel.ForAddress("http://127.0.0.1:8080",
            new GrpcChannelOptions
            {
                HttpHandler = _handler,
                DisposeHttpClient = true
            });
        _client = new Auth.AuthClient(_channel);
    }
    private void OnEnable()
    {
        FocusIdField();
    }

    void InitializeInputField()
    {
        _idField.text = "";
        _pwField.text = "";
        FocusIdField();
    }
    void OnClickCreateAccount()
    {
        InitializeInputField();
        _createAccountPanel.SetActive(true);
        this.gameObject.SetActive(false);
    }
    private async void OnClickLogin()
    {
        var id = _idField.text?.Trim();
        var password = _pwField.text ?? "";
        if (string.IsNullOrEmpty(id)) // 아이디 입력필드가 비워져있을때
        {
            AuthNotice_UI.Instance.gameObject.SetActive(true);
            AuthNotice_UI.Instance.ShowNotice(NoticeCode.LoginFailNullID);
            return;
        }
        if (string.IsNullOrEmpty(password)) // 패스워드 입력필드가 비워져있을때
        {
            AuthNotice_UI.Instance.gameObject.SetActive(true);
            AuthNotice_UI.Instance.ShowNotice(NoticeCode.LoginFailNullPW);
            return;
        }
        AuthNotice_UI.Instance.ShowNotice(NoticeCode.DoLogin);
        try
        {
            var loginReply = await Authenticate.LoginAsync(_client, id, password);
            if (loginReply.success)
            {
                Authenticate.Jwt = loginReply.jwt;
                ConnectJwtLogin(Authenticate.Jwt);
            }
            else
            {
                AuthNotice_UI.Instance.gameObject.SetActive(true);
                AuthNotice_UI.Instance.ShowNotice(NoticeCode.LoginFailNullAccount);
            }
        }
        catch (RpcException rEX)
        {
            Debug.LogError(rEX.StatusCode);
            AuthNotice_UI.Instance.gameObject.SetActive(true);
            AuthNotice_UI.Instance.ShowNotice(NoticeCode.LoginFailGrpcError);
        }
        catch (Exception ex)
        {
            Debug.LogError(ex.Message);
            AuthNotice_UI.Instance.gameObject.SetActive(true);
            AuthNotice_UI.Instance.ShowNotice(NoticeCode.LoginFailNullAccount);
        }
    }
    void ConnectJwtLogin(string jwtToken)
    {
        var token = jwtToken;
        if (string.IsNullOrEmpty(token))
        {
            Debug.LogWarning("[UI] accessToken이 비어있습니다.");
            return;
        }
        
        var req = new C_JwtLoginRequest { AccessToken = token };
        var sendBuffer = ServerPacketManager.MakeSendBuffer(req); // PKT_C_JwtLoginRequest로 매핑됨
        NetworkManager.Instance.Send(sendBuffer);
        Debug.Log($"[UI] JWT 로그인 요청 전송: len={sendBuffer.Count}");
    }
    void Update()
    {
        if (!gameObject.activeInHierarchy)
            return;

        // TAB: ID → PW로 포커스 이동
        if (Input.GetKeyDown(KeyCode.Tab))
        {
            HandleTabNavigation();
        }

        // ENTER: 로그인 시도 (어디에 포커스 있어도)
        if (Input.GetKeyDown(KeyCode.Return) || Input.GetKeyDown(KeyCode.KeypadEnter))
        {
            OnClickLogin();
        }
    }

    // === 키보드 네비게이션 유틸 ===

    void FocusIdField()
    {
        if (_idField == null || EventSystem.current == null)
        {
            Debug.LogWarning("[Auth_UI] FocusIdField: _idField 또는 EventSystem.current가 null입니다.");  
            return;
        }

        EventSystem.current.SetSelectedGameObject(_idField.gameObject);
        _idField.ActivateInputField();
    }

    void FocusPwField()
    {
        if (_pwField == null || EventSystem.current == null)
            return;

        EventSystem.current.SetSelectedGameObject(_pwField.gameObject);
        _pwField.ActivateInputField();
    }

    void HandleTabNavigation()
    {
        if (EventSystem.current == null)
            return;

        var current = EventSystem.current.currentSelectedGameObject;

        // ID 입력창에 포커스가 있을 때만 PW로 넘김
        if (current == _idField.gameObject)
        {
            FocusPwField();
        }
        // 나중에 필요하면 PW에서 다시 ID로 돌아가는 로직도 추가 가능
        // else if (current == _pwField.gameObject) { FocusIdField(); }
    }

}
