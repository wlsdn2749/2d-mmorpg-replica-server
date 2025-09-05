using Google.Protobuf;
using Google.Protobuf.Protocol;
using Mono.Cecil.Cil;
using ServerCore;
using System;
using System.Linq;
using Unity.VisualScripting.Antlr3.Runtime;
using UnityEngine;
using UnityEngine.SceneManagement;
using UnityEngine.TextCore.Text;

namespace Packet
{
    static class NetDebug
    {
        // 내 플레이어ID가 있으면 넣어두면 "ME" 표시 가능 (없으면 -1 유지)
        public static int MyPlayerId = -1;

        public static string DirToStr(EDirection dir) => dir switch
        {
            EDirection.DirUp => "UP   ↑",
            EDirection.DirDown => "DOWN ↓",
            EDirection.DirLeft => "LEFT ←",
            EDirection.DirRight => "RIGHT→",
            _ => dir.ToString()
        };

        public static string PosToStr(Vector2Info pos)
            => pos is null ? "(?,?)" : $"({pos.X},{pos.Y})";
    }
    public class ServerPacketHandler 
    {
        internal static void HANDLE_Invalid(PacketSession session, IMessage message)
        {
            throw new NotImplementedException();
        }
        internal static void HANDLE_S_JwtLoginReply(PacketSession session, S_JwtLoginReply reply)
        {
            switch (reply.Result) // 프로토 C# 코드 생성 시 보통 PascalCase enum이 됩니다 (Success 등). 필요하면 이름 맞춰 수정
            {
                case ELoginResult.Success: // 또는 LoginResult.Success

                    //Console.WriteLine($"[JWT VALIDATION OK]");
                   
                    UnityEngine.Debug.Log($"[LOGIN OK] accountId={reply.Result}");
                    AuthNotice_UI.Instance.gameObject.SetActive(true);
                    AuthNotice_UI.Instance.ShowNotice(NoticeCode.LoginSuccess);
                    CharacterList_UI.Instance.gameObject.SetActive(true);
                    break;

                case ELoginResult.InvalidToken: // InvalidToken

                    //Console.WriteLine("[JWT VALIDATION] Invalid token. Please re-auth.");
                    UnityEngine.Debug.Log("[JWT VALIDATION] Invalid token. Please re-auth.");
                    //토큰 재발급 UX로 전환
                    break;

                case ELoginResult.TokenExpired: // TokenExpired

                    Console.WriteLine("[JWT VALIDATION] Token expired. Get a new token.");
                    UnityEngine.Debug.Log("[JWT VALIDATION] Token expired. Get a new token.");
                    // 리프레시 토큰/재로그인 유도
                    break;

                case ELoginResult.ServerError: // ServerError
                default:

                    //Console.WriteLine($"[JWT VALIDATION] Server error (code={(int)reply.Result}). Try again later.");
                    UnityEngine.Debug.Log($"[JWT VALIDATION] Server error (code={(int)reply.Result}). Try again later.");
                    break;
            }
        }
        internal static void HANDLE_S_CreateCharacterReply(PacketSession session, S_CreateCharacterReply reply)
        {
            var result = reply;
            AuthNotice_UI.Instance.gameObject.SetActive(true);
            AuthNotice_UI.Instance.ShowNotice(NoticeCode.CreateCharacterSuccess);
            UnityEngine.Debug.Log($"[CreateCharacterReply] 결과: {result.Success}.");
            UnityEngine.Debug.Log($"[CreateCharacterReply] 결과: {result.Detail}.");
            
        }  

        internal static void HANDLE_S_CharacterListReply(PacketSession session, S_CharacterListReply reply)
        {
            UnityEngine.Debug.Log($"[S_CharacterListReply] 전송받음");
            AuthNotice_UI.Instance.gameObject.SetActive(true); 
            AuthNotice_UI.Instance.ShowNotice(NoticeCode.RecvCharacterListSuccess);
            if (reply.Characters == null)
            {
                UnityEngine.Debug.Log("계정 내 생성된 캐릭터가 없습니다.");
            }
            CharacterList_UI.Instance.SetCharacterList(reply.Characters);
        }

        internal static void HANDLE_S_EnterGame(PacketSession session, S_EnterGame enter)
        {
            AuthNotice_UI.Instance.gameObject.SetActive(false);
            if (enter.Success != 1)
            {
                UnityEngine.Debug.LogError("EnterGame 실패");
                return;
            }
            //Console.WriteLine("[S_EnterGame] 게임 접속 완료");
            Debug.Log("[S_EnterGame] 게임 접속 완료");
        }

        internal static void HANDLE_S_PlayerList(PacketSession session, S_PlayerList list)
        {
            foreach (var p in list.Players)
            {
                bool isLocal = (p.Id == list.MyPlayerId);
                Debug.Log($" {p.Pos.X},{p.Pos.Y}");
            }
            GetSceneNameByMapId(list.MapId);
            LoadingSceneManager.LoadScene(list.MapId);
            var palyer = list.Players;
            

            Debug.Log("맵 씬 로딩 시작");
            //맵 씬 활성 직후 전체 스폰
            SceneTransition.RunAfterGameplaySceneLoaded(() =>
            {
                Debug.Log("씬 로딩 완료 후 플레이어 스폰 처리");
                foreach (var p in list.Players)
                {
                    bool isLocal = (p.Id == list.MyPlayerId);
                    PlayerSpawner.SafeSpawn(p, isLocal);
                    Debug.Log($"{p.Pos.X},{p.Pos.Y}");
                    Debug.Log($"플레이어 스폰 처리: {p.Username} (ID: {p.Id}) {(isLocal ? "(ME)" : "")}");
                }
            });
            #region 디버깅용        
            // 맵ID에 따른 씬 로딩 시뮬레이션
            //string sceneName = GetSceneNameByMapId(list.MapId);
            //Debug.Log($">>> 씬 로딩 시뮬레이션: '{sceneName}' 로딩 중...");
            //Debug.Log($">>> 맵 배경 및 UI 초기화 완료");

            //// 내 플레이어 ID 저장
            //NetDebug.MyPlayerId = list.MyPlayerId;
            //Debug.Log($"내 플레이어 ID: {list.MyPlayerId}");

            //// 다른 플레이어 정보
            //Debug.Log($"현재 룸에 있는 다른 플레이어 수: {list.Players.Count-1}");
            //foreach (var player in list.Players)
            //{
            //    var pos = NetDebug.PosToStr(player.Pos);
            //    var dir = NetDebug.DirToStr(player.Direction);
            //    Debug.Log($"  - 플레이어ID: {player.Id}, 이름: {player.Username}, 위치: {pos}, 방향: {dir}");
            //}
            #endregion
        }

        private static string GetSceneNameByMapId(int mapId)
        {
            return mapId switch
            {
                1 => "GoguryeoScene",
                2 => "BaekjeScene",
                3 => "FieldScene",
                _ => $"알 수 없는 맵 (ID: {mapId})"
            };
        }
        static void SafeSpawn(PlayerInfo info, bool isLocal)
        {
            if (PlayerSpawner.HasInstance)
            {
                PlayerSpawner.Instance.SpawnNow(info, isLocal);
                Debug.Log("세이프 스폰 호출");
            }
            else
            {
                SceneTransition.RunAfterGameplaySceneLoaded(() =>
                    PlayerSpawner.Instance.SpawnNow(info, isLocal));
            }
        }

        static void SafeRemove(int id)
        {
            if (PlayerSpawner.HasInstance)
            {
                PlayerSpawner.RemovePlayer(id);
            }
            else
            {
                SceneTransition.RunAfterGameplaySceneLoaded(() =>
                    PlayerSpawner.RemovePlayer(id));
            }
        }
        internal static void HANDLE_S_BroadcastPlayerEnter(PacketSession session, S_BroadcastPlayerEnter broadEnter)
        {
            Debug.Log("[S_BroadcastPlayerEnter] 누군가 접속해서 그 정보를 받아옴");
            SafeSpawn(broadEnter.Player, isLocal: false);
        }

        internal static void HANDLE_S_BroadcastPlayerLeave(PacketSession session, S_BroadcastPlayerLeave leave)
        {
            SafeRemove(leave.PlayerId);
            Debug.Log("[S_BroadcastPlayerLeave] 누군가 나가서 그 정보를 받아옴");
        }

        internal static void HANDLE_S_PlayerMoveReply(PacketSession session, S_PlayerMoveReply reply)
        {

            var pos = reply.NewPos;
            var posStr = NetDebug.PosToStr(pos);
            var dirStr = NetDebug.DirToStr(reply.Direction);

            string resultStr = reply?.Result.ToString() ?? "N/A";
            int tick = reply?.Tick ?? -1;

            string meTag = (reply.PlayerId == NetDebug.MyPlayerId && NetDebug.MyPlayerId >= 0) ? " (ME)" : "";

            Debug.Log($"[S_PlayerMoveReply] pid={reply.PlayerId}{meTag} " +
                      $"dir={dirStr} pos={posStr} result={resultStr} tick={tick}");

            switch (reply.Result)
            {
                case EMoveResult.MoveOk:
                    // 정상 이동
                    var go = PlayerSpawner.Get(reply.PlayerId);
                    if (go)
                    {
                        var avatar = go.GetComponent<PlayerAvatar>();
                        avatar?.SetDirection(reply.Direction);

                        var newPos = new Vector3(reply.NewPos.X, reply.NewPos.Y, 0);
                        if ((go.transform.position - newPos).sqrMagnitude < 0.25f)
                            avatar?.SmoothMoveTo(newPos);
                        else
                            go.transform.position = newPos;

                        go.GetComponent<PlayerIdentity>()?.SetLastServerTick(reply.Tick);
                    }
                    break;

                case EMoveResult.MoveBlocked:
                    Debug.LogWarning("[MoveResult] 이동이 막혔습니다.");
                    // 예: 벽 충돌 애니메이션, 경고 UI
                    break;

                case EMoveResult.MoveCooldown:
                    Debug.LogWarning("[MoveResult] 이동 쿨다운 중.");
                    // 예: UI 알림
                    break;

                case EMoveResult.MoveDir:
                    Debug.Log("[MoveResult] 단순 방향 전환.");
                    // 방향 전환만 반영
                    var dirGo = PlayerSpawner.Get(reply.PlayerId);
                    dirGo?.GetComponent<PlayerAvatar>()?.SetDirection(reply.Direction);
                    break;

                case EMoveResult.MoveUnknown:
                default:
                    Debug.LogError("[MoveResult] 알 수 없는 이동 응답.");
                    break;
            }
            //if (reply.Result != EMoveResult.MoveOk)
            //{
            //    Debug.Log("")
            //}
            //Debug.Log($"[S_PlayerMoveReply] recv pid={reply.PlayerId}  selected={SessionStore.SelectedCharId}");
            //if ((int)SessionStore.SelectedCharId != reply.PlayerId) return;
            //var newPos = new Vector3(reply.NewPos.X, reply.NewPos.Y, 0);
            //var my = PlayerSpawner.Get(reply.PlayerId);     // 네 스포너/레지스트리에서 꺼내는 함수로 대체
            //if (!my) return;

            //// 방향 세팅(애니메이션)
            //my.GetComponent<PlayerAvatar>()?.SetDirection(reply.Direction);
            //var cur = my.transform.position;
            //if ((cur - newPos).sqrMagnitude < 0.25f)
            //    my.GetComponent<PlayerAvatar>()?.SmoothMoveTo(newPos); // 선택: 보간 이동 API
            //else
            //    my.transform.position = newPos;

            //// 입력 예측 중이면 종료
            ////my.GetComponent<PlayerController>()?.StopPreviewAndSnap(newPos);

            //// (선택) 최신 tick 기록해서 과거 브로드캐스트 무시
            //my.GetComponent<PlayerIdentity>()?.SetLastServerTick(reply.Tick);
            #region 디버깅용
            // 안전하게 널 체크
            //var pos = reply.NewPos;
            //var posStr = NetDebug.PosToStr(pos);
            //var dirStr = NetDebug.DirToStr(reply.Direction);

            //// result/tick 필드가 없을 수도 있으니 Try 포맷
            //string resultStr = reply?.Result.ToString() ?? "N/A";
            //int tick = reply?.Tick ?? -1;

            //string meTag = (reply.PlayerId == NetDebug.MyPlayerId && NetDebug.MyPlayerId >= 0) ? " (ME)" : "";
            //Debug.Log(
            //    $"[S_PlayerMoveReply] pid={reply.PlayerId}{meTag} " +
            //    $"dir={dirStr} pos={posStr} result={resultStr} tick={tick}");
            //Console.WriteLine(
            //    $"[S_PlayerMoveReply] pid={reply.PlayerId}{meTag} " +
            //    $"dir={dirStr} pos={posStr} result={resultStr} tick={tick}");
            #endregion
        }

        internal static void HANDLE_S_BroadcastPlayerMove(PacketSession session, S_BroadcastPlayerMove move)
        {
            foreach (var mv in move.PlayerMoves)
            {
                var go = PlayerSpawner.Get(mv.PlayerId);
                if (!go) continue;

                // 내 캐릭이면 서버 reply에서 이미 처리했을 가능성 큼 → tick 비교해서 오래된 건 무시
                var id = go.GetComponent<PlayerIdentity>();
                if (id && id.LastServerTick > move.Tick) continue;

                var target = new Vector3(mv.NewPos.X, mv.NewPos.Y, 0);
                go.GetComponent<PlayerAvatar>()?.SetDirection(mv.Direction);
                go.GetComponent<PlayerAvatar>()?.SmoothMoveTo(target); // 보간 이동 (다른 유저는 항상 보간 추천)
                id?.SetLastServerTick(move.Tick);
            }
            #region 디버깅
            int tick = move?.Tick ?? -1;
            Console.WriteLine($"[S_BroadcastPlayerMove] tick={tick} count={move.PlayerMoves.Count}");

            foreach (var m in move.PlayerMoves)
            {
                var pos = m.NewPos;
                var posStr = NetDebug.PosToStr(pos);
                var dirStr = NetDebug.DirToStr(m.Direction);
                string meTag = (m.PlayerId == NetDebug.MyPlayerId && NetDebug.MyPlayerId >= 0) ? " (ME)" : "";

                Console.WriteLine($"  - pid={m.PlayerId}{meTag} dir={dirStr} pos={posStr}");
            }
            #endregion
        }

        internal static void HANDLE_S_ChangeRoomBegin(PacketSession session, S_ChangeRoomBegin begin)
        {
            Console.WriteLine($"[S_ChangeRoomBegin] Begin Change Room");

            var pkt = new Google.Protobuf.Protocol.C_ChangeRoomReady {
                TransitionId = begin.TransitionId,
            };

            session.Send(ServerPacketManager.MakeSendBuffer(pkt));
        }

        internal static void HANDLE_S_ChangeRoomCommit(PacketSession session, S_ChangeRoomCommit commit)
        {
            Console.WriteLine($"[S_ChangeRoomCommit] Room Has Change into ...");
            
            Debug.Log($"[S_ChangeRoomCommit] Room Has Change into ...");

        }

        internal static void HANDLE_S_LeaveGame(PacketSession session, S_LeaveGame game)
        {
            Console.WriteLine($"[S_LeaveGame] Game Has left.");
            Debug.Log($"[S_LeaveGame] Game Has left.");
        }
        internal static void HANDLE_S_SpawnMonster(PacketSession session, S_SpawnMonster spawnMonster)
        {
            
        }
        internal static void HANDLE_S_DespawnMonster(PacketSession session, S_DespawnMonster despawnMonster)
        {

        }
        internal static void HANDLE_S_BroadcastMonsterMove(PacketSession session, S_BroadcastMonsterMove broadMonsterMove)
        {

        }
        internal static void HANDLE_S_BroadcastMonsterAttack(PacketSession session, S_BroadcastMonsterAttack broadMonsterAtk)
        {

        }
        internal static void HANDLE_S_BroadcastMonsterDeath(PacketSession session, S_BroadcastMonsterDeath broadMonsterDeath)
        {

        }
        internal static void HANDLE_S_BroadcastPlayerAttack(PacketSession session, S_BroadcastPlayerAttack broadPlayerAtk)
        {

        }
        internal static void HANDLE_S_InventoryReply(PacketSession session, S_InventoryReply invenApply)
        {

        }
        internal static void HANDLE_S_ItemUseReply(PacketSession session, S_ItemUseReply useItemApply)
        {

        }
        internal static void HANDLE_S_InventoryUpdate(PacketSession session, S_InventoryUpdate invenUpdate)
        {

        }
    }
}
