using UnityEngine;

public class PartyWindowToggle : MonoBehaviour
{
    [SerializeField] private GameObject partyListWindow; // Window_PartyList
    [SerializeField] private GameObject myPartyWindow;   // Window_MyParty

    void Awake()
    {
        if (partyListWindow) partyListWindow.SetActive(false);
        if (myPartyWindow) myPartyWindow.SetActive(false);
    }

    void Update()
    {
        if (Input.GetKeyDown(KeyCode.P))
            ToggleUI();
    }

    void ToggleUI()
    {
        bool inParty = PartyState.Instance.InParty;

        if (inParty)
        {
            bool newState = !myPartyWindow.activeSelf;
            myPartyWindow.SetActive(newState);
            if (partyListWindow) partyListWindow.SetActive(false);
        }
        else
        {
            bool newState = !partyListWindow.activeSelf;
            partyListWindow.SetActive(newState);
            if (myPartyWindow) myPartyWindow.SetActive(false);

            if (newState)
                PartyNet.RequestPartyList();
        }
    }
}
