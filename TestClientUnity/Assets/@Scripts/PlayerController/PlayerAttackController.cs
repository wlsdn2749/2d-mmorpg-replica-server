using Google.Protobuf.Protocol;
using Packet;
using UnityEngine;

public class PlayerAttackController : MonoBehaviour
{
    [SerializeField] float attackCooldown = 0.5f;
    float _nextAttackTime;
    PlayerIdentity _playerIdentity;
    Animator _animator; 
    void Awake()
    {
        _playerIdentity = GetComponent<PlayerIdentity>();
        _animator = GetComponent<Animator>();
    }
    void MeleeAttack()
    {
        if (!_playerIdentity.IsLocalPlayer) return;
        if (Time.time < _nextAttackTime) return;
        if (Input.GetKeyDown(KeyCode.A)||Input.GetKeyDown(KeyCode.Space))
        {
            _nextAttackTime = Time.time + attackCooldown;
            _animator.SetTrigger("Attack"); // 애니메이션 이벤트로 서버에 공격 패킷 전송
        }
    }
    void PlayerNormalAttack() // Aniamition Event
    {
        if (!_playerIdentity.IsLocalPlayer) return;
        var req = new C_PlayerAttackRequest();
        var send = ServerPacketManager.MakeSendBuffer(req);
        NetworkManager.Instance.Send(send);
    }
    // Update is called once per frame
    void Update()
    {
        MeleeAttack();
    }
}
