using UnityEngine;
using Google.Protobuf.Protocol; // EDirection

[RequireComponent(typeof(Animator))]
[RequireComponent(typeof(PlayerIdentity))]
public class PlayerAvatar : MonoBehaviour
{
    [SerializeField] private Animator _animator;

    void Awake()
    {
        _animator = GetComponent<Animator>();
    }

    public void PlayerAttackAnimation()
    {
        // TODO: 트리거만 호출
        //_animator.SetTrigger("Attack");
    }
    public void PlayerMoveAnimation(bool onMove)
    {
        if (!onMove)
        {
            _animator.SetBool("IsMoving", false);
        }
        else
        {
            _animator.SetBool("IsMoving", true);
        }
    }
    public void OnDamage()
    {
        _animator.SetTrigger("Hurt");
    }
    public void PlayerDeath()
    {
        _animator.SetBool("IsDie",true);
    }
    public void SetDirection(EDirection dir)
    {
        // Animator 정수 파라미터 "Dir" 사용
        switch (dir)
        {
            case EDirection.DirUp: _animator.SetInteger("Dir", 0); break;
            case EDirection.DirDown: _animator.SetInteger("Dir", 1); break;
            case EDirection.DirLeft: _animator.SetInteger("Dir", 2); break;
            case EDirection.DirRight: _animator.SetInteger("Dir", 3); break;
        }
    }
}
