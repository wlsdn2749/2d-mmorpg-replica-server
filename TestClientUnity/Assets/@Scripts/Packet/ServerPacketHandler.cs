using Google.Protobuf;
using Google.Protobuf.Protocol;
using ServerCore;
using System;
using System.Collections.Generic;
using UnityEngine;

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
        internal static void HANDLE_S_DeleteCharacterReply(PacketSession session, S_DeleteCharacterReply delete)
        {
            var result = delete.Success;
            if (result)
            {
                var req = new C_CharacterListRequest();
                var send = ServerPacketManager.MakeSendBuffer(req);
                NetworkManager.Instance.Send(send);
            }
            else
            {
                Debug.Log(delete.ErrorMessage);
            }
            
        }
        internal static void HANDLE_S_EnterGame(PacketSession session, S_EnterGame enter)
        {
            AuthNotice_UI.Instance.gameObject.SetActive(false);
            if (enter.Success != 1)
            {
                UnityEngine.Debug.LogError("EnterGame 실패");
                return;
            }
            Debug.Log("[S_EnterGame] 게임 접속 완료");
        }

        internal static void HANDLE_S_PlayerList(PacketSession session, S_PlayerList list)
        {
            MonsterSync.OnMapActivated(list.MapId);

            if (!WorldFlowState.HasEnteredWorld||WorldFlowState.OnCharacterChange)
            {
                // ▶ 최초 접속 루트: 여기서만 씬 로드
                string sceneName = GetSceneNameByMapId(list.MapId);

                // 🎯 씬 로딩 시작 알림(중요)
                MonsterSync.OnGameplaySceneWillLoad();

                LoadingSceneManager.OnSceneActivated = () =>
                {
                    // 🎯 씬 활성 알림(펜딩된 몬스터 스냅샷/스폰 플러시)
                    MonsterSync.OnGameplaySceneActivated();

                    // 스폰
                    PlayerSpawner.EnsureExists();
                    PlayerSpawner.DespawnAll();

                    int my = list.MyPlayerId;
                    foreach (var p in list.Players)
                        PlayerSpawner.SafeSpawn(p, p.Id == my);

                    RoomTransitionManager.Instance?.UnlockInputAfterSpawn();

                    WorldFlowState.HasEnteredWorld = true;
                    WorldFlowState.OnCharacterChange = false;
                    WorldFlowState.ActiveMapId = list.MapId;
                };

                LoadingSceneManager.LoadScene(sceneName);
                return;
            }

            // ▶ 인게임 루트(이미 월드 진입 후)
            // 씬 로딩은 Begin이 이미 했다. 여기서는 스폰만.
            if (WorldFlowState.TransitionInProgress)
            {
                // (Begin에서 이미 OnGameplaySceneWillLoad 호출되어 있어야 함)
                LoadingSceneManager.OnSceneActivated = () =>
                {
                    // 🎯 씬 활성 알림 (여기서 몬스터 flush)
                    MonsterSync.OnGameplaySceneActivated();

                    PlayerSpawner.EnsureExists();
                    PlayerSpawner.DespawnAll();

                    int my = list.MyPlayerId;
                    foreach (var p in list.Players)
                        PlayerSpawner.SafeSpawn(p, p.Id == my);

                    RoomTransitionManager.Instance?.UnlockInputAfterSpawn();
                };
                // 주의: 여기서 LoadScene 호출 금지!
            }
            else
            {
                // 씬이 이미 활성 상태(같은 맵에서 재동기화 등): 즉시 스폰
                // (씬은 활성 상태라 OnGameplaySceneActivated 한 번 호출해 플러시)
                MonsterSync.OnGameplaySceneActivated();

                PlayerSpawner.EnsureExists();
                PlayerSpawner.DespawnAll();

                int my = list.MyPlayerId;
                foreach (var p in list.Players)
                    PlayerSpawner.SafeSpawn(p, p.Id == my);

                RoomTransitionManager.Instance?.UnlockInputAfterSpawn();
                WorldFlowState.ActiveMapId = list.MapId;
            }
        }

        private static string GetSceneNameByMapId(int mapId)
        {
            return mapId switch
            {
                1 => "Map_Goguryeo",
                2 => "Map_Baekje",
                3 => "Map_HuntingField",
                _ => $"Map_{mapId}",
            };
        }
        internal static void HANDLE_S_BroadcastPlayerEnter(PacketSession session, S_BroadcastPlayerEnter broadEnter)
        {
            Debug.Log("[S_BroadcastPlayerEnter] 누군가 접속해서 그 정보를 받아옴");
            PlayerSpawner.SafeSpawn(broadEnter.Player, isLocal: false);
        }

        internal static void HANDLE_S_BroadcastPlayerLeave(PacketSession session, S_BroadcastPlayerLeave leave)
        {
            PlayerSpawner.RemovePlayer(leave.PlayerId);
            Debug.Log("[S_BroadcastPlayerLeave] 누군가 나가서 그 정보를 받아옴");
        }

        internal static void HANDLE_S_PlayerMoveReply(PacketSession session, S_PlayerMoveReply reply)
        {
            if (reply.PlayerId != NetDebug.MyPlayerId) return;
            var pos = reply.NewPos;
            var posStr = NetDebug.PosToStr(pos);
            var dirStr = NetDebug.DirToStr(reply.Direction);

            string resultStr = reply?.Result.ToString() ?? "N/A";
            int tick = reply?.Tick ?? -1;
            string meTag = (reply.PlayerId == NetDebug.MyPlayerId && NetDebug.MyPlayerId >= 0) ? " (ME)" : "";

            Debug.Log($"[S_PlayerMoveReply] pid={reply.PlayerId}{meTag} dir={dirStr} pos={posStr} result={resultStr} tick={tick}");

            switch (reply.Result)
            {
                case EMoveResult.MoveOk:
                    {
                        var go = PlayerSpawner.Get(reply.PlayerId);
                        if (!go)
                        {
                            Debug.LogWarning($"[MoveOk] target not found: pid={reply.PlayerId}");
                            break;
                        }

                        // 방향은 아바타에서
                        var avatar = go.GetComponent<PlayerAvatar>();
                        if (!avatar)
                        {
                            Debug.LogError($"[MoveOk] PlayerAvatar missing on pid={reply.PlayerId}");
                            break;
                        }
                        avatar.SetDirection(reply.Direction);

                        // 위치 이동은 전용 mover로
                        var target = new Vector3(reply.NewPos.X, reply.NewPos.Y, 0);
                        var mover = go.GetComponent<SimpleMover>();
                        if (!mover) mover = go.AddComponent<SimpleMover>();
                        mover.SetTarget(target); // 부드럽게 이동

                        go.GetComponent<PlayerIdentity>()?.SetLastServerTick(reply.Tick);
                        break;
                    }

                case EMoveResult.MoveBlocked:
                    {
                        Debug.LogWarning("[MoveResult] 이동이 막혔습니다.");
                        // 필요 시 막힌 위치로 하드스냅:
                        // var go = PlayerSpawner.Get(reply.PlayerId);
                        // var mover = go?.GetComponent<SimpleMover>();
                        // mover?.HardSnap(new Vector3(reply.NewPos.X, reply.NewPos.Y, 0));
                        break;
                    }

                case EMoveResult.MoveCooldown:
                    {
                        Debug.LogWarning("[MoveResult] 이동 쿨다운 중.");
                        break;
                    }

                case EMoveResult.MoveDir:
                    {
                        Debug.Log("[MoveResult] 단순 방향 전환.");
                        var dirGo = PlayerSpawner.Get(reply.PlayerId);
                        var avatar = dirGo?.GetComponent<PlayerAvatar>();
                        if (!avatar)
                        {
                            Debug.LogWarning($"[MoveDir] PlayerAvatar missing on pid={reply.PlayerId}");
                            break;
                        }
                        avatar.SetDirection(reply.Direction);
                        break;
                    }

                case EMoveResult.MoveUnknown:
                default:
                    {
                        Debug.LogError("[MoveResult] 알 수 없는 이동 응답.");
                        break;
                    }
            }
        }

        internal static void HANDLE_S_BroadcastPlayerMove(PacketSession session, S_BroadcastPlayerMove move)
        {
            foreach (var mv in move.PlayerMoves)
            {
                var go = PlayerSpawner.Get(mv.PlayerId);
                if (!go)
                {
                    // (선택) 아직 스폰 전이면 무시만 하고 지나감. 절대 Destroy/Despawn 금지
                    Debug.LogWarning($"[MOVE/BC] GO missing pid={mv.PlayerId} → ignore");
                    continue;
                }

                var avatar = go.GetComponent<PlayerAvatar>();
                avatar?.SetDirection(mv.Direction);

                var mover = go.GetComponent<SimpleMover>() ?? go.AddComponent<SimpleMover>();
                mover.SetTarget(new Vector3(mv.NewPos.X, mv.NewPos.Y, 0));
            }
        }
        internal static void HANDLE_S_ChangeRoomBegin(PacketSession session, S_ChangeRoomBegin begin)
        {
            Console.WriteLine($"[S_ChangeRoomBegin] Begin Change Room");
            Debug.Log($"[HANDLE_S_ChangeRoomBegin] Room Has Change into ...");
            RoomTransitionManager.Instance.OnChangeRoomBegin(begin);
            Debug.Log("맵 씬 로딩 시작");
            //맵 씬 활성 직후 전체 스폰
        }

        internal static void HANDLE_S_ChangeRoomCommit(PacketSession session, S_ChangeRoomCommit commit)
        {
            Console.WriteLine($"[S_ChangeRoomCommit] Room Has Change into ...");
            
            RoomTransitionManager.Instance.OnChangeRoomCommit(commit);
            Debug.Log($"[S_ChangeRoomCommit] Room Has Change into ...");
        }

        internal static void HANDLE_S_LeaveGame(PacketSession session, S_LeaveGame game)
        {
            Console.WriteLine($"[S_LeaveGame] Game Has left.");
            
            if (game.Success !=true)
            {
                return;
            }
            switch (LeaveGameContext.LastLeaveReason)
            {
                case ELeaveReason.LeaveLogout:
                    Debug.Log($"[S_LeaveGame] CharacterSelectUI");
                    LoginManagement.SetLoingEntryMode(LoginEntryMode.ColdStart);

                    PlayerSpawner.DespawnAll();
                    RoomTransitionManager.Instance?.ResetState(); // 아래 ResetState 구현
                    LoadingSceneManager.LoadScene("AuthScene");
                    break;

                case ELeaveReason.LeaveChangeCharacter:
                    Debug.Log($"[S_LeaveGame] CharacterSelectUI");
                    LoginManagement.SetLoingEntryMode(LoginEntryMode.AfterLeaveToCharacterSelect);

                    PlayerSpawner.DespawnAll();
                    RoomTransitionManager.Instance?.ResetState(); // 아래 ResetState 구현
                    LoadingSceneManager.LoadScene("AuthScene");
                    break;

                case ELeaveReason.LeaveDisconnect:
                    LoginManagement.SetLoingEntryMode(LoginEntryMode.ColdStart);
                    LoadingSceneManager.LoadScene("AuthScene");
                    break;

                default:
                    LoadingSceneManager.LoadScene("AuthScene");
                    break;
            }

        }
        internal static void HANDLE_S_MonsterList(PacketSession session, S_MonsterList list)
        {
            MonsterSync.ApplySnapshot(list);
        }
        internal static void HANDLE_S_SpawnMonster(PacketSession session, S_SpawnMonster spawnMonster)
        {
            var m = spawnMonster.Monster;
            MonsterSync.OnSpawn(m);
        }
        internal static void HANDLE_S_DespawnMonster(PacketSession session, S_DespawnMonster despawnMonster)
        {
            MonsterSync.OnDespawn(despawnMonster);
        }
        internal static void HANDLE_S_BroadcastMonsterMove(PacketSession session, S_BroadcastMonsterMove broadMonsterMove)
        {
            MonsterSync.OnMove(broadMonsterMove);
        }
        internal static void HANDLE_S_BroadcastMonsterAttack(PacketSession session, S_BroadcastMonsterAttack broadMonsterAtk)
        {
            MonsterSync.OnAttack(broadMonsterAtk);
            PlayerStatus.Instance.OnDamage(broadMonsterAtk);
        }
        internal static void HANDLE_S_BroadcastMonsterDeath(PacketSession session, S_BroadcastMonsterDeath broadMonsterDeath)
        {
            MonsterSync.OnDespawn(broadMonsterDeath);
        }
        internal static void HANDLE_S_BroadcastPlayerTryAttack(PacketSession session, S_BroadcastPlayerTryAttack playerAttack)
        {
            var attacker = PlayerSpawner.Get(playerAttack.PlayerId);
            if (attacker)
            {
                var pid = attacker.GetComponent<PlayerIdentity>();
                var anim = attacker.GetComponent<Animator>();
                if (pid == null || !pid.IsLocalPlayer)
                    anim?.SetTrigger("Attack");
            }
        }
        internal static void HANDLE_S_BroadcastPlayerAttack(PacketSession session, S_BroadcastPlayerAttack broadPlayerAtk)
        {
            var damageText = ObjectPoolManager.Instance.GetObject("MonsterDamageText");
            damageText.GetComponent<DamageText>().Show(broadPlayerAtk.Damage, MonsterSync.MonsterPos(broadPlayerAtk));
            var go = MonsterSpawner.Get(broadPlayerAtk.TargetId);
            if (!go)
            {
                return;
            }
            var mh = go.GetComponent<MonsterHealth>();
            if (!mh) return;
            Debug.Log(broadPlayerAtk.HpAfter);
            mh.InitByAttackPacket(broadPlayerAtk.HpAfter);
        }
        internal static void HANDLE_S_InventoryReply(PacketSession session, S_InventoryReply invenApply)
        {
            var list = new List<InventorySlot>(invenApply.Slots.Count);
            foreach (var p in invenApply.Slots)
            {
                list.Add(new InventorySlot
                {
                    slotIndex = p.SlotIndex,
                    itemId = p.ItemId,
                    count = p.Count,
                    isQuickslot = p.IsQuickslot,
                    
                });
                Debug.Log($"[Inv] Slot {p.SlotIndex}: itemId={p.ItemId}, count={p.Count}, quickslot={p.IsQuickslot}");
            }
            var model = InventoryManager.Instance.Model;
            model.ApplySnapshot(list);
            HUDManager.Instance.ShowInventory_UI(); // 인벤 UI 켜기 
            Debug.Log($"[Inv] S_InventoryReply applied: {list.Count} slots");
        }
        internal static void HANDLE_S_ItemUseReply(PacketSession session, S_ItemUseReply useItemApply)
        {

        }
        internal static void HANDLE_S_InventoryUpdate(PacketSession session, S_InventoryUpdate invenUpdate)
        {

        }
        internal static void HANDLE_S_SystemMessage(PacketSession session, S_SystemMessage sysMsg)
        {
            Debug.Log($"[SystemMessage] {sysMsg.Message}");
            
        }
        internal static void HANDLE_S_NpcInteractReply(PacketSession session, S_NpcInteractReply npcInter)
        {

        }
        internal static void HANDLE_S_NpcShopOpen(PacketSession session, S_NpcShopOpen shopOpen)
        {

        }
        internal static void HANDLE_S_NpcShopBuyReply(PacketSession session, S_NpcShopBuyReply buyItem)
        {

        }
        internal static void HANDLE_S_PlayerStat(PacketSession session, S_PlayerStat playerInfo)
        {
            Debug.Log($"최대 체력 : {playerInfo.StatInfo.MaxHp}");
            Debug.Log($"현재 체력 : {playerInfo.StatInfo.Hp}");
            Debug.Log($"현재 경험치 : {playerInfo.StatInfo.CurExp}");
            Debug.Log($"최대 경험치 : {playerInfo.StatInfo.MaxExp}");
            Debug.Log($"레벨 : {playerInfo.StatInfo.Level}");
            Debug.Log($"돈 : {playerInfo.StatInfo.Money}");
            PlayerStatus.Instance.SetPlayerStatus(playerInfo.StatInfo);
        }
        public static void HANDLE_S_BroadcastPlayerHpChanged(PacketSession arg1, S_BroadcastPlayerHpChanged hpChanged)
        {
            PlayerStatus.Instance.UpdateHp(hpChanged);
        }
        public static void HANDLE_S_BroadcastPlayerDeath(PacketSession arg1, S_BroadcastPlayerDeath playerDie)
        {
            Debug.Log($"플레이어 죽음{playerDie.PlayerId} / {playerDie.KillerMonsterId}");
        }
        public static void HANDLE_S_BroadcastPlayerChat(PacketSession session, S_BroadcastPlayerChat chat)
        {
            //var pInfo = new S_BroadcastPlayerChat().PlayerChatInfos;
            //foreach (var info in pInfo)
            //{
                
            //}
        }
    }
}
