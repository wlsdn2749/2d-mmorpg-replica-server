# C_PartyJoinResponse (61)

### **파티 가입 요청 수락/거절 (리더 전용)**

- 설명: 파티 리더가 대기 중인 가입 요청을 수락하거나 거절하는 응답을 전송한다. 리스트 기반 선택 방식으로, 특정 요청자의 PlayerId를 지정하여 처리한다.
- 입력: `C_PartyJoinResponse`
  ```protobuf
  message C_PartyJoinResponse {
      int32 partyId = 1;        // 파티 ID
      int32 requesterPid = 2;   // 수락/거절할 요청자의 PlayerId
      bool accept = 3;           // 수락 여부 (true=수락, false=거절)
  }
  ```
- 처리 로직:
  - 세션, 플레이어, 룸 검증
  - `PartyManager::FindParty(partyId)` 호출
  - **리더 권한 확인**: `party->GetLeader() == player`
  - `PartyManager::FindRequesterById(partyId, requesterPid)` 호출
    - `_partyJoinRequests` 맵에서 특정 요청자 검색
  - **수락 처리** (`accept == true`):
    - `PartyManager::JoinParty(partyId, requester)` 호출
    - 성공 시 `PartyManager::RemoveJoinRequest()` 호출
    - `PARTY_UPDATE_MEMBER_JOIN` 브로드캐스트
    - 요청자에게 `S_PartyJoinReply` (success=true) 전송
  - **거절 처리** (`accept == false`):
    - `PartyManager::RemoveJoinRequest(partyId, requester)` 호출
    - 요청자에게 `S_PartyJoinReply` (success=false) 전송
- 출력:
  - `S_PartyJoinReply` (59번): 요청자에게 결과 전송
  - `S_BroadcastPartyUpdate` (55번): 수락 시 파티원들에게 브로드캐스트
- 데이터 구조:
  - `PartyManager::_partyJoinRequests`: map<int32, Vector<PlayerRef>>
  - `Party::_members`: 파티 멤버 목록

### FIFO vs 리스트 기반 선택
- **이전 방식 (FIFO)**: 큐의 맨 앞 요청자만 처리
- **현재 방식 (리스트 기반)**: requesterPid로 특정 요청자 지정
- 리더가 `C_PartyJoinRequestList` (64번)로 전체 목록 조회 후 원하는 요청자 선택 가능

### 에러 조건
- 파티가 존재하지 않는 경우
- 요청자가 리더가 아닌 경우
- requesterPid에 해당하는 요청자를 찾을 수 없는 경우

### 관련 파일
- `GameServer/ClientPacketHandler.cpp` (Line 812-859)
- `GameServer/PartyManager.cpp` (Line 72-88, 212-240, 242-262)
