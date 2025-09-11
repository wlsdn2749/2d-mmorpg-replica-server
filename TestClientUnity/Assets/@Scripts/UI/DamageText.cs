using TMPro;
using UnityEngine;

public class DamageText : PoolAble
{
    [Header("Motion")]
    [SerializeField] float moveSpeed = 1.5f;     // 위로 떠오르는 속도 (유닛/초)
    [SerializeField] float gravity = 0f;         // 살짝 내려오게 하고 싶으면 > 0
    [SerializeField] Vector2 spawnJitter = new(0.1f, 0.05f); // 스폰 랜덤 오프셋

    [Header("Visual")]
    [SerializeField] float lifeTime = 1.0f;      // 표시 시간(초)
    [SerializeField] float maxFontSize = 15f;    // 시작 폰트 크기
    [SerializeField] float minFontSize = 10f;    // 끝 폰트 크기
    [SerializeField] float alphaFadeSpeed = 3f;  // 알파 감소 속도 배수

    [Header("Colors")]
    [SerializeField] Color normalColor = Color.white;
    [SerializeField] Color missColor = new(1f, 1f, 1f, 0.8f);
    [SerializeField] Color critColor = new(1f, 0.3f, 0.2f, 1f);

    // --- 내부 상태 ---
    TMP_Text _tmp;
    Vector3 _velocity;       // 화면 상향 이동 속도
    float _timeLeft;         // 남은 시간
    float _startFontSize;    // 원복용
    Color _baseColor;        // 원복용

    // 외부에서 필요하면 참조 (예: 스폰 후 재사용)
    public float Damage;

    void Awake()
    {
        _tmp = GetComponent<TMP_Text>();
        _startFontSize = maxFontSize; // 기본값 저장
        _baseColor = normalColor;
    }

    void OnEnable()
    {
        // 기본 리셋 (스폰 전에 Setup 호출 권장)
        InternalResetVisuals();
    }

    void Update()
    {
        if (_timeLeft <= 0f) return;

        float dt = Time.deltaTime;
        _timeLeft -= dt;

        // 이동 (위로 떠오르다가, 원하면 gravity로 감속/낙하)
        _velocity.y -= gravity * dt;
        transform.position += _velocity * dt;

        // 폰트 크기 Lerp
        float lifeRatio = Mathf.Clamp01(1f - (_timeLeft / lifeTime)); // 0→1
        float size = Mathf.Lerp(maxFontSize, minFontSize, lifeRatio);
        _tmp.fontSize = size;

        // 알파 감소
        Color c = _tmp.color;
        c.a = Mathf.Clamp01(c.a - alphaFadeSpeed * dt);
        _tmp.color = c;

        if (_timeLeft <= 0f || c.a <= 0.01f)
        {
            // 풀에 반환
            ReleaseObject();
        }
    }
    public void Show(float damage, Vector3 worldPos, bool isCritical = false)
    {
        Damage = damage;

        // 위치 + 약간의 랜덤 오프셋
        Vector3 jitter = new(
            Random.Range(-spawnJitter.x, spawnJitter.x),
            Random.Range(0f, spawnJitter.y),
            0f
        );
        transform.position = worldPos + jitter;

        // 텍스트/색상
        if (damage <= 0f)
        {
            _tmp.text = "Miss";
            _tmp.color = missColor;
        }
        else
        {
            _tmp.text = isCritical ? Mathf.RoundToInt(damage).ToString() + "!"
                                   : Mathf.RoundToInt(damage).ToString();
            _tmp.color = isCritical ? critColor : normalColor;
        }

        // 속도/타이머 리셋
        _velocity = Vector3.up * moveSpeed;
        _timeLeft = lifeTime;
        _tmp.fontSize = maxFontSize;
    }

    // 내부 초기화 (OnEnable 시 호출)
    void InternalResetVisuals()
    {
        _timeLeft = 0f;                // Update가 바로 Release 못하도록
        _velocity = Vector3.up * moveSpeed;
        _tmp.color = normalColor;      // 기본색
        _tmp.fontSize = maxFontSize;   // 기본크기
        _tmp.text = string.Empty;
    }
}