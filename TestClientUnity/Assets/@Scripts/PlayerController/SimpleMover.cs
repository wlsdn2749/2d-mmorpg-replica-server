using UnityEngine;

public class SimpleMover : MonoBehaviour
{
    [SerializeField] float _moveSpeed = 4f;
    [SerializeField] float _arriveEps = 0.01f;
    [SerializeField] PlayerAvatar _playerAvatar;
    private Vector3? _target;
    public void Awake()
    {
        _playerAvatar = GetComponent<PlayerAvatar>();
    }
    public void SetTarget(Vector3 worldPos) 
    {
        _target = worldPos; 
    }
    public void PlayerMove()
    {
        _playerAvatar.PlayerMoveAnimation(_target.HasValue);
        if (!_target.HasValue)
        { 
            return;
        }
        var cur = transform.position;
        var dst = _target.Value;
        var next = Vector3.MoveTowards(cur, dst, _moveSpeed * Time.deltaTime);
        transform.position = next;
        if ((next - dst).sqrMagnitude <= _arriveEps * _arriveEps) _target = null;
    }
    public void HardSnap(Vector3 worldPos) { transform.position = worldPos; _target = null; }

    void Update()
    {
        PlayerMove();
    }
}
