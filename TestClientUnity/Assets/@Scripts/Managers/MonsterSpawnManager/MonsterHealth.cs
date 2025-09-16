using UnityEngine;
using System;

public class MonsterHealth : MonoBehaviour
{
    [field: SerializeField] public int MaxHp { get; private set; } = 0;
    [field: SerializeField] public int CurrentHp { get; private set; } = 0;

    public event Action<int, int> OnHpChanged;

    public void InitByAttackPacket(int afterHp)
    {
        SetHp(afterHp);
    }
    public void SetMaxHp(int monsterTypeId,int fallBack = 100)
    {
        var def = MonsterRegistry.Instance.Get(monsterTypeId);
        MaxHp = def?.MaxHp ?? fallBack; 
        CurrentHp = MaxHp;
    }
    public void SetHp(int hp)
    {
        hp = Mathf.Clamp(hp, 0, MaxHp > 0 ? MaxHp : int.MaxValue);
        CurrentHp = hp;
        OnHpChanged?.Invoke(CurrentHp, MaxHp);
    }
}