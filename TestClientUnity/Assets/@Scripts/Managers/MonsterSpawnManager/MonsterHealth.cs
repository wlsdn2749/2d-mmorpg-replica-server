using UnityEngine;
using System;

public class MonsterHealth : MonoBehaviour
{
    [field: SerializeField] public int MaxHp { get; private set; } = 100;
    [field: SerializeField] public int CurrentHp { get; private set; } = 100;

    public event Action<int, int> OnHpChanged;

    public void Init(int maxHp, int curHp)
    {
        MaxHp = Mathf.Max(1, maxHp);
        CurrentHp = Mathf.Clamp(curHp, 0, MaxHp);
        OnHpChanged?.Invoke(CurrentHp, MaxHp);
    }

    public void SetHp(int curHp)
    {
        CurrentHp = Mathf.Clamp(curHp, 0, MaxHp);
        OnHpChanged?.Invoke(CurrentHp, MaxHp);
    }

    public void ApplyDamage(int dmg)
    {
        SetHp(CurrentHp - Mathf.Max(0, dmg));
    }
}
