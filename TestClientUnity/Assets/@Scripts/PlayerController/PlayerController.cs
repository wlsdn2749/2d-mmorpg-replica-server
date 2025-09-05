using Google.Protobuf.Protocol;
using Packet;
using UnityEngine;

public class PlayerController : MonoBehaviour
{
    //[SerializeField] float previewSpeed = 5f; // 서버 확정 전, 살짝 미리 움직일 때 속도(선택)

    //int _clientSeq = 0;         // 클라측 입력 번호 (서버가 reply에 echo)
    //Vector3? _previewTarget;    // 클라 예측을 위한 임시 목표지점

    void Update()
    {
        if (Input.GetMouseButtonDown(1) && GetComponent<PlayerIdentity>().IsLocalPlayer)
        {

            var world = Camera.main.ScreenToWorldPoint(Input.mousePosition);
            world.z = 0;

            
            var req = new C_PlayerMoveRequest
            {
                ClickWorldPos = new Vector2Info { X = (int)world.x, Y = (int)world.y }
            };
            var sendBuffer = ServerPacketManager.MakeSendBuffer(req);
            NetworkManager.Instance.Send(sendBuffer);
            Debug.Log("이동 패킷 전송 완료");
        }

        // (선택) 미세 예측: 목표지점 쪽으로 살짝만 부드럽게 이동
        //if (_previewTarget.HasValue)
        //{
        //    var pos = transform.position;
        //    transform.position = Vector3.MoveTowards(pos, _previewTarget.Value, previewSpeed * Time.deltaTime);
        //    if (Vector3.SqrMagnitude(transform.position - _previewTarget.Value) < 0.0004f)
        //        _previewTarget = null;
        //}
    }

    // 서버 확정이 오면 불려서 예측 종료
    //public void StopPreviewAndSnap(Vector3 newPos)
    //{
    //    _previewTarget = null;
    //    transform.position = newPos;
    //}
}
