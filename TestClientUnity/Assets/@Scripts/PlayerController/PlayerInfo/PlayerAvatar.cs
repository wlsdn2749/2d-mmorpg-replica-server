using Google.Protobuf.Protocol;
using UnityEngine;

public class PlayerAvatar : MonoBehaviour
{
    [SerializeField] float moveSpeed = 4f;       // MoveTowards용: 초당 m
    [SerializeField] float arriveEps = 0.01f;     // 도착 판정 허용 오차
    Vector3? _target;
    [SerializeField] private string initialState = "Idle";
    private Animator _animator;
    void Start()
    {
        _animator = GetComponent<Animator>();   
    }
    public void ApplyAppearance(int Id, string userName)
    {
        var id = GetComponent<PlayerIdentity>()??gameObject. AddComponent<PlayerIdentity>();
        id.Id = Id;
        id.Username = userName ?? $"Player_{Id}";   
        if (!string.IsNullOrEmpty(initialState))
        {
            _animator.Play(initialState,0,0);   
        }
    }
    public void SmoothMoveTo(Vector3 worldPos) => _target = worldPos;
    public void HardSnap(Vector3 worldPos)
    {
        transform.position = worldPos;
        _target = null;
    }
    
    public void SetDirection(EDirection dir)
    {
        switch (dir)
        {
            case EDirection.DirUp:
                _animator.SetInteger("Dir", 0);
                break;
            case EDirection.DirDown:
                _animator.SetInteger("Dir", 1);
                break;
            case EDirection.DirLeft:
                _animator.SetInteger("Dir", 2);
                break;
            case EDirection.DirRight:
                _animator.SetInteger("Dir", 3);
                break;
                // UP/DOWN은 Animator 파라미터로 처리 권장
        }
    }

    void Update()
    {
        if (!_target.HasValue) return;

        var cur = transform.position;
        var dst = _target.Value;

        Vector3 next;
        // 일정 속도 이동
        next = Vector3.MoveTowards(cur, dst, moveSpeed * Time.deltaTime);
        _animator.SetBool("IsMoving", true);
        transform.position = next;

        if ((next - dst).sqrMagnitude <= arriveEps * arriveEps)
        { 
            _target = null; // 도착
            _animator.SetBool("IsMoving", false);
        }
    }
}
