using UnityEngine;
using TMPro;

public class PartyInvitePopup : MonoBehaviour
{
    [SerializeField] TMP_Text text;
    int _partyId;

    void OnEnable() => PartyState.Instance.OnInvite += OnInvite;
    void OnDisable() => PartyState.Instance.OnInvite -= OnInvite;

    void OnInvite(int inviterPid, string inviterName, int partyId)
    {
        _partyId = partyId;
        text.text = $"{inviterName} 님이 파티 초대했습니다.";
        gameObject.SetActive(true);
    }
    public void OnClickAccept() { PartyNet.RespondInvite(_partyId, true); gameObject.SetActive(false); }
    public void OnClickReject() { PartyNet.RespondInvite(_partyId, false); gameObject.SetActive(false); }
}
