using Google.Protobuf.Protocol;
using Packet;
using UnityEngine;

[RequireComponent(typeof(PlayerIdentity))]
public class PlayerController : MonoBehaviour
{
    [SerializeField] float previewSpeed = 5f;

    private PlayerIdentity _identity;
    private Vector3? _previewTarget;

    void Awake()
    {
        _identity = GetComponent<PlayerIdentity>();
        // 프리팹 기본값은 disabled 권장. Init 후 로컬일 때만 Enable됨.
    }

    void OnEnable()
    {
        // 혹시라도 잘못 켜졌을 때 방어
        if (_identity != null && !_identity.IsLocalPlayer)
        {
            Debug.LogWarning($"[PlayerController] Enabled on remote! Disabling. Id={_identity.Id}");
            enabled = false;
        }
    }

    void Update()
    {
        if (_identity == null || !_identity.IsLocalPlayer) return;

        if (Input.GetMouseButtonDown(1))
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

            // 미세 예측(선택) - 서버 확정 오면 StopPreviewAndSnap에서 정정
            _previewTarget = world;
        }

        if (_previewTarget.HasValue)
        {
            transform.position = Vector3.MoveTowards(transform.position, _previewTarget.Value, previewSpeed * Time.deltaTime);
            if ((transform.position - _previewTarget.Value).sqrMagnitude < 0.0004f)
                _previewTarget = null;
        }
    }

    public void StopPreviewAndSnap(Vector3 newPos)
    {
        _previewTarget = null;
        transform.position = newPos;
    }
}