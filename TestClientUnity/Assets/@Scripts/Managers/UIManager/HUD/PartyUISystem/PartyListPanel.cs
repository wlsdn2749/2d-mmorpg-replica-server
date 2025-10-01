using UnityEngine;
using TMPro;
using System.Collections.Generic;
using Google.Protobuf.Protocol;

public class PartyListPanel : MonoBehaviour
{
    [SerializeField] Transform content;
    [SerializeField] PartyListRow rowPrefab;

    void OnEnable() { PartyState.Instance.OnPartyList += OnList; PartyNet.RequestPartyList(); }
    void OnDisable() { PartyState.Instance.OnPartyList -= OnList; }

    void OnList(List<PartyInfo> infos)
    {
        foreach (Transform c in content) Destroy(c.gameObject);
        foreach (var p in infos)
        {
            var row = Instantiate(rowPrefab, content);
            row.Bind(p);
        }
    }
}

public class PartyListRow : MonoBehaviour
{
    [SerializeField] TMP_Text title;
    int _partyId;

    public void Bind(PartyInfo info)
    {
        _partyId = info.PartyId;
        title.text = $"{info.PartyName} [{info.CurMemberCount}/{info.MaxMemberCount}]";
    }

    public void OnClickJoin() => PartyNet.RequestJoin(_partyId);
}
