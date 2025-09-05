using Google.Protobuf.Protocol;
using System;
using TMPro;
using UnityEngine;
using UnityEngine.UI;

public class CharacterSlot_UI : MonoBehaviour
{
    #region 
    [SerializeField] private Image highlightImg; // 선택 시 켜줄 테두리/백그라운드 (옵션
    #endregion
    #region TMP_Texts
    [SerializeField] private TMP_Text _userName;
    [SerializeField] private TMP_Text _userGender;
    [SerializeField] private TMP_Text _userLevel;
    #endregion
    #region Buttons
    [SerializeField] private Button _selectBtn;
    #endregion
    #region Variables
    [SerializeField] private int _index;
    #endregion

    public void SetupSlot(CharacterSummaryInfo client, int index,Action<int,string> onSelect)
    {
        _index = index;
        _userName.text = client.Username;
        switch (client.Gender)
        {
            case EGender.GenderMale:
                _userGender.text = "남자";
                break;
            case EGender.GenderFemale:
                _userGender.text = "여자";
                break;
        }
        _userLevel.text = client.Level.ToString();
        _selectBtn.onClick.RemoveAllListeners();
        _selectBtn.onClick.AddListener(() => onSelect?.Invoke(_index,_userName.text));
        SetSelected(false);
    }
    public string SetSelected(bool selected)
    {
        if (highlightImg) highlightImg.enabled = selected;
        return _userName.text;
    }

}
