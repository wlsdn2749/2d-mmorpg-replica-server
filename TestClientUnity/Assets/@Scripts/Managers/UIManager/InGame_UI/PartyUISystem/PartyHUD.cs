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


