using UnityEngine;

public class PortalMeta : MonoBehaviour
{
    // 규칙 : 맵 Id
    // 고구려 = 1
    // 백제   = 2
    // 사냥터 = 3
    // srcMapId = 접촉한 해당 포탈
    // dstMapId = 이동해야할 포탈 
    // ex) 고구려에서 사냥터로 이동하는 포탈ID는  >> 1 0 0 3
    // ex) 사냥터에서 고구려로 이동하는 포탈ID는  >> 3 0 0 1
    public string SrcPortalId; 
    public string DstMapId;    
    public string DstPortalId; 
}
