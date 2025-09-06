using Google.Protobuf.Protocol;
using TMPro;
using UnityEngine;

[RequireComponent(typeof(Animator))]
[RequireComponent(typeof(SpriteRenderer))]
public class PlayerIdentity : MonoBehaviour
{
    public int Id { get; private set; }
    public string Username { get; private set; }
    public bool IsLocalPlayer { get; private set; }
    public int LastServerTick { get; private set; }

    [Header("Refs (auto)")]
    public SpriteRenderer spriteRenderer;
    public Animator animator;

    [Header("UI")]
    [SerializeField] private TMP_Text _playerNameText;

    [Header("Local Only Components")]
    [SerializeField] private MonoBehaviour[] localOnlyComponents;  // 예: PlayerController, 카메라팔로우 등
    [SerializeField] private MonoBehaviour[] remoteOnlyComponents; // 필요시

    void Awake()
    {
        spriteRenderer = GetComponent<SpriteRenderer>();
        animator = GetComponent<Animator>();
        // 프리팹에서 PlayerController 등은 기본 disabled 권장
    }

    public void Init(PlayerInfo info, bool isLocal)
    {
        Id = info.Id;
        Username = string.IsNullOrWhiteSpace(info.Username) ? $"Player_{Id}" : info.Username;
        IsLocalPlayer = isLocal;

        gameObject.name = $"Player_{Id}_{Username}";
        if (_playerNameText != null) _playerNameText.text = Username;

        // 로컬/원격 컴포넌트 토글
        if (localOnlyComponents != null)
            foreach (var c in localOnlyComponents) if (c) c.enabled = IsLocalPlayer;

        if (remoteOnlyComponents != null)
            foreach (var c in remoteOnlyComponents) if (c) c.enabled = !IsLocalPlayer;

        // 태그/레이어 분리 추천 (충돌/카메라 마스크 사고 방지)
        gameObject.tag = IsLocalPlayer ? "PlayerLocal" : "PlayerRemote";
        gameObject.layer = LayerMask.NameToLayer(IsLocalPlayer ? "PlayerLocal" : "PlayerRemote");
    }

    public void SetLastServerTick(int tick) => LastServerTick = tick;

    void OnDestroy()
    {
        Debug.LogWarning($"[PlayerIdentity] Destroyed: Id={Id}, Local={IsLocalPlayer}, GO={name}");
    }
}
