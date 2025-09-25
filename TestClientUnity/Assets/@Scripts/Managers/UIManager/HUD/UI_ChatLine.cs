using Google.Protobuf.Protocol;
using TMPro;
using UnityEngine;

public class UI_ChatLine : MonoBehaviour
{
    [SerializeField] TMP_Text _text;

    public void SetText(string t,EChatType chatType)
    {
        if (_text != null) 
        {
            _text.text = t;
            switch (chatType)
            {
                case EChatType.ChatAll:
                    _text.color = Color.white;
                    break;
                case EChatType.ChatRoom:
                    _text.color = Color.yellow;
                    break;
            }

        }

        else
            Debug.LogWarning("[ChatLine] TMP_Text not assigned!");
    }
}
