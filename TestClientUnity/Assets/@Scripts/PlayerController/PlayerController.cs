using Google.Protobuf.Protocol;
using Packet;
using UnityEngine;

[RequireComponent(typeof(PlayerIdentity))]
public class PlayerController : MonoBehaviour
{
    private const float _moveCooldown = 0.5f;
    [SerializeField] private float _curMoveCooldown = 0.5f;
    private PlayerIdentity _identity;
    [SerializeField] private SimpleMover _simpleMover;
    void Awake()
    {
        _identity = GetComponent<PlayerIdentity>();
        _simpleMover = GetComponent<SimpleMover>();
        // 프리팹 기본값은 disabled 권장. Init 후 로컬일 때만 Enable됨.
    }

    void OnEnable()
    {
        // 혹시라도 잘못 켜졌을 때 방어
        if (_identity == null && !_identity.IsLocalPlayer)
        {
            Debug.LogWarning($"[PlayerController] Enabled on remote! Disabling. Id={_identity.Id}");
            enabled = false;
        }
    }
    private void MoveCharacter()
    {
        if (_identity == null || !_identity.IsLocalPlayer) 
        {
            return; 
        }
        _curMoveCooldown -= Time.deltaTime;
        if (Input.GetMouseButtonDown(1) && _curMoveCooldown <= 0)
        {
            var cam = Camera.main;
            if (cam == null) return;

            var world = cam.ScreenToWorldPoint(Input.mousePosition);
            world.z = 0;

            // 패킷 전송
            var req = new C_PlayerMoveRequest
            {
                ClickWorldPos = new Vector2Info { X = (int)world.x, Y = (int)world.y }
            };
            var sendBuffer = ServerPacketManager.MakeSendBuffer(req);
            NetworkManager.Instance.Send(sendBuffer);
            _curMoveCooldown = _moveCooldown;
        }
    }
    void Update()
    {
        MoveCharacter();
    }
}