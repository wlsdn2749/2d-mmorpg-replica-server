using UnityEngine;
using UnityEngine.UI;
using TMPro;
using Packet; // PartyNet 사용

public class LeaderActionWindowUI : MonoBehaviour
{
    [SerializeField] TMP_Text _titleText;          // "OOO 에게 무엇을 하시겠습니까?" 같은 텍스트
    [SerializeField] Button _buttonKick;           // Button_KickMember
    [SerializeField] Button _buttonDelegate;       // Button_DelegateLeader
    [SerializeField] Button _buttonCancel;         // Button_Cancel

    int _targetPid = -1;
    string _targetName = "";

    void Awake()
    {
        gameObject.SetActive(false);

        if (_buttonKick) _buttonKick.onClick.AddListener(OnClickKick);
        if (_buttonDelegate) _buttonDelegate.onClick.AddListener(OnClickDelegate);
        if (_buttonCancel) _buttonCancel.onClick.AddListener(Close);
    }

    public void Open(int targetPid)
    {
        _targetPid = targetPid;

        // 이름은 PartyState에서 찾아오기
        if (PartyState.Instance.Members.TryGetValue(_targetPid, out var m))
            _targetName = m.PlayerName;
        else
            _targetName = $"PID:{_targetPid}";

        if (_titleText)
            _titleText.text = $"{_targetName} 에게 어떤 행동을 할까요?";

        gameObject.SetActive(true);
    }

    void Close()
    {
        gameObject.SetActive(false);
        _targetPid = -1;
        _targetName = "";
    }

    void OnClickKick()
    {
        if (_targetPid < 0) return;

        PartyNet.Kick(_targetPid); // 이미 구현되어 있음
        Close();
    }

    void OnClickDelegate()
    {
        if (_targetPid < 0) return;

        PartyNet.DelegateLeader(_targetPid);
        Close();
    }
}
