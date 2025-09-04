using Cysharp.Net.Http;
using Google.Protobuf.Protocol;
using Grpc.Core;
using Grpc.Net.Client;
using Mmorpg2d.Auth;
using Packet;
using System;
using TMPro;
using UnityEditor;
using UnityEngine;
using UnityEngine.Apple.ReplayKit;
using UnityEngine.Networking.PlayerConnection;
using UnityEngine.UI;
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
        _channel = GrpcChannel.ForAddress("http://127.0.0.1:8080",
            new GrpcChannelOptions
            {
                HttpHandler = _handler,
                DisposeHttpClient = true
            });
        _client = new Auth.AuthClient(_channel);
    }
    void InitializeInputField()
    {
        _idField.text = "";
        _pwField.text = "";
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
            AuthNotice_UI.Instance.ShowNotice(NoticeCode.LoginFailNullAccount);
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
}
