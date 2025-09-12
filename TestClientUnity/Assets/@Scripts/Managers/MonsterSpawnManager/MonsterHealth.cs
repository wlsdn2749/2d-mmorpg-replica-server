using UnityEngine;
using System;

public class MonsterHealth : MonoBehaviour
{
    [field: SerializeField] public int MaxHp { get; private set; } = 0;
    [field: SerializeField] public int CurrentHp { get; private set; } = 0;

    public event Action<int, int> OnHpChanged;

    public void InitByAttackPacket(int afterHp, int damage)
    {
        if (MaxHp == 0) // 처음 계산될 때만
        {
            MaxHp = afterHp + damage;
            Debug.Log($"[MonsterHealth] MaxHp 유추: {MaxHp}");
        }
        SetHp(afterHp-damage);
    }

    public void SetHp(int hp)
    {
        hp = Mathf.Clamp(hp, 0, MaxHp > 0 ? MaxHp : int.MaxValue);
        CurrentHp = hp;
        OnHpChanged?.Invoke(CurrentHp, MaxHp);
    }
}