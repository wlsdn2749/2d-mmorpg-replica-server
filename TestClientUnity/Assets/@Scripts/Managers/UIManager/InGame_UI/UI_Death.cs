using Google.Protobuf.Protocol;
using Packet;
using UnityEngine;
using UnityEngine.Rendering;
using UnityEngine.UI;

public class UI_Death : MonoBehaviour
{
    [SerializeField] private Button _respawnBtn;
    void Start()
    {
        if (_respawnBtn) _respawnBtn.onClick.AddListener(OnClickRespawn);
    }
    public void OnClickRespawn()
    {
        var req = new C_PlayerDeathReady();
        var send = ServerPacketManager.MakeSendBuffer(req);
        NetworkManager.Instance.Send(send);
        Debug.Log("[DeathReady] Player Death Commit");
        HUDManager.Instance.ShowDeathPanel(false);
    }
}
