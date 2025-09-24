using TMPro;
using UnityEngine;

public class UI_ChatLine : MonoBehaviour
{
    [SerializeField] TMP_Text _text;

    public void SetText(string t)
    {
        if (_text != null)
            _text.text = t;
        else
            Debug.LogWarning("[ChatLine] TMP_Text not assigned!");
    }
}
