# C_PartyLeave (54)

### **파티 탈퇴 / 강퇴 요청**

- 설명: 클라이언트가 자발적으로 파티를 탈퇴하거나, 리더가 다른 멤버를 강퇴하는 요청을 전송한다.
- 입력: `C_PartyLeave`
  ```protobuf
  message C_PartyLeave {
      oneof action {
          bool selfLeave = 1;    // true면 자발적 탈퇴
          int32 targetPid = 2;   // 값이 있으면 해당 플레이어 강퇴
      }
  }
  ```
- 처리 로직:

  #### 자발적 탈퇴 (selfLeave = true)
  - 세션 및 플레이어 검증
  - Room의 JobQueue에 `HandlePartyLeave(player, true, playerId)` 작업 큐잉
  - `PartyManager::LeaveParty(player)` 호출
  - **리더인 경우**:
    - `PartyManager::DisbandParty(partyId)` 호출
    - 모든 파티원의 partyId 초기화
    - `_parties` 맵에서 파티 제거
    - `PARTY_UPDATE_DISBANDED` 브로드캐스트
  - **일반 멤버인 경우**:
    - `Party::RemoveMember(player)` 호출
    - `_playerToParty` 맵에서 제거
    - 플레이어의 partyId = 0 설정
    - `PARTY_UPDATE_MEMBER_LEAVE` 브로드캐스트

  #### 강퇴 (targetPid)
  - 요청자의 파티 ID 확인
  - 대상 플레이어를 전체 룸에서 검색 (`RoomManager::FindPlayerInAllRooms()`)
  - Room의 JobQueue에 `HandlePartyLeave(player, false, targetPid)` 작업 큐잉
  - `PartyManager::kickMember(partyId, kicker, target)` 호출
  - **리더 권한 확인**: `party->GetLeader() == kicker`
  - **같은 파티 확인**: `IsSameParty(kicker, target)`
  - 대상 멤버 제거 및 partyId 초기화
  - `PARTY_UPDATE_MEMBER_LEAVE` 브로드캐스트

- 출력: `S_BroadcastPartyUpdate` (55번)
  - 해산 시: `updateType = PARTY_UPDATE_DISBANDED`
  - 탈퇴/강퇴 시: `updateType = PARTY_UPDATE_MEMBER_LEAVE`
- 데이터 구조:
  - `Party::_leader`: 파티 리더
  - `Party::_members`: 파티 멤버 목록
  - `PartyManager::_parties`: 전체 파티 맵
  - `PartyManager::_playerToParty`: 플레이어→파티ID 맵

### 에러 조건
- 파티에 속해있지 않은 경우
- 강퇴 시 리더가 아닌 경우
- 대상 플레이어를 찾을 수 없는 경우
- 같은 파티가 아닌 경우

### 관련 파일
- `GameServer/ClientPacketHandler.cpp` (Line 659-706)
- `GameServer/Room.cpp` (Line 404-445)
- `GameServer/PartyManager.cpp` (Line 46-136)
