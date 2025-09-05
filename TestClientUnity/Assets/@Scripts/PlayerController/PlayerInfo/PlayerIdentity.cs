using Google.Protobuf.Protocol;
using TMPro;
using UnityEngine;

public class PlayerIdentity : MonoBehaviour
{
    public int Id;
    public string Username;
    public bool IsLocalPlayer;
    public int LastServerTick { get; private set; }
    public SpriteRenderer spriteRenderer;
    public Animator animator;
    [SerializeField] private TMP_Text _playerNameText;
    void Awake()
    {
        spriteRenderer = GetComponent<SpriteRenderer>();
        animator = GetComponent<Animator>();
    }

    public void Init(PlayerInfo info, bool isLocal)
    {
        Id = info.Id;
        Username = info.Username;
        IsLocalPlayer = isLocal;
        gameObject.name = $"Player_{Id}_{Username}";
        if (_playerNameText != null)
            _playerNameText.text = Username;
    }
    public void SetLastServerTick(int tick)
    {
        LastServerTick = tick;
    }
}
