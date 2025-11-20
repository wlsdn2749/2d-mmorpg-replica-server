using Google.Protobuf.Protocol;
using TMPro;
using UnityEngine;
using UnityEngine.UI;

public class PartyHUDSlot : MonoBehaviour
{
    [SerializeField] TMP_Text nameText;
    [SerializeField] TMP_Text levelText;
    [SerializeField] Image hpFill;
    int _pid;

    public void Bind(PartyMemberStatusInfo info, int leaderId)
    {
        _pid = info.PlayerId;
        nameText.text = (info.PlayerId == leaderId ? "👑 " : "") + info.PlayerName;
        levelText.text = $"Lv.{info.Level}";
        hpFill.fillAmount = info.MaxHp > 0 ? (float)info.Hp / info.MaxHp : 0f;
    }
    public void Refresh()
    {
        if (!PartyState.Instance.Members.TryGetValue(_pid, out var m)) return;
        levelText.text = $"Lv.{m.Level}";
        hpFill.fillAmount = m.MaxHp > 0 ? (float)m.Hp / m.MaxHp : 0f;
    }
}
