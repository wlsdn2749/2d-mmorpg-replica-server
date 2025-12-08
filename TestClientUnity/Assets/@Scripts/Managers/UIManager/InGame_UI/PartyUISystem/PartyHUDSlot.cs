using Google.Protobuf.Protocol;
using TMPro;
using UnityEngine;
using UnityEngine.UI;

public class PartyHUDSlot : MonoBehaviour
{
    [SerializeField] GameObject leaderUI;
    [SerializeField] GameObject memberUI;   
    [SerializeField] TMP_Text leaderNameText;
    [SerializeField] TMP_Text memberNameText;
    [SerializeField] TMP_Text levelText;
    [SerializeField] Image hpFill;
    int _pid;
    private void Start()
    {
        leaderUI.SetActive(false);
        memberUI.SetActive(false);
    }
    public void Bind(PartyMemberStatusInfo info, int leaderId)
    {
        _pid = info.PlayerId;
        levelText.text = $"Lv.{info.Level}";
        hpFill.fillAmount = info.MaxHp > 0 ? (float)info.Hp / info.MaxHp : 0f;
        if (_pid == leaderId)
        {
            leaderUI.SetActive(true);
            memberUI.SetActive(false);
            leaderNameText.text = info.PlayerName;
        }
        else
        {
            leaderUI.SetActive(false);
            memberUI.SetActive(true);
            memberNameText.text = info.PlayerName;
        }
    }
    public void Refresh()
    {
        if (!PartyState.Instance.Members.TryGetValue(_pid, out var m)) return;
        levelText.text = $"Lv.{m.Level}";
        hpFill.fillAmount = m.MaxHp > 0 ? (float)m.Hp / m.MaxHp : 0f;
    }
}
