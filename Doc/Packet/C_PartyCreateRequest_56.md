# C_PartyCreateRequest (56)

### **공개 파티 생성 요청**

- 설명: 클라이언트가 파티명을 지정하여 공개 파티를 생성하는 요청을 전송한다. 파티명이 있는 파티는 다른 플레이어가 파티 목록에서 조회하고 가입 요청할 수 있다.
- 입력: `C_PartyCreateRequest`
  ```protobuf
  message C_PartyCreateRequest {
      string partyName = 1;  // 파티 이름
  }
  ```
- 처리 로직:
  - 세션 InRoom 상태 검증
  - 플레이어 및 룸 유효성 확인
  - 플레이어의 파티 소속 확인
    - 이미 파티에 속해있으면 실패 응답
  - `PartyManager::CreatePartyWithName(player, partyName)` 호출
    - `_nextPartyId.fetch_add(1)`로 고유 파티 ID 생성 (atomic 연산)
    - Party 객체 생성 (partyId, partyName, leader 설정)
    - `_parties` 맵에 추가
    - `_playerToParty` 맵에 플레이어→파티ID 매핑 추가
    - 플레이어의 `SetPartyId()` 호출
  - `PartyService::SendPartyStatusUpdate()` 호출
    - `PARTY_UPDATE_MEMBER_JOIN` 브로드캐스트
- 출력:
  - `S_PartyCreateReply` (57번): 생성 결과
    - `success`: 성공 여부
    - `message`: "Party created" / "Already in party" / "Failed to create party"
  - `S_BroadcastPartyUpdate` (55번): 생성자에게 파티 상태 전송
- 데이터 구조:
  - `Party`: 파티 객체 (partyId, partyName, leader, members)
  - `PartyManager::_parties`: 전체 파티 맵
  - `PartyManager::_playerToParty`: 플레이어→파티ID 맵
  - `PartyManager::_nextPartyId`: atomic<int32> 파티 ID 생성기

### 초대 전용 파티 vs 공개 파티
- **초대 전용 파티**: `CreateParty()` 사용, partyName = ""
- **공개 파티**: `CreatePartyWithName()` 사용, partyName 지정
- 공개 파티는 `C_PartyList` (62번)로 조회 가능

### 관련 파일
- `GameServer/ClientPacketHandler.cpp` (Line 708-744)
- `GameServer/PartyManager.cpp` (Line 27-44)
