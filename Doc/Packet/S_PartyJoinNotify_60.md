# S_PartyJoinNotify (60)

### **파티 가입 요청 알림 (리더 전용)**

- 설명: 서버가 파티 리더에게 누군가 파티 가입을 요청했음을 알린다.
- 입력: `C_PartyJoinRequest` 처리 중
- 처리 로직:
  - 가입 요청이 성공적으로 큐에 추가되면 생성
  - 파티 리더의 세션을 찾아 전송
- 출력: `S_PartyJoinNotify`
  ```protobuf
  message S_PartyJoinNotify {
      int32 joinPlayerId = 1;  // 요청자 플레이어 ID
      int32 PartyId = 2;        // 파티 ID
      int32 leaderId = 3;       // 리더 ID
  }
  ```
- 데이터 구조:
  - `Party::_leader`: 파티 리더
  - `PartyManager::_partyJoinRequests`: 가입 요청 큐

### 관련 파일
- `GameServer/ClientPacketHandler.cpp` (Line 746-810)

### 클라이언트 처리
리더는 이 알림을 받으면:
1. UI에 알림 뱃지 표시 (예: "가입 요청 2건")
2. `C_PartyJoinRequestList` (64번)로 전체 요청 목록 조회
3. 각 요청자에 대해 `C_PartyJoinResponse` (61번)로 수락/거절

### 특징
- **리더에게만** 전송됨
- 여러 명이 요청하면 각각에 대해 알림 전송
- 요청은 큐에 저장되어 리더가 나중에 처리 가능
