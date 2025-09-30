using Google.Protobuf.Protocol;
using Packet;
using System;
using System.Net.NetworkInformation;
using UnityEngine;
using UnityEngine.UI;

public class UI_SystemManager : MonoBehaviour
{
    [SerializeField] private Button _gameExitBtn;
    [SerializeField] private Button _gameLogOutBtn;
    [SerializeField] private Button _goSelectCharacterBtn;
    [SerializeField] private Button _inventoryUIBtn;
    [SerializeField] private Button _lowItemRequestBtn;
    [SerializeField] private Button _highItemRequestBtn;
    [SerializeField] private GameObject _inventoryPanel;
    private void Awake()
    {
        if (_gameLogOutBtn) _gameLogOutBtn.onClick.AddListener(OnClickLogOut);
        if (_goSelectCharacterBtn) _goSelectCharacterBtn.onClick.AddListener(OnClickGoSelectCharacter);
        if (_gameExitBtn) _gameExitBtn.onClick.AddListener(OnClickGameExit);
        if (_inventoryUIBtn) _inventoryUIBtn.onClick.AddListener(OnClickInventoryUI);
        if (_lowItemRequestBtn) _lowItemRequestBtn.onClick.AddListener(OnClickLowItemReq);
        if (_highItemRequestBtn) _highItemRequestBtn.onClick.AddListener(OnClickHighItemReq);
    }

    private void OnClickInventoryUI()
    {
        if (_inventoryPanel.activeSelf)
        {
            _inventoryPanel.SetActive(false);
        }
        else
        {
            _inventoryPanel.SetActive(true);
            InventoryManager.Instance.RequestInventory();
        }
    }
    void OnClickLowItemReq()
    {
        var req = new C_GiveItemRequest { ItemId = 10001,Count = 1};
        var send = ServerPacketManager.MakeSendBuffer(req);
        NetworkManager.Instance.Send(send);
        Debug.Log($"[Player Item Request]");
    }
    void OnClickHighItemReq()
    {
        var req = new C_GiveItemRequest { ItemId = 10002, Count = 1 };
        var send = ServerPacketManager.MakeSendBuffer(req);
        NetworkManager.Instance.Send(send);
        Debug.Log($"[Player Item Request]");
    }
    void ItemRequest()
    {

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
