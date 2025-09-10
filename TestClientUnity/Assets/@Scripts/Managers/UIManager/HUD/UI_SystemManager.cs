using Google.Protobuf.Protocol;
using Packet;
using System.Net.NetworkInformation;
using UnityEngine;
using UnityEngine.UI;

public class UI_SystemManager : MonoBehaviour
{
    [SerializeField] private Button _gameExitBtn;
    [SerializeField] private Button _gameLogOutBtn;
    [SerializeField] private Button _goSelectCharacterBtn;
    private void Awake()
    {
        if (_gameLogOutBtn) _gameLogOutBtn.onClick.AddListener(OnClickLogOut);
        if (_goSelectCharacterBtn) _goSelectCharacterBtn.onClick.AddListener(OnClickGoSelectCharacter);
        if (_gameExitBtn) _gameExitBtn.onClick.AddListener(OnClickGameExit);
    }
    void OnClickLogOut()
    {
        WorldFlowState.HasEnteredWorld = false;
        LeaveGameContext.SendLeave(ELeaveReason.LeaveLogout);
    }
    void OnClickGoSelectCharacter()
    {
        WorldFlowState.OnCharacterChange = true;
        LeaveGameContext.SendLeave(ELeaveReason.LeaveChangeCharacter);
    }
    void OnClickGameExit()
    {
        WorldFlowState.HasEnteredWorld = false;
#if UNITY_EDITOR
        UnityEditor.EditorApplication.isPlaying = false;
#else
        Application.Quit();
#endif
    }
}
