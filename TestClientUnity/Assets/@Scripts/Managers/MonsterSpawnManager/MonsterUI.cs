using UnityEngine;
using UnityEngine.UI;

public class MonsterUI : MonoBehaviour
{
    [Header("Refs")]
    [SerializeField] private SpriteRenderer bg;          // 배경
    [SerializeField] private SpriteRenderer fill;        // 채우기
    [SerializeField] private Transform fillPivot;        // ← 왼쪽 기준 스케일용


    private MonsterHealth _health;
    private SpriteRenderer _ownerRenderer;     // 몬스터의 스프라이트
    private float _targetFill01 = 1f;
    [SerializeField] private float lerpTime = 0.08f;     // 0이면 즉시

    void Reset()
    {
        // 에디터에서 프리팹 만들 때 자동 참조 시도
        var sr = GetComponentsInChildren<SpriteRenderer>(true);
        foreach (var s in sr)
        {
            if (s.name.ToLower().Contains("bg")) bg = s;
            if (s.name.ToLower().Contains("fill")) fill = s;
        }
        if (fill != null) fillPivot = fill.transform.parent;
    }

    public void Bind(Transform owner, SpriteRenderer ownerRenderer, MonsterHealth health)
    {
        transform.SetParent(owner, worldPositionStays: false);
        _ownerRenderer = ownerRenderer;
        _health = health;



        // HP 이벤트 연결 및 초기 반영
        if (_health != null)
        {
            _health.OnHpChanged -= OnHpChanged;
            _health.OnHpChanged += OnHpChanged;
            OnHpChanged(_health.CurrentHp, _health.MaxHp);
        }

        gameObject.SetActive(true);
    }

    public void Unbind()
    {
        if (_health != null) _health.OnHpChanged -= OnHpChanged;
        _health = null;
        _ownerRenderer = null;
        gameObject.SetActive(false);
    }

    void OnDestroy()
    {
        if (_health != null) _health.OnHpChanged -= OnHpChanged;
    }

    private void OnHpChanged(int cur, int max)
    {
        max = Mathf.Max(1, max);
        _targetFill01 = Mathf.Clamp01((float)cur / max);
        if (Mathf.Approximately(lerpTime, 0f))
            ApplyFillImmediate(_targetFill01);
    }

    void Update()
    {
        if (!Mathf.Approximately(lerpTime, 0f))
        {
            // 부드럽게 보간
            float current = (fillPivot != null) ? fillPivot.localScale.x : 1f;
            float next = Mathf.Lerp(current, _targetFill01, 1 - Mathf.Pow(1 - 0.6f, Time.unscaledDeltaTime / lerpTime));
            ApplyFillImmediate(next);
        }
    }

    private void ApplyFillImmediate(float t)
    {
        t = Mathf.Clamp01(t);
        if (fillPivot != null) fillPivot.localScale = new Vector3(t, 1f, 1f);
        // (선택) HP 꽉 차면 자동 숨김
        // bool visible = t < 0.999f;
        // if (bg) bg.enabled = visible;
        // if (fill) fill.enabled = visible;
    }
}
