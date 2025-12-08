using UnityEngine;

/// <summary>
/// 월드에서 플레이어를 클릭했을 때 컨텍스트 메뉴를 띄우는 핸들러.
/// PlayerIdentity와 같은 오브젝트에 붙인다.
/// </summary>
[RequireComponent(typeof(BoxCollider2D))]
public class PlayerWorldClickHandler : MonoBehaviour
{
    private PlayerIdentity _identity;


    void Awake()
    {
        _identity = GetComponentInParent<PlayerIdentity>();
        if (_identity == null)
            Debug.LogError("[PlayerWorldClickHandler] PlayerIdentity가 없습니다.");
    }

    // 💡 2D에서 가장 간단한 클릭 처리: OnMouseUpAsButton (Camera + Collider 필요)
    void OnMouseUpAsButton()
    {
        if (_identity == null) return;
        if (_identity.IsLocalPlayer) return; // 자기 자신 클릭이면 무시

        // 플레이어 오른쪽에 메뉴 띄우기
        Vector3 contextUIPos = _identity.transform.position + Vector3.right * 1.5f;

        Debug.Log($"[PlayerWorldClickHandler] Click on pid={_identity.Id}, name={_identity.Username}");

        PlayerContextMenuUI.Instance.Open(_identity.Id, _identity.Username, contextUIPos);
    }
}
