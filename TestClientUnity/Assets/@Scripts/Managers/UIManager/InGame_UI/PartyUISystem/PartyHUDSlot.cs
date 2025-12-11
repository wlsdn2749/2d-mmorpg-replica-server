using Google.Protobuf.Protocol;
using TMPro;
using UnityEngine;
using UnityEngine.UI;

public class PartyHUDSlot : MonoBehaviour
{
    [SerializeField] Image leaderIcon;
    [SerializeField] TMP_Text memberNameText;
    [SerializeField] TMP_Text levelText;
    [SerializeField] Image hpFill;
    int _pid;
    public void Bind(PartyMemberStatusInfo info, int leaderId)
    {
        _pid = info.PlayerId;
        levelText.text = $"Lv.{info.Level}";
        memberNameText.text = info.PlayerName;
        hpFill.fillAmount = info.MaxHp > 0 ? (float)info.Hp / info.MaxHp : 0f;
        if (_pid == leaderId)
        {
            leaderIcon.enabled = true;
        }
        else
        {
            leaderIcon.enabled = false;
        }
    }
    public void Refresh()
    {
        if (!PartyState.Instance.Members.TryGetValue(_pid, out var m)) return;
        levelText.text = $"Lv.{m.Level}";
        hpFill.fillAmount = m.MaxHp > 0 ? (float)m.Hp / m.MaxHp : 0f;
    }
}
