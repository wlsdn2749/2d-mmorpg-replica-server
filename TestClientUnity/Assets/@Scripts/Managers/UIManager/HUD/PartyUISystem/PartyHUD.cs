using UnityEngine;
using TMPro;
using UnityEngine.UI;
using System.Linq;
using System.Collections.Generic;
using Google.Protobuf.Protocol;

public class PartyHUD : MonoBehaviour
{
    [SerializeField] Transform content;
    [SerializeField] PartyHUDSlot slotPrefab;

    void OnEnable() { PartyState.Instance.OnPartyChanged += Rebuild; PartyState.Instance.OnPartyStatusUpdated += Refresh; Rebuild(); }
    void OnDisable() { PartyState.Instance.OnPartyChanged -= Rebuild; PartyState.Instance.OnPartyStatusUpdated -= Refresh; }

    void Rebuild()
    {
        foreach (Transform c in content) Destroy(c.gameObject);
        IReadOnlyDictionary<int, PartyMemberStatusInfo> mem = PartyState.Instance.Members;
        foreach (var m in mem.Values.OrderBy(v => v.IsLeader ? 0 : 1))
        {
            var slot = Instantiate(slotPrefab, content);
            slot.Bind(m, PartyState.Instance.PartyLeaderId);
        }
        gameObject.SetActive(mem.Count > 0);
    }
    void Refresh()
    {
        foreach (PartyHUDSlot s in content.GetComponentsInChildren<PartyHUDSlot>())
            s.Refresh();
    }
}

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
