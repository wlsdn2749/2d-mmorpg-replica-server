# C_PartyJoinRequest (58)

### **공개 파티 가입 요청**

- 설명: 클라이언트가 공개 파티 목록에서 선택한 파티에 가입 요청을 전송한다. 요청은 파티 리더에게 알림으로 전달되며, 리더가 수락/거절을 결정한다.
- 입력: `C_PartyJoinRequest`
  ```protobuf
  message C_PartyJoinRequest {
      int32 partyId = 1;  // 가입하려는 파티 ID
  }
  ```
- 처리 로직:
  - 세션, 플레이어, 룸 유효성 검증
  - `PartyManager::FindParty(partyId)` 호출하여 파티 존재 확인
  - `Party::IsFull()` 체크 (MAX_MEMBERS = 4)
  - `PartyManager::AddJoinRequest(partyId, player)` 호출
    - `_partyJoinRequests` 맵에 요청자 추가
    - 중복 요청 방지 (이미 요청한 플레이어면 false 반환)
  - 파티 리더의 세션 찾기
  - 리더에게 `S_PartyJoinNotify` (60번) 패킷 전송
- 출력:
  - `S_PartyJoinReply` (59번): 요청자에게 결과 전송
    - `success`: 요청 전송 성공 여부
    - `message`: "Request sent to leader" / "Party not found" / "Party is full" / "Request already pending"
  - `S_PartyJoinNotify` (60번): 파티 리더에게 알림
- 데이터 구조:
  - `Party`: 파티 정보 및 멤버 목록
  - `PartyManager::_partyJoinRequests`: map<int32, Vector<PlayerRef>> (partyId → 요청자 목록)

### 가입 플로우
1. 플레이어가 `C_PartyList` (62번)로 파티 목록 조회
2. 원하는 파티 선택 후 `C_PartyJoinRequest` 전송
3. 서버가 리더에게 `S_PartyJoinNotify` 전송
4. 리더가 `C_PartyJoinRequestList` (64번)로 대기 요청 목록 조회
5. 리더가 `C_PartyJoinResponse` (61번)로 수락/거절

### 에러 조건
- 파티가 존재하지 않는 경우
- 파티 인원이 가득 찬 경우 (4/4)
- 이미 같은 파티에 요청을 보낸 경우

### 관련 파일
- `GameServer/ClientPacketHandler.cpp` (Line 746-810)
- `GameServer/PartyManager.cpp` (Line 182-199)
