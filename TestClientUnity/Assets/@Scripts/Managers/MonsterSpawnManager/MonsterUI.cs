using UnityEngine;

public class MonsterUI : MonoBehaviour
{
    [SerializeField] private Transform fillPivot;
    [SerializeField] private float lerpSpeed = 10f; // 0이면 즉시 반영
    public Vector3 DamageTextOffset;
    private MonsterHealth _health;
    private float _targetFill = 1f;

    void Awake()
    {
        _health = GetComponentInParent<MonsterHealth>();
        if (_health != null)
            _health.OnHpChanged += OnHpChanged;
    }

    void OnDestroy()
    {
        if (_health != null)
            _health.OnHpChanged -= OnHpChanged;
    }

    void OnHpChanged(int cur, int max)
    {
        if (max <= 0) return;
        _targetFill = Mathf.Clamp01((float)cur / max);

        if (lerpSpeed <= 0f)
            ApplyFill(_targetFill);
    }

    void Update()
    {
        if (lerpSpeed > 0f)
        {
            float current = fillPivot.localScale.x;
            float next = Mathf.Lerp(current, _targetFill, Time.deltaTime * lerpSpeed);
            ApplyFill(next);
        }
    }

    private void ApplyFill(float t)
    {
        if (fillPivot != null)
            fillPivot.localScale = new Vector3(t, fillPivot.transform.localScale.y, 1f);
    }
}