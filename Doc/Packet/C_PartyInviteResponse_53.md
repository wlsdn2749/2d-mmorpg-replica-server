# C_PartyInviteResponse (53)

### **파티 초대 응답 (수락/거절)**

- 설명: 초대받은 플레이어가 파티 초대를 수락하거나 거절하는 응답을 서버에 전송한다.
- 입력: `C_PartyInviteResponse`
  ```protobuf
  message C_PartyInviteResponse {
      int32 partyId = 1;    // 파티 ID
      bool accept = 2;       // 수락 여부 (true=수락, false=거절)
  }
  ```
- 처리 로직:
  - 세션 InRoom 상태 검증
  - 응답자(invitee) 플레이어 유효성 확인
  - **거절 처리** (`accept == false`):
    - 추가 처리 없이 종료
  - **수락 처리** (`accept == true`):
    - Room의 JobQueue에 `HandlePartyInviteResponse` 작업 큐잉
    - `PartyManager::JoinParty(partyId, player)` 호출
    - 파티 멤버 목록에 플레이어 추가
    - 플레이어의 partyId 설정
    - `PartyService::SendPartyStatusUpdate()` 자동 호출
    - 파티원 전체에게 `S_BroadcastPartyUpdate` 브로드캐스트
- 출력: `S_BroadcastPartyUpdate` (55번)
  - `updateType`: `PARTY_UPDATE_MEMBER_JOIN`
  - `members`: 현재 파티원 전체 상태
- 데이터 구조:
  - `Party::_members`: 파티 멤버 Vector
  - `PartyManager::_playerToParty`: 플레이어→파티ID 맵

### 에러 조건
- 플레이어가 이미 다른 파티에 속해있는 경우 (`IsInParty()` 체크)
- 파티가 가득 찬 경우 (`Party::IsFull()` 체크, MAX_MEMBERS=4)

### 관련 파일
- `GameServer/ClientPacketHandler.cpp` (Line 637-657)
- `GameServer/Room.cpp` (Line 397-402)
- `GameServer/PartyManager.cpp` (Line 72-88)
