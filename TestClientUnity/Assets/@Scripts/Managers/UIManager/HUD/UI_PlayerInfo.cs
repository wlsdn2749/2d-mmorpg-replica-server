using Google.Protobuf.Protocol;
using TMPro;
using UnityEngine;
using UnityEngine.UI;

public class UI_PlayerInfo : MonoBehaviour
{
    [SerializeField] private TMP_Text _playerLevelText;
    [SerializeField] private TMP_Text _playerHealthText;
    [SerializeField] private TMP_Text _playerExpText;
    [SerializeField] private TMP_Text _playerMoneyText;
    [SerializeField] private Image _playerHpBar;
    [SerializeField] private Image _playerExpBar;
    void Start()
    {
        
    }
    public void InitInfo(PlayerStatInfo info)
    {
        if (info == null)
            return;
        _playerLevelText.text = $"{info.Level}";
        _playerHealthText.text = $"{info.Hp} / {info.MaxHp}";
        _playerExpText.text = $"{info.Exp}";
        _playerMoneyText.text = $"{info.Money}";
    }
    public void UpdateHpBar(float hp, float maxHp)
    {
        _playerHpBar.fillAmount = hp / maxHp;
        _playerHealthText.text = $"{hp} / {maxHp}";
    }
    public void UpdateExpBar(float exp, float nextExp)
    {
        _playerExpBar.fillAmount = exp / nextExp;
        _playerExpText.text = $"{exp} / {nextExp}";
    }
    // Update is called once per frame
    void Update()
    {
        
    }
}
