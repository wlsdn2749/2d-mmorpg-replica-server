using UnityEngine;
using Google.Protobuf.Protocol;

public class MonsterAvatar : MonoBehaviour
{
    [SerializeField] private float _moveSpeed = 3f;
    [SerializeField] private float _arriveEps = 0.01f;
    private Vector3? _target;
    private Animator _anim;

    void Awake() { _anim = GetComponent<Animator>(); }

    public void SmoothMoveTo(Vector3 worldPos) => _target = worldPos;

    public void SetDirection(EDirection dir)
    {
        if (_anim) _anim.SetInteger("Dir", (int)dir);
    }
    public void HardSnap(Vector3 worldPos)
    {
        transform.position = worldPos;
        _target = null;
        if (_anim) _anim.SetBool("IsMoving", false);
    }

    public void Move()
    {
        if (!_target.HasValue) return;

        var cur = transform.position;
        var dst = _target.Value;
        var next = Vector3.MoveTowards(cur, dst, _moveSpeed * Time.deltaTime);
        transform.position = next;

        if (_anim) _anim.SetBool("IsMoving", true);

        if ((next - dst).sqrMagnitude <= _arriveEps * _arriveEps)
        {
            _target = null;
            if (_anim) _anim.SetBool("IsMoving", false);
        }
    }

    void Update()
    {
        Move();
    }
}

