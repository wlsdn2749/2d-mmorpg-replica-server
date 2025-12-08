using Cysharp.Net.Http;
using Grpc.Core;
using Grpc.Net.Client;
using Mmorpg2d.Auth;
using System;
using System.Threading.Tasks;
using TMPro;
using UnityEngine;
using UnityEngine.UI;
using UnityEngine.EventSystems;
using System.Collections;
public static class Authenticate 
{
    public static string Jwt = "";
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
    public static async Task<(bool available, string detail)> CheckEmailAsync(Auth.AuthClient client, string email)
    {
        try
        {
            var reply = await client.CheckEmailAsync(
                new CheckEmailRequest { Email = (email ?? "").Trim().ToLowerInvariant() });

            return (reply.Available, reply.Detail);
        }
        catch (RpcException ex)
        {
            Debug.LogError($"[중복확인 RPC 오류] {ex.StatusCode} / {ex.Status.Detail}");
            return (false, "서버 통신 오류");
        }
        catch (Exception ex)
        {
            Debug.LogError($"[중복확인 예외] {ex.Message}");
            return (false, "예외 발생");
        }
    }
    public static async Task<(bool success, string detail, string jwt)> LoginAsync(Auth.AuthClient client, string id, string password)
    {
        try
        {
            var reply = await client.LoginAsync(new LoginRequest() 
            { 
                Email = id, 
                Password = password
            });
            Debug.Log("gRPC 응답: " + reply.Success + "\ngRPC Detail" + reply.Detail + "\nJwt" + reply.Jwt);
              
            return (reply.Success,reply.Detail,reply.Jwt);
        }
        catch (System.Exception ex)
        {
            Debug.LogError("gRPC 오류: " + ex.Message);
            return (false,ex.Message,"");
        }
    }
}

public class CreateAccount_UI : MonoBehaviour
{
    private YetAnotherHttpHandler _handler;
    private GrpcChannel _channel;
    private Auth.AuthClient _client;

    // 이메일 중복 체크가 통과했는지 여부
    private bool _checkID = false;

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
    [SerializeField] private GameObject _authorizePanel;    // 로그인 패널
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

        if (_idField) _idField.onValueChanged.AddListener(OnIdChanged);

        // 비밀번호 / 비밀번호 확인 입력 시마다 메시지 갱신
        if (_pwField) _pwField.onValueChanged.AddListener(_ => UpdatePasswordHint());
        if (_pwCheckField) _pwCheckField.onValueChanged.AddListener(_ => UpdatePasswordHint());
    }

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

        if (_idField == null || EventSystem.current == null)
            yield break;

        EventSystem.current.SetSelectedGameObject(_idField.gameObject);
        _idField.Select();
        _idField.ActivateInputField();
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

        InitializePanel();
        gameObject.SetActive(false);   // 시작은 로그인 패널만 보이게
    }

    private void Update()
    {
        if (!gameObject.activeInHierarchy)
            return;

        // TAB 키로 입력창 이동 (ID → PW → PW체크 → 다시 ID)
        if (Input.GetKeyDown(KeyCode.Tab))
        {
            HandleTabNavigation();
        }

        // Enter / KeypadEnter 로 회원가입 시도
        if (Input.GetKeyDown(KeyCode.Return) || Input.GetKeyDown(KeyCode.KeypadEnter))
        {
            OnClickCreateAccount();
        }
    }

    private void HandleTabNavigation()
    {
        if (EventSystem.current == null)
            return;

        var current = EventSystem.current.currentSelectedGameObject;

        if (current == _idField.gameObject)
        {
            FocusPwField();
        }
        else if (current == _pwField.gameObject)
        {
            FocusPwCheckField();
        }
        else if (current == _pwCheckField.gameObject)
        {
            FocusIdField();
        }
        else
        {
            // 어떤 것도 선택 안 돼 있으면 ID부터
            FocusIdField();
        }
    }

    private void FocusIdField()
    {
        if (_idField == null || EventSystem.current == null) return;
        EventSystem.current.SetSelectedGameObject(_idField.gameObject);
        _idField.ActivateInputField();
    }

    private void FocusPwField()
    {
        if (_pwField == null || EventSystem.current == null) return;
        EventSystem.current.SetSelectedGameObject(_pwField.gameObject);
        _pwField.ActivateInputField();
    }

    private void FocusPwCheckField()
    {
        if (_pwCheckField == null || EventSystem.current == null) return;
        EventSystem.current.SetSelectedGameObject(_pwCheckField.gameObject);
        _pwCheckField.ActivateInputField();
    }

    // ================== 버튼 이벤트 ==================

    private async void OnClickCreateAccount()
    {
        var id = _idField.text?.Trim();
        var pw = _pwField.text ?? "";
        var pw2 = _pwCheckField.text ?? "";

        // 입력 검증
        if (string.IsNullOrEmpty(id) || string.IsNullOrEmpty(pw) || string.IsNullOrEmpty(pw2))
        {
            AuthNotice_UI.Instance.gameObject.SetActive(true);
            AuthNotice_UI.Instance.ShowNotice(NoticeCode.CreateAccountFail);
            return;
        }

        if (pw != pw2)
        {
            _pwRecheckText.text = "비밀번호가 일치하지 않습니다.";
            _pwRecheckText.color = Color.red;
            AuthNotice_UI.Instance.gameObject.SetActive(true);
            AuthNotice_UI.Instance.ShowNotice(NoticeCode.CreateAccountFail);
            return;
        }

        if (!_checkID)
        {
            _idUseableText.text = "Email 중복 확인이 필요합니다.";
            _idUseableText.color = Color.red;
            return;
        }

        _createAccountBtn.interactable = false;

        // 실제 가입 호출
        var ok = await Authenticate.DoCreateAccountAsync(_client, id, pw);

        // UI 업데이트
        AuthNotice_UI.Instance.gameObject.SetActive(true);
        AuthNotice_UI.Instance.ShowNotice(ok ? NoticeCode.CreateAccountSucess : NoticeCode.CreateAccountFail);

        _createAccountBtn.interactable = true;
    }

    void OnClickExitCreateAccount() // 회원가입 패널 나가기
    {
        // 지금은 "정말 나갈건지" 알림만 띄우는 상태인 것 같아서 그대로 둠
        AuthNotice_UI.Instance.gameObject.SetActive(true);
        AuthNotice_UI.Instance.ShowNotice(NoticeCode.CheckExitCreateAccountPanel);
    }

    private async void OnClickIdDuplicateCheck() // 아이디 중복확인 버튼
    {
        var id = _idField.text?.Trim();

        if (string.IsNullOrEmpty(id))
        {
            _idUseableText.text = "아이디를 입력해주세요!";
            _idUseableText.color = Color.red;
            _checkID = false;
            return;
        }

        if (!IsValidEmail(id))
        {
            _idUseableText.text = "Email 형식으로 입력해주세요!";
            _idUseableText.color = Color.red;
            _checkID = false;
            return;
        }

        var ok = await Authenticate.CheckEmailAsync(_client, id);
        if (ok.available)
        {
            _idUseableText.text = "사용 가능한 Email입니다!";
            _idUseableText.color = Color.green;
            _checkID = true;
        }
        else
        {
            _idUseableText.text = "이미 존재하는 Email입니다!";
            _idUseableText.color = Color.red;
            _checkID = false;
        }
    }

    // ================== 입력 변화 처리 ==================

    private bool IsValidEmail(string s)
    {
        try
        {
            var _ = new System.Net.Mail.MailAddress(s);
            return true;
        }
        catch
        {
            return false;
        }
    }

    // ID 텍스트가 바뀔 때 호출
    private void OnIdChanged(string value)
    {
        var email = value?.Trim() ?? "";

        // ID 텍스트가 바뀌면 중복 확인 결과는 무효화
        _checkID = false;

        if (string.IsNullOrEmpty(email))
        {
            _idUseableText.text = "";
            _idUseableText.color = Color.white;
            return;
        }

        if (!IsValidEmail(email))
        {
            _idUseableText.text = "Email 형식이 올바르지 않습니다.";
            _idUseableText.color = Color.red;
        }
        else
        {
            _idUseableText.text = "Email 중복확인이 필요합니다.";
            _idUseableText.color = Color.yellow;
        }
    }

    // 비밀번호 두 칸 중 하나라도 바뀔 때 메시지 갱신
    private void UpdatePasswordHint()
    {
        var pw = _pwField.text ?? "";
        var pw2 = _pwCheckField.text ?? "";

        if (string.IsNullOrEmpty(pw) && string.IsNullOrEmpty(pw2))
        {
            _pwRecheckText.text = "비밀번호를 입력해주세요.";
            _pwRecheckText.color = Color.white;
            return;
        }

        if (string.IsNullOrEmpty(pw2))
        {
            _pwRecheckText.text = "비밀번호가 일치하지 않습니다.";
            _pwRecheckText.color = Color.red;
            return;
        }

        if (pw == pw2)
        {
            _pwRecheckText.text = "비밀번호가 일치합니다!";
            _pwRecheckText.color = Color.green;
        }
        else
        {
            _pwRecheckText.text = "비밀번호가 일치하지 않습니다.";
            _pwRecheckText.color = Color.red;
        }
    }

    public void InitializePanel()
    {
        _pwRecheckText.text = "";
        _idUseableText.text = "";
        _idField.text = "";
        _pwField.text = "";
        _pwCheckField.text = "";
        _checkID = false;
    }
}
