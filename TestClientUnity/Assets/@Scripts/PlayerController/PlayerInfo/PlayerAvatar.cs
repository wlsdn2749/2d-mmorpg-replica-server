using UnityEngine;
using Google.Protobuf.Protocol; // EDirection

[RequireComponent(typeof(Animator))]
[RequireComponent(typeof(PlayerIdentity))]
public class PlayerAvatar : MonoBehaviour
{
    [SerializeField] float moveSpeed = 4f;
    [SerializeField] float arriveEps = 0.01f;
    [SerializeField] private string initialState = "Idle";

    private Vector3? _target;
    private Animator _animator;
    private PlayerIdentity _identity;

    void Awake()
    {
        _animator = GetComponent<Animator>();
        _identity = GetComponent<PlayerIdentity>();
        if (!string.IsNullOrEmpty(initialState))
            _animator.Play(initialState, 0, 0f);
    }

    // ✅ 외부에서는 PlayerIdentity.Init만 호출하세요. 중복 세팅 금지.
    // ApplyAppearance 제거(혼선을 막기 위해)

    public void SmoothMoveTo(Vector3 worldPos) => _target = worldPos;

    public void HardSnap(Vector3 worldPos)
    {
        transform.position = worldPos;
        _target = null;
        _animator.SetBool("IsMoving", false);
    }

    public void PlayerAttackAnimation()
    {
        // TODO: 트리거만 호출
        //_animator.SetTrigger("Attack");
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

        // 필요시 좌우 반전
        if (dir == EDirection.DirLeft || dir == EDirection.DirRight)
        {
            var s = transform.localScale;
            s.x = (dir == EDirection.DirLeft) ? -Mathf.Abs(s.x) : Mathf.Abs(s.x);
            transform.localScale = s;
        }
    }

    void Update()
    {
        if (!_target.HasValue) return;

        var next = Vector3.MoveTowards(transform.position, _target.Value, moveSpeed * Time.deltaTime);
        _animator.SetBool("IsMoving", true);
        transform.position = next;

        if ((next - _target.Value).sqrMagnitude <= arriveEps * arriveEps)
        {
            _target = null;
            _animator.SetBool("IsMoving", false);
        }
    }
}
