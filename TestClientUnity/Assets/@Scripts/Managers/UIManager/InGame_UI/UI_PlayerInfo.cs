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
        _playerExpText.text = $"{info.CurExp}/{info.MaxExp}";
        _playerMoneyText.text = $"{info.Money}";
    }
    public void UpdateHpBar()
    { 
        _playerHpBar.fillAmount = PlayerStatus.Instance.GetCurHp() / PlayerStatus.Instance.GetMaxHp();
        _playerHealthText.text = $"{PlayerStatus.Instance.GetCurHp()} / {PlayerStatus.Instance.GetMaxHp()}";
    }
    public void UpdateExpBar()
    {
        _playerExpBar.fillAmount = PlayerStatus.Instance.GetCurExp() / PlayerStatus.Instance.GetMaxExp();
        _playerExpText.text = $"{PlayerStatus.Instance.GetCurExp()} / {PlayerStatus.Instance.GetMaxExp()}";
    }
    // Update is called once per frame
    void Update()
    {
        UpdateHpBar();
        UpdateExpBar();
    }
}
