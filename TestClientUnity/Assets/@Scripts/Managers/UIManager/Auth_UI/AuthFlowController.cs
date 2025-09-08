using Google.Protobuf;
using Google.Protobuf.Protocol;
using Packet;
using System.Collections.Generic;
using TMPro;
using UnityEngine;
using UnityEngine.UI;

public class AuthFlowController : MonoBehaviour
{
    [Header("Panels")]
    [SerializeField] private GameObject loginPanel;
    [SerializeField] private GameObject characterSelectPanel;
    void Start()
    {
        // 네트워크 매니저는 DDOL 단일 인스턴스라고 가정
        ApplyEntryMode(LoginManagement.NextLoginEntryMode);
    }

    void ApplyEntryMode(LoginEntryMode mode)
    {
        switch (mode)
        {
            case LoginEntryMode.AfterLeaveToCharacterSelect:
                ShowCharacterSelect();
                break;


            case LoginEntryMode.ColdStart:
                break;
            default:
                break;
        }
    }
    void ShowCharacterSelect()
    {
        loginPanel.SetActive(false);
        characterSelectPanel.SetActive(true);
    }
}
