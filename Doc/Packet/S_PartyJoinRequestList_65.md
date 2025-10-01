# S_PartyJoinRequestList (65)

### **파티 가입 요청 리스트 응답**

- 설명: 서버가 파티 리더에게 대기 중인 가입 요청자들의 목록을 전송한다.
- 입력: `C_PartyJoinRequestList` 처리 완료 후
- 처리 로직:
  - PartyManager에서 수집한 요청자 정보를 패킷에 담아 전송
  - 빈 목록일 수 있음 (대기 요청이 없는 경우)
- 출력: `S_PartyJoinRequestList`
  ```protobuf
  message S_PartyJoinRequestList {
      int32 partyId = 1;
      repeated PartyJoinRequesterInfo requesters = 2;
  }

  message PartyJoinRequesterInfo {
      int32 playerId = 1;      // 요청자 플레이어 ID
      string playerName = 2;    // 요청자 이름
      int32 level = 3;          // 요청자 레벨
  }
  ```
- 데이터 구조: `PartyJoinRequesterInfo` 배열

### PartyJoinRequesterInfo 필드 설명
- `playerId`: 요청자의 고유 ID (C_PartyJoinResponse의 requesterPid로 사용)
- `playerName`: 요청자의 캐릭터 이름
- `level`: 요청자의 레벨 (리더가 참고할 수 있는 정보)

### 관련 파일
- `GameServer/ClientPacketHandler.cpp` (Line 876-914)

### 클라이언트 UI 예시
```
[파티 가입 요청 (2건)]
┌─────────────────────────────────┐
│ PlayerB (Lv.8)                  │
│ PlayerId: 100                   │
│               [수락] [거절]      │
├─────────────────────────────────┤
│ PlayerC (Lv.12)                 │
│ PlayerId: 101                   │
│               [수락] [거절]      │
└─────────────────────────────────┘
```

### 클라이언트 처리
- `requesters.Count == 0`: "대기 중인 요청이 없습니다" 메시지 표시
- `requesters.Count > 0`: 각 요청자 정보 표시 및 수락/거절 버튼
- 수락 클릭 시: `C_PartyJoinResponse(partyId, playerId, true)` 전송
- 거절 클릭 시: `C_PartyJoinResponse(partyId, playerId, false)` 전송
