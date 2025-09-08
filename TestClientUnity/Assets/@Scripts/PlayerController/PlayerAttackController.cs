using Google.Protobuf.Protocol;
using Packet;
using UnityEngine;

public class PlayerAttackController : MonoBehaviour
{
    [SerializeField] float attackCooldown = 0.5f;
    float _nextAttackTime;
    PlayerIdentity _id;
    Animator _anim; 
    void Awake()
    {
        _id = GetComponent<PlayerIdentity>();
        _anim = GetComponent<Animator>();
    }
    void MeleeAttack()
    {
        if (!_id.IsLocalPlayer) return;
        if (Time.time < _nextAttackTime) return;
        if (Input.GetKeyDown(KeyCode.A)||Input.GetKeyDown(KeyCode.Space))
        {
            _nextAttackTime = Time.time + attackCooldown;


            // 서버에 공격 요청(타깃 선택은 서버 로직에 맡기는 설계)
            var req = new C_PlayerAttackRequest();
            var send = ServerPacketManager.MakeSendBuffer(req);
            NetworkManager.Instance.Send(send);
            Debug.Log("공격 패킷 송신");
        }
    }
    // Update is called once per frame
    void Update()
    {
        MeleeAttack();
    }
}
