using Google.Protobuf.Protocol;
using UnityEngine;

public class PlayerAvatar : MonoBehaviour
{
    [SerializeField] float moveLerpSpeed = 12f;
    Vector3? _lerpTarget;
    [SerializeField] private string initialState = "Idle";
    private Animator _animator;
    private SpriteRenderer _spriteRenderer;
    void Start()
    {
        _animator = GetComponent<Animator>();   
        _spriteRenderer = GetComponent<SpriteRenderer>();
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
    public void SmoothMoveTo(Vector3 worldPos) => _lerpTarget = worldPos;

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
        if (_lerpTarget.HasValue)
        {
            var pos = transform.position;
            var target = _lerpTarget.Value;
            transform.position = Vector3.MoveTowards(pos, target, moveLerpSpeed * Time.deltaTime);
            if ((transform.position - target).sqrMagnitude < 0.0004f)
                _lerpTarget = null;
        }
    }
}
