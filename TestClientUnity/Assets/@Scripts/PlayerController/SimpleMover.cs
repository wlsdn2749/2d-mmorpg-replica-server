using UnityEngine;

public class SimpleMover : MonoBehaviour
{
    [SerializeField] float moveSpeed = 6f;
    [SerializeField] float arriveEps = 0.01f;
    Vector3? _target;

    public void SetTarget(Vector3 worldPos) => _target = worldPos;
    public void HardSnap(Vector3 worldPos) { transform.position = worldPos; _target = null; }

    void Update()
    {
        if (!_target.HasValue) return;
        var cur = transform.position;
        var dst = _target.Value;
        var next = Vector3.MoveTowards(cur, dst, moveSpeed * Time.deltaTime);
        transform.position = next;
        if ((next - dst).sqrMagnitude <= arriveEps * arriveEps) _target = null;
    }
}
