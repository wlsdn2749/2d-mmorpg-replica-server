# C_PartyJoinRequestList (64)

### **파티 가입 요청 리스트 조회 (리더 전용)**

- 설명: 파티 리더가 자신의 파티에 대한 대기 중인 가입 요청 목록을 조회한다. 리스트 기반 선택 방식의 핵심 패킷으로, 리더는 이 목록을 보고 특정 요청자를 수락/거절할 수 있다.
- 입력: `C_PartyJoinRequestList`
  ```protobuf
  message C_PartyJoinRequestList {
      int32 partyId = 1;  // 조회할 파티 ID
  }
  ```
- 처리 로직:
  - 세션 및 플레이어 검증
  - `PartyManager::FindParty(partyId)` 호출
  - **리더 권한 확인**: `party->GetLeader() == player`
    - 리더가 아니면 빈 목록 또는 에러 반환
  - `PartyManager::GetJoinRequesters(partyId)` 호출
    - `_partyJoinRequests[partyId]` 맵에서 Vector<PlayerRef> 조회
    - 빈 벡터 반환 가능 (요청이 없는 경우)
  - 각 요청자의 정보를 `PartyJoinRequesterInfo`로 변환
    - playerId, playerName, level 수집
- 출력: `S_PartyJoinRequestList` (65번)
  - `partyId`: 파티 ID
  - `repeated PartyJoinRequesterInfo requesters`: 요청자 목록
- 데이터 구조:
  - `PartyManager::_partyJoinRequests`: map<int32, Vector<PlayerRef>>
  - `Player`: 요청자의 playerId, username, level

### 리스트 기반 선택의 장점
- **FIFO 방식의 문제점**: 먼저 온 요청부터 처리, 원하는 플레이어 선택 불가
- **리스트 기반 방식**:
  - 모든 대기 요청을 한 번에 확인
  - PlayerId로 특정 요청자 지정하여 수락/거절
  - 예: PlayerB, PlayerC가 요청 → 리더가 PlayerC만 수락 가능

### 에러 조건
- 파티가 존재하지 않는 경우
- 요청자가 리더가 아닌 경우 (조회 권한 없음)

### 관련 파일
- `GameServer/ClientPacketHandler.cpp` (Line 876-914)
- `GameServer/PartyManager.cpp` (Line 264-275)

### 사용 플로우
1. 리더가 `S_PartyJoinNotify` (60번) 알림 받음
2. 리더가 `C_PartyJoinRequestList` 전송
3. 서버가 `S_PartyJoinRequestList` 응답
4. 리더가 목록에서 원하는 요청자 선택
5. 리더가 `C_PartyJoinResponse` (61번)로 수락/거절
