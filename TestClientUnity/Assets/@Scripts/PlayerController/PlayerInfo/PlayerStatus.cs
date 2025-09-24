using Google.Protobuf.Protocol;
using System.Security.Cryptography.X509Certificates;
using UnityEngine;

public class PlayerStatus : MonoBehaviour
{
    public static PlayerStatus Instance { get; private set; }   
    [SerializeField] private float _maxHp;
    [SerializeField] private float _curHp;
    [SerializeField] private int _curExp;
    [SerializeField] private float _maxExp;
    [SerializeField] private int _money;
    [SerializeField] private int _level;
    private void Awake()
    {
        if (Instance == null)
        {
            Instance = this;
            DontDestroyOnLoad(gameObject);
        }
        else
        {
            Destroy(gameObject);
        }
    }
    public void SetPlayerStatus(PlayerStatInfo info)
    {
        _maxHp = info.MaxHp;
        _curHp = info.Hp;
        _curExp   = info.CurExp;
        _maxExp = info.MaxExp;
        _money = info.Money;
        _level = info.Level;
        HUDManager.Instance.SetPlayerInfo(info);
    }
    
    public float GetMaxHp()
    {
        return _maxHp;
    }
    public float GetCurHp()
    {
        return _curHp;
    }
    public float GetMaxExp()
    {
        return _maxExp;
    }
    public float GetCurExp()
    {
        return _curExp;
    }
    public void OnDamage(S_BroadcastMonsterAttack monsterAttack)
    {
        var go = PlayerSpawner.Get(monsterAttack.TargetPid);
        if (go == null) return;
        _curHp = monsterAttack.HpAfter;
        var av = go.GetComponent<PlayerAvatar>();
        _curHp -= monsterAttack.Damage;
        av.OnDamage();
        var damageText = ObjectPoolManager.Instance.GetObject("PlayerDamageText");
        damageText.GetComponent<DamageText>().Show(monsterAttack.Damage, go.transform.position);
    }
    public void UpdateHp(S_BroadcastPlayerHpChanged hpChanged)
    {
        var go =  PlayerSpawner.Get(hpChanged.PlayerId);
        if (go == null) return;
        _curHp = hpChanged.Hp;
        _maxHp = hpChanged.MaxHp;
    }
}
