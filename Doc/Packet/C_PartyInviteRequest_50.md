# C_PartyInviteRequest (50)

### **파티 초대 요청**

- 설명: 클라이언트가 특정 플레이어를 자신의 파티에 초대하는 요청을 서버에 전송한다.
- 입력: `C_PartyInviteRequest`
  ```protobuf
  message C_PartyInviteRequest {
      int32 targetPid = 1;  // 초대할 대상 플레이어 ID
  }
  ```
- 처리 로직:
  - 세션이 InRoom 상태인지 검증
  - 초대자의 플레이어 객체 유효성 확인
  - 초대자의 파티 조회 또는 생성
    - 파티가 없으면 `PartyManager::CreateParty()` 호출하여 새 파티 생성
    - 파티가 있으면 기존 파티 반환
  - 대상 플레이어를 전체 룸에서 검색 (`RoomManager::FindPlayerInAllRooms()`)
  - 대상 플레이어의 유효성 및 룸 소속 여부 확인
  - 대상 플레이어가 속한 Room의 JobQueue에 `HandlePartyInvite` 작업 큐잉
  - Room에서 대상 플레이어에게 `S_PartyInviteNotify` 패킷 전송
- 출력: `S_PartyInviteReply` (52번)
  - `success`: 초대 요청 성공 여부
  - `errorMessage`: 실패 시 에러 메시지 ("Player not found", "Player not in room")
- 데이터 구조:
  - `Party`: 파티 정보 (partyId, leader, members)
  - `PartyManager`: 전역 파티 관리 (_parties, _playerToParty 맵)
  - Room JobQueue: 파티 초대 알림 직렬화

### 관련 파일
- `GameServer/ClientPacketHandler.cpp` (Line 584-635)
- `GameServer/PartyManager.cpp` (Line 8-24)
- `GameServer/Room.cpp` (Line 383-395)
