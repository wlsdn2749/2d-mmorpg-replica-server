using System;
using UnityEngine;
using UnityEngine.UI;
using TMPro;      // TextMeshPro 사용 시
using ServerCore; // Connector, ServerSession
using Google.Protobuf;
using Google.Protobuf.Protocol; // C_JwtLoginRequest, C_EnterGame
using Packet;                   // PacketID, ServerPacketManager

public class SimpleNetworkUI : MonoBehaviour
{
    // Start is called once before the first execution of Update after the MonoBehaviour is created
    [Header("UI - Buttons")]
    [SerializeField] private Button jwtLoginButton;
    [SerializeField] private Button characterListReqButton;
    [SerializeField] private Button enterGameButton;
    
    void Start()
    {
        // 버튼 바인딩
        
        if (enterGameButton) enterGameButton.onClick.AddListener(OnClickEnterGame);
        if (characterListReqButton) characterListReqButton.onClick.AddListener(OnClickCharacterListReqButton);
    }
    // Update is called once per frame
    void Update()
    {
        
    }
    
    #region UI Handlers
    

    void OnClickEnterGame()
    {
        
        var req = new C_EnterGame { PlayerIndex = 0 };
        var sendBuffer = ServerPacketManager.MakeSendBuffer(req); // PKT_C_EnterGame로 매핑됨

        NetworkManager.Instance.Send(sendBuffer);
        Debug.Log($"[UI] 게임 접속 요청 전송: playerIndex={0}, len={sendBuffer.Count}");
    }

    void OnClickCharacterListReqButton()
    {
        var req = new C_CharacterListRequest() { };
        var sendBuffer = ServerPacketManager.MakeSendBuffer(req); // PKT_C_CharacterListRequest로 매핑됨

        NetworkManager.Instance.Send(sendBuffer);
        Debug.Log($"[UI] 캐릭터 리스트 전송 요청: playerIndex={0}, len={sendBuffer.Count}");
    }
    #endregion
}
