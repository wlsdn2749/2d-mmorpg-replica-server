using UnityEngine;

public class PlayerAvatar : MonoBehaviour
{
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
    // Update is called once per frame
    void Update()
    {
        
    }
}
